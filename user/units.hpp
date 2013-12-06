#ifndef _UNITS_HPP_
#define _UNITS_HPP_

#include "stdint.h"
#include "arm_math.h"


////////////
// generic ratio-based conversion
// explain:
//  <name>_<dim><unit>: "quantity divided by its unit"
//  <dim><unit1>_<dim><unit2>: "1 <unit1> represented in <unit2>"
// example:
//  x_Lmm == 1000
//  x_Lm  == CONV(x_Lmm, Lmm, Lm)
//        == x_Lmm / (Lm / Lmm)
//        == 1000 / (1000 / 1)

#define CONV(val, unit, new_unit) ((unit > new_unit) ? (val) * (unit / new_unit) : (val) / (new_unit / unit))


////////////
// time

typedef uint32_t units_time;

static const units_time Tclk = 1ul;
static const units_time Tus = 72 * Tclk;
static const units_time Tms = 1000 * Tus;
static const units_time Ts = 1000 * Tms;

static const units_time Tstep = 36000 * Tclk;


////////////
// length

typedef uint32_t units_len;

static const units_len Lmm = 1ul;
static const units_len Lm = 1000 * Lmm;


////////////
// angle

typedef float units_angle;

static const units_angle Arad = 1.0;
static const units_angle Adeg = PI/180.0;


#endif//_UNITS_HPP_
