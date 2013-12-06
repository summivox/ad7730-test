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
    //	RCC_ENR(APB2, IOPAEN)=1;
    //	RCC_ENR(APB2, IOPBEN)=1;
    //	RCC_ENR(APB2, IOPCEN)=1;
    //	RCC_ENR(APB2, IOPDEN)=1;
    //	RCC_ENR(APB2, IOPEEN)=1;
    //	RCC_ENR(APB2, IOPFEN)=1;
    //	RCC_ENR(APB2, IOPGEN)=1;

#define E(pin, type, edge) do{ \
    pin##_Exti::enable(); \
    pin##_Exti::set_##edge(); \
    pin.set_exti(); \
}while(false)

#define I(pin) pin.conf(GPIO_IN_FLOATING)
#define O(pin) pin.conf(GPIO_OUT_PP)
#define OD(pin) pin.conf(GPIO_OUT_OD)
#define P(pin) pin.conf(GPIO_AF_PP)
#define PD(pin) pin.conf(GPIO_AF_OD)

    //configure each group of pins, including alarm external interrupt
    //NOTE: ISR is NOT configured
#define INIT_GROUP(n) \
    E(I_##n##_ALM, GPIO_IN_FLOATING, rising); \
    I(I_##n##_N_OT); \
    I(I_##n##_P_OT); \
    I(I_##n##_nCOIN); \
    I(I_##n##_nS_RDY); \
    I(I_##n##_nTGON); \
    O_##n##_CLR     =   0;  O(O_##n##_CLR); \
    O_##n##_nALM_RST=   1;  O(O_##n##_nALM_RST); \
    O_##n##_nP_CON  =   1;  O(O_##n##_nP_CON); \
    O_##n##_nS_ON   =   1;  O(O_##n##_nS_ON); \
    P(P_##n##_PULS); \
    P(P_##n##_SIGN); \

    INIT_GROUP(0);
    INIT_GROUP(1);
    //INIT_GROUP(2);
    //INIT_GROUP(3);

#undef INIT_GROUP

    //configure external DAC pins
    P(P_DAC_SCK);
    P(P_DAC_MOSI);
    O_DAC_nSS		=	1;	O(O_DAC_nSS);

#undef PD
#undef P
#undef OD
#undef O
#undef I
#undef E
}
