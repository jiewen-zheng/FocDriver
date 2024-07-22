//
// Created by moon on 2023/9/27.
//

#include "utils/soft_timer/soft_timer.h"
#include "stdio.h"

uint32_t HAL_GetTick() {
    static unsigned int time = 0;

    return time++;
}

void timerCallback(SoftTimerID id, void *data) {
    if (id->name == "timer1") {
        printf("timer1\r\n");
        int *user_data = (int *) data;
        *user_data = 10;
    } else if (id->name == "timer2") {
        printf("timer2\r\n");
        printf("user data is: %d\r\n", *(int *)data);
    }

}

void Main() {

    sTimerInitialize(HAL_GetTick);

    int data = 0;
    sTimerCreate("timer1", 500, timerCallback, &data, CirculateMode);
    sTimerCreate("timer2", 600, timerCallback, &data, OnceMode);

    sTimerStart("timer1");
    sTimerStart("timer2");

    while (true) {
        sTimerProcess();
    }
}
