//
// Created by Monster on 2023/6/7.
//
#include "common_inc.h"
#include "tim.h"

#include "gpio.h"
#include "adc.h"
#include <cstdio>

#include "FocDriver.h"
#include "Commander/Commander.h"

#include "soft_timer/soft_timer.h"

Motor     motor(7, 2.3);
Commander commander(256, '=', '!');
//MT6701    mt6701{
//        {{GPIOB, GPIO_PIN_7},
//        {GPIOB, GPIO_PIN_6},
//        _delayUs}
//};

AS5600              as5600;
LowSideCurrentSense lcs(0.0002f, 10, false); // No A phase amp

Drv8301 drvM0{
        DEF_POWER_SUPPLY, DEF_VOLTAGE_LIMIT,
        {M0_CS_GPIO_Port, M0_CS_Pin},
        {EN_GATE_GPIO_Port, EN_GATE_Pin},
        {FAULT_GPIO_Port, FAULT_Pin}
};
Drv8301 drvM1{
        DEF_POWER_SUPPLY, DEF_VOLTAGE_LIMIT,
        {M1_CS_GPIO_Port, M1_CS_Pin},
        {EN_GATE_GPIO_Port, EN_GATE_Pin},
        {FAULT_GPIO_Port, FAULT_Pin}
};

//target variable
float    target_velocity = 0;
uint16_t adcData[2]      = {5, 10};

void plot_msg(soft_timer_t *timer);
void serial_handler(soft_timer_t *timer);

void receivedCommand(unsigned char *data, unsigned int size) {
    commander.write((char *) data, size);
}

void doTarget(char *cmd) { commander.scalar(&target_velocity, cmd); }

void doLpfAngle(char *cmd) { commander.scalar(&motor.lpf_angle.Tf, cmd); }

void doLpfVelocity(char *cmd) { commander.scalar(&motor.lpf_velocity.Tf, cmd); }

void doAP(char *cmd) { commander.scalar(&motor.pid_angle.P, cmd); }

void doAI(char *cmd) { commander.scalar(&motor.pid_angle.I, cmd); }

void doAD(char *cmd) { commander.scalar(&motor.pid_angle.D, cmd); }

void doVP(char *cmd) { commander.scalar(&motor.pid_velocity.P, cmd); }

void doVI(char *cmd) { commander.scalar(&motor.pid_velocity.I, cmd); }

void doVD(char *cmd) { commander.scalar(&motor.pid_velocity.D, cmd); }

void doVRamp(char *cmd) { commander.scalar(&motor.pid_velocity.out_ramp, cmd); }

void doARamp(char *cmd) { commander.scalar(&motor.pid_angle.out_ramp, cmd); }

void commander_init() {
    commander.add("T", doTarget, "target"); //!< use method: "cmd=value", example: "T=2.0"
    commander.add("LA", doLpfAngle, "angle lpf");
    commander.add("LV", doLpfVelocity, "velocity lpf");

    commander.add("AP", doAP, "angle p");
    commander.add("AI", doAI, "angle i");
    commander.add("AD", doAD, "angle d");
    commander.add("AR", doARamp, "angle ramp");

    commander.add("VP", doVP, "velocity p");
    commander.add("VI", doVI, "velocity i");
    commander.add("VD", doVD, "velocity d");
    commander.add("VR", doVRamp, "vel ramp");
}

/**
 * 两个驱动器的enGate引脚使用同一个gpio
 * 在外部统一管理
 * @param en
 */
void EnGate(bool en) {
    if (en)
        HAL_GPIO_WritePin(EN_GATE_GPIO_Port, EN_GATE_Pin, GPIO_PIN_SET);
    else
        HAL_GPIO_WritePin(EN_GATE_GPIO_Port, EN_GATE_Pin, GPIO_PIN_RESET);
}

void DC_CAL(bool cal) {
    if (cal)
        HAL_GPIO_WritePin(M1_DC_CAL_GPIO_Port, M1_DC_CAL_Pin, GPIO_PIN_SET);
    else
        HAL_GPIO_WritePin(M1_DC_CAL_GPIO_Port, M1_DC_CAL_Pin, GPIO_PIN_RESET);

}

extern "C" void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
    if (hadc->Instance == ADC2) {
//        testPin.toggle();
//        testPin.write(true);
//        _delayUs(2);
//        testPin.write(false);

    }
}

extern "C" void HAL_ADCEx_InjectedConvCpltCallback(ADC_HandleTypeDef *hadc) {
    if (hadc->Instance == ADC2) {
//        testPin.toggle();
        adcData[0] = HAL_ADCEx_InjectedGetValue(hadc, ADC_INJECTED_RANK_1);
        adcData[1] = HAL_ADCEx_InjectedGetValue(hadc, ADC_INJECTED_RANK_2);
    }
}

float readADC(LowSideCurrentSense::LowSidePhase phase) {
    switch (phase) {
    case LowSideCurrentSense::LowSide_A:
        return 0;
    case LowSideCurrentSense::LowSide_B:
        return ((float) adcData[0] / 4096) * 3.3f;
    case LowSideCurrentSense::LowSide_C:
        return ((float) adcData[1] / 4096) * 3.3f;
    default:
        return 0;
    }
}

extern "C" void Main() {
//    testPin.config(GPIO_MODE_OUTPUT_PP);

    HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_3);
//    HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_4);   //!< adc trigger sample

//    __HAL_TIM_CLEAR_IT(&htim8, TIM_IT_UPDATE);
//    __HAL_TIM_ENABLE_IT(&htim8, TIM_IT_UPDATE);
//    HAL_ADC_Start_DMA(&hadc2, (uint32_t *) adcData, 2);


    drvM1.config(10);
    while (!drvM1.init()) {
        _delay(2000);
    }

//    DRIVE_LOG("DRV: Start DC calibration");
//    DC_CAL(true);
//    _delay(1000);
//    DC_CAL(false);
//    _delay(1000);

//    __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_4, TIM_PERIOD - 2);   //!< start adc
//    HAL_ADCEx_InjectedStart_IT(&hadc2);    //!< 注入转换结束中断
//
//    lcs.linkADCCallback(readADC);
//    lcs.init();

    as5600.init();
    motor.linkEncoder(&as5600);
    motor.linkDriver(&drvM1);

//    motor.limit.current = 0.5f;
    motor.limit.voltage   = 6;
    motor.controller_mode = Motor::ANGLE;
//    motor.velocity_limit = 5;
    motor.init();
    motor.initFOC(EncoderBase::CW);

    commander_init();

    soft_timer_init(HAL_GetTick);
    soft_timer_create(10, plot_msg, NULL);
    soft_timer_create(200, serial_handler, NULL);

    for (;;) {
        /** Drv8301 error check */
        drvM1.faultHandler();

        motor.loopFOC();
        motor.move(target_velocity);

        soft_timer_handler();
    }
}

void plot_msg(soft_timer_t *timer) {
    DRIVE_LOG("%f,%f,%f,%f",
              motor.target,
              motor.shaft_velocity,
              motor.shaft_angle,
              motor.voltage.q
             );
}

void serial_handler(soft_timer_t *timer) {
    commander.run();
}