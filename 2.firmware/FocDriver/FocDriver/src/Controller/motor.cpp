//
// Created by Monster on 2023/6/5.
//

#include "motor.h"
#include "../Common/DriveLog.h"
#include <cmath>

/**
 * Enable motor driver
 */
void Motor::enable() {
    driver->enable();
    driver->setPwm(0, 0, 0);

    _enable = true;
}

/**
 * Disable motor driver
 */
void Motor::disable() {
    driver->setPwm(0, 0, 0);
    driver->disable();

    _enable = false;
}

Motor::MotorStatus Motor::getStatus() {
    return status;
}

/**
 * Motor hardware init function
 * @param zero_electric_offset - Sensor zero electric offset
 * @param sensor_dir - Sensor direction ( CW or CCW )
 * @return
 */
bool Motor::init(float zero_electric_offset, EncoderBase::Direction sensor_dir) {
    if (!driver || !driver->isReady()) {
        status = MotorStatus::INIT_FAILED;
        DRIVE_LOG("MOT: Init not possible, No driver.");
        return false;
    }

    status           = MotorStatus::INITIALIZING;
    DRIVE_LOG("MOT: Init...");

    // sanity check for the voltage limit configuration
    if (voltage_limit > driver->voltage_power_supply) {
        voltage_limit = driver->voltage_power_supply;
    }
    // constrain voltage for sensor alignment
    if (voltage_sensor_align > voltage_limit) {
        voltage_sensor_align = voltage_limit;
    }

//    if(driver) driver->init();
//    if (encoder) encoder->init();
//    if (currentSense) currentSense->init();

    if (!currentSense && _isset(phase_resistance)) {
        float limit = current_limit * phase_resistance;
        voltage_limit = (limit < voltage_limit) ? limit : voltage_limit;
    }

    // update the controller limits
    if (currentSense) {
        pid_current_q._limit = voltage_limit;
        pid_current_d._limit = voltage_limit;
    }
    if (_isset(phase_resistance) || torque_controller_mode != TorqueControlMode::VOLTAGE) {
        pid_velocity._limit = current_limit;
    } else {
        pid_velocity._limit = voltage_limit;
    }
    pid_angle._limit = velocity_limit;

    DRIVE_LOG("MOT: Enable driver.");
    enable();
    _delay(500);

    status = MotorStatus::UNCALIBRATED;

    return true;
}

/**
 ** Function initializing FOC algorithm
 * and aligning sensor's and motors' zero position
 *
 * - If zero_electric_offset parameter is set the alignment procedure is skipped
 *
 * @param zero_electric_offset value of the sensors absolute position electrical offset in respect to motor's electrical 0 position.
 * @param encoder_dir sensor natural direction - default is CW
 * @return
 */
int Motor::initFOC(EncoderBase::Direction sensor_dir, float zero_electric_offset) {
    int exit_flag = 1; // success

    status = MotorStatus::CALIBRATING;

    // align motor if necessary
    if (_isset(zero_electric_offset)) {
        zero_electric_angle = zero_electric_offset;
    }

    // sensor and motor alignment - can be skipped
    // by setting encoder->direction and motor.zero_electric_angle
    if (encoder) {
        encoder->direction = sensor_dir;

        exit_flag = alignSensor();
        encoder->update();
        shaft_angle = shaftAngle();
    } else {
        DRIVE_LOG("MOT: No encoder.");
    }

    _delay(100);

    // aligning the current sensor - can be skipped
    if (currentSense) {
        if (!currentSense->initialized) {
            DRIVE_LOG("MOT: Align failed, Current sense not initialized.");
            exit_flag = 0;
        } else {
            exit_flag = currentSense->driverAlign(voltage_sensor_align);
        }
    } else {
        DRIVE_LOG("MOT: No current sensor.");
    }

    if (exit_flag) {
        DRIVE_LOG("MOT: Ready.");
        status = MotorStatus::READY;
    } else {
        DRIVE_LOG("MOT: Init FOC failed, Disable driver.");
        status = MotorStatus::CALIB_FILED;
        disable();
    }

    return exit_flag;
}

