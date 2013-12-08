#ifndef _LASER_HPP_
#define _LASER_HPP_

#include "conf.hpp"


extern bool laser_is_on;
extern float laser_tone; //[0, 1]

void laser_init() AUTORUN;

void laser_set(bool state);
inline void laser_on(void) { laser_set(true); }
inline void laser_off(void) { laser_set(false); }

void laser_set_tone(float t);


//CONV(N_drive_cmd_max, Tstep, Tms)

#endif//_LASER_HPP_
