//
// Created by Monster on 2023/6/5.
//

#ifndef _MOTOR_H
#define _MOTOR_H

#include "../Sensors/Encoder/encoder_base.h"
#include "../Sensors/CurrentSense/current_sense_base.h"
#include "../Drivers/driver_base.h"

#include "src/Common/math_utils.h"
#include "lowpass_filter.h"
#include "pid.h"

enum ControlMode_t : uint8_t {
    TORQUE = 0x00,  //!< Torque control
    VELOCITY,       //!< Velocity motion control
    ANGLE,          //!< Position/angle motion control
    VELOCITY_OPEN_LOOP,
    ANGLE_OPEN_LOOP
};

enum TorqueControlMode_t : uint8_t {
    VOLTAGE = 0x00,     //!< Torque control using voltage
    DC_CURRENT,         //!< Torque control using DC current (one current magnitude)
    FOC_CURRENT,        //!< torque control using dq currents
};

enum MotorStatus_t : uint8_t {
    UNINITIALIZED,  //!< Motor is not yet initialized
    INITIALIZED,    //!< Motor initialization is in progress
    UNCALIBRATED,   //!< Motor is initialized, but not calibrated (open loop possible)
    CALIBRATING,    //!< Motor calibration in progress
    READY,          //!< Motor is initialized and calibrated (closed loop possible)
    ERROR,          //!< Motor is in error state (recoverable, e.g. overcurrent protection active)
    CALIB_FILED,    //!< Motor calibration failed (possibly recoverable)
    INIT_FAILED,    //!< Motor initialization failed (not recoverable)
};

class Motor {
public:
    /**
    * Default constructor - setting all variabels to default values
    */
    explicit Motor(int _polePairs, float _phaseResistance = NOT_SET, float _phaseInductance = NOT_SET) :
            pole_pairs(_polePairs), phase_resistance(_phaseResistance), phase_inductance(_phaseInductance) {

        // torque control type is voltage by default
        torque_controller_mode = TorqueControlMode_t::VOLTAGE;

        controller_mode = ControlMode_t::ANGLE;
        voltage_limit = 12.0f;
        current_limit = 0.2f;
        velocity_limit = 20.0f;

        // aligning voltage [V]
        voltage_sensor_align = 1.0f;
    }

    // configuration structures
    TorqueControlMode_t torque_controller_mode;
    ControlMode_t controller_mode; //!< parameter determining the control loop to be used

    // limiting variables
    float voltage_limit;
    float current_limit;
    float velocity_limit;

    // controllers and low pass filters
    LowPassFilter lpf_current_q{0.005f};
    LowPassFilter lpf_current_d{0.005f};
    LowPassFilter lpf_velocity{0.1f};
    LowPassFilter lpf_angle{0.03f};
    PIDController pid_current_q{3.0f, 300.0f, 0.0f, 0.0f, 12.0f};
    PIDController pid_current_d{3.0f, 300.0f, 0.0f, 0.0f, 12.0f};
    PIDController pid_velocity{0.5f, 10, 0.0f, 1000.0f, 12.0f};
    PIDController pid_angle{20.0f, 0, 0, 0, 20.0f};

    // sensor related variables
    float sensor_offset;    //!< user defined sensor zero offset
    float zero_electric_angle = NOT_SET;    //!< user defined sensor zero offset
    int sensor_direction = NOT_SET; //!< if sensor_direction == Direction::CCW then direction will be flipped to CW
    float voltage_sensor_align; //!< sensor and motor align voltage parameter

    // state variables
    float target;           //!< current target value - depends of the controller
    float shaft_angle;      //!< current motor angle
    float electrical_angle; //!< current electrical angle
    float shaft_velocity;   //!< current motor velocity
    float set_current;      //!< target curren ( q current )
    float set_velocity;     //!< target velocity
    float set_angle;        //!< target angle
    DQVoltage_t voltage;    //!< current d and q voltage set to the motor
    DQCurrent_t current;    //!< current d and q current measured

    // other device object
    DriverBase *driver = nullptr;
    EncoderBase *encoder = nullptr;
    CurrentSenseBase *currentSense = nullptr;

public:
    void enable() { _enable = true; }

    void disalbe() { _enable = false; }


    /** Motor hardware init function */
    bool init(float zero_electric_offset = NOT_SET, EncoderBase::Direction sensor_dir = EncoderBase::CW);

    /** Function linking a motor and other device */
    void linkDriver(DriverBase *_driver);

    void linkEncoder(EncoderBase *_encoder);

    void linkCurrentSense(CurrentSenseBase *_current_sense);

    /** Iterative function looping FOC algorithm, setting Uq on the Motor */
    void loopFOC();

    /** Function executing the control loops set by the controller parameter of the BLDCMotor. */
    void move(float new_target = NOT_SET);

    // State calculation methods
    /** Estimate, Shaft angle calculation in radians [rad] */
    float shaftAngle();

    /** Estimate, Shaft angle calculation function in radian per second [rad/s] */
    float shaftVelocity();

    /** Electrical angle calculation */
    float electricalAngle();

    /**
    * Method using FOC to set Uq to the motor at the optimal angle
    * Heart of the FOC algorithm
    */
    void setPhaseVoltage(float Uq, float Ud, float angle_el);

protected:
    /**
     * Function initializing FOC algorithm
     * and aligning sensor's and motors' zero position
     */
    int initFOC(float zero_electric_offset, EncoderBase::Direction sensor_dir);

private:
    /**
     * FOC methods
     */

    /* Sensor alignment to electrical 0 angle of the motor */
    int alignSensor();

    /** open loop movement for target velocity */
    float velocityOpenloop(float target_velocity);

    /** open loop movement towards the target angle */
    float angleOpenloop(float target_angle);

private:

    bool _enable = false;   //!< enabled or disabled motor flag
    MotorStatus_t status = MotorStatus_t::UNINITIALIZED;
    float phase_resistance = NOT_SET; //!< motor phase resistance
    float phase_inductance = NOT_SET; //!< motor phase inductance
    int pole_pairs = 7;     //!< motor pole pairs number

    float Ua, Ub, Uc;       //!< Current phase voltages Ua,Ub and Uc set to motor

    uint64_t open_loop_timestamp; //!< open loop variables
};


#endif //_MOTOR_H
