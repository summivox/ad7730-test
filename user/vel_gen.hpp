#ifndef _VELGEN_HPP_
#define _VELGEN_HPP_

#include "units.hpp"
#include "generator.h"


//generates linear velocity curve `v1` -> `v2` -> `v3`
//  acceleration: `a` (constant)
//  path length: `s`
$generator(vel_gen) {
    int i;
    float v1, v3, half_a, s, s1, step2;
    int n1, n2, n3;

    explicit vel_gen(float a, float v1, float v2, float v3, float s);
    $emit_decl(float);
};

#endif//_VELGEN_HPP_
