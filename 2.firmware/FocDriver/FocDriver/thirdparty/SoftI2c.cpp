//
// Created by moon on 2023/11/2.
//

#include "SoftI2c.h"

void SoftI2c::init() {
    scl.config(GPIO_MODE_OUTPUT_PP, GPIO_NOPULL);
    sda.config(GPIO_MODE_OUTPUT_PP, GPIO_NOPULL);

    sda.write(true);
    scl.write(true);
}

bool SoftI2c::write(uint8_t dev_id, uint8_t mem_addr, uint8_t *data, uint16_t size) {
    init();

    start();
    writeData(dev_id << 1);
    if (!checkAck()) return false;
    this->delay(WAITE_TIME);

    writeData(mem_addr);
    if (!checkAck()) return false;
    this->delay(WAITE_TIME);

    while (size--) {
        writeData(*data++);
        if (!checkAck()) return false;
        this->delay(WAITE_TIME);
    }
    stop();

    return true;
}

bool SoftI2c::read(uint8_t dev_id, uint8_t mem_addr, uint8_t *data, uint16_t size) {
    init();

    start();
    writeData(dev_id << 1);
    if (!checkAck())
        return false;
    this->delay(WAITE_TIME);

    writeData(mem_addr);
    if (!checkAck())
        return false;
    this->delay(WAITE_TIME);

    start();
    writeData((dev_id << 1) | 0x01);
    if (!checkAck())
        return false;
//    this->delay(WAITE_TIME);
    while (size--) {
        this->delay(WAITE_TIME);
        *data++ = readData();

        if (size != 0)
            sendAck();
    }
    stop();

    return true;
}

void SoftI2c::clockOut() {
    scl.write(true);
    this->delay(WAITE_TIME);
    scl.write(false);
}

void SoftI2c::start() {
    sda.write(true);
    scl.write(true);
    this->delay(WAITE_TIME);
    sda.write(false);
    this->delay(WAITE_TIME);
    scl.write(false);
}

void SoftI2c::stop() {
    sda.write(false);
    scl.write(true);
    this->delay(WAITE_TIME);
    sda.write(true);
    this->delay(WAITE_TIME);
}

bool SoftI2c::checkAck() {
    int ack = 0;
    sda.config(GPIO_MODE_INPUT, GPIO_NOPULL);

    scl.write(true);
    this->delay(WAITE_TIME);
    for (int i = CHECK_ACK_COUNT; i > 0; i--) {
        if (!sda.read()) {
            ack = 1;
            break;
        }
    }

    scl.write(false);
    sda.config(GPIO_MODE_OUTPUT_PP, GPIO_NOPULL);

    if (!ack) {
        stop();
        return false;
    }
//    this->delay(WAITE_TIME);
    return true;
}

void SoftI2c::sendAck() {
    sda.config(GPIO_MODE_OUTPUT_PP, GPIO_NOPULL);
    sda.write(false);
    this->delay(WAITE_TIME);

    scl.write(true);
    this->delay(WAITE_TIME);
    scl.write(false);

    sda.config(GPIO_MODE_OUTPUT_PP, GPIO_NOPULL);
    this->delay(WAITE_TIME);
}

void SoftI2c::sendNotAck() {
    sda.config(GPIO_MODE_OUTPUT_PP, GPIO_NOPULL);
    sda.write(true);
    this->delay(WAITE_TIME);

    scl.write(true);
    this->delay(WAITE_TIME);
    scl.write(false);

    sda.config(GPIO_MODE_OUTPUT_PP, GPIO_NOPULL);
    this->delay(WAITE_TIME);
}

uint8_t SoftI2c::readData() {
    uint8_t data = 0;

    sda.config(GPIO_MODE_INPUT, GPIO_NOPULL);
    for (int i = 8; i--;) {
        scl.write(true);
        data <<= 1;
        if (sda.read()) data |= 0x01;

        this->delay(WAITE_TIME);
        scl.write(false);
        this->delay(WAITE_TIME);
    }
    sda.config(GPIO_MODE_OUTPUT_PP, GPIO_NOPULL);

    return data;
}

void SoftI2c::writeData(uint8_t data) {

    scl.write(false);

    for (int i = 7; i >= 0; i--) {

        sda.write(data & (1 << i));
        this->delay(WAITE_TIME);
        clockOut();
    }
}







