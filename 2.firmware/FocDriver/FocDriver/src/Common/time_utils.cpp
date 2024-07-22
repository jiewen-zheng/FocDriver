//
// Created by Monster on 2023/6/2.
//

#include "time_utils.h"
#include "tim.h"

#define TIM_TIME_BASE       TIM14

unsigned int _micros() {
    uint32_t ms, cycle_cnt;

    do {
        ms        = HAL_GetTick();
        cycle_cnt = TIM_TIME_BASE->CNT;
    } while (ms != HAL_GetTick());

    return (ms * 1000) + cycle_cnt;
}

/**
 * delay function
 * @param ms - delay millisecond
 */
void _delay(unsigned int ms) {
    HAL_Delay(ms - 1);
}

void _delayUs(unsigned int us) {
    uint32_t start = _micros();

    while (_micros() - start < us) {
        asm volatile ("nop");
    }
}


