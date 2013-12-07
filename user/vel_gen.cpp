#include "stdafx.h"
#pragma hdrstop
using namespace std;

#include "vel_gen.hpp"

vel_gen::vel_gen(float a, float v1, float v2, float v3, float s) : v1(v1), v3(v3), s(s) {
    //validity
    //assert(a  >  0);
    //assert(s  >  0);
    //assert(v1 >= 0);
    //assert(v3 >= 0);

    //pre-compute
    /*float*/ half_a = a*.5;
    float a_2 = a*2.0;
    float a_s_2 = a_2*s;

    float v_large, v_small;
    if (v1 > v3) {
        v_large = v1; v_small = v3;
    } else {
        v_large = v3; v_small = v1;
    }

    //check & adjust for too short path
    float v_lim_v_lim = a_s_2 + v_small*v_small;
    if (v_large*v_large > v_lim_v_lim) {
        v2 = sqrt(v_lim_v_lim);
    }
    if (v2 < v1) v1 = v2;
    if (v2 < v3) v3 = v2;
    float vm = sqrt((v1*v1 + v3*v3)/2.0 + a*s);
    if (vm < v2) v2 = vm;
    if (v2 < v1) v1 = v2;
    if (v2 < v3) v3 = v2;

    //attack(1), sustain(2), release(3); all aligned to time step
    float dt1 = (v2 - v1)/a; //attack/release time
    float dt3 = (v2 - v3)/a;
    /*int*/ n1 = floor(CONV(dt1, Ts, Tstep)); //err on shorter accel time
    /*int*/ n3 = floor(CONV(dt3, Ts, Tstep));
    float dt1i = CONV(float(n1), Tstep, Ts); //aligned attack/release time
    float dt3i = CONV(float(n3), Tstep, Ts);
    /*float*/ s1 = (v1 + half_a*dt1i)*dt1i; //length of attack/release
    float s3 = (v3 + half_a*dt3i)*dt3i;
    float s2 = s - s1 - s3; //length of sustain
    float dt2 = s2/v2;
    /*int*/ n2 = ceil(CONV(dt2, Ts, Tstep)); //err on slower speed (than max)
    /*float*/ step2 = s2/n2;
}

$emit_impl(vel_gen, float) {
    float t; //scratch var -- needn't save

    //generate attack [n1]
    for (/*int*/ i = 1 ; i <= n1 ; ++i) {
        /*float*/ t = CONV(float(i), Tstep, Ts);
        $yield((v1 + half_a*t)*t);
    }
    //generate sustain [n2 - 1]
    for (/*int*/ i = 1 ; i < n2 ; ++i) {
        $yield(s1 + step2 * i);
    }
    //generate release [n3]
    for (/*int*/ i = n3 ; i >= 1 ; --i) {
        /*float*/ t = CONV(float(i), Tstep, Ts);
        $yield(s - (v3 + half_a*t)*t);
    }
    $yield(s);
} $stop;
