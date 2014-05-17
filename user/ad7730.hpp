#ifndef _AD7730_HPP_
#define _AD7730_HPP_

#include "conf.hpp"

static const uint32_t ad7730_blank = 0x80808080u; //placeholder sent when reading

extern uint32_t ad7730_data;


////////////
// registers
//  `get()`/`set()` : transfer register from/to AD7730

union ad7730_status_t {
    uint8_t all;
    __packed struct {
        uint8_t _3_0    : 4; //[ 3: 0] reserved
        uint8_t noref   : 1; //[ 4: 4] no reference voltage
        uint8_t stby    : 1; //[ 5: 5] standby
        uint8_t n_stdy  : 1; //[ 6: 6] 0 => filter steady
        uint8_t n_rdy   : 1; //[ 7: 7] 0 => data ready
    } fields;

    void get();
    void set();
};
extern ad7730_status_t ad7730_status;

union ad7730_mode_t {
    uint16_t all;
    __packed struct {
        uint16_t ch         : 2; //[ 1: 0] input channel
        uint16_t bo         : 1; //[ 2: 2] burnout current enable
        uint16_t clkdis     : 1; //[ 3: 3] master clock disable
        uint16_t range      : 2; //[ 5: 4] input range (10 mV * 2^range)
        uint16_t _6         : 1; //[ 6: 6] reserved
        uint16_t hiref      : 1; //[ 7: 7] reference : 0 => 2.5 V ; 1 => 5 V
        uint16_t wl         : 1; //[ 8: 8] word length : 0 => 16 bit ; 1 => 24 bit
        uint16_t d          : 2; //[10: 9] digital output
        uint16_t den        : 1; //[11:11] digital output enable
        uint16_t b_u        : 1; //[12:12] 0 => bipolar ; 1 => unipolar
        uint16_t md         : 3; //[15:13] mode
    } fields;

    enum ch_t {
        P1_N1 = 0,
        P2_N2 = 1,
        N1_N1 = 2,
        N1_N2 = 3,
    };
    enum md_t {
        IDLE = 0, //sync/idle
        CONT = 1, //continuous conversion
        SING = 2, //single conversion
        STBY = 3, //standby
        INT0 = 4, //internal calibrate zero
        INT1 = 5, //internal calibrate full
        EXT0 = 6, //external calibrate zero
        EXT1 = 7, //external calibrate full
    };

    void get();
    void set();
};
extern ad7730_mode_t ad7730_mode;

union ad7730_filter_t {
    uint32_t all;
    __packed struct {
        uint32_t dl         : 4; //[ 3: 0] delay selection
        uint32_t chp        : 1; //[ 4: 4] chop enable
        uint32_t ac         : 1; //[ 5: 5] excitation: 0 => DC ; 1 => AC
        uint32_t _7_6       : 2; //[ 7: 6] reserved
        uint32_t fast       : 1; //[ 8: 8] FASTStep enable
        uint32_t skip       : 1; //[ 9: 9] FIR filter skip
        uint32_t _11_10     : 2; //[11:10] reserved
        uint32_t sf         : 12;//[23:12] sinc^3 filter
    } fields;

    void get();
    void set();
};
extern ad7730_filter_t ad7730_filter;

#define TRIVIAL_REG(uintx_t, reg)        \
    union ad7730_##reg##_t {             \
        uintx_t all;                     \
        void get();                      \
        void set();                      \
    };                                   \
    extern ad7730_##reg##_t ad7730_##reg

TRIVIAL_REG(uint8_t , dac);
TRIVIAL_REG(uint32_t, offset);
TRIVIAL_REG(uint32_t, gain);
TRIVIAL_REG(uint32_t, test);

#undef TRIVIAL_REG


void ad7730_init() AUTORUN;

////////////
// blocking ops

void ad7730_reset();
void ad7730_get();
void ad7730_calib(ad7730_mode_t::md_t md);


////////////
// data transfer

uint32_t ad7730_read_oneshot();

void ad7730_read_start();
//uint32_t ad7730_read_cont();
void ad7730_read_stop();



#endif//_AD7730_HPP_
