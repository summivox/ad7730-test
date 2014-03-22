#include "stdafx.h"
#pragma hdrstop
using namespace std;

#include "ad7730.hpp"

#include "pinout.hpp"


////////////
// clock output (temporary -- should use crystal)

static void clock_init() AUTORUN;
static void clock_init() {
    RCC_ENR(APB1, TIM2EN) = 1;
    RCC_RSTR(APB1, TIM2RST) = 1;
    RCC_RSTR(APB1, TIM2RST) = 0;

    TIM2->PSC = 0;
    TIM2->ARR = 15 - 1; //4.8 MHz (should have been 4.9152 MHz)

    TIM2->CCMR2 = TIM_CCMR2_OC3PE | (TIM_CCMR2_OC3M_0 * 6);
    TIM2->CCR3 = TIM2->ARR / 2;

    TIM2->EGR = TIM_EGR_UG;
    TIM2->SR = ~TIM_SR_UIF;

    TIM2->CCER = TIM_CCER_CC3E;
    TIM2->CR1 |= TIM_CR1_CEN;
}


////////////
// simple SPI wrappers

#define SPI_WAIT_UNTIL(SPI, flag, val) while (!!(SPI->SR & SPI_SR_##flag) != val);
static uint8_t SPI_send1(SPI_TypeDef* SPI, uint8_t tx) {
    SPI->DR = tx;
    SPI_WAIT_UNTIL(SPI, RXNE, 1);
    return SPI->DR;
}
static uint16_t SPI_send2(SPI_TypeDef* SPI, uint16_t tx) {
    uint16_t rx = 0;

    SPI->DR = tx >> 8;

    SPI_WAIT_UNTIL(SPI, TXE, 1);
    SPI->DR = tx & 0xffu;
    SPI_WAIT_UNTIL(SPI, RXNE, 1);
    rx = SPI->DR << 8;

    SPI_WAIT_UNTIL(SPI, RXNE, 1);
    rx |= SPI->DR;

    return rx;
}
static uint32_t SPI_send3(SPI_TypeDef* SPI, uint32_t tx) {
    uint32_t rx = 0;

    SPI->DR = (tx >> 16) & 0xffu;

    SPI_WAIT_UNTIL(SPI, TXE, 1);
    SPI->DR = (tx >> 8) & 0xffu;
    SPI_WAIT_UNTIL(SPI, RXNE, 1);
    rx = (rx << 8) | SPI->DR;
    SPI_WAIT_UNTIL(SPI, TXE, 1);
    SPI->DR = (tx >> 0) & 0xffu;
    SPI_WAIT_UNTIL(SPI, RXNE, 1);
    rx = (rx << 8) | SPI->DR;

    SPI_WAIT_UNTIL(SPI, RXNE, 1);
    rx = (rx << 8) | SPI->DR;

    return rx;
}
static uint32_t SPI_send4(SPI_TypeDef* SPI, uint32_t tx) {
    uint32_t rx = 0;

    SPI->DR = (tx >> 24) & 0xffu;

    SPI_WAIT_UNTIL(SPI, TXE, 1);
    SPI->DR = (tx >> 16) & 0xffu;
    SPI_WAIT_UNTIL(SPI, RXNE, 1);
    rx = (rx << 8) | SPI->DR;
    SPI_WAIT_UNTIL(SPI, TXE, 1);
    SPI->DR = (tx >> 8) & 0xffu;
    SPI_WAIT_UNTIL(SPI, RXNE, 1);
    rx = (rx << 8) | SPI->DR;
    SPI_WAIT_UNTIL(SPI, TXE, 1);
    SPI->DR = (tx >> 0) & 0xffu;
    SPI_WAIT_UNTIL(SPI, RXNE, 1);
    rx = (rx << 8) | SPI->DR;

    SPI_WAIT_UNTIL(SPI, RXNE, 1);
    rx = (rx << 8) | SPI->DR;

    return rx;
}
static void SPI_send(SPI_TypeDef* SPI, uint8_t* tx, uint8_t* rx, size_t n) {
    SPI->DR = *tx++;
    while (--n > 0) {
        SPI_WAIT_UNTIL(SPI, TXE, 1);
        SPI->DR = *tx++;
        SPI_WAIT_UNTIL(SPI, RXNE, 1);
        *rx++ = SPI->DR;
    }
    SPI_WAIT_UNTIL(SPI, RXNE, 1);
    *rx++ = SPI->DR;
}


////////////
// init

