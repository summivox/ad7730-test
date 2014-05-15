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

#if ADC_ENABLED
    printf("### initializing ADCs..."); fflush(stdout);
    adc_init();
    printf("done\r\n");
#endif//ADC_ENABLED

#if STEPPER_ENABLED
    printf("### moving probe to home..."); fflush(stdout);
    float Vmil_nominal = 0.05 * force_FS_Lmil * adc_sample_rate;
    stepper_home(Vmil_nominal, Vmil_nominal / 4);
    printf("done\r\n");

    //TODO: touchdown sequence
#endif//STEPPER_ENABLED

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
    dac_ramp_start(0, (max_Pkpa/reg_FS_Pkpa)*4095, ramp_Ts*1000);
#if ADC_ENABLED
    adc_start();
#endif//ADC_ENABLED

    while (dac_ramp_running) {
        os_evt_wait_or(1, FOREVER);
#if ADC_ENABLED
        if (ad7730_data <= (1 - force_FS_digital) * 0x8000) {
            break;
        }
#endif//ADC_ENABLED
        printf("%04x,%04x\r\n", ad7730_data, ad7686_data);
    }

    O_VALVE = 0; //shut off pressure first!
    dac_ramp_stop();
    *dac_ramp_data = 0;
#if ADC_ENABLED
    adc_stop();
#endif//ADC_ENABLED

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
