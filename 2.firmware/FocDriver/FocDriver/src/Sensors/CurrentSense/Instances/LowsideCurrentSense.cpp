//
// Created by moon on 2023/11/7.
//

#include "LowSideCurrentSense.h"
#include "../../../Common/time_utils.h"


void LowSideCurrentSense::init() {
    calibrateOffsets();
}

bool LowSideCurrentSense::driverAlign(float align_voltage) {
    return false;
}


PhaseCurrent_t LowSideCurrentSense::getPhaseCurrents() {
    PhaseCurrent_t current = {0};

    if (!readADC) return current;

    current.a = APhase ? (readADC(LowSide_A) - offset_ia) * gain_a : 0;
    current.b = BPhase ? (readADC(LowSide_B) - offset_ib) * gain_b : 0;
    current.c = CPhase ? (readADC(LowSide_C) - offset_ic) * gain_c : 0;
    return current;
}

void LowSideCurrentSense::linkADCCallback(LowSideCurrentSense::readPhaseADCVoltage readADC_) {
    readADC = readADC_;
}

void LowSideCurrentSense::driverSyncLowSide() {

}

void LowSideCurrentSense::calibrateOffsets() {
    const int calibration_rounds = 2000;

    offset_ia = 0;
    offset_ib = 0;
    offset_ic = 0;

    if (!readADC) {
        DRIVE_LOG("LCS: Not ADC callback.");
        return;
    }
    /* read adc voltage */
    for (int i = 0; i < calibration_rounds; i++) {
        if (APhase) offset_ia += readADC(LowSide_A);
        if (BPhase) offset_ib += readADC(LowSide_B);
        if (CPhase) offset_ic += readADC(LowSide_C);
        _delay(1);
    }

    if (APhase) offset_ia = offset_ia / calibration_rounds;
    if (BPhase) offset_ib = offset_ib / calibration_rounds;
    if (CPhase) offset_ic = offset_ic / calibration_rounds;
}




