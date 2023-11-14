//
// Created by Monster on 2023/6/5.
//

#include "encoder_base.h"
#include <cmath>

void EncoderBase::update() {
    float angle = getAngle();
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
    float Ts = (float) (angle_timestamp - velocity_timestamp) * 1e-6f;
    // Quick fix for strange cases (micros overflow)
    if (Ts <= 0.0f) {
        velocity_last       = angle_last;
        rotation_count_last = rotation_count;
        velocity_timestamp  = angle_timestamp;
        return velocity;
    }
    if (Ts < min_elapsed_time) return velocity; // don't update velocity if deltaT is too small

    // velocity calculate
    velocity = ( (float)(rotation_count - rotation_count_last) * _2PI + (angle_last - velocity_last)) / Ts;

    velocity_last       = angle_last;
    rotation_count_last = rotation_count;
    velocity_timestamp  = angle_timestamp;
    return velocity;
}

int32_t EncoderBase::getRotationCount() {
    return rotation_count;
}

void EncoderBase::variable_init() {
    // Initialize all the internal variables of EncoderBase
    // to ensure a "smooth" startup (without a 'jump' from zero)
    getAngle();  // call once
    _delayUs(1);
    velocity_last      = getAngle();
    velocity_timestamp = _micros();
    _delay(1);

    getAngle(); // call once
    _delayUs(1);
    angle_last      = getAngle();
    angle_timestamp = _micros();
}



