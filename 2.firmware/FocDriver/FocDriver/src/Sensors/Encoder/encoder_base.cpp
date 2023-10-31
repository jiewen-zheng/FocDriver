//
// Created by Monster on 2023/6/5.
//

#include "encoder_base.h"
#include "../../Common/time_utils.h"
#include "../../Common/math_utils.h"
#include <cmath>

void EncoderBase::update() {
    float angle = getRawAngle();
    angle_timestamp = _micros();

    float delta_angle = angle - angle_last;
    // If overflow happened track it as full rotation
    if (std::abs(delta_angle) > (0.8f * _2PI)) {
        rotation_count += (delta_angle > 0) ? -1 : 1;
    }

    angle_last = angle;
}

float EncoderBase::getFullAngle() {
    return (float) rotation_count * _2PI + angle_last;
}

float EncoderBase::getMechanicalAngle() {
    return angle_last;
}

float EncoderBase::getVelocity() {
    // calculate sample time
    float time = (float) (angle_timestamp - velocity_timestamp) * 1e-6f;
    // Quick fix for strange cases (micros overflow)
    if (time <= 0) time = 1e-3f;

    // velocity calculate
    float vel = ((float) (rotation_count - rotation_count_last) * _2PI + (angle_last - velocity_last)) / time;

    velocity_last = angle_last;
    rotation_count_last = rotation_count;
    velocity_timestamp = angle_timestamp;

    return vel;
}

int32_t EncoderBase::getRotationCount() {
    return rotation_count;
}

void EncoderBase::variable_init() {
    // Initialize all the internal variables of EncoderBase
    // to ensure a "smooth" startup (without a 'jump' from zero)
    getRawAngle();  // call once
    _delayUs(1);
    velocity_last = getRawAngle();
    velocity_timestamp = _micros();
    _delay(1);

    getRawAngle(); // call once
    _delayUs(1);
    angle_last = getRawAngle();
    angle_timestamp = _micros();
}



