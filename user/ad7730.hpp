#ifndef _AD7730_HPP_
#define _AD7730_HPP_

#include "conf.hpp"


////////////
// bitfields

__packed struct ad7730_status {
    uint8_t _reserved_3_0   : 4;
    uint8_t noref           : 1;
    uint8_t stby            : 1;
    uint8_t stdy            : 1;
    uint8_t rdy             : 1;
};
__packed struct ad7730_mode {
    enum channel {
        P1_N1 = 0,
        P2_N2 = 1,
        N1_N1 = 2,
        N1_N2 = 3,
    };
    uint16_t ch             : 2;

    uint16_t bo             : 1;
    uint16_t clkdis         : 1;
    uint16_t range          : 2;
    uint16_t _reserved_6    : 1;
    uint16_t hiref          : 1;
    uint16_t wl             : 1;
    uint16_t d              : 2;
    uint16_t den            : 1;
    uint16_t b_u            : 1;

    enum mode {
        IDLE = 0,
        CONT = 1,
        SING = 2,
        STBY = 3,
        INT0 = 4,
        INT1 = 5,
        EXT0 = 6,
        EXT1 = 7,
    };
    uint16_t md             : 3;

    void init() {
        *(uint16_t*)(this) = 0x01B0;
    }
};


void ad7730_init() AUTORUN;

void ad7730_reset();
uint8_t ad7730_get_status();
void ad7730_set_offset(uint32_t offset);
uint32_t ad7730_get_offset();



#endif//_AD7730_HPP_
