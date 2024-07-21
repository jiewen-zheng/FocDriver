//
// Created by Monster on 2023/6/5.
//

#ifndef DRIVER_BASE_H
#define DRIVER_BASE_H

class DriverBase {
public:
    float voltage_power_supply = 12.0f; //!< power supply voltage
    float voltage_limit        = 6.0f; //!< limiting voltage set to the motor
    float duty_a               = 0;
    float duty_b               = 0;
    float duty_c               = 0;

public:
    explicit DriverBase(float _voltage_power_supply, float _voltage_limit) :
            voltage_power_supply(_voltage_power_supply), voltage_limit(_voltage_limit) {}

    ~DriverBase() {}

    virtual bool init() = 0;
    virtual void enable() = 0;
    virtual void disable() = 0;
    virtual bool isReady() = 0;

    virtual void setPwm(float Ua, float Ub, float Uc) = 0;

private:

};

#endif // DRIVER_BASE_H
