//
// Created by Monster on 2023/6/5.
//

#ifndef _ENCODER_BASE_H
#define _ENCODER_BASE_H

#include <stdint-gcc.h>

class EncoderBase {
public:
    explicit EncoderBase() = default;

    enum Direction : int8_t {
        CW = 1,  // clockwise
        CCW = -1, // counter clockwise
        UNKNOWN = 0   // not yet known or invalid state
    };

    Direction direction = CW;

    virtual void init() = 0;

    virtual void update() = 0;

    virtual float getAngle() = 0;

    virtual float getVelocity() = 0;

    virtual float getMechanicalAngle() = 0;
};


#endif //_ENCODER_BASE_H
