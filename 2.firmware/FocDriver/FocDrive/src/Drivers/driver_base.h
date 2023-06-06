//
// Created by Monster on 2023/6/5.
//

#ifndef FOCDRIVE_DRIVER_BASE_H
#define FOCDRIVE_DRIVER_BASE_H


class DriverBase {
public:
    explicit DriverBase()= default;


    float voltage_power_supply = 12.0f;

    virtual void init() = 0;

    virtual void setPwm(float Ua, float Ub, float Uc) = 0;
};


#endif //FOCDRIVE_DRIVER_BASE_H
