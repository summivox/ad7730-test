#ifndef _CONF_HPP_
#define _CONF_HPP_

#include "misc.hpp"
#include "units.hpp"


////////////
// debug flags

#define ADC_ENABLED true
#define STEPPER_ENABLED true
#define DAC_RAMP_PRINT true


////////////
// buffer size


////////////
// pneumatic regulator

static const float reg_min_Pkpa = 1;
static const float reg_FS_Pkpa = 33;


////////////
// 4-20 mA pressure transducer

static const float gage_LRV_Pkpa = 0;
static const float gage_URV_Pkpa = 20;


////////////
// stepper driver

static const uint32_t pulse_width_min_Tclk = 500;
static const uint32_t period_min_Tclk = 800; //min period (in ticks) <=> max pulse frequency
static const uint32_t period_max_Tclk = 1<<24; //max period (in ticks)

static const int Lpulse_Lstep = 1<<6; //stepper driver microsteps
static const int Lstep_Lrev = 200;
static const int Lmil_Lrev = 12;
static const float Lpulse_Lmil = 1. * Lpulse_Lstep * Lstep_Lrev / Lmil_Lrev;

static const int stepper_speed_max_Vmil = 60;

static const int stepper_travel_Lmil = 300;
static const int32_t stepper_travel_Lpulse = stepper_travel_Lmil * Lpulse_Lmil;

//home switch marks: -1 => lo, +1 => hi
static const int stepper_home_dir = +1;


////////////
// ADCs

static const int adc_sample_rate = 100;
static const int ad7730_clk = 4915200;


////////////
// force sensor (Futek LSB200)

static const float force_FS_out_abs = 0.6781; // (mV/V)
static const float force_FS_out_rel = force_FS_out_abs * 5 / 10; // (mV/V) * (V) / (mV)
static const float force_FS_Lmil = 4; //deflection at FS

// all force thresholds relative to force sensor FS
// + => pull (tension)
// - => push (compress)
static const float force_touchdown_thres = -0.03;
static const float force_bailout_thres = -0.8;


////////////
// other derived params

static const float Vmil_nominal = 0.02 * force_FS_Lmil * adc_sample_rate; //reference speed for stepper


#endif//_CONF_HPP_