/**
 * Motor driver linking method
 * @param _driver - Driver class instance
 */
void Motor::linkDriver(DriverBase *_driver) {
    driver = _driver;
}

/**
 * Encoder linking method
 * @param _encoder Encoder instance
 */
void Motor::linkEncoder(EncoderBase *_encoder) {
    encoder = _encoder;
}

/**
 * CurrentSense linking method
 * @param _current_sense - CurrentSense instance
 */
void Motor::linkCurrentSense(CurrentSenseBase *_current_sense) {
    currentSense = _current_sense;
}

/**
 * Function running FOC algorithm in real-time
 * it calculates the gets motor angle and sets the appropriate voltages
 * to the phase pwm signals
 * - the faster you can run it the better Arduino UNO ~1ms, Bluepill ~ 100us
 */
void Motor::loopFOC() {
    /**
     * update senso
     * - do this even in open-loop mode, as user may be switching between modes and
     * we could lose track of full rotations otherwise.
     */
    if (encoder) encoder->update();

    // if open-loop do nothing
    if (controller_mode == ControlMode::ANGLE_OPEN_LOOP ||
        controller_mode == ControlMode::VELOCITY_OPEN_LOOP) {
        return;
    }

    // if disable do nothing
    if (!_enable) return;

    // which is in range 0-2PI
    electrical_angle = electricalAngle();
    switch (torque_controller_mode) {
        case TorqueControlMode::VOLTAGE:
            // no need to do anything really
            voltage.q = _isset(phase_resistance) ? set_current * phase_resistance : set_current;
            voltage.q = _constrain(voltage.q, -voltage_limit, voltage_limit);
            // voltage.d = 0;
            break;

        case TorqueControlMode::DC_CURRENT:
            // read overall current magnitude
            current.q     = currentSense->getDCCurrent(electrical_angle);
            // filter the value
            current.q     = lpf_current_q(current.q);
            // calculate the phase voltage
            voltage.q     = pid_current_q(set_current - current.q);
            // D voltage - lag compensation
            if (_isset(phase_inductance))
                voltage.d = _constrain(-set_current * shaft_velocity * pole_pairs * phase_inductance,
                                       -voltage_limit, voltage_limit);
            else voltage.d = 0;
            break;

        case TorqueControlMode::FOC_CURRENT:
            if (!currentSense) return;
            // read dq currents
            current = currentSense->getFOCCurrents(electrical_angle);
            // filter the values
            current.q = lpf_current_q(current.q);
            current.d = lpf_current_d(current.d);
            // calculate the phase voltage
            voltage.q = pid_current_q(set_current - current.q);
            voltage.d = pid_current_d(-current.d);
            break;
        default:
            // no torque control selected
            DRIVE_LOG("MOT: no torque control selected.");
            break;
    }

    // set the phase voltage - FOC heart function :)
    setPhaseVoltage(voltage.q, voltage.d, electrical_angle);
}

/**
 * Iterative function running outer loop of the FOC algorithm
 * Behavior of this function is determined by the motor.controller variable
 * It runs either angle, velocity or torque loop
 * - needs to be called iteratively it is asynchronous function
 * - if target is not set it uses motor.target value
 * @param new_target - Either voltage, angle or velocity based on the motor.controller
 *                     If it is not set the motor will use the target set in its variable motor.target
 */
