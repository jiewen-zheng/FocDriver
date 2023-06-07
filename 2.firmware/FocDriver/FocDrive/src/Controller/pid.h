//
// Created by Monster on 2023/6/2.
//

#ifndef _PID_H
#define _PID_H

#include "../Common/time_utils.h"
#include "../Common/math_utils.h"

class PIDController {
public:
    /**
     *
     * @param P - Proportional gain
     * @param I - Integral gain
     * @param D - Derivative gain
     * @param ramp - Maximum speed of change of the output value
     * @param limit - Maximum output value
     */
    PIDController(float P, float I, float D, float ramp, float limit)
            : _P(P), _I(I), _D(D), _out_ramp(ramp), _limit(limit) {
        error_last = 0;
        output_last = 0;
        integral_last = 0;
        timestamp_last = _micros();
    }

    ~PIDController() = default;

    /**
     * @brief 函数调用运算符（）重载
     * @param error
     * @return
     */
    float operator()(float error);


    float _P = 0;   //!< Proportional gain
    float _I = 0;   //!< Integral gain
    float _D = 0;   //!<  Derivative gain
    float _out_ramp = 0;    //!< Maximum speed of change of the output value
    float _limit = 0;       //!< Maximum output value

protected:
    float error_last;   //!< last tracking error value
    float output_last;  //!< last pid output value
    float integral_last;//!< last integral component value
    unsigned long timestamp_last = 0;   //!< Last execution timestamp
};


#endif //_PID_H
