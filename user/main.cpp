#include "stdafx.h"
#pragma hdrstop
using namespace std;

#include <iostream>
#include <sstream>
#include <string>

#include "conf.hpp"
#include "pinout.hpp"
#include "misc.hpp"
#include "math.hpp"

#include "drive.hpp"
#include "ad7730.hpp"


__task void main_task(){
    printf(
        "\r\n\r\n"
        "### ad7730-test %s %s\r\n"
        "\r\n",
        __DATE__, __TIME__
    );
    os_dly_wait(1000);

    ad7730_reset();
    os_dly_wait(1000);
    ad7730_get();

    printf("\r\n");
    
    //power-on register check
    printf("<<< status: 0x%02X\r\n", ad7730_status.all);
    printf("<<< mode:   0x%04X\r\n", ad7730_mode  .all);
    printf("<<< filter: 0x%06X\r\n", ad7730_filter.all);

    printf("\r\n");

    //internal full scale calib at 80 mV range (see datasheet)
    printf("### calib: internal full..."); fflush(stdout);
    ad7730_calib(ad7730_mode_t::INT1);
    printf("done.\r\n");
    ad7730_gain.get();
    printf("           gain  : 0x%06X\r\n", ad7730_gain.all);

    printf("\r\n");

    //set range and throughput
    ad7730_mode.fields.range = 0; //10 mV
    ad7730_mode.fields.wl = 0; //16 bit
    ad7730_mode.set();

    ad7730_filter.fields.sf = 100; // 4.8 MHz / (16 * 3 * 100) = 1 kSPS
    ad7730_filter.set();

    //check registers
    printf("<<< status: 0x%02X\r\n", ad7730_status.all);
    printf("<<< mode:   0x%04X\r\n", ad7730_mode  .all);
    printf("<<< filter: 0x%06X\r\n", ad7730_filter.all);

    printf("\r\n");

    //external zero scale calib
    printf("### calib: external zero..."); fflush(stdout);
    ad7730_calib(ad7730_mode_t::EXT0);
    printf("done.\r\n");
    ad7730_offset.get();
    printf("           offset: 0x%06X\r\n", ad7730_offset.all);

    printf("\r\n");

    os_dly_wait(1000);

    while (1) {
        printf("\r\n### 100ms * 100 :\r\n");

        ad7730_read_start();
        os_itv_set(50);
        for (int n = 100 ; n --> 0 ; ) {
            /*
            if (E_AD7730_nRDY == 0) {
                uint32_t x = ad7730_read();
                printf("%06X\r\n", x);
            }
            ad7730_status.get();
            */
            printf("[%3d] %06X\r\n", n, ad7730_data);
            os_itv_wait();
        }
        ad7730_read_stop();
        os_dly_wait(200);
    }
}

int main(){
    os_sys_init_prio(main_task, 0x80);
}
