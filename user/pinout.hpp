#ifndef _PINOUT_HPP_
#define _PINOUT_HPP_

#include "misc.hpp"

void pinout_init() AUTORUN;


////////////
// debug indicators

#define O_D0 PE7
#define O_D1 PE8
#define O_D2 PE9
#define O_D3 PE10

#define O_D(d0, d1, d2, d3) do {    \
    O_D0 = d0;                      \
    O_D1 = d1;                      \
    O_D2 = d2;                      \
    O_D3 = d3;                      \
} while (0)


////////////
// digital I/O

#define O_VALVE PC5 //valve: 1 => pressure, 0 => atmosphere


////////////
// DAC
//  Ch1 : pneumatic regulator

#define A_DAC1 PA4


////////////
// timers
//  TIM4 : remap 1'b1

//stepper : TIM4
#define P_PULS        PD12
#define O_SIGN        PD13

//home sensor (active low, pullup)
#define E_nHOME       PD14
#define E_nHOME_PORT  'D'
#define E_nHOME_EXTI  Exti<14>


////////////
// comm
//  SPI1
//  SPI2
//  USART2 (CTS/RTS)
//  USART3

//AD7686 : SPI1
#define SPI_AD7686 SPI1

#define O_AD7686_CNV PC4

#define P_SPI1_SCK PA5
#define I_SPI1_MISO PA6
#define P_SPI1_MOSI PA7

//AD7730 : SPI2
#define SPI_AD7730 SPI2

#define O_AD7730_nSS PB12

#define P_SPI2_SCK PB13
#define I_SPI2_MISO PB14
#define P_SPI2_MOSI PB15

#define E_AD7730_nRDY PD8
#define E_AD7730_nRDY_PORT 'D'
#define E_AD7730_nRDY_EXTI Exti<8>

//HART modem : USART2
#define USART_HART USART2

#define I_USART2_nCTS PA0
#define P_USART2_nRTS PA1
#define P_USART2_TX PA2
#define I_USART2_RX PA3

//binary streaming : USART3
#define USART_BIN USART3

#define P_USART3_TX PB10
#define I_USART3_RX PB11


#endif//_PINOUT_HPP_
