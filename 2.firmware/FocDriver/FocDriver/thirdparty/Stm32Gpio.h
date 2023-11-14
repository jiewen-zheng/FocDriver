//
// Created by moon on 2023/10/19.
//

#ifndef STM32EXAMPLE_STM32GPIO_H
#define STM32EXAMPLE_STM32GPIO_H

#include "gpio.h"

class Stm32Gpio {
public:
//    static const Stm32Gpio none;
    Stm32Gpio() :
            port(nullptr), pin(0) {}

    Stm32Gpio(GPIO_TypeDef *_port, uint32_t _pin) :
            port(_port), pin(_pin) {}

    bool config(uint32_t mode, uint32_t pull = GPIO_NOPULL, uint32_t speed = GPIO_SPEED_FREQ_LOW);

    void write(bool state);
    bool read();
    void toggle();

    bool subscribe(bool risingEdge, bool fallingEdge, void (*callback)(void *), void *data);
    void unsubscribe();

    uint16_t getPinNumber();
    inline IRQn_Type getIrqNumber();

public:
    GPIO_TypeDef     *port;
    uint32_t         pin;
    uint32_t         lastMode{};
    GPIO_InitTypeDef GPIO_InitStruct = {0};
};

#endif //STM32EXAMPLE_STM32GPIO_H