void Motor::move(float new_target) {

    // read value even if motor is disabled to keep the monitoring updated but not in openloop mode
    if (controller_mode != ControlMode::ANGLE_OPEN_LOOP &&
        controller_mode != ControlMode::VELOCITY_OPEN_LOOP) {
        shaft_angle = shaftAngle();
    }
    // read value even if motor is disabled to keep the monitoring updated
    shaft_velocity = shaftVelocity();

    // if disabled do nothing
    if (!_enable) return;

    // set internal target variable
    if (_isset(new_target)) target = new_target;

    // calculate the back-emf voltage if KV_rating available U_bemf = vel*(1/KV)
    if (_isset(kv_rating)) voltage_bemf = shaft_velocity / kv_rating / _RPM_TO_RADS;

    if (!currentSense && _isset(phase_resistance)) {
        current.q = voltage.q / phase_resistance;
    }

    switch (controller_mode) {
        case ControlMode::TORQUE:
            voltage.q = _isset(phase_resistance) ? _constrain(
                    target * phase_resistance + voltage_bemf,
                    -voltage_limit, voltage_limit) : target;

            voltage.d = _isset(phase_inductance) ? _constrain(
                    -target * shaft_velocity * (float) pole_pairs * phase_inductance,
                    -voltage_limit, voltage_limit) : 0;
            set_current = voltage.q;
            break;

        case ControlMode::ANGLE:
            set_angle    = target;
            set_velocity = pid_angle(set_angle - shaft_angle);
            set_current  = pid_velocity(set_velocity - shaft_velocity);

            voltage.q = _isset(phase_resistance) ? _constrain(
                    set_current * phase_resistance + voltage_bemf,
                    -voltage_limit, voltage_limit) : set_current;

            voltage.d = _isset(phase_inductance) ? _constrain(
                    -set_current * shaft_velocity * (float) pole_pairs * phase_inductance,
                    -voltage_limit, voltage_limit) : 0;
            break;

        case ControlMode::VELOCITY:
            set_velocity = target;
            set_current  = pid_velocity(set_velocity - shaft_velocity);

            voltage.q = _isset(phase_resistance) ? _constrain(
                    set_current * phase_resistance + voltage_bemf,
                    -voltage_limit, voltage_limit) : set_current;

            voltage.d = _isset(phase_inductance) ? _constrain(
                    -set_current * shaft_velocity * (float) pole_pairs * phase_inductance,
                    -voltage_limit, voltage_limit) : 0;
            break;

        case ControlMode::VELOCITY_OPEN_LOOP:
            set_velocity = target;
            voltage.q = velocityOpenLoop(set_velocity);
            voltage.d = 0;
            break;

        case ControlMode::ANGLE_OPEN_LOOP:
            set_angle = target;
            voltage.q = angleOpenLoop(set_angle);
            voltage.d = 0;
            break;

        default:
            DRIVE_LOG("MOT: no controller mode selected.");
            break;
    }
}

/**
* Method using FOC to set Uq to the motor at the optimal angle
* Heart of the FOC algorithm
*
* @param Uq Current voltage in q axis to set to the motor
* @param Ud Current voltage in d axis to set to the motor
* @param angle_el current electrical angle of the motor
*/
void Motor::setPhaseVoltage(float Uq, float Ud, float angle_el) {

    float uOUT;

    // a bit of optimisation
    if (Ud != 0) {
        uOUT     = _sqrt(Ud * Ud + Uq * Uq) / driver->voltage_limit;
        angle_el = _normalizeAngle(angle_el + std::atan2(Uq, Ud));
    } else {
        uOUT     = Uq / driver->voltage_limit;
        // angle normalisation in between 0 and 2pi
        // only necessary if using _sin and _cos - approximation functions
        angle_el = _normalizeAngle(angle_el + _PI_2);
    }
    // find the sector we are in currently
    int   sector = std::floor(angle_el / _PI_3) + 1;
    // calculate the duty cycles
    float T1     = _SQRT3 * _sin((float) sector * _PI_3 - angle_el) * uOUT;
    float T2     = _SQRT3 * _sin(angle_el - ((float) sector - 1.0f) * _PI_3) * uOUT;

    /**
     * TODO:
     * flag (1) centered modulation around driver limit /2  or  (0) pulled to 0
     */
    float T0 = 1 - T1 - T2;
    // float T0 = 0;

    // calculate the duty cycles(times)
    float Ta, Tb, Tc;
    switch (sector) {
        case 1:
            Ta = T1 + T2 + T0 / 2;
            Tb = T2 + T0 / 2;
            Tc = T0 / 2;
            break;
        case 2:
            Ta = T1 + T0 / 2;
            Tb = T1 + T2 + T0 / 2;
            Tc = T0 / 2;
            break;
        case 3:
            Ta = T0 / 2;
            Tb = T1 + T2 + T0 / 2;
            Tc = T2 + T0 / 2;
            break;
        case 4:
            Ta = T0 / 2;
            Tb = T1 + T0 / 2;
            Tc = T1 + T2 + T0 / 2;
            break;
        case 5:
            Ta = T2 + T0 / 2;
            Tb = T0 / 2;
            Tc = T1 + T2 + T0 / 2;
            break;
        case 6:
            Ta = T1 + T2 + T0 / 2;
            Tb = T0 / 2;
            Tc = T1 + T0 / 2;
            break;
        default:
            // possible error state
            Ta = 0;
            Tb = 0;
            Tc = 0;
    }

    // calculate the phase voltages and center
    Ua = Ta * driver->voltage_limit;
    Ub = Tb * driver->voltage_limit;
    Uc = Tc * driver->voltage_limit;

    driver->setPwm(Ua, Ub, Uc);
}

