//
// Created by Monster on 2023/6/6.
//

#ifndef _DRIVELOG_H
#define _DRIVELOG_H

#include <cstdio>

#define DRIVE_LOG_ENABLE 1

class DriveLog {

};

#if DRIVE_LOG_ENABLE

#define DRIVE_LOG(format, ...) \
    printf(format "\r\n", ##__VA_ARGS__)

#else   //DRIVE_LOG_ENABLE

#define DRIVE_LOG(format, ...)

#endif  //DRIVE_LOG_ENABLE

#endif //_DRIVELOG_H
