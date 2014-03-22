#include "stdafx.h"
#pragma hdrstop
using namespace std;

#include "pinout.hpp"


////////////
// interrupt hooks

#define HOOK_START if (0) {}
#define HOOK(name)                     \
    else if (name##_EXTI::pending()) { \
        void name##_IRQHandler();      \
        name##_IRQHandler();           \
        name##_EXTI::clear();          \
    }

IRQ_DECL(EXTI9_5_IRQn, EXTI9_5_IRQHandler, 3, 1);
void EXTI9_5_IRQHandler() {
    HOOK_START
        HOOK(E_AD7730_nRDY)
}


////////////
// init

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
#define ID(pin, idle) do {pin = idle; pin.conf(GPIO_IN_PULL);} while (0)
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
    O(O_SPI2_nSS); O_SPI2_nSS = 0;
    P(P_SPI2_SCK);
    I(I_SPI2_MISO);
    P(P_SPI2_MOSI);

    //ID(E_AD7730_nRDY, 1);
    I(E_AD7730_nRDY);
    E_AD7730_nRDY_EXTI::set_port(E_AD7730_nRDY_PORT);
    E_AD7730_nRDY_EXTI::set_falling();

    NVIC_EnableIRQ(EXTI9_5_IRQn);


#undef I
#undef ID
#undef O
#undef OD
#undef P
#undef PD
}

#undef HOOK
