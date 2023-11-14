//
// Created by Monster on 2023/6/2.
//

#ifndef _TIME_UTILS_H
#define _TIME_UTILS_H


/**
 * Get microseconds in the system.
 * hardware specific
 */
unsigned int _micros();

 /**
  * Delay in milliseconds.
  * @param ms Delay milliseconds.
  */
void _delay(unsigned int ms);

/**
 * Delay in microsecond.
 * @param ms Delay microsecond.
 */
void _delayUs(unsigned int us);


#endif //_TIME_UTILS_H
