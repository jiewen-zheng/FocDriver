//
// Created by Monster on 2023/6/7.
//

#ifndef FOCDRIVE_DRIVER_DRV8301_H
#define FOCDRIVE_DRIVER_DRV8301_H

#include "../driver_base.h"

class DriverDrv8301 : public DriverBase {
public:
    explicit DriverDrv8301(float _voltage_power_supply) :
            DriverBase(_voltage_power_supply) {}

    bool init() override;

    void enable() override;

    void disable() override;

    void setPwm(float Ua, float Ub, float Uc) override;

private:
    bool _enable = false;
};


#endif //FOCDRIVE_DRIVER_DRV8301_H
