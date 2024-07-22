//
// Created by Monster on 2023/6/2.
//

#include "math_utils.h"
#include "cmath"
#include "stdint.h"

/**
 * Function approximating the sine calculation by using fixed size array
 *  ~40us ( float array )
 *  ~50us ( int array )
 *  it has to receive an angle in between 0 and 2PI
 * @param a - angle in between 0 and 2PI
 * @return
 */
float _sin(float a) {
    static uint16_t sine_array[65] = {
            0, 804, 1608, 2411, 3212, 4011, 4808, 5602, 6393, 7180, 7962, 8740, 9512, 10279, 11039, 11793, 12540, 13279,
            14010, 14733, 15447, 16151, 16846, 17531, 18205, 18868, 19520, 20160, 20788, 21403, 22006, 22595, 23170,
            23732, 24279, 24812, 25330, 25833, 26320, 26791, 27246, 27684, 28106, 28511, 28899, 29269, 29622, 29957,
            30274, 30572, 30853, 31114, 31357, 31581, 31786, 31972, 32138, 32286, 32413, 32522, 32610, 32679, 32729,
            32758, 32768};

    int32_t      t1, t2;
    unsigned int i    = (unsigned int) (a * (64 * 4 * 256.0f / _2PI));
    int          frac = i & 0xff;
    i = (i >> 8) & 0xff;
    if (i < 64) {
        t1 = (int32_t) sine_array[i];
        t2 = (int32_t) sine_array[i + 1];
    } else if (i < 128) {
        t1 = (int32_t) sine_array[128 - i];
        t2 = (int32_t) sine_array[127 - i];
    } else if (i < 192) {
        t1 = -(int32_t) sine_array[-128 + i];
        t2 = -(int32_t) sine_array[-127 + i];
    } else {
        t1 = -(int32_t) sine_array[256 - i];
        t2 = -(int32_t) sine_array[255 - i];
    }
    return (1.0f / 32768.0f) * (t1 + (((t2 - t1) * frac) >> 8));
}

/**
 * Function approximating cosine calculation by using fixed size array
 *  ~55us ( float array )
 *  ~56us ( int array )
 * @param a - angle in between 0 and 2PI
 * @return
 */
float _cos(float a) {
    float a_sin = a + _PI_2;
    a_sin = a_sin > _2PI ? a_sin - _2PI : a_sin;
    return _sin(a_sin);
}

/**
 * normalizing radian angle to [0, 2PI]
 * @param angle - angle to be normalized
 * @return
 */
float _normalizeAngle(float angle) {
    float a = fmod(angle, _2PI);
    return a >= 0 ? a : (a + _2PI);
}

/**
 * Electrical angle calculation
 * @param shaft_angle current angle
 * @param pole_pairs pole pairs
 * @return
 */
float _electricalAngle(float shaft_angle, int pole_pairs) {
    return (shaft_angle * pole_pairs);
}

/**
 *  * Function approximating square root function
 *  - using fast inverse square root
 * @param value - number
 * @return
 */
float _sqrtApprox(float value) {
    long  i;
    float y;
    // float x;
    // const float f = 1.5F; // better precision

    // x = number * 0.5F;
    y = value;
    i = *(long *) &y;
    i = 0x5f375a86 - (i >> 1);
    y = *(float *) &i;
    // y = y * ( f - ( x * y * y ) ); // better precision
    return value * y;
}