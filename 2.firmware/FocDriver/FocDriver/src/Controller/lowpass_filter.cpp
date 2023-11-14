//
// Created by Monster on 2023/6/5.
//

#include "lowpass_filter.h"

float LowPassFilter::operator()(float input) {
    auto  timestamp = _micros();
    float dt        = (float) (timestamp - timestamp_last) * 1e-6f;

    if (dt < 0.0f) {
        dt = 1e-3f;
    } else if (dt > 0.3f) {
        output_last    = input;
        timestamp_last = timestamp;
        return input;
    }

    if(Tf == 0) return input;

    float alpha  = Tf / (Tf + dt);
    float output = alpha * output_last + (1.0f - alpha) * input;
    output_last    = output;
    timestamp_last = timestamp;

    return output;
}
