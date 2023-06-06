//
// Created by Monster on 2023/6/2.
//

#ifndef _TIME_UTILS_H
#define _TIME_UTILS_H

#include <stdint-gcc.h>

/**
 * Function implementing delay() function in milliseconds
 * - blocking function
 * - hardware specific

 * @param ms - number of milliseconds to wait
 */
void _delay(unsigned long ms);

/**
 * Function implementing timestamp getting function in microseconds
 * hardware specific
 */
unsigned long _micros();

#endif //_TIME_UTILS_H
