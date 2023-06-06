//
// Created by Monster on 2023/6/5.
//

#include "motor.h"
#include "../Common/DriveLog.h"
#include <cmath>

/**
 * Motor hardware init function
 * @param zero_electric_offset - Sensor zero electric offset
 * @param sensor_dir - Sensor direction ( CW or CCW )
 * @return
 */
bool Motor::init(float zero_electric_offset, EncoderBase::Direction sensor_dir) {
    if (driver)driver->init();
    if (encoder)encoder->init();
    if (currentSense)currentSense->init();

    if (!currentSense && _isset(phase_resistance)) {
        float limit = current_limit * phase_resistance;
        voltage_limit = limit < voltage_limit ? limit : voltage_limit;
    }

    // sanity check for the voltage limit configuration
    if (voltage_limit > driver->voltage_power_supply) {
        voltage_limit = driver->voltage_power_supply;
    }
    // constrain voltage for sensor alignment
    if (voltage_sensor_align > voltage_limit) {
        voltage_sensor_align = voltage_limit;
    }

    // update the controller limits
    if (currentSense) {
        pid_current_q._limit = voltage_limit;
        pid_current_d._limit = voltage_limit;
    }
    if (_isset(phase_resistance)) {
        pid_velocity._limit = current_limit;
    } else {
        pid_velocity._limit = voltage_limit;
    }
    pid_angle._limit = velocity_limit;

    DRIVE_LOG("MOT: Enable driver.");
    status = MotorStatus_t::UNCALIBRATED;
    return initFOC(zero_electric_offset, sensor_dir);
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
 ** Function initializing FOC algorithm
 * and aligning sensor's and motors' zero position
 *
 * - If zero_electric_offset parameter is set the alignment procedure is skipped
 *
 * @param zero_electric_offset value of the sensors absolute position electrical offset in respect to motor's electrical 0 position.
 * @param encoder_dir sensor natural direction - default is CW
 * @return
 */
int Motor::initFOC(float zero_electric_offset, EncoderBase::Direction sensor_dir) {
    int exit_flag = 0;

    // align motor if necessary
    if (_isset(zero_electric_offset)) {
        zero_electric_angle = zero_electric_offset;
        encoder->direction = sensor_dir;
    }

    // sensor and motor alignment - can be skipped
    // by setting motor.sensor_direction and motor.zero_electric_angle
    if (encoder) {
        exit_flag = alignSensor();

        encoder->update();
        shaft_angle = shaftAngle();
    } else {
        DRIVE_LOG("MOT: No encoder.");
    }

    // aligning the current sensor - can be skipped
    if (currentSense) {
        if (!currentSense->initialized) {
            DRIVE_LOG("MOT: Align failed, Current not initialized.");
        } else {
            exit_flag = currentSense->driverAlign(voltage_sensor_align);
        }
    } else {
        DRIVE_LOG("MOT: No current sensor.");
    }

    if (exit_flag) {
        DRIVE_LOG("MOT: Ready.");
        status = MotorStatus_t::READY;
    } else {
        DRIVE_LOG("MOT: FOC init failed.");
        status = MotorStatus_t::CALIB_FILED;
    }

    return exit_flag;
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
    if (controller_mode == ControlMode_t::ANGLE_OPEN_LOOP ||
        controller_mode == ControlMode_t::VELOCITY_OPEN_LOOP) {
        return;
    }

    // if disable do nothing
    if (!_enable) return;

    // which is in range 0-2PI
    electrical_angle = electricalAngle();
    switch (torque_controller_mode) {
        case TorqueControlMode_t::VOLTAGE:
            // no need to do anything really
            voltage.q = _isset(phase_resistance) ? set_current * phase_resistance : set_current;
            voltage.q = _constrain(voltage.q, -voltage_limit, voltage_limit);
            // voltage.d = 0;
            break;

        case TorqueControlMode_t::DC_CURRENT:
            // read overall current magnitude
            current.q = currentSense->getDCCurrent(electrical_angle);
            // filter the value
            current.q = lpf_current_q(current.q);
            // calculate the phase voltage
            voltage.q = pid_current_q(set_current - current.q);
            // D voltage - lag compensation
            if (_isset(phase_inductance))
                voltage.d = _constrain(-set_current * shaft_velocity * pole_pairs * phase_inductance,
                                       -voltage_limit, voltage_limit);
            else voltage.d = 0;
            break;

        case TorqueControlMode_t::FOC_CURRENT:
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


    // if disabled do nothing
    if (!_enable) return;

    // set internal target variable
    if (_isset(new_target)) target = new_target;

    switch (controller_mode) {
        case ControlMode_t::TORQUE:
//            voltage.q = _isset(phase_resistance) ? target * phase_resistance : target;
            voltage.q = target;
            voltage.d = _isset(phase_inductance) ? _constrain(
                    -target * shaft_velocity * (float) pole_pairs * phase_inductance,
                    -voltage_limit, voltage_limit) : 0;
            set_current = voltage.q;
            break;

        case ControlMode_t::ANGLE:
            set_angle = target;
            set_velocity = pid_angle(set_angle - shaft_angle);
            set_current = pid_velocity(set_velocity - shaft_velocity);

            voltage.d = _isset(phase_inductance) ? _constrain(
                    -set_current * shaft_velocity * (float) pole_pairs * phase_inductance,
                    -voltage_limit, voltage_limit) : 0;
            break;

        case ControlMode_t::VELOCITY:
            set_velocity = target;
            set_current = pid_velocity(set_current - shaft_velocity);

            voltage.d = _isset(phase_inductance) ? _constrain(
                    -set_current * shaft_velocity * (float) pole_pairs * phase_inductance,
                    -voltage_limit, voltage_limit) : 0;
            break;

        case ControlMode_t::VELOCITY_OPEN_LOOP:
            set_velocity = target;
            voltage.q = velocityOpenloop(set_velocity);
            voltage.d = 0;
            break;

        case ControlMode_t::ANGLE_OPEN_LOOP:
            set_angle = target;
            voltage.q = angleOpenloop(set_angle);
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
        uOUT = _sqrt(Ud * Ud + Uq * Uq) / driver->voltage_power_supply;
        angle_el = _normalizeAngle(angle_el + std::atan2(Uq, Ud));
    } else {
        uOUT = Uq / driver->voltage_power_supply;
        // angle normalisation in between 0 and 2pi
        // only necessary if using _sin and _cos - approximation functions
        angle_el = _normalizeAngle(angle_el + _PI_2);
    }
    // find the sector we are in currently
    int sector = std::floor((angle_el / _PI_3) + 1);
    // calculate the duty cycles
    float t1 = _SQRT3 * _sin((float) sector * _PI_3 - angle_el) * uOUT;
    float t2 = _SQRT3 * _sin(angle_el - ((float) sector - 1.0f) * _PI_3) * uOUT;
    float t0 = 1 - t1 - t2;

    // calculate the duty cycles(times)
    float Ta, Tb, Tc;
    switch (sector) {
        case 1:
            Ta = t1 + t2 + t0 / 2;
            Tb = t2 + t0 / 2;
            Tc = t0 / 2;
            break;
        case 2:
            Ta = t1 + t0 / 2;
            Tb = t1 + t2 + t0 / 2;
            Tc = t0 / 2;
            break;
        case 3:
            Ta = t0 / 2;
            Tb = t1 + t2 + t0 / 2;
            Tc = t2 + t0 / 2;
            break;
        case 4:
            Ta = t0 / 2;
            Tb = t1 + t0 / 2;
            Tc = t1 + t2 + t0 / 2;
            break;
        case 5:
            Ta = t2 + t0 / 2;
            Tb = t0 / 2;
            Tc = t1 + t2 + t0 / 2;
            break;
        case 6:
            Ta = t1 + t2 + t0 / 2;
            Tb = t0 / 2;
            Tc = t1 + t0 / 2;
            break;
        default:
            // possible error state
            Ta = 0;
            Tb = 0;
            Tc = 0;
    }

    // calculate the phase voltages and center
    Ua = Ta * driver->voltage_power_supply;
    Ub = Tb * driver->voltage_power_supply;
    Uc = Tc * driver->voltage_power_supply;

    currentSense->pwm_duty_a = Ta;
    currentSense->pwm_duty_b = Tb;
    currentSense->pwm_duty_c = Tc;

    driver->setPwm(Ua, Ub, Uc);
}

int Motor::alignSensor() {
    return 0;
}

/**
 * estimate angle calculation
 * @return
 */
float Motor::shaftAngle() {
    // if no sensor linked return previous value ( for open loop )
    if (!encoder) return shaft_angle;
    return (float) encoder->direction * encoder->getAngle() - sensor_offset;
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
    if (!encoder) return electrical_angle;
    return _normalizeAngle((float) (encoder->direction * pole_pairs) * encoder->getMechanicalAngle()
                           - zero_electric_angle);
}

/**
 * Function (iterative) generating open loop movement for target velocity
 * it uses voltage_limit variable
 * @param target_velocity - rad/s
 * @return
 */
float Motor::velocityOpenloop(float target_velocity) {
    auto time = _micros();

    float Ts = (float) (time - open_loop_timestamp) * 1e-6f;
    if (Ts <= 0 || Ts > 0.5f) Ts = 1e-3f;

    shaft_angle = _normalizeAngle(shaft_angle + target_velocity * Ts);
    // for display purposes
    shaft_velocity = target_velocity;

    // use voltage limit or current limit
    float Uq = voltage_limit;
    Uq = _isset(phase_resistance) ?
         _constrain(current_limit * phase_resistance, -voltage_limit, voltage_limit) : Uq;

    setPhaseVoltage(Uq, 0, _normalizeAngle(shaft_angle) * (float) pole_pairs);
    open_loop_timestamp = time;

    return Uq;
}

/**
 * Function (iterative) generating open loop movement towards the target angle
 * it uses voltage_limit and velocity_limit variables
 * @param target_angle - rad
 * @return
 */
float Motor::angleOpenloop(float target_angle) {
    auto time = _micros();

    float Ts = (float) (time - open_loop_timestamp) * 1e-6f;
    if (Ts <= 0 || Ts > 0.5f)Ts = 1e-3f;

    // calculate the necessary angle to move from current position towards target angle
    // with maximal velocity (velocity_limit)
    // TODO sensor precision: this calculation is not numerically precise. The angle can grow to the point
    //                        where small position changes are no longer captured by the precision of floats
    //                        when the total position is large.
    if (std::abs(target_angle - shaft_angle) > std::abs(velocity_limit * Ts)) {
        shaft_angle += _sign(target_angle - shaft_angle) * std::abs(velocity_limit) * Ts;
        shaft_velocity = velocity_limit;
    } else {
        shaft_angle = target_angle;
        shaft_velocity = 0;
    }

    // use voltage limit or current limit
    float Uq = voltage_limit;
    Uq = _isset(phase_resistance) ?
         _constrain(current_limit * phase_resistance, -voltage_limit, voltage_limit) : Uq;

    setPhaseVoltage(Uq, 0, _normalizeAngle(shaft_angle) * (float)pole_pairs);
    open_loop_timestamp = time;

    return Uq;
}






