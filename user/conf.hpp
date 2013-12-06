#ifndef _CONF_HPP_
#define _CONF_HPP_

#include "misc.hpp"
#include "units.hpp"


////////////
// rasterization

static const int Lmm_Lpulse = 128;


////////////
// buffer size

static const int N_drive_cmd_max = 20; //drive command


////////////
// axis

static const int N_axis_count = 2;
static const U32 min_pulse_width_Tclk = 50; //minimal pulse width
static const U32 min_period_Tclk = 300; //min period (in ticks) <=> max pulse frequency

#define FOR_EACH_AXIS(N_axis) for (int N_axis = N_axis_count ; N_axis --> 0 ; )


#endif//_CONF_HPP_
