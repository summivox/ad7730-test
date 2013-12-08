#ifndef _PINOUT_HPP_
#define _PINOUT_HPP_

#include "misc.hpp"

void pinout_init() AUTORUN;


////////////
// timers
//  TIM2: remap 2'b11
//  TIM4: remap 1'b1

//axis x : TIM4(drive), TIM3(encoder)
#define P_x_PULS        PD12
#define P_x_SIGN        PD13
#define I_x_QEP_A       PA6
#define I_x_QEP_B       PA7

//axis y : TIM8(drive), TIM5(encoder)
#define P_y_PULS        PC6
#define P_y_SIGN        PC7
#define I_y_QEP_A       PA0
#define I_y_QEP_B       PA1

//laser  : TIM2
#define P_LASER         PB10


#endif//_PINOUT_HPP_
