//
// Created by moon on 2023/11/1.
//

#include "MT6701.h"
#include "i2c.h"

void MT6701::init() {
    i2c.init();

    config();
    variable_init();
}

void MT6701::config(Direction dir) {
    direction = dir;

}

float MT6701::getAngle() {
    float angle = getRawCount();

    return (angle / (float) 16384) * _2PI;
}

float MT6701::getRawCount() {
    uint8_t temp[2];

    if (mode == MODE_I2C) {
        temp[0] = readReg(AngleData2Reg);
        temp[1] = readReg(AngleData1Reg);

        uint16_t angle = ((uint16_t) temp[0] << 6) | (temp[1] >> 2);

        return (float)angle;
        return (float) angle * 360 / 16384;
    }

    return 0;
}

uint8_t MT6701::readReg(uint8_t reg) {
    uint8_t data[1] = {0};

    i2c.read(MT6701_DEVICE_ADDR, reg, data, 1);

    return data[0];
}

bool MT6701::writeReg(uint8_t reg, uint8_t data) {
    return i2c.write(MT6701_DEVICE_ADDR, reg, &data, 1);
}



