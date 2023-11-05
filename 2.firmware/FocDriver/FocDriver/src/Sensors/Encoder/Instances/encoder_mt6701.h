//
// Created by moon on 2023/11/1.
//

#ifndef ENCODER_MT6701_H
#define ENCODER_MT6701_H

#include "../encoder_base.h"
#include "../../../Common/DriveLog.h"

#include "thirdparty/SoftI2c.h"

class MT6701 : public EncoderBase {
public:
#define SLAVE_ID    ((uint8_t)0x06)

    enum Mode {
        MODE_I2C,
        MODE_ABZ,
    };

    enum RegMap : uint8_t {
        AngleData2Reg       = 0x03,
        AngleData1Reg       = 0x04,
        UVW_MuxReg          = 0x25,
        ABZ_Mux_DirReg      = 0x29,
        UVW_ABZ_ResReg      = 0x30,
        ABZ_ResReg          = 0x31,
        HYSt_ZPW_ZERO_Reg   = 0x32,
        ZERO_Reg            = 0x33,
        PWM_FreqPol_ModeReg = 0x34,
        A_STOP_START_Reg    = 0x3E,
        A_START_Reg         = 0x3F,
        A_STOP_Reg          = 0x40
    };

    enum RegDivide : uint8_t {
        AngleData2_Bit = 0xFF,
        AngleData1_Bit = 0xFC,
        UVW_Mux_Bit    = 0x80,
        ABZ_Mux_Bit    = 0x40,
        DIR_Bit        = 0b10,
        UVW_Res_Bit    = 0xF0,
        ABZ_Res2_Bit   = 0b11,
        ABZ_Res1_Bit   = 0xFF,
        HYSt_Bit       = 0xC0,
        ZPW_Bit        = 0x30,
        ZERO2_Bit      = 0x0F,
        ZERO1_Bit      = 0xFF,
        PWM_Freq_Bit   = 0x80,
        PWM_Pol_Bit    = 0x40,
        OUT_MODE_Bit   = 0x20,
        A_STOP2_Bit    = 0xF0,
        A_START2_Bit   = 0x0F,
        A_START_Bit    = 0xFF,
        A_STOP_Bit     = 0xFF,
    };

public:
    MT6701() { mode = MODE_ABZ; }
    explicit MT6701(SoftI2c i2c_) :
            i2c(i2c_) { mode = MODE_I2C; }
    ~MT6701() {}

    void init() override;
    float getRawAngle() override;

    float getAngle();

    void config(Direction dir = CW);

private:
    uint8_t readReg(uint8_t reg);
    bool writeReg(uint8_t reg, uint8_t data);

private:
    Mode    mode;
    SoftI2c i2c;
};

#endif //ENCODER_MT6701_H
