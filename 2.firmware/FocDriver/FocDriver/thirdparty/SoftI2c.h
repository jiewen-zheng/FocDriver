//
// Created by moon on 2023/11/2.
//

#ifndef SOFT_I2C_H
#define SOFT_I2C_H

#include "Stm32Gpio.h"

class SoftI2c {
#define WAITE_TIME      10
#define CHECK_ACK_COUNT 50
    typedef void (*I2cDelay)(uint32_t time);

public:
    SoftI2c(Stm32Gpio scl_, Stm32Gpio sda_, I2cDelay delay_) :
            scl(scl_), sda(sda_), delay(delay_) {}

    SoftI2c();

    void init();

    bool write(uint8_t dev_id, uint8_t mem_addr, uint8_t *data, uint16_t size);
    bool read(uint8_t dev_id, uint8_t mem_addr, uint8_t *data, uint16_t size);

protected:
     void clockOut();
     void start();
     void stop();
     bool checkAck();
     void sendAck();
     void sendNotAck();
    void writeData(uint8_t data);
    uint8_t readData();

private:
    Stm32Gpio scl;
    Stm32Gpio sda;
    I2cDelay  delay;
};

#endif // SOFT_I2C_H
