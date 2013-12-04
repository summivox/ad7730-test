//pinout
#ifndef _PINOUT_HPP_
#define _PINOUT_HPP_

#include "misc.hpp"

void pinout_init() AUTORUN;




////axis pins

//axis[0] - TIM2

#define I_0_ALM         PC3
#define I_0_N_OT        PF4
#define I_0_P_OT        PF3
#define I_0_nCOIN       PC0
#define I_0_nS_RDY      PC2
#define I_0_nTGON       PC1
#define O_0_CLR         PF0
#define O_0_nALM_RST    PF5
#define O_0_nP_CON      PF2
#define O_0_nS_ON       PF1
#define P_0_PULS        PA0
#define P_0_SIGN        PA1

#define I_0_ALM_PORT    'C'
#define I_0_ALM_PIN     3
#define I_0_ALM_Exti    Exti<3>

#define I_0_ALM_IRQn        EXTI3_IRQn
#define I_0_ALM_IRQHandler  EXTI3_IRQHandler

//axis[1] - TIM3

#define I_1_ALM         PE8
#define I_1_N_OT        PF12
#define I_1_P_OT        PF11
#define I_1_nCOIN       PG0
#define I_1_nS_RDY      PE7
#define I_1_nTGON       PG1
#define O_1_CLR         PC4
#define O_1_nALM_RST    PF15
#define O_1_nP_CON      PB0
#define O_1_nS_ON       PC5
#define P_1_PULS        PA6
#define P_1_SIGN        PA7

#define I_1_ALM_PORT    'E'
#define I_1_ALM_PIN     8
#define I_1_ALM_Exti    Exti<8>

#define I_1_ALM_IRQn        EXTI9_5_IRQn
#define I_1_ALM_IRQHandler  EXTI9_5_IRQHandler

//axis[2] - TIM4

#define I_2_ALM         PD7 //PD4
#define I_2_N_OT        PG14 //PG11
#define I_2_P_OT        PG13 //PG12
#define I_2_nCOIN       PD4 //PD7
#define I_2_nS_RDY      PD6
#define I_2_nTGON       PD5
#define O_2_CLR         PG10 //PG15
#define O_2_nALM_RST    PG15 //PG10
#define O_2_nP_CON      PG12 //PG13
#define O_2_nS_ON       PG11 //PG14
#define P_2_PULS        PB6
#define P_2_SIGN        PB7

#define I_2_ALM_PORT    'D'
#define I_2_ALM_PIN     7
#define I_2_ALM_Exti    Exti<7>

#define I_2_ALM_IRQn        EXTI9_5_IRQn
#define I_2_ALM_IRQHandler  EXTI9_5_IRQHandler

//axis[3] - TIM8

#define I_3_ALM         PD15 //PD12
#define I_3_N_OT        PG7 //PG4
#define I_3_P_OT        PG6 //PG5
#define I_3_nCOIN       PD12 //PD15
#define I_3_nS_RDY      PD14
#define I_3_nTGON       PD13
#define O_3_CLR         PG3 //PG8
#define O_3_nALM_RST    PG8 //PG3
#define O_3_nP_CON      PG5 //PG6
#define O_3_nS_ON       PG4 //PG7
#define P_3_PULS        PC6
#define P_3_SIGN        PC7

#define I_3_ALM_PORT    'D'
#define I_3_ALM_PIN     15
#define I_3_ALM_Exti    Exti<15>

#define I_3_ALM_IRQn        EXTI15_10_IRQn
#define I_3_ALM_IRQHandler  EXTI15_10_IRQHandler




////external DAC(SPI) pins

#define P_DAC_SCK		PB13
#define P_DAC_MOSI		PB15
#define O_DAC_nSS		PB12

#define DAC_SPI			SPI2
#define DAC_EN			RCC_ENR(APB1, SPI2EN)
#define DAC_RST			RCC_RSTR(APB1, SPI2RST)

#endif//_PINOUT_HPP_
