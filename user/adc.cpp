#include "stdafx.h"
#pragma hdrstop
using namespace std;

#include "adc.hpp"

#include "pinout.hpp"
#include "spi.hpp"
#include "ad7730.hpp"
#include "ad7686.hpp"


//NOTE: AD7686 requires no software setup
void adc_init() {
    //boot-up
    ad7730_reset();
    os_dly_wait(1000);
    ad7730_get();

    //internal full scale calib at 80 mV range (see datasheet)
    ad7730_calib(ad7730_mode_t::INT1);
    ad7730_gain.get();

    //set range and throughput
    ad7730_mode.fields.range = 0; //10 mV
    ad7730_mode.fields.wl = 0; //16 bit
    ad7730_mode.set();

    ad7730_filter.fields.chp = 1; //chopping
    ad7730_filter.fields.sf = ad7730_clk / (16 * 3) / adc_sample_rate;
    ad7730_filter.fields.fast = 1; //fast step response
    ad7730_filter.set();

    //external zero scale calib
    ad7730_calib(ad7730_mode_t::EXT0);
    ad7730_offset.get();
}

static bool adc_on = false;

void adc_start() {
    ad7730_read_start();
    O_AD7686_CNV = 0;
    O_AD7730_nSS = 1;
    adc_on = true;
}
void adc_stop() {
    adc_on = false;
    O_AD7686_CNV = 0;
    O_AD7730_nSS = 1;
    ad7730_read_stop();
}

//synchronize reads from both ADCs, triggered by AD7730 conversion complete
void E_AD7730_nRDY_IRQHandler() {
    if (E_AD7730_nRDY != 0) {
        O_AD7686_CNV = 1;
        return; //BREAKPOINT HERE -- shouldn't really happen
    }
    O_AD7686_CNV = 1; //AD7686 read end (conv start)
    O_AD7730_nSS = 0; //AD7730 read start

    ad7730_data = SPI_send1(SPI_AD7730, (uint16_t)ad7730_blank);

    O_AD7686_CNV = 0; //AD7686 read start
    O_AD7730_nSS = 1; //AD7730 read end

    ad7686_data = SPI_send1(SPI_AD7686, 0);

    adc_sample_handler();
}
