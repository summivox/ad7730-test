#include "stdafx.h"
#pragma hdrstop
using namespace std;

#include "laser.hpp"


IRQ_DECL(TIM2_IRQn, TIM2_IRQHandler, 3, 0);

bool laser_is_on = false;
int laser_tone = 0;

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
    
    TIM2->DIER = TIM_DIER_UIE;
    NVIC_EnableIRQ(TIM2_IRQn);
}

void laser_on() {
    TIM2->EGR = TIM_EGR_UG;
    TIM2->SR = ~TIM_SR_UIF;
    TIM2->CCER = TIM_CCER_CC3E;
    TIM2->CR1 |= TIM_CR1_CEN;
    laser_is_on = true;
}

void laser_off() {
    TIM2->CCER = 0;
    TIM2->CR1 &= ~TIM_CR1_CEN;
    laser_is_on = false;
}

void laser_set(bool state) {
    if (state) laser_on(); else laser_off();
}

void laser_set_tone(int t) {
    if (t < 0) t = 0;
    if (t > 100) t = 100;
    laser_tone = t;

    //TODO: move to dynamic toning
    TIM2->CCR3 = t * laser_period_Tus / 100;
}

void TIM2_IRQHandler() {
    //TODO
    TIM2->SR = ~TIM_SR_UIF;
}
