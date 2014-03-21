#include "stdafx.h"
#pragma hdrstop
using namespace std;

#include "ad7730.hpp"


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
        SPI_CR1_BR_0    * 6 | //36 MHz / 2^7 = 281.25 kHz
        SPI_CR1_SPE     * 1 |
        SPI_CR1_SSI     * 1 |
        SPI_CR1_SSM     * 1 ;
}


////////////
// magic numbers

enum ad7730_magic {
    BLANK = 0x80808080u, //placeholder sent when reading
    WRITE = 0x00u, //write register
    READ  = 0x10u, //read register
    START = 0x20u, //enter continuous reading mode
    STOP  = 0x30u, //exit continuous reading mode
};
enum ad7730_reg {
    REG_STATUS = 0,
    REG_DATA = 1,
    REG_MODE = 2,
    REG_FILTER = 3,
    REG_DAC = 4,
    REG_OFFSET = 5,
    REG_GAIN = 6,
    REG_TEST = 7,
};


////////////
// ops

void ad7730_reset() {
    SPI_send4(SPI2, 0xffffffffu);
}

uint8_t ad7730_get_status() {
    SPI_send1(SPI2, READ | REG_STATUS);
    return SPI_send1(SPI2, BLANK);
}

void ad7730_set_offset(uint32_t offset) {
    SPI_send1(SPI2, WRITE | REG_OFFSET);
    SPI_send3(SPI2, offset);
}
uint32_t ad7730_get_offset() {
    SPI_send1(SPI2, READ | REG_OFFSET);
    return SPI_send3(SPI2, BLANK);
}
