#include "stdafx.h"
#pragma hdrstop
using namespace std;

#include <iostream>
#include <sstream>
#include <string>

#include "conf.hpp"
#include "misc.hpp"
#include "math.hpp"

#include "drive.hpp"


void clock_init() {
    RCC_ENR(APB1, TIM2EN) = 1;
    RCC_RSTR(APB1, TIM2RST) = 1;
    RCC_RSTR(APB1, TIM2RST) = 0;

    TIM2->PSC = 0;
    TIM2->ARR = 15 - 1; //4.8 MHz (should have been 4.9152 MHz)

    TIM2->CCMR1 = TIM_CCMR1_OC31E | (TIM_CCMR1_OC1M_0 * 6);
    TIM2->CCR1 = TIM2->ARR / 2;

    TIM2->EGR = TIM_EGR_UG;
    TIM2->SR = ~TIM_SR_UIF;
    TIM2->CR1 |= TIM_CR1_CEN;
}


__task void main_task(){
    printf(
        "\r\n\r\n"
        "### ad7730-test %s %s\r\n"
        "\r\n",
        __DATE__, __TIME__
    );
    drive_start();
    clock_init();
    os_dly_wait(100);

    string cmd;
    while (1) {
        cin >> cmd;
        if (0) {
        } else if (cmd == "path") {
        } else if (cmd == "feed") {
        }
    }
}

int main(){
    os_sys_init_prio(main_task, 0x80);
}
