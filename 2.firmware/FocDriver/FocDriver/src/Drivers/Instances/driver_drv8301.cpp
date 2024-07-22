//
// Created by Monster on 2023/6/7.
//

#include "driver_drv8301.h"
#include "../../Common/math_utils.h"

#include "spi.h"
#include "tim.h"

void Drv8301::reset() {
    disable();
    enable();
}

Drv8301::FaultType Drv8301::getError() {
    uint16_t fault1 = readReg(StatusReg_1);
    uint16_t fault2 = readReg(StatusReg_2);

    return FaultType((uint32_t) fault1 | ((uint32_t) (fault2 & 0x0080) << 16));
}

bool Drv8301::getFault() {
    return !fault_gpio.read();
}

bool Drv8301::config(float _gain) {
    uint8_t gain           = 3;    //!< max gian setting.
    float   gain_choices[] = {10.0f, 20.0f, 40.0f, 80.0f};

    _gain = (_gain > gain_choices[gain]) ? gain_choices[gain] : _gain;
    while (gain && (gain_choices[gain] > _gain)) {
        gain--;
    }

    RegisterFile newFile{};

    newFile.register_1 =
            (0 << 6)        //!< "Over current Trip" = OC_ADJ_SET / MOS-FET RDS(on), example: 26.17 A = 0.123 V/ 4.7 mΩ
            | (0b01 << 4)   //!< OCP_MODE: OC latch shut down mode.
            | (0b1 << 3)    //!< 3x PWM mode.(0: 6xPWM, 1: 3xPWM)
            | (0b0 << 2)    //!< GATE_RESET: don't reset latched faults.
            | (0b01 << 0);  //!< gate-drive peak current: 0.7A

    newFile.register_2 =
            (0b0 << 6)      //!< OC_TOFF: CBC mode.
            | (0b00 << 4)   //!< CH1 CH2 calibration off (normal operation)
            | (gain << 2)   //!< gain
            | (0b00 << 0);  //!< OCTW_MODE: Report both OT and OC at nOCTW pin.

    if ((regFile.register_1 != newFile.register_1) || (regFile.register_2 != newFile.register_2)) {
        regFile = newFile;
        state   = State::Uninitialized;
        disable();
        return false;
    }

    return true;
}

bool Drv8301::init() {
    if (state == State::Ready) {
        return true;
    }

    //! Reset DRV chip. The enable pin also controls the SPI interface.
    disable();
    state = State::Uninitialized;
    enable();

    //!< log drive id
    uint16_t status_data = readReg(StatusReg_2);
    DRIVE_LOG("DRV: ID = 0x%x", status_data & 0x000F);

    //! write config
    writeReg(ControlReg_1, regFile.register_1);
    writeReg(ControlReg_2, regFile.register_2);

    _delayUs(100);  //!< Wait for config to be applied.
    state = State::StartupChecks;

    uint16_t reg1_data = readReg(ControlReg_1) & 0x7FF;
    uint16_t reg2_data = readReg(ControlReg_2) & 0x7FF;
    if (reg1_data != regFile.register_1 || reg2_data != regFile.register_2) {
        DRIVE_LOG("DRV: Control register write filed.");
        return false;
    }

    if (getFault() || (getError() != NoFault)) {
        DRIVE_LOG("DRV: Fault.");
        return false;
    }

    state = State::Ready;
    DRIVE_LOG("DRV: Init success.");
    return state == State::Ready;
}

void Drv8301::enable() {
    gate_gpio.write(true);
    _delay(200);

    _enable = true;
}

void Drv8301::disable() {
    setPwm(0, 0, 0);

    gate_gpio.write(false);
    _delay(10);  //!< Full reset must be greater than 13us ~ 15us.

    _enable = false;
}

bool Drv8301::isReady() {
    return _enable && (state == Ready);
}

void Drv8301::setPwm(float Ua, float Ub, float Uc) {
    if (!_enable) return;

    /** Log phase voltage */
//    float debug[3] = {Ua, Ub, Uc};
//    DriveLog::sendProtocol(debug, 3);

    Ua = _constrain(Ua, 0.0f, voltage_limit);
    Ub = _constrain(Ub, 0.0f, voltage_limit);
    Uc = _constrain(Uc, 0.0f, voltage_limit);

    duty_a = _constrain(Ua / voltage_power_supply, 0.0f, 1.0f);
    duty_b = _constrain(Ub / voltage_power_supply, 0.0f, 1.0f);
    duty_c = _constrain(Uc / voltage_power_supply, 0.0f, 1.0f);

    /**
     * TODO: Confirm 3 phase
     * abc acb bac bca cab cba
     */
    __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_1, duty_a * (float) TIM_PERIOD);
    __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_2, duty_b * (float) TIM_PERIOD);
    __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_3, duty_c * (float) TIM_PERIOD);
}

uint16_t Drv8301::readReg(uint8_t reg) {
    uint16_t txData = ((uint16_t) reg << 11) | 0x8000;  //!< read command
    uint16_t rxData = 0xFFFF;

    ncs_gpio.write(false);

    HAL_SPI_Transmit(&hspi3, (uint8_t *) &txData, 1, 1000);
    ncs_gpio.write(true);

    _delayUs(1);

    ncs_gpio.write(false);
    HAL_SPI_TransmitReceive(&hspi3, (uint8_t *) &txData, (uint8_t *) &rxData, 1, 1000);
    ncs_gpio.write(true);

    return rxData;
}

bool Drv8301::writeReg(uint8_t reg, uint16_t data) {
    uint16_t txData = ((uint16_t) reg << 11) | (data & 0x7FF);

    uint16_t rxData;

    ncs_gpio.write(false);
    HAL_StatusTypeDef state = HAL_SPI_Transmit(&hspi3, (uint8_t *) &txData, 1, 1000);
    ncs_gpio.write(true);

    return state == HAL_OK;
}

void Drv8301::checks() {
    if (state != Uninitialized && !fault_gpio.read()) {
        state = Uninitialized;
    }
}

void Drv8301::faultHandler() {
    if (!getFault()) {
        /* No Fault happen */
        return;
    }

    /* Stop motor now */
    disable();
    DRIVE_LOG("DRV: Error!");
    while (true) {
        DRIVE_LOG("nFAULT Bit: 0x%x", readReg(Drv8301::StatusReg_1));
        _delay(1000);
    }

}



