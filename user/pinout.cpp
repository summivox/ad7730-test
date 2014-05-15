#include "stdafx.h"
#pragma hdrstop
using namespace std;

#include "pinout.hpp"


////////////
// interrupt hooks

#define HOOK(name)                     \
    if (name##_EXTI::pending()) {      \
        void name##_IRQHandler();      \
        name##_IRQHandler();           \
        name##_EXTI::clear();          \
    }

IRQ_DECL(EXTI9_5_IRQn, EXTI9_5_IRQHandler, 3, 1);
void EXTI9_5_IRQHandler() {
    HOOK(E_AD7730_nRDY);
}

IRQ_DECL(EXTI15_10_IRQn, EXTI15_10_IRQHandler, 2, 1);
void EXTI15_10_IRQHandler() {
    HOOK(E_nHOME);
}

#undef HOOK


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
    //PG::enable();

#define I(pin) pin.conf(GPIO_IN_FLOATING)
#define ID(pin, idle) do {pin = idle; pin.conf(GPIO_IN_PULL);} while (0)
#define O(pin) pin.conf(GPIO_OUT_PP)
#define OD(pin) pin.conf(GPIO_OUT_OD)
#define P(pin) pin.conf(GPIO_AF_PP)
#define PD(pin) pin.conf(GPIO_AF_OD)


    ////////////
    // JTAG: SWD only (3'b010)
    // FIXME: silicon bug?

    AFIO->MAPR = (AFIO->MAPR & (~AFIO_MAPR_SWJ_CFG)) | (AFIO_MAPR_SWJ_CFG_0 * 1);
    //AFIO->MAPR = (AFIO->MAPR & (~AFIO_MAPR_SWJ_CFG)) | (AFIO_MAPR_SWJ_CFG_0 * 2);
    if (AFIO->MAPR & AFIO_MAPR_SWJ_CFG != (AFIO_MAPR_SWJ_CFG_0 * 2)) {
        while (1);
    }


    ////////////
    // debug indicators
    O(O_D0);
    O(O_D1);
    O(O_D2);
    O(O_D3);
    O_D(0, 0, 0, 0);


    ////////////
    // digital I/O

    O(O_VALVE); O_VALVE = 0;


    ////////////
    // DAC
    //  Ch1 : pneumatic regulator

    RCC_ENR(APB1, DACEN) = 1;
    RCC_RSTR(APB1, DACRST) = 1;
    RCC_RSTR(APB1, DACRST) = 0;

    A_DAC1.conf(GPIO_ANALOG);
    A_DAC2.conf(GPIO_ANALOG);


    ////////////
    // external interrupts
    NVIC_EnableIRQ(EXTI9_5_IRQn);
    NVIC_EnableIRQ(EXTI15_10_IRQn);

    ////////////
    // timers
    //  TIM4: remap 1'b1

    AFIO->MAPR |= AFIO_MAPR_TIM4_REMAP;

    //stepper : TIM4
    P(P_PULS);
    O(O_SIGN);

    ID(E_nHOME, 1);
    E_nHOME_EXTI::set_port(E_nHOME_PORT);


    ////////////
    // comm
    //  SPI1
    //  SPI2
    //  USART2 (CTS/RTS)
    //  USART3

    //AD7686 : SPI1
    RCC_ENR(APB2, SPI1EN) = 1;
    RCC_RSTR(APB2, SPI1RST) = 1;
    RCC_RSTR(APB2, SPI1RST) = 0;

    O(O_AD7686_CNV); O_AD7686_CNV = 1;

    P(P_SPI1_SCK);
    I(I_SPI1_MISO);
    //P(P_SPI1_MOSI);

    //AD7730 : SPI2
    RCC_ENR(APB1, SPI2EN) = 1;
    RCC_RSTR(APB1, SPI2RST) = 1;
    RCC_RSTR(APB1, SPI2RST) = 0;

    O(O_AD7730_nSS); O_AD7730_nSS = 1;

    P(P_SPI2_SCK);
    I(I_SPI2_MISO);
    P(P_SPI2_MOSI);

    //ID(E_AD7730_nRDY, 1);
    I(E_AD7730_nRDY);
    E_AD7730_nRDY_EXTI::set_port(E_AD7730_nRDY_PORT);
    E_AD7730_nRDY_EXTI::set_falling();

    //HART modem : USART2
    I(I_USART2_nCTS);
    P(P_USART2_nRTS);
    P(P_USART2_TX);
    I(I_USART2_RX);

    //binary streaming : USART3
    P(P_USART3_TX);
    I(I_USART3_RX);


#undef I
#undef ID
#undef O
#undef OD
#undef P
#undef PD
}
