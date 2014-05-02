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
//#define P_CLOCK         PB10


////////////
// comm
//  SPI2

//AD7730 : SPI2
#define O_SPI2_nSS PB12
#define P_SPI2_SCK PB13
#define I_SPI2_MISO PB14
#define P_SPI2_MOSI PB15

#define E_AD7730_nRDY PD8
#define E_AD7730_nRDY_PORT 'D'
#define E_AD7730_nRDY_EXTI Exti<8>


#endif//_PINOUT_HPP_
