//
// Created by Monster on 2023/6/7.
//
#include "common_inc.h"
#include "tim.h"

#include "gpio.h"
#include <cstdio>

#include "FocDriver.h"
#include "Commander/Commander.h"

Motor     motor(7);
Commander commander(256);

MT6701 mt6701{
        {{GPIOB, GPIO_PIN_7},
         {GPIOB, GPIO_PIN_6},
         _delayUs}
};

Drv8301 drvM0{
        12.0f, 12.0f,
        {M0_CS_GPIO_Port, M0_CS_Pin},
        {},
        {EN_GATE_GPIO_Port, EN_GATE_Pin}
};

Drv8301 drvM1{
        12.0f, 12.0f,
        {M1_CS_GPIO_Port, M1_CS_Pin},
        {},
        {FAULT_GPIO_Port, FAULT_Pin}
};

//target variable
float target_velocity = 0;

void receivedCommand(unsigned char *data, unsigned int size) {
    commander.write((char *) data, size);
}

void doTarget(char *cmd) { commander.scalar(&target_velocity, cmd); }

void EnGate(bool en) {
    if (en)
        HAL_GPIO_WritePin(EN_GATE_GPIO_Port, EN_GATE_Pin, GPIO_PIN_SET);
    else
        HAL_GPIO_WritePin(EN_GATE_GPIO_Port, EN_GATE_Pin, GPIO_PIN_RESET);
}

void Main() {
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);

    EnGate(false);
    _delay(10);
    EnGate(true);
    _delay(500);

    drvM1.config(10);
    while (!drvM1.init()) {
        _delay(1000);
    }

    mt6701.init();

    motor.linkEncoder(&mt6701);
    motor.linkDriver(&drvM1);
    motor.voltage_limit = 6; //!< [V] current = voltage / resistance, so try to be well under 1Amp
    motor.controller_mode = Motor::VELOCITY_OPEN_LOOP;
    motor.init();
    motor.initFOC(EncoderBase::CW);


    commander.add("T", doTarget, "Set Target");
    for (;;) {
        /* Drv8301 error check */
        if(drvM1.getFAULT()){
            EnGate(false);
            DRIVE_LOG("nFAULT Pin: %d", drvM1.getFAULT());
            DRIVE_LOG("nFAULT B2: %x", drvM1.readReg(Drv8301::StatusReg_1));
            drvM1.setPwm(0, 0, 0);
            while (1);
        }

        motor.loopFOC();
        motor.move(target_velocity);


        commander.run();
//
//        DRIVE_LOG("A :%.2f", mt6701.getAngle());
//        _delay(100);
    }
}