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

#include "stepper.hpp"
#include "adc.hpp"


void adc_sample_handler() {
    //TODO
}

__task void main_task(){
    printf(
        "\r\n\r\n"
        "### e-chuck-ctrl\t%s %s\r\n"
        "\r\n",
        __DATE__, __TIME__
    );
    os_dly_wait(1000);

    printf("### AFIO->MAPR == 0x%08X\r\n", AFIO->MAPR);
    
    printf("### stepper_home\r\n");
    stepper_home(5, 3);

    /*
    adc_init();

    printf("\r\n");
    printf("<<< AD7730 registers:");
    printf("    status: 0x%02X\r\n", ad7730_status.all);
    printf("    mode  : 0x%04X\r\n", ad7730_mode  .all);
    printf("    filter: 0x%06X\r\n", ad7730_filter.all);
    printf("    gain  : 0x%06X\r\n", ad7730_gain  .all);
    printf("    offset: 0x%06X\r\n", ad7730_offset.all);
    printf("\r\n");

    os_dly_wait(1000);
    */

    while (1) {
    }
}

int main(){
    os_sys_init_prio(main_task, 0x80);
}
