#ifndef _CONF_HPP_
#define _CONF_HPP_

#include "misc.hpp"
#include "units.hpp"


////////////
// buffer size

static const int N_drive_cmd_max = 20; //drive command


////////////
// axis output

static const int N_axis_count = 2;
static const U32 min_pulse_width_Tclk = 50; //minimal pulse width
static const U32 min_period_Tclk = 300; //min period (in ticks) <=> max pulse frequency

#define FOR_EACH_AXIS(N_axis) for (int N_axis = N_axis_count ; N_axis --> 0 ; )


////////////
// machine-specific

static const int Lmm_Lpulse = 1000;

static const float V_feed_start = 5;
static const float V_feed_max   = 50;
static const float V_move_max   = 100;
static const float Acc_line_max = 100;
static const float Acc_arc_max  = 100;


#endif//_CONF_HPP_
