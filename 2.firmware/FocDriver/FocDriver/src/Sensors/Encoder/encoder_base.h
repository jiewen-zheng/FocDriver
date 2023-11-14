//
// Created by Monster on 2023/6/5.
//

#ifndef _ENCODER_BASE_H
#define _ENCODER_BASE_H

#include <cstdint>
#include "../../Common/time_utils.h"
#include "../../Common/math_utils.h"

class EncoderBase {
public:
    enum Direction : int {
        CW      = 1,    // clockwise
        CCW     = -1,   // counter-clockwise
        UNKNOWN = 0     // not yet known or invalid state
    };

public:
    explicit EncoderBase() {}

    Direction direction = CW;

    virtual void init() = 0;

    virtual float getAngle() = 0;

    /** Updates the sensor values by reading the hardware sensor. */
    virtual void update();

    /** Get current position (in rad) including full rotations and shaft angle. */
    virtual float getFullAngle();

    /** Get mechanical shaft angle in the range 0 to 2PI. */
    virtual float getMechanicalAngle();

    /** Get current angular velocity ( rad/s ) */
    virtual float getVelocity();

    /** Get the number of full rotations */
    virtual int32_t getRotationCount();

public:
    float min_elapsed_time = 0.000100; // default is 100 microseconds, or 10kHz

protected:
    virtual void variable_init();

    float    velocity            = 0.0f;
    float    angle_last          = 0.0f;    //!< result of last call to getSensorAngle(), used for cmd_count rotations and velocity
    uint32_t angle_timestamp     = 0;       //!< timestamp of last call to getFullAngle, used for velocity
    float    velocity_last       = 0.0f;
    uint32_t velocity_timestamp  = 0;
    int32_t  rotation_count      = 0;
    int32_t  rotation_count_last = 0;
};

#endif //_ENCODER_BASE_H
