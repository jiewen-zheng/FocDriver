//
// Created by Monster on 2023/6/5.
//

#ifndef FOCDRIVE_DRIVER_BASE_H
#define FOCDRIVE_DRIVER_BASE_H


class DriverBase {
public:
    explicit DriverBase(float _voltage_power_supply) :
            voltage_power_supply(_voltage_power_supply) {

    }

    float voltage_power_supply = 12.0f;
    float duty_a = 0;
    float duty_b = 0;
    float duty_c = 0;

public:

    virtual bool init() = 0;

    virtual void enable() = 0;

    virtual void disable() = 0;

    virtual void setPwm(float Ua, float Ub, float Uc) = 0;
};


#endif //FOCDRIVE_DRIVER_BASE_H
