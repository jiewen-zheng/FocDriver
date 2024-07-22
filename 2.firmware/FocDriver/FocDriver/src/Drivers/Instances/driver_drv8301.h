//
// Created by Monster on 2023/6/7.
//

#ifndef FOCDRIVE_DRIVER_DRV8301_H
#define FOCDRIVE_DRIVER_DRV8301_H

#include "cstdint"
#include "../driver_base.h"
#include "../../Common/time_utils.h"
#include "../../Common/DriveLog.h"
#include "thirdparty/Stm32Gpio.h"


class Drv8301 : public DriverBase {
public:
    enum FaultType : uint32_t {
        NoFault = 0,      //!< No Fault

        // Status Register 1
        FET_LC_OC = 1 << 0, //!< FET Low side, Phase C Over Current fault
        FET_HC_OC = 1 << 1, //!< FET High side, Phase C Over Current fault
        FET_LB_OC = 1 << 2, //!< FET Low side, Phase B Over Current fault
        FET_HB_OC = 1 << 3, //!< FET High side, Phase B Over Current fault
        FET_LA_OC = 1 << 4, //!< FET Low side, Phase A Over Current fault
        FET_HA_OC = 1 << 5, //!< FET High side, Phase A Over Current fault
        OTW       = 1 << 6, //!< Over Temperature Warning fault
        OTSD      = 1 << 7, //!< Over Temperature Shut Down fault
        PVDD_UV   = 1 << 8, //!< Power supply Vdd Under Voltage fault
        GVDD_UV   = 1 << 9, //!< GVdd Under Voltage fault
        FAULT     = 1 << 10,

        // Status Register 2
        GVDD_OV = 1 << 23,    //!< GVdd Over Voltage fault
    };

    enum Register {
        StatusReg_1  = 0x00,
        StatusReg_2  = 0x01,
        ControlReg_1 = 0x02,
        ControlReg_2 = 0x03,
    };

    enum State {
        Uninitialized,
        StartupChecks,
        Ready,
    } state = Uninitialized;

    struct RegisterFile {
        uint16_t register_1;
        uint16_t register_2;
    };

public:
    Drv8301(float _voltage_power_supply, float _voltage_limit, Stm32Gpio _cs, Stm32Gpio _gate, Stm32Gpio _fault) :
            DriverBase(_voltage_power_supply, _voltage_limit),
            ncs_gpio(_cs), gate_gpio(_gate), fault_gpio(_fault) {}

    void reset();

    bool config(float _gain);
    FaultType getError();
    bool getFault();
    void faultHandler();
    void checks();

    bool init() override;
    bool isReady() override;
    void enable() override;
    void disable() override;
    void setPwm(float Ua, float Ub, float Uc) override;

public:
    uint16_t readReg(uint8_t reg);
    bool writeReg(uint8_t reg, uint16_t data);

private:
    bool      _enable = false;
    Stm32Gpio ncs_gpio;
    Stm32Gpio gate_gpio;
    Stm32Gpio fault_gpio;

    RegisterFile regFile;
};

#endif //FOCDRIVE_DRIVER_DRV8301_H
