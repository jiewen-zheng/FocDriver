//
// Created by Monster on 2023/6/2.
//

#include "pid.h"

float PIDController::operator()(float error) {
    // calculate the time from the last call
    auto  time = _micros();
    float Ts   = (float) (time - timestamp_last) * 1e-6f;
    // quick fix for strange cases (micros overflow)
    if (Ts <= 0 || Ts > 0.5f) Ts = 1e-3f;

    // proportional part
    // u_p  = P *e(k)
    float pTerm = P * error;
    // Tustin transform of the integral part
    // u_ik = u_ik_1  + I*Ts/2*(ek + ek_1)
    float iTerm = integral_last + I * Ts * 0.5f * (error + error_last);
    // antiwindup - limit the output
    iTerm = _constrain(iTerm, -limit, limit);
    // Discrete derivation
    // u_dk = D(ek - ek_1)/Ts
    float dTerm = D * (error - error_last) / Ts;

    // sum all the components
    float output = pTerm + iTerm + dTerm;
    // antiwindup - limit the output variable
    output         = _constrain(output, -limit, limit);

    if (out_ramp > 0) {
        // limit the acceleration by ramping the output
        float output_rate = (output - output_last) / Ts;
        if (output_rate > out_ramp)
            output = output_last + out_ramp * Ts;
        else if (output < -out_ramp)
            output = output_last - out_ramp * Ts;
    }
    // saving for the next pass
    integral_last  = iTerm;
    output_last    = output;
    error_last     = error;
    timestamp_last = time;
    return output;
}

void PIDController::reset() {
    integral_last = 0;
    output_last   = 0;
    error_last    = 0;
}


