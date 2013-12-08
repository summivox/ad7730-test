#ifndef _QEP_HPP_
#define _QEP_HPP_

#include "conf.hpp"


////////////
// position (encoder pulses)
//  updated real-time

extern uint16_t volatile * const qep_x;
extern uint16_t volatile * const qep_y;

void qep_init(void) AUTORUN;
void qep_on(void);
void qep_off(void);


////////////
// velocity (encoder pulses / s)

extern float qep_vel_x;
extern float qep_vel_y;
extern float qep_vel;

void qep_vel_reset(void);
void qep_vel_init(void);
void qep_vel_upd(void);


#endif//_QEP_HPP_
