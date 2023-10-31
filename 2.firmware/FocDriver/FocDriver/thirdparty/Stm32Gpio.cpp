//
// Created by moon on 2023/10/19.
//

#include "Stm32Gpio.h"

#define N_EXIT  16

struct Subscriptions {
    GPIO_TypeDef *port{};
    void (*callback)(void *) = nullptr;
    void *data = nullptr;
} static subscriptions[N_EXIT];


bool Stm32Gpio::config(uint32_t mode, uint32_t pull, uint32_t speed) {
    if (port == GPIOA) {
        __HAL_RCC_GPIOA_CLK_ENABLE();
    } else if (port == GPIOB) {
        __HAL_RCC_GPIOB_CLK_ENABLE();
    } else if (port == GPIOC) {
        __HAL_RCC_GPIOC_CLK_ENABLE();
    } else if (port == GPIOD) {
        __HAL_RCC_GPIOD_CLK_ENABLE();
    } else if (port == GPIOE) {
        __HAL_RCC_GPIOE_CLK_ENABLE();
    } else if (port == GPIOF) {
        __HAL_RCC_GPIOF_CLK_ENABLE();
    } else if (port == GPIOG) {
        __HAL_RCC_GPIOG_CLK_ENABLE();
    } else {
        return false;
    }

    GPIO_InitStruct.Pin   = pin;
    GPIO_InitStruct.Mode  = mode;
    GPIO_InitStruct.Pull  = pull;
    GPIO_InitStruct.Speed = speed;
    HAL_GPIO_Init(port, &GPIO_InitStruct);

    return true;
}

void Stm32Gpio::write(bool state) {
    if (port)
        HAL_GPIO_WritePin(port, pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

bool Stm32Gpio::read() {
    return port && HAL_GPIO_ReadPin(port, pin);
}

bool Stm32Gpio::subscribe(bool risingEdge, bool fallingEdge, void (*callback)(void *), void *data) {
    uint16_t pin_number = getPinNumber();
    if (pin_number >= N_EXIT)
        return false;

    if (risingEdge && fallingEdge) {
        GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
    } else if (risingEdge) {
        GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    } else if (fallingEdge) {
        GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    } else {
        return false;
    }

    struct Subscriptions &subscription = subscriptions[pin_number];

    HAL_GPIO_Init(port, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(getIrqNumber(), 5, 0);
    HAL_NVIC_EnableIRQ(getIrqNumber());

    __HAL_GPIO_EXTI_CLEAR_IT(pin);

    subscription.port     = port;
    subscription.callback = callback;
    subscription.data     = data;

    return true;
}

void Stm32Gpio::unsubscribe() {
    uint16_t pin_number = getPinNumber();
    if (pin_number >= N_EXIT)
        return;

    struct Subscriptions &subscription = subscriptions[pin_number];

    if (port != subscription.port)
        return; // the subscription was not for this GPIO

    CLEAR_BIT(EXTI->IMR, pin);
    __HAL_GPIO_EXTI_CLEAR_IT(pin);

    subscription.port     = nullptr;
    subscription.callback = nullptr;
    subscription.data     = nullptr;
}

uint16_t Stm32Gpio::getPinNumber() {
    uint16_t pin_number = 0;
    uint16_t pin_mask   = pin >> 1;

    while (pin_mask) {
        pin_mask >>= 1;
        pin_number++;
    }
    return pin_number;
}

IRQn_Type Stm32Gpio::getIrqNumber() {
    switch (getPinNumber()) {
        case 0:
            return EXTI0_IRQn;
        case 1:
            return EXTI1_IRQn;
        case 2:
            return EXTI2_IRQn;
        case 3:
            return EXTI3_IRQn;
        case 4:
            return EXTI4_IRQn;
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
            return EXTI9_5_IRQn;
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
            return EXTI15_10_IRQn;
        default:
            return (IRQn_Type) 0; // impossible
    }
}


