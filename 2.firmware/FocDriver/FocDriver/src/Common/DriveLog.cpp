//
// Created by Monster on 2023/6/6.
//

#include "DriveLog.h"

#if DRIVE_LOG_ENABLE

#include "usart.h"

uint8_t debug[64] = {0x03, 0xFC};

void DriveLog::println(const char *msg) {
    printf("%s\n", msg);
}

void DriveLog::sendProtocol(const float *data, uint16_t size) {
    uint8_t u8[4];
    int     count = 0;

    uint16_t number     = size;
    while (number--) {
        floatToU8array(u8, *(data + count));
        debug[2 + (count * 4) + 0] = u8[0];
        debug[2 + (count * 4) + 1] = u8[1];
        debug[2 + (count * 4) + 2] = u8[2];
        debug[2 + (count * 4) + 3] = u8[3];
        count++;
    }
    debug[size * 4 + 2] = 0xFC;
    debug[size * 4 + 3] = 0x03;

//    printf("%.*s\n", (size * 4 + 4), debug);
    HAL_UART_Transmit(&huart2, debug, size * 4 + 4, 1000);
}

void DriveLog::floatToU8array(uint8_t *u8array, float data) {
    uint8_t farray[4];
    *(float *) farray = data;

    u8array[0] = farray[0];
    u8array[1] = farray[1];
    u8array[2] = farray[2];
    u8array[3] = farray[3];
}


#endif