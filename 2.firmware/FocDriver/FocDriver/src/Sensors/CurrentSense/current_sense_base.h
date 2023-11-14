//
// Created by Monster on 2023/6/5.
//

#ifndef FOCDRIVE_CURRENT_SENSE_BASE_H
#define FOCDRIVE_CURRENT_SENSE_BASE_H

#include "../../Common/math_utils.h"
#include "../../Drivers/driver_base.h"

class CurrentSenseBase {
public:
    bool  initialized = false;
    float pwm_duty_a{};
    float pwm_duty_b{};
    float pwm_duty_c{};

public:
    virtual void init() = 0;
    virtual bool driverAlign(float align_voltage) = 0;
    virtual PhaseCurrent_t getPhaseCurrents() = 0;

    void linkDriver(DriverBase *driver_) {
        driver = driver_;
    }

    /** Function reading the magnitude of the current set to the motor */
    virtual float getDCCurrent(float electrical_angle = 0);

    /** Function used for FOC contorl, it reads the DQ currents of the motor */
    virtual DQCurrent_t getFOCCurrents(float electrical_angle);

private:
    DriverBase *driver = nullptr;
};

#endif //FOCDRIVE_CURRENT_SENSE_BASE_H
