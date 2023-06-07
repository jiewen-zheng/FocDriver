//
// Created by Monster on 2023/6/7.
//

#include "driver_drv8301.h"
#include "../../Common/math_utils.h"

bool DriverDrv8301::init() {
    return false;
}

void DriverDrv8301::enable() {
    _enable = true;
}

void DriverDrv8301::disable() {
    _enable = false;
}

void DriverDrv8301::setPwm(float Ua, float Ub, float Uc) {
    Ua = _constrain(Ua, 0.0f, voltage_power_supply);
    Ub = _constrain(Ub, 0.0f, voltage_power_supply);
    Uc = _constrain(Uc, 0.0f, voltage_power_supply);

    duty_a = Ua / voltage_power_supply;
    duty_b = Ua / voltage_power_supply;
    duty_c = Uc / voltage_power_supply;

    /**
     * TODO: Implement mcu pwm setting method
     */

}
