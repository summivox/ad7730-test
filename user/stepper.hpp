#ifndef _STEPPER_HPP_
#define _STEPPER_HPP_

#include "conf.hpp"
#include "units.hpp"


extern bool volatile stepper_running;
extern int8_t stepper_dir;
extern int32_t volatile stepper_pos_Lpulse;
extern int32_t stepper_limit_lo_Lpulse;
extern int32_t stepper_limit_hi_Lpulse;

void stepper_init() AUTORUN;
void stepper_run(float Vmil, bool blind = false); //blind: do not keep track of current position
void stepper_stop();
void stepper_force_stop();

void stepper_home(float coarse_Vmil, float fine_Vmil);


#endif//_STEPPER_HPP_
