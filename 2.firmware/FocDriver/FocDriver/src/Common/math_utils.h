//
// Created by Monster on 2023/6/2.
//

#ifndef __MATH_UTILS_H_
#define __MATH_UTILS_H_


// sign function
#define _sign(a) ( ( (a) < 0 )  ?  -1   : ( (a) > 0 ) )
#ifndef _round
#define _round(x) ((x)>=0?(long)((x)+0.5f):(long)((x)-0.5f))
#endif
#define _constrain(amt, low, high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
#define _sqrt(a) (_sqrtApprox(a))
#define _isset(a) ( (a) != (NOT_SET) )
#define _UNUSED(v) (void) (v)

// utility defines
#define _2_SQRT3 1.15470053838f
#define _SQRT3 1.73205080757f
#define _1_SQRT3 0.57735026919f
#define _SQRT3_2 0.86602540378f
#define _SQRT2 1.41421356237f
#define _120_D2R 2.09439510239f
#define _PI 3.14159265359f
#define _PI_2 1.57079632679f
#define _PI_3 1.0471975512f
#define _2PI 6.28318530718f
#define _3PI_2 4.71238898038f
#define _PI_6 0.52359877559f
#define _RPM_TO_RADS 0.10471975512f

#define NOT_SET (-12345.0)
#define _HIGH_IMPEDANCE 0
#define _HIGH_Z _HIGH_IMPEDANCE
#define _ACTIVE 1
#define _NC (NOT_SET)

#define MIN_ANGLE_DETECT_MOVEMENT (_2PI/101.0f)

// dq current structure
struct DQCurrent_t {
    float d;
    float q;
};
// dq voltage structs
struct DQVoltage_t {
    float d;
    float q;
};
// phase current structure
struct PhaseCurrent_t {
    float a;
    float b;
    float c;
};


/**
 * function approximating the sine calculation by using fixed size array
 */
float _sin(float a);

/**
 * function approximating cosine calculation by using fixed size array
 */
float _cos(float a);

/** normalizing radian angle to [0,2PI] */
float _normalizeAngle(float angle);

/** Electrical angle calculation */
float _electricalAngle(float shaft_angle, int pole_pairs);

/** square root approximation function using */
float _sqrtApprox(float value);

#endif //_MATH_UTILS_H
