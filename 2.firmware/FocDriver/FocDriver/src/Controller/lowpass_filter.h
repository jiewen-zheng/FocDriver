//
// Created by Monster on 2023/6/5.
//

#ifndef _LOWPASS_FILTER_H
#define _LOWPASS_FILTER_H

#include "src/Common/time_utils.h"

class LowPassFilter {
public:
    /**
     * @param Tf - Low pass filter time constant
     */
    explicit LowPassFilter(float Tf_) :
            Tf(Tf_) {
        output_last    = 0;
        timestamp_last = _micros();
    }

    ~LowPassFilter() = default;

    float operator()(float input);

public:
    float Tf;   //!< Low pass filter time constant

protected:
    unsigned long timestamp_last;   //!< Last execution timestamp
    float         output_last;      //!< filtered value in previous execution step
};

#endif //_LOWPASS_FILTER_H
