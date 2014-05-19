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
    return (1+force_FS_out_rel*rel) * 0x8000;
}

static void home() {
    printf("### moving probe to home...\r\n");
    stepper_home(Vmil_nominal, Vmil_nominal / 4);
    printf("    done\r\n");
}

static void touchdown() {
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
    }
}

static uint16_t gage_LRV_adc = 0x3139; //20140518-1038
static uint16_t gage_URV_adc = 0xf63d;
static const size_t gage_calib_n = adc_sample_rate * 1;

template <typename T>
static void adc_dump_n(T* src, T* buf, size_t n) {
    os_evt_clr(1, os_tsk_self());
    T* end = buf + n;
    while (buf != end) {
        os_evt_wait_or(1, FOREVER);
        os_evt_clr(1, os_tsk_self());
        *buf++ = *src;
    }
}

static void calib() {
    uint32_t* buf = new uint32_t[gage_calib_n];
    uint32_t sum;

    printf("### set AO to  4mA (LRV) and press enter..."); fflush(stdout);
    fgetc(stdin);
    adc_dump_n(&ad7686_data, buf, gage_calib_n);
    sum = 0;
    for (int i = 0 ; i < gage_calib_n ; ++i) sum += buf[i];
    gage_LRV_adc = uint16_t(sum / gage_calib_n);
    printf("<<< LRV ADC value : 0x%04x\r\n\r\n", gage_LRV_adc);

    printf("### set AO to 20mA (URV) and press enter..."); fflush(stdout);
    fgetc(stdin);
    adc_dump_n(&ad7686_data, buf, gage_calib_n);
    sum = 0;
    for (int i = 0 ; i < gage_calib_n ; ++i) sum += buf[i];
    gage_URV_adc = uint16_t(sum / gage_calib_n);
    printf("<<< URV ADC value : 0x%04x\r\n\r\n", gage_URV_adc);

    printf("### done\r\n\r\n");

    delete buf;
}

static void run() {
    /*
    printf("??? ramp time (s) : "); fflush(stdout);
    float ramp_Ts; cin >> ramp_Ts;
    printf("\r\n");

    printf("??? max pressure (kPa) : "); fflush(stdout);
    float max_Pkpa; cin >> max_Pkpa;
    printf("\r\n");
    */

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
    
    printf(
        "\r\n"
        "### test end\r\n"
    );
}

static OS_TID main_tid;
static __task void main_task(){
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
    adc_start();
    printf("    done\r\n");

    
    printf("### CLI\r\n");

    string cmd;
    while (1) {
        cin >> cmd;
        if (0) {
        } else if (cmd[0] == 'h') {
            home();
        } else if (cmd[0] == 't') {
            touchdown();
        } else if (cmd[0] == 'c') {
            calib();
        } else if (cmd[0] == 'a') {
            printf("### adc test\r\n");
            os_evt_clr(1, os_tsk_self());
            for (int i = 0 ; i < adc_sample_rate ; ++i) {
                os_evt_wait_or(1, FOREVER);
                os_evt_clr(1, os_tsk_self());
                printf("%04x,%04x\r\n", ad7730_data, ad7686_data);
                if (ad7730_data > 0x9000 || ad7730_data < 0x7000) {
                    O_D(0, 0, 0, 1);
                    adc_stop();
                    printf("!!! ad7730 corrupt!\r\n");
                    while (1) /*dead*/;
                }
            }
            printf("    done\r\n");
        } else if (cmd[0] == 'r') {
            run();
        } else {
            printf(
                "### commands:\r\n"
                "    h(ome), t(ouchdown), c(alib), a(dc), r(un), e(nd)\r\n"
            );
        }
    }

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
    os_evt_set(1, main_tid);
}


int main(){
    os_sys_init_prio(main_task, 0x80);
}
