#ifndef _MATH_HPP_
#define _MATH_HPP_


#include "math.h"
#include "arm_math.h"

static const float TWO_PI = 2*PI;
static const float HALF_PI = .5*PI;

inline float round(float x) {
    return floor(x + .5);
}

inline float wrap_angle(float a) {
    if      (a > +PI) a -= TWO_PI;
    else if (a < -PI) a += TWO_PI;
    return a;
}

#endif//_MATH_HPP_