void ad7730_init() {
    RCC_ENR(APB1, SPI2EN) = 1;
    RCC_RSTR(APB1, SPI2RST) = 1;
    RCC_RSTR(APB1, SPI2RST) = 0;

    SPI2->CR1 =
        SPI_CR1_CPHA    * 1 | //trailing edge
        SPI_CR1_CPOL    * 0 | //idle low, active high
        SPI_CR1_MSTR    * 1 |
        SPI_CR1_BR_0    * 3 | //36 MHz / 2^4 = 2.25 MHz
        SPI_CR1_SPE     * 1 |
        SPI_CR1_SSI     * 1 |
        SPI_CR1_SSM     * 1 ;
}


////////////
// registers

enum ad7730_magic {
    BLANK = 0x80808080u, //placeholder sent when reading
    WRITE = 0x00u, //write register
    READ  = 0x10u, //read register
    START = 0x20u, //enter continuous reading mode
    STOP  = 0x30u, //exit continuous reading mode
};
enum ad7730_reg {
    //lower-case for easy macro
    reg_status = 0,
    reg_data = 1,
    reg_mode = 2,
    reg_filter = 3,
    reg_dac = 4,
    reg_offset = 5,
    reg_gain = 6,
    reg_test = 7,
};


uint32_t ad7730_data;

#define DEF_REG(name, SPI_sendx, uintx_t)            \
    ad7730_##name##_t ad7730_##name;                 \
    void ad7730_##name##_t::get() {                  \
        SPI_send1(SPI2, READ | reg_##name);          \
        this->all = SPI_sendx(SPI2, (uintx_t)BLANK); \
    }                                                \
    void ad7730_##name##_t::set() {                  \
        SPI_send1(SPI2, WRITE | reg_##name);         \
        SPI_sendx(SPI2, this->all);                  \
    }

DEF_REG(status , SPI_send1, uint8_t);
DEF_REG(mode   , SPI_send2, uint16_t);
DEF_REG(filter , SPI_send3, uint32_t);
DEF_REG(dac    , SPI_send1, uint8_t);
DEF_REG(offset , SPI_send3, uint32_t);
DEF_REG(gain   , SPI_send3, uint32_t);
DEF_REG(test   , SPI_send3, uint32_t);

#undef DEF_REG


////////////
// blocking ops

void ad7730_reset() {
    SPI_send4(SPI2, 0xffffffffu);
}

void ad7730_get() {
    ad7730_status.get();
    ad7730_mode.get();
    ad7730_filter.get();
    ad7730_dac.get();
    ad7730_offset.get();
    ad7730_gain.get();
    ad7730_test.get();
}

void ad7730_calib(ad7730_mode_t::md_t md) {
    ad7730_mode.get();
    ad7730_mode.fields.md = md;
    ad7730_mode.set();
    while (E_AD7730_nRDY == 1) ;
    ad7730_mode.get();
    ad7730_status.get();
}

uint32_t ad7730_read() {
    SPI_send1(SPI2, READ | reg_data);
    if (ad7730_mode.fields.wl == 1) {
        return ad7730_data = SPI_send3(SPI2, (uint32_t)BLANK); //24 bit
    } else {
        return ad7730_data = SPI_send2(SPI2, (uint16_t)BLANK); //16 bit
    }
}


////////////
// continuous transfer

static bool cont = false;

void ad7730_read_start() {
    //reset mode to continuous conversion
    ad7730_mode.get();
    ad7730_mode.fields.md = ad7730_mode_t::IDLE;
    ad7730_mode.set();
    for (volatile int _ = 10 ; _ --> 0 ; ) ;
    ad7730_mode.fields.md = ad7730_mode_t::CONT;
    ad7730_mode.set();

    //setup continuous read
    SPI_send1(SPI2, START | reg_data);

    //wait for data
    cont = true;
    E_AD7730_nRDY_EXTI::clear();
    E_AD7730_nRDY_EXTI::enable();
}
void ad7730_read_stop() {
    //stop accepting data
    E_AD7730_nRDY_EXTI::disable();
    E_AD7730_nRDY_EXTI::clear();
    cont = false;

    //stop continuous read
    SPI_WAIT_UNTIL(SPI2, TXE, 1);
    SPI_send1(SPI2, STOP);
}
void E_AD7730_nRDY_IRQHandler() {
    if (!cont) return;
    //parallel to : ad7730_read()
    if (ad7730_mode.fields.wl == 1) {
        ad7730_data = SPI_send3(SPI2, (uint32_t)BLANK); //24 bit
    } else {
        ad7730_data = SPI_send2(SPI2, (uint16_t)BLANK); //16 bit
    }
}