/**
 * Sensor alignment to electrical 0 angle of the motor
 * @return
 */
int Motor::alignSensor() {
    int exit_flag = 1; // success
    DRIVE_LOG("MOT: Align encoder");

    // if unknown natural direction
    if (encoder->direction == EncoderBase::Direction::UNKNOWN) {
        // find natural direction
        for (int i = 0; i <= 1000; i++) {
            float angle = _3PI_2 + _2PI * i / 500.0f;
            setPhaseVoltage(voltage_sensor_align, 0, angle);
//            encoder->update();
            _delay(2);
        }
        // take and angle in the middle
        encoder->update();
        float mid_angle = encoder->getFullAngle();

        // move one electrical revolution backwards
        for (int i = 1000; i >= 0; i--) {
            float angle = _3PI_2 + _2PI * (float) i / 500.0f;
            setPhaseVoltage(voltage_sensor_align, 0, angle);
//            encoder->update();
            _delay(2);
        }
        encoder->update();
        float end_angle = encoder->getFullAngle();

        setPhaseVoltage(0, 0, 0);
        /** TODO: Power off the drive if necessary */
        _delay(200);

        // Determine the direction the sensor moved
        float delta_angle = std::fabs(mid_angle - end_angle);
        if (mid_angle == end_angle) {
            DRIVE_LOG("MOT: Failed to notice movement");
            return 0;
        } else if (mid_angle < end_angle) {
            DRIVE_LOG("MOT: encoder->direction==CCW");
            encoder->direction = EncoderBase::Direction::CCW;
        } else {
            DRIVE_LOG("MOT: encoder->direction==CW");
            encoder->direction = EncoderBase::Direction::CW;
        }

        // check pole pair number
        if (std::fabs(delta_angle*pole_pairs - _2PI) > 0.5f) {
            // 0.5f is arbitrary number it can be lower or higher!
//            exit_flag = 0;
            DRIVE_LOG("MOT: PP check: fail - estimated pp: %f", _2PI / delta_angle);
        } else {
            DRIVE_LOG("MOT: PP check: OK!");
        }
    } else
        DRIVE_LOG("MOT: Skip dir calib.");

    // zero electric angle not known
    if (!_isset(zero_electric_angle)) {
        // align the electrical phases of the motor and sensor
        // set angle -90(270 = 3PI/2) degrees
        setPhaseVoltage(voltage_sensor_align, 0, _3PI_2);
        _delay(700);
        encoder->update();

        // get the current zero electric angle
        zero_electric_angle = 0; // Clear offset first
        zero_electric_angle = electricalAngle();

        _delay(20);
        DRIVE_LOG("MOT: Zero electric angle: %f", zero_electric_angle);

        //stop everything
        setPhaseVoltage(0, 0, 0);
        /** TODO: Power off the drive if necessary */
        _delay(200);
    } else
        DRIVE_LOG("MOT: Skip offset calib.");

    return exit_flag;
}

