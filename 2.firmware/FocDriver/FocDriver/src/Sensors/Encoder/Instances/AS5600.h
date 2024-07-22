//
// Created by moon on 2023/11/12.
//

#ifndef ENCODER_AS5600_H
#define ENCODER_AS5600_H

#include "../encoder_base.h"
#include "../../../Common/DriveLog.h"

class AS5600 : public EncoderBase {
public:
#define AS5600_DEVICE_ADDR  ((uint8_t)0x36)
#define RESOLUTION_RATIO    4096    //!< 12 bit Resolution ratio

    enum RegMap : uint8_t {
        RawAngleReg1 = 0x0C,
        RawAngleReg2 = 0x0D,
        AngleReg1    = 0x0E,
        AngleReg2    = 0x0F,
    };

public:
    AS5600() {
        min_elapsed_time = 0.00005; // 50 microseconds
    }

    ~AS5600() {}

    void init() override;
    float getAngle() override;

    int getRawCount();

private:
    uint8_t readReg(uint8_t reg);
    bool readReg(uint8_t reg, uint8_t *data, uint16_t size);
    bool writeReg(uint8_t reg, uint8_t data);

};

#endif //ENCODER_AS5600_H
