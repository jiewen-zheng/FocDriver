//
// Created by moon on 2023/11/12.
//

#include "AS5600.h"

#include "i2c.h"

void AS5600::init() {
    direction = CW;
    variable_init();
}

float AS5600::getAngle() {
    uint16_t angle = getRawCount();
    return ((float) angle / (float) RESOLUTION_RATIO) * _2PI;
}

int AS5600::getRawCount() {
    uint8_t data[2];


    readReg(RawAngleReg1, data, 2);
    uint16_t raw_count = ((uint16_t) data[0] << 8) | (uint16_t) data[1];

    return raw_count;
}

uint8_t AS5600::readReg(uint8_t reg) {
    uint8_t data[2];

    HAL_I2C_Mem_Read(&hi2c1, AS5600_DEVICE_ADDR << 1, reg, 1,
                     data, 1, 100);

    return data[0];
}

bool AS5600::readReg(uint8_t reg, uint8_t *data, uint16_t size) {
    return HAL_I2C_Mem_Read(&hi2c1, AS5600_DEVICE_ADDR << 1, reg, 1,
                            data, size, 200);
}

bool AS5600::writeReg(uint8_t reg, uint8_t data) {
    return false;
}



