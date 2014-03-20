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
    //  TIM4: remap 1'b1

    AFIO->MAPR |= AFIO_MAPR_TIM4_REMAP;

    //axis x : TIM4(drive), TIM3(encoder)
    P(P_x_PULS);
    P(P_x_SIGN);
    I(I_x_QEP_A);
    I(I_x_QEP_B);

    //clock : TIM2
    P(P_CLOCK);

#undef I
#undef O
#undef OD
#undef P
#undef PD
}
