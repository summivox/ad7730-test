#ifndef _CONF_HPP_
#define _CONF_HPP_

#include "misc.hpp"
#include "units.hpp"


////////////
// debug flags

#define QEP_MOCKUP //use drive pulse counter as QEP

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
// machine-specific & physical

static const int Lmm_Lpulse = 512;

static const float V_feed_start = 5;
static const float V_feed_max   = 50;
static const float V_move_max   = 100;
static const float Acc_line_max = 200;
static const float Acc_arc_max  = 100;
static const float Acc_move_max = 400;

//near-tangent handling: v = v0 * (1 - (theta/theta0)^2), clipped to vmin
static const float V_feed_tan   = 25;
static const float feed_tan_cutoff = CONV(15, Adeg, Arad);

//laser:
//  duty% = (tone% * (vel/vel0)) ^ gamma
//  factor = 1/vel0 : larger => more output
//  gamma correction : larger => more output
static const int laser_period_Tus = 2000;
static const float laser_tone_factor = 1/(V_feed_max * 0.7);
static const float laser_tone_gamma  = 1/1.3;

//QEP velocity:
static const float qep_vel_period_Ts = CONV(float(laser_period_Tus), Tus, Ts); //implemented on same timer
static const float qep_vel_tau_Ts = CONV(2.0, Tms, Ts); //smoothing time constant
static const int qep_vel_zero_Npulse = 0.1*Lmm_Lpulse; //"near-zero" thershold


#endif//_CONF_HPP_
