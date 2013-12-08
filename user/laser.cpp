#include "stdafx.h"
#pragma hdrstop
using namespace std;

#include "laser.hpp"

#include "math.hpp"
#include "conf.hpp"

#include "qep.hpp"


IRQ_DECL(TIM2_IRQn, TIM2_IRQHandler, 3, 0);

bool laser_is_on = false;
float laser_tone = 0;

void laser_init() {
    laser_is_on = false;
    laser_tone = 0;

    RCC_ENR(APB1, TIM2EN) = 1;
    RCC_RSTR(APB1, TIM2RST) = 1;
    RCC_RSTR(APB1, TIM2RST) = 0;

    TIM2->PSC = CONV(1, Tus, Tclk) - 1;
    TIM2->ARR = laser_period_Tus - 1;

    TIM2->CCMR2 = TIM_CCMR2_OC3PE | (TIM_CCMR2_OC3M_0 * 6);
    TIM2->CCR3 = 0;

    TIM2->EGR = TIM_EGR_UG;
    TIM2->SR = ~TIM_SR_UIF;
    TIM2->CR1 |= TIM_CR1_CEN;

    TIM2->DIER = TIM_DIER_UIE;
    NVIC_EnableIRQ(TIM2_IRQn);

    qep_vel_init();
}

void laser_set(bool state) {
    TIM2->CCER = state ? TIM_CCER_CC3E : 0;
}

void laser_set_tone(float t) {
    if (t <= 0) t = 0;
    if (t >= 1) t = 1;
    laser_tone = t;
}

void TIM2_IRQHandler() {
    qep_vel_upd();
    float duty = qep_vel/Lmm_Lpulse * laser_tone * laser_tone_factor;
    if (duty >= 0.95) duty = 1;
    else duty = pow(duty, laser_tone_gamma);
    int duty_n = round(duty * laser_period_Tus);
    TIM2->CCR3 = duty_n;
    TIM2->SR = ~TIM_SR_UIF;
}
