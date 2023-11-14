//
// Created by Monster on 2023/6/6.
//

#ifndef DRIVE_LOG_H
#define DRIVE_LOG_H

#include "stdint.h"
#include "stdio.h"

#define DRIVE_LOG_ENABLE 1

#if DRIVE_LOG_ENABLE

class DriveLog {
public:
    static void println(const char *msg);
    static void sendProtocol(const float *data, uint16_t size);

    float u8arrayToFloat(uint8_t *data);
    static void floatToU8array(uint8_t *u8array, float data);

};

#define DRIVE_LOG(format, ...)  printf(format"\r\n", ##__VA_ARGS__)

#else   //! DRIVE_LOG_ENABLE
#define DRIVE_LOG(format, ...)
#endif  //! DRIVE_LOG_ENABLE

#endif //DRIVE_LOG_H
