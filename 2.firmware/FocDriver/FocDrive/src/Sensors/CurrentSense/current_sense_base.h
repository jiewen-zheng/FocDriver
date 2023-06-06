//
// Created by Monster on 2023/6/5.
//

#ifndef FOCDRIVE_CURRENT_SENSE_BASE_H
#define FOCDRIVE_CURRENT_SENSE_BASE_H

#include "../../Common/math_utils.h"

class CurrentSenseBase {
public:
    CurrentSenseBase() = default;

    bool initialized = false;
    float pwm_duty_a;
    float pwm_duty_b;
    float pwm_duty_c;

public:
    virtual void init() = 0;

    virtual bool driverAlign(float align_voltage) = 0;

    virtual float getDCCurrent(float electrical_angle) = 0;

    virtual DQCurrent_t getFOCCurrents(float electrical_angle) = 0;
};


#endif //FOCDRIVE_CURRENT_SENSE_BASE_H
