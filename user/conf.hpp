#ifndef _CONF_HPP_
#define _CONF_HPP_

#include "misc.hpp"
#include "units.hpp"


////////////
// debug flags


////////////
// buffer size

static const int N_drive_cmd_max = 20; //drive command


////////////
// axis output

static const int N_axis_count = 1;
static const U32 min_pulse_width_Tclk = 50; //minimal pulse width
static const U32 min_period_Tclk = 300; //min period (in ticks) <=> max pulse frequency

#define FOR_EACH_AXIS(N_axis) for (int N_axis = N_axis_count ; N_axis --> 0 ; )


////////////
// machine-specific & physical

static const int Lmm_Lpulse = 128;

//QEP velocity:
static const float qep_vel_period_Ts = CONV(2000.0, Tus, Ts);
static const float qep_vel_tau_Ts = CONV(2.0, Tms, Ts); //smoothing time constant
static const int qep_vel_zero_Npulse = 0.1*Lmm_Lpulse; //"near-zero" thershold


#endif//_CONF_HPP_
