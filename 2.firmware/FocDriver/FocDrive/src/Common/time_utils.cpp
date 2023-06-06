//
// Created by Monster on 2023/6/2.
//

#include "time_utils.h"
#include "stm32f4xx_hal.h"

void _delay(unsigned long ms) {
    HAL_Delay(ms);
}

unsigned long _micros() {
    return 0;
}
