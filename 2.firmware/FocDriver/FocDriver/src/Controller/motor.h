//
// Created by Monster on 2023/6/5.
//

#ifndef MOTOR_H
#define MOTOR_H

#include "../Sensors/Encoder/encoder_base.h"
#include "../Sensors/CurrentSense/current_sense_base.h"
#include "../Drivers/driver_base.h"

#include "../../foc_defaults.h"
#include "../Common/math_utils.h"
#include "lowpass_filter.h"
#include "pid.h"

class Motor {
public:
    enum MotorStatus : uint8_t {
        UNINITIALIZED = 0x00,  //!< Motor is not yet initialized
        INITIALIZING,   //!< Motor initialization is in progress
        UNCALIBRATED,   //!< Motor is initialized, but not calibrated (open loop possible)
        CALIBRATING,    //!< Motor calibration in progress
        READY,          //!< Motor is initialized and calibrated (closed loop possible)
        ERROR,          //!< Motor is in error state (recoverable, e.g. overcurrent protection active)
        CALIB_FILED,    //!< Motor calibration failed (possibly recoverable)
        INIT_FAILED,    //!< Motor initialization failed (not recoverable)
    };

    enum ControlMode : uint8_t {
        TORQUE = 0x00,      //!< Torque control
        VELOCITY,           //!< Velocity motion control
        ANGLE,              //!< Position/angle motion control
        VELOCITY_OPEN_LOOP, //!< Velocity open-loop control
        ANGLE_OPEN_LOOP,    //!< Position/angle open-loop control
    };

    enum TorqueControlMode : uint8_t {
        VOLTAGE     = 0x00,     //!< Torque control using voltage
        DC_CURRENT  = 0x01,     //!< Torque control using DC current (one current magnitude)
        FOC_CURRENT = 0x02,     //!< torque control using dq currents
    };

public:
    /**
    * Default constructor - setting all variabels to default values
    */
    explicit Motor(int _polePairs, float _phaseResistance = NOT_SET, float _phaseInductance = NOT_SET,
                   float _kv = NOT_SET) :
            pole_pairs(_polePairs), phase_resistance(_phaseResistance), phase_inductance(_phaseInductance) {

        // torque control type is voltage by default
        torque_controller_mode = TorqueControlMode::VOLTAGE;

        controller_mode = ControlMode::VELOCITY_OPEN_LOOP;
        voltage_limit   = DEF_POWER_SUPPLY;     //!< 12Volt power supply voltage
        current_limit   = DEF_CURRENT_LIMIT;    //!< 0.5Amps current limit by default
        velocity_limit  = DEF_VELOCITY_LIMIT;   //!< angle velocity limit

        // sensor and motor align voltage
        voltage_sensor_align = 3.0f;    //!< voltage for sensor and motor zero alignment

        voltage_bemf = 0;

        if (_isset(_kv)) kv_rating = _kv * _SQRT2;  //!< 1/sqrt(2) - rms value

        // default target value
        target = 0;
        voltage.q = 0;
        voltage.d = 0;
        set_current = 0;
        current.q = 0;
        current.d = 0;
    }

    // configuration structures
    TorqueControlMode torque_controller_mode;
    ControlMode       controller_mode; //!< parameter determining the control loop to be used

    // controllers and low pass filters
    LowPassFilter lpf_current_q{0.005f};
    LowPassFilter lpf_current_d{0.005f};
    LowPassFilter lpf_velocity{0.05f};
    LowPassFilter lpf_angle{0.001};
    PIDController pid_current_q{3.0f, 300.0f, 0.0f, 0.0f, DEF_VOLTAGE_LIMIT};
    PIDController pid_current_d{3.0f, 300.0f, 0.0f, 0.0f, DEF_VOLTAGE_LIMIT};
    PIDController pid_velocity{0.01f, 0.2, 0.0, 0.0, DEF_VOLTAGE_LIMIT};
    PIDController pid_angle{2.0f, 0, 0, 100, DEF_VELOCITY_LIMIT};

    // limiting variables
    float voltage_limit;
    float current_limit;
    float velocity_limit;

    // sensor related variables
    float sensor_offset       = 0.0f;     //!< user defined sensor zero offset
    float zero_electric_angle = NOT_SET; //!< absolute zero electric angle - if available
    float voltage_sensor_align;     //!< sensor and motor align voltage parameter

    // state variables
    float       target;           //!< current target value - depends of the controller
    float       shaft_angle;      //!< current motor angle
    float       shaft_velocity;   //!< current motor velocity
    float       electrical_angle; //!< current electrical angle
    float       set_current;      //!< target curren ( q current )
    float       set_velocity;     //!< target velocity
    float       set_angle;        //!< target angle
    DQVoltage_t voltage;    //!< current d and q voltage set to the motor
    DQCurrent_t current;    //!< current d and q current measured
    float       voltage_bemf;     //!< estimated backemf voltage (if provided KV constant)

    // other device object
    DriverBase       *driver       = nullptr;
    EncoderBase      *encoder      = nullptr;
    CurrentSenseBase *currentSense = nullptr;

public:
    bool _enable = false;   //!< enabled or disabled motor flag

public:
    virtual void enable();
    virtual void disable();

    MotorStatus getStatus();

    /** Motor hardware init function */
    virtual bool init(float zero_electric_offset = NOT_SET, EncoderBase::Direction sensor_dir = EncoderBase::CW);

    /**
     * Function initializing FOC algorithm
     * and aligning sensor's and motors' zero position
     */
    virtual int initFOC(EncoderBase::Direction sensor_dir = EncoderBase::Direction::UNKNOWN, float zero_electric_offset = NOT_SET);

    /** Function linking a motor and other device */
    void linkDriver(DriverBase *_driver);
    void linkEncoder(EncoderBase *_encoder);
    void linkCurrentSense(CurrentSenseBase *_current_sense);

    /** Iterative function looping FOC algorithm, setting Uq on the Motor */
    virtual void loopFOC();

    /** Function executing the control loops set by the controller parameter of the BLDCMotor. */
    virtual void move(float new_target = NOT_SET);

    /**
     * Method using FOC to set Uq to the motor at the optimal angle
     * Heart of the FOC algorithm
     */
    virtual void setPhaseVoltage(float Uq, float Ud, float angle_el);

    /**
     * State calculation methods
     * */
    /** Estimate, Shaft angle calculation in radians [rad] */
    float shaftAngle();

    /** Estimate, Shaft angle calculation function in radian per second [rad/s] */
    float shaftVelocity();

    /** Electrical angle calculation */
    float electricalAngle();

private:
    /**
     * FOC methods
     */

    /* Sensor alignment to electrical 0 angle of the motor */
    int alignSensor();

    /** open loop movement for target velocity */
    float velocityOpenLoop(float target_velocity);

    /** open loop movement towards the target angle */
    float angleOpenLoop(float target_angle);

private:
    MotorStatus status           = MotorStatus::UNINITIALIZED;
    float       phase_resistance = NOT_SET; //!< motor phase resistance
    float       phase_inductance = NOT_SET; //!< motor phase inductance
    float       kv_rating        = NOT_SET; //!< motor kv rating (rmp/v)
    int         pole_pairs       = 7;       //!< motor pole pairs number

    float Ua, Ub, Uc;       //!< Current phase voltages Ua,Ub and Uc set to motor

    unsigned long open_loop_timestamp; //!< open loop variables
};

#endif //! MOTOR_H
