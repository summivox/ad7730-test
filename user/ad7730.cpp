#include "stdafx.h"
#pragma hdrstop
using namespace std;

#include "ad7730.hpp"

#include "pinout.hpp"
#include "spi.hpp"


////////////
// init

enum SPI_CONF {
    SPI_CONF_BASE =
        SPI_CR1_CPHA    * 1 | //trailing edge
        SPI_CR1_CPOL    * 0 | //idle low, active high
        SPI_CR1_MSTR    * 1 |
        SPI_CR1_BR_0    * 4 | //36 MHz / 2^(4+1) = 1.125 MHz
        SPI_CR1_SPE     * 1 |
        SPI_CR1_SSI     * 1 |
        SPI_CR1_SSM     * 1 ,
    SPI_CONF_8bit =
        SPI_CONF_BASE       |
        SPI_CR1_DFF     * 0 ,
    SPI_CONF_16bit =
        SPI_CONF_BASE       |
        SPI_CR1_DFF     * 1 ,
};

void ad7730_init() {
    SPI_AD7730->CR1 = SPI_CONF_8bit;
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
        O_AD7730_nSS = 0;                            \
        SPI_send1(SPI2, READ | reg_##name);          \
        this->all = SPI_sendx(SPI2, (uintx_t)BLANK); \
        O_AD7730_nSS = 1;                            \
    }                                                \
    void ad7730_##name##_t::set() {                  \
        O_AD7730_nSS = 0;                            \
        SPI_send1(SPI2, WRITE | reg_##name);         \
        SPI_sendx(SPI2, this->all);                  \
        O_AD7730_nSS = 1;                            \
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
    SPI_AD7730->CR1 = 0;
    SPI_AD7730->CR1 = SPI_CONF_8bit;
    O_AD7730_nSS = 0;
    SPI_send4(SPI2, 0xffffffffu);
    O_AD7730_nSS = 1;
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


////////////
// data transfer

uint32_t ad7730_read_oneshot() {
    O_AD7730_nSS = 0;
    SPI_send1(SPI2, READ | reg_data);
    if (ad7730_mode.fields.wl == 1) {
        ad7730_data = SPI_send3(SPI2, (uint32_t)BLANK); //24 bit
    } else {
        ad7730_data = SPI_send2(SPI2, (uint16_t)BLANK); //16 bit
    }
    O_AD7730_nSS = 1;
    return ad7730_data;
}

//static bool cont = false;
void ad7730_read_start() {
    //reset mode to continuous conversion
    ad7730_mode.get();
    ad7730_mode.fields.md = ad7730_mode_t::IDLE;
    ad7730_mode.set();
    for (volatile int _ = 10 ; _ --> 0 ; ) ;
    ad7730_mode.fields.md = ad7730_mode_t::CONT;
    ad7730_mode.set();

    //setup continuous read
    O_AD7730_nSS = 0;
    SPI_send1(SPI2, START | reg_data);
    O_AD7730_nSS = 1;

    SPI_AD7730->CR1 = 0;
    SPI_AD7730->CR1 = SPI_CONF_16bit;

    //wait for data
    //cont = true;
    E_AD7730_nRDY_EXTI::clear();
    E_AD7730_nRDY_EXTI::enable();
}
/*
uint32_t ad7730_read_cont() {
    if (!cont) return 0xffffffffu;
    //parallel to : ad7730_read_oneshot()
    O_AD7730_nSS = 0;
    if (ad7730_mode.fields.wl == 1) {
        ad7730_data = SPI_send3(SPI2, (uint32_t)BLANK); //24 bit
    } else {
        ad7730_data = SPI_send2(SPI2, (uint16_t)BLANK); //16 bit
    }
    O_AD7730_nSS = 1;
    return ad7730_data;
}
*/
void ad7730_read_stop() {
    //stop accepting data
    E_AD7730_nRDY_EXTI::disable();
    E_AD7730_nRDY_EXTI::clear();
    //cont = false;

    //stop continuous read
    SPI_AD7730->CR1 = 0;
    SPI_AD7730->CR1 = SPI_CONF_8bit;

    O_AD7730_nSS = 0;
    SPI_send1(SPI2, STOP);
    O_AD7730_nSS = 1;
}
