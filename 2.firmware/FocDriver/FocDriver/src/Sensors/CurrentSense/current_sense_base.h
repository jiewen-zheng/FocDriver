//
// Created by Monster on 2023/6/5.
//

#ifndef FOCDRIVE_CURRENT_SENSE_BASE_H
#define FOCDRIVE_CURRENT_SENSE_BASE_H

#include "../../Common/math_utils.h"

class CurrentSenseBase {
public:
    bool initialized = false;
    float pwm_duty_a;
    float pwm_duty_b;
    float pwm_duty_c;

public:
    virtual void init() = 0;

    virtual bool driverAlign(float align_voltage) = 0;

    virtual PhaseCurrent_t getPhaseCurrents() = 0;

    /** Function reading the magnitude of the current set to the motor */
    virtual float getDCCurrent(float electrical_angle = 0);

    /** Function used for FOC contorl, it reads the DQ currents of the motor */
    virtual DQCurrent_t getFOCCurrents(float electrical_angle);
};


#endif //FOCDRIVE_CURRENT_SENSE_BASE_H
