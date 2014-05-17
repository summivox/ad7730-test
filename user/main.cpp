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
#include "dac_ramp.hpp"


#include "spi.hpp"


//ADC value from relative force
static inline uint32_t ad7730_from_rel(float rel) {
    return (1+force_FS_digital*rel) * 0x8000;
}


OS_TID main_tid;
__task void main_task(){
    main_tid = os_tsk_self();
    printf(
        "\r\n\r\n"
        "### e-chuck-ctrl\t%s %s\r\n"
        "\r\n",
        __DATE__, __TIME__
    );
    os_dly_wait(2000);

    printf("### initializing ADCs..."); fflush(stdout);
    adc_init();
    printf("done\r\n");

    printf("### moving probe to home...\r\n");
    stepper_home(Vmil_nominal, Vmil_nominal / 4);
    printf("    done\r\n");

    printf("### press enter to start touchdown\r\n");
    fgetc(stdin);

    adc_start();

    printf("### touchdown... (thres=%04x, limit=%d)\r\n",
           ad7730_from_rel(force_touchdown_thres),
           stepper_limit_lo_Lpulse);

    stepper_run(-Vmil_nominal / 2);
    while (stepper_running) {
        os_evt_wait_or(1, FOREVER);
        os_evt_clr(1, os_tsk_self());
        if (ad7730_data <= ad7730_from_rel(force_touchdown_thres)) {
            break;
        }
        printf("%04x\r\n", ad7730_data);
    }
    if (stepper_running) {
        stepper_stop();
        printf("    done\r\n");
    } else {
        printf("    error: hit limit\r\n");
        goto END;
    }

    printf(
        "\r\n"
        "### ready for testing\r\n\r\n"
    );

    printf("??? ramp time (s) : "); fflush(stdout);
    float ramp_Ts; cin >> ramp_Ts;
    printf("\r\n");

    printf("??? max pressure (kPa) : "); fflush(stdout);
    float max_Pkpa; cin >> max_Pkpa;
    printf("\r\n");

    printf(
        "### test start\r\n\r\n"
        "ad7730_data, ad7686_data\r\n"
    );

    O_VALVE = 1;
    /*
    dac_ramp_start
        ( (reg_min_Pkpa/reg_FS_Pkpa)*4095
        , (max_Pkpa/reg_FS_Pkpa)*4095
        , ramp_Ts*1000
        );
    */

    while (1/*dac_ramp_running*/) {
        os_evt_wait_or(1, FOREVER);
        os_evt_clr(1, os_tsk_self());
        if (ad7730_data <= ad7730_from_rel(force_bailout_thres)) {
            break;
        }
        printf("%04x,%04x\r\n", ad7730_data, ad7686_data);
    }

    O_VALVE = 0; //shut off pressure first!
    /*
    dac_ramp_stop();
    *dac_ramp_data = 0;
    */

    adc_stop();

    printf(
        "\r\n\r\n"
        "### END\r\n"
    );

END:
    while (1) {
        os_dly_wait(1);
    }
}

void adc_sample_handler() {
    isr_evt_set(1, main_tid);
}


int main(){
    os_sys_init_prio(main_task, 0x80);
}
