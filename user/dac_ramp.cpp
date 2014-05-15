#include "stdafx.h"
#pragma hdrstop
using namespace std;

#include "dac_ramp.hpp"

#include <stdio.h>

#include "conf.hpp"
#include "pinout.hpp"


bool volatile dac_ramp_running;
uint32_t volatile * dac_ramp_data;

void dac_ramp_init() {
    DAC->CR = DAC_CR_EN1
            | DAC_CR_TSEL1_0 * 7 //3'b111 software trigger
            | DAC_CR_EN2
            | DAC_CR_TSEL2_0 * 7 //3'b111 software trigger             ;
            ;

    //set initial value to 0
    DAC->DHR12R1 = 0;
    DAC->DHR12R2 = 0;
    DAC->SWTRIGR = DAC_SWTRIGR_SWTRIG1
                 | DAC_SWTRIGR_SWTRIG2
                 ;

    dac_ramp_data = &(DAC->DHR12R1);
}

static uint16_t limit;
static int period_Tms;

static OS_TID dac_ramp_tid;
static void __task dac_ramp_task() {
    os_itv_set(period_Tms);
    while (dac_ramp_running && *dac_ramp_data < limit) {
        ++*dac_ramp_data;
        DAC->SWTRIGR = DAC_SWTRIGR_SWTRIG1;
#if DAC_RAMP_PRINT
        printf("%2.3f\r\n", *dac_ramp_data * (reg_FS_Pkpa/4095));
#endif//DAC_RAMP_PRINT
        os_itv_wait();
    }
    dac_ramp_running = false;
    os_tsk_delete_self();
}

void dac_ramp_start(uint16_t start, uint16_t stop, uint32_t Tms) {
    *dac_ramp_data = start;
    limit = stop;
    period_Tms = Tms / (stop - start);
    if (period_Tms < 1) period_Tms = 1;
    dac_ramp_running = true;
    dac_ramp_tid = os_tsk_create(dac_ramp_task, 0x40);
}

void dac_ramp_stop() {
    dac_ramp_running = false;
    os_tsk_delete(dac_ramp_tid);
}
