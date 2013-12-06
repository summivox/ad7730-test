#include "stdafx.h"
#pragma hdrstop
using namespace std;

#include "vel_gen.hpp"

#include "units.hpp"


//generate linear velocity curve `v1` -> `vm` -> `v3` with path length `s`
//return value: `n` < `n_max`
//`ret[0, n)`: path length curve, sampled at integer `(Tstep/Tclk)`
int vel_gen(float a, float vm, float v1, float v3, float s, float* ret, int n_max) {
    //validity
    //assert(a  >  0);
    //assert(s  >  0);
    //assert(v1 >= 0);
    //assert(v3 >= 0);

    //pre-compute
    float half_a = a*.5;
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
        vm = sqrt(v_lim_v_lim);
    }
    if (vm < v1) v1 = vm;
    if (vm < v3) v3 = vm;
    float vm_a = sqrt((v1*v1 + v3*v3)/2.0 + a*s);
    if (vm_a < vm) vm = vm_a;
    if (vm < v1) v1 = vm;
    if (vm < v3) v3 = vm;

    //attack(1), sustain(2), release(3); all aligned to time step
    float dt1 = (vm - v1)/a; //attack/release time
    float dt3 = (vm - v3)/a;
    int n1 = floor(dt1/(Tstep/Tclk)); //err on shorter accel time
    int n3 = floor(dt3/(Tstep/Tclk));
    float dt1i = n1*(Tstep/Tclk); //aligned attack/release time
    float dt3i = n3*(Tstep/Tclk);
    float s1 = (v1 + half_a*dt1i)*dt1i; //length of attack/release
    float s3 = (v3 + half_a*dt3i)*dt3i;
    float s2 = s - s1 - s3; //length of sustain
    float dt2 = s2/vm;
    int n2 = ceil(dt2/(Tstep/Tclk)); //err on slower speed (than max)

    //check for overrun
    if (n1 + n2 + n3 > n_max) return 0;

    //generate attack [n1]
    for (int i = 1 ; i <= n1 ; ++i) {
        float t = i*(Tstep/Tclk);
        *ret++ = (v1 + half_a*t)*t;
    }
    //generate sustain [n2 - 1]
    float step2 = s2/n2;
    for (int i = 1 ; i < n2 ; ++i) {
        *ret++ = s1 + step2 * i;
    }
    //generate release [n3]
    for (int i = n3 ; i >= 1 ; --i) {
        float t = i*(Tstep/Tclk);
        *ret++ = s - (v3 + half_a*t)*t;
    }
    *ret++ = s;

    return n1 + n2 + n3;
}
