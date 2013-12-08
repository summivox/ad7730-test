#ifndef _PARSER_HPP_
#define _PARSER_HPP_

#include <iostream>

#include "generator.h"

#include "seg.hpp"


//generates linear velocity curve `v1` -> `vm` -> `v3`
//  acceleration: `a` (constant)
//  path length: `s`
$generator(parser) {
    istream& in;

    float x_curr, y_curr;
    float x_start, y_start; //sub-path starting point

    char cmd;
    bool is_rel;

    float x, y, rx, ry, phi;
    int flag1, flag2;

    //TODO: cx, cy, ... 

    explicit parser(istream& in, float x, float y)
                   :in(in), 
                    x_curr(x), y_curr(y),
                    x_start(x), y_start(y) {};
    $emit_decl(Seg*);
};

#endif//_PARSER_HPP_
