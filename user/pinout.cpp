#include "stdafx.h"
#pragma hdrstop
using namespace std;

#include "pinout.hpp"

void pinout_init(){

    //switch on all GPIOs
    PA::enable();
    PB::enable();
    PC::enable();
    PD::enable();
    PE::enable();
    PF::enable();
    PG::enable();

#define I(pin) pin.conf(GPIO_IN_FLOATING)
#define O(pin) pin.conf(GPIO_OUT_PP)
#define OD(pin) pin.conf(GPIO_OUT_OD)
#define P(pin) pin.conf(GPIO_AF_PP)
#define PD(pin) pin.conf(GPIO_AF_OD)


    ////////////
    // timers
    //  TIM2: remap 2'b11
    //  TIM4: remap 1'b1

    AFIO->MAPR = (AFIO->MAPR & ~AFIO_MAPR_TIM2_REMAP) | (AFIO_MAPR_TIM2_REMAP_0*3);
    AFIO->MAPR |= AFIO_MAPR_TIM4_REMAP;

    //axis x : TIM4(drive), TIM3(encoder)
    P(P_x_PULS);
    P(P_x_SIGN);
    I(I_x_QEP_A);
    I(I_x_QEP_B);

    //clock : TIM2
    P(P_CLOCK);


    ////////////
    // comm
    //  SPI2

    //AD7730 : SPI2
    P(P_SPI2_SCK);
    I(I_SPI2_MISO);
    P(P_SPI2_MOSI);


#undef I
#undef O
#undef OD
#undef P
#undef PD
}