/**
 * estimate angle calculation
 * @return
 */
float Motor::shaftAngle() {
    // if no sensor linked return previous value ( for open loop )
    if (!encoder) return shaft_angle;
    return (float) encoder->direction * lpf_angle(encoder->getFullAngle()) - sensor_offset;
}

/**
 * shaft velocity calculation
 * @return
 */
float Motor::shaftVelocity() {
    // if no sensor linked return previous value ( for open loop )
    if (!encoder) return shaft_velocity;
    return (float) encoder->direction * lpf_velocity(encoder->getVelocity());
}

/**
 * Electrical angle calculation
 * @return
 */
float Motor::electricalAngle() {
    // if no sensor linked return previous value ( for open loop )
    if (!encoder) {
        return electrical_angle;
    }
    return _normalizeAngle((float) (encoder->direction * pole_pairs) * encoder->getMechanicalAngle()
                           - zero_electric_angle);
}

/**
 * Function (iterative) generating open loop movement for target velocity
 * it uses voltage_limit variable
 * @param target_velocity - rad/s
 * @return
 */
float Motor::velocityOpenLoop(float target_velocity) {
    auto time = _micros();

    float Ts = (float) (time - open_loop_timestamp) * 1e-6f;
    // quick fix for strange cases (micros overflow + timestamp not defined)
    if (Ts <= 0 || Ts > 0.5f) Ts = 1e-3f;

    shaft_angle    = _normalizeAngle(shaft_angle + target_velocity * Ts);
    // for display purposes
    shaft_velocity = target_velocity;

    // use voltage limit or current limit
    float Uq = _isset(phase_resistance) ?
               _constrain(current_limit * phase_resistance + std::fabs(voltage_bemf),
                          -voltage_limit, voltage_limit) : voltage_limit;
    if (_isset(phase_resistance)) {
        current.q = (Uq - std::fabs(voltage_limit)) / phase_resistance;
    }

    setPhaseVoltage(Uq, 0, _electricalAngle(shaft_angle, pole_pairs));

    // save timestamp for next call
    open_loop_timestamp = time;

    return Uq;
}

/**
 * Function (iterative) generating open loop movement towards the target angle
 * it uses voltage_limit and velocity_limit variables
 * @param target_angle - rad
 * @return
 */
float Motor::angleOpenLoop(float target_angle) {
    auto time = _micros();

    float Ts = (float) (time - open_loop_timestamp) * 1e-6f;
    if (Ts <= 0 || Ts > 0.5f)Ts = 1e-3f;

    // calculate the necessary angle to move from current position towards target angle
    // with maximal velocity (velocity_limit)
    /** TODO sensor precision: this calculation is not numerically precise. The angle can grow to the point
     *                         where small position changes are no longer captured by the precision of floats
     *                          when the total position is large.
     */
    if (std::abs(target_angle - shaft_angle) > std::abs(velocity_limit * Ts)) {
        shaft_angle += _sign(target_angle - shaft_angle) * std::abs(velocity_limit) * Ts;
        shaft_velocity = velocity_limit;
    } else {
        shaft_angle    = target_angle;
        shaft_velocity = 0;
    }

    // use voltage limit or current limit
    float Uq = voltage_limit;
    Uq = _isset(phase_resistance) ?
         _constrain(current_limit * phase_resistance, -voltage_limit, voltage_limit) : Uq;

    setPhaseVoltage(Uq, 0, _normalizeAngle(shaft_angle) * (float) pole_pairs);
    open_loop_timestamp = time;

    return Uq;
}








