//
// Created by Monster on 2023/6/7.
//
#include "common_inc.h"

#include "gpio.h"
#include <cstdio>

#include "FocDriver.h"
#include "Commander/Commander.h"

Motor     motor(7);
Commander commander(256);

//target variable
float target_velocity = 0;

void receivedCommand(unsigned char *data, unsigned int size) {
    commander.write((char *) data, size);
}

void doTarget(char* cmd) { commander.scalar(&target_velocity, cmd); }

void EnGate(bool en) {
    if (en)
        HAL_GPIO_WritePin(EN_GATE_GPIO_Port, EN_GATE_Pin, GPIO_PIN_SET);
    else
        HAL_GPIO_WritePin(EN_GATE_GPIO_Port, EN_GATE_Pin, GPIO_PIN_RESET);
}

Drv8301 drvM0{
        12.0f, 6.0f,
        {M0_CS_GPIO_Port, M0_CS_Pin},
        {},
        {EN_GATE_GPIO_Port, EN_GATE_Pin}
};

Drv8301 drvM1{
        12.0f, 6.0f,
        {M1_CS_GPIO_Port, M1_CS_Pin},
        {},
        {FAULT_GPIO_Port, FAULT_Pin}
};

void Main() {
    EnGate(false);
    _delay(10);
    EnGate(true);
    _delay(500);

    drvM1.config(10);
    while (!drvM1.init()) {
        _delay(1000);
    }

    motor.linkDriver(&drvM1);
    motor.voltage_limit = 3; //!< [V] current = voltage / resistance, so try to be well under 1Amp
    motor.controller_mode = Motor::ANGLE_OPEN_LOOP;
    motor.init();

    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);

    commander.add("T", doTarget, "Set Target");

    for (;;) {
        /* Drv8301 error check */
        if(drvM1.getFAULT()){
            EnGate(false);
            DRIVE_LOG("nFAULT Pin: %d", drvM1.getFAULT());
            drvM1.setPwm(0, 0, 0);
            while (1);
        }

        motor.move(target_velocity);

        commander.run();
//        _delayUs(500);
    }
}