#include "stdafx.h"
#pragma hdrstop
using namespace std;

#include "qep.hpp"

#include "conf.hpp"


////////////
// position (encoder pulses)
//  updated real-time

#ifndef QEP_MOCKUP
extern uint16_t volatile * const qep_x = &(TIM5->CNT);
extern uint16_t volatile * const qep_y = &(TIM3->CNT);
#endif

static void qep_init_1(TIM_TypeDef* TIMX) {
    TIMX->ARR = 0xffff; //full range
    TIMX->SMCR = (TIM_SMCR_SMS_0 * 3); //SMS: 3'b011 QEP mode
    TIMX->CCMR1 = (TIM_CCMR1_CC1S_0 * 1) //input capture ch1 -> timer pin 1
                | (TIM_CCMR1_CC2S_0 * 1) //input captuer ch2 -> timer pin 2
                ;
    TIMX->CCER = (TIM_CCER_CC1E)
               | (TIM_CCER_CC2E)
               ;
}
void qep_init() {
    RCC_ENR(APB1, TIM5EN) = 1; RCC_RSTR(APB1, TIM5RST) = 1; RCC_RSTR(APB1, TIM5RST) = 0;
    RCC_ENR(APB1, TIM3EN) = 1; RCC_RSTR(APB1, TIM3RST) = 1; RCC_RSTR(APB1, TIM3RST) = 0;
    qep_init_1(TIM5);
    qep_init_1(TIM3);
}

void qep_on() {
    TIM5->EGR = TIM_EGR_UG; //reset counter
    TIM3->EGR = TIM_EGR_UG;
    TIM5->CR1 |= TIM_CR1_CEN; //start
    TIM3->CR1 |= TIM_CR1_CEN;
}

void qep_off() {
    TIM5->CR1 &=~ TIM_CR1_CEN; //stop
    TIM3->CR1 &=~ TIM_CR1_CEN;
}


////////////
// velocity (encoder pulses / s)

float qep_vel_x;
float qep_vel_y;
float qep_vel;

static float alpha; //exponential averaging decay factor

struct observer {
    int16_t n_last;
    float v_last;

    void reset(int16_t n) {
        n_last = n;
        v_last = 0;
    }

    float operator()(int16_t n) {
        int16_t dn = n - n_last;
        n_last = n;
        float v = dn/qep_vel_period_Ts;
        return v_last += (v - v_last)*alpha;
    }
};
static observer ox, oy;

void qep_vel_reset() {
    qep_vel_x = 0;
    qep_vel_y = 0;
    qep_vel   = 0;

    ox.reset(*qep_x);
    oy.reset(*qep_y);
}

void qep_vel_init() {
    qep_vel_reset();

    alpha = 1 - exp(-qep_vel_period_Ts/qep_vel_tau_Ts);
    if      (alpha > 0.8) alpha = 0.8;
    else if (alpha < 0.2) alpha = 0.2;

    /*
    printf("### qep_vel:\r\n"
           "    period: %8.3lf s\r\n"
           "    tau   : %8.3lf s\r\n"
           "    alpha : %8.3lf\r\n",
           qep_vel_period_Ts, qep_vel_tau_Ts, alpha); //debug
    */
}

void qep_vel_upd() {
    qep_vel_x = ox(*qep_x);
    qep_vel_y = oy(*qep_y);
    arm_sqrt_f32(qep_vel_x*qep_vel_x + qep_vel_y*qep_vel_y, &qep_vel);
}
