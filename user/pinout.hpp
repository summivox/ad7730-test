#ifndef _PINOUT_HPP_
#define _PINOUT_HPP_

#include "misc.hpp"

void pinout_init() AUTORUN;


////////////
// timers
//  TIM4: remap 1'b1

//axis x : TIM4(drive), TIM3(encoder)
#define P_x_PULS        PD12
#define P_x_SIGN        PD13
#define I_x_QEP_A       PA6
#define I_x_QEP_B       PA7

//clock : TIM2
#define P_CLOCK         PA0


#endif//_PINOUT_HPP_
