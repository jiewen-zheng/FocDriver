//
// Created by moon on 2023/11/7.
//

#ifndef FOCDRIVE_LOWSIDECURRENTSENSE_H
#define FOCDRIVE_LOWSIDECURRENTSENSE_H

#include "../current_sense_base.h"
#include "../../../Common/DriveLog.h"

class LowSideCurrentSense : public CurrentSenseBase {
public:
    enum LowSidePhase : uint8_t {
        LowSide_A,
        LowSide_B,
        LowSide_C
    };

    typedef float (*readPhaseADCVoltage)(LowSidePhase phase);

    float gain_a;   //!< phase A gain
    float gain_b;   //!< phase B gain
    float gain_c;   //!< phase C gain

    float offset_ia;    //!< zero current A voltage offset value
    float offset_ib;    //!< zero current B voltage offset value
    float offset_ic;    //!< zero current C voltage offset value

public:
    explicit LowSideCurrentSense(float resistor_, float gain_, bool A = true, bool B = true, bool C = true) :
            shunt_resistor(resistor_), amp_gain(gain_),
            APhase(A), BPhase(B), CPhase(C) {

        /* calc volts to amps */
        volts_to_amps_ratio = 1.0f / shunt_resistor / amp_gain;
        gain_a              = volts_to_amps_ratio;
        gain_b              = volts_to_amps_ratio;
        gain_c              = volts_to_amps_ratio;
    }

    void init() override;
    bool driverAlign(float align_voltage) override;
    PhaseCurrent_t getPhaseCurrents() override;

    void linkADCCallback(readPhaseADCVoltage readADC_);

    void driverSyncLowSide();
    void calibrateOffsets();

private:
    float shunt_resistor;   //!< 分流电阻值
    float amp_gain;         //!< 放大器增益
    float volts_to_amps_ratio;  //!< 伏安比

    readPhaseADCVoltage readADC = nullptr;  //!< adc读取回调函数

    bool APhase, BPhase, CPhase;    //!< 低侧电流传感器连接状态
};

#endif //FOCDRIVE_LOWSIDECURRENTSENSE_H
