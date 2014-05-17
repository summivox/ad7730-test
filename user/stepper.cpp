#include "stdafx.h"
#pragma hdrstop
using namespace std;

#include "stepper.hpp"

#include <math.h>
#include <assert.h>

#include "conf.hpp"
#include "pinout.hpp"


bool volatile stepper_running;
int8_t stepper_dir;
int32_t volatile stepper_pos_Lpulse;
int32_t stepper_limit_lo_Lpulse;
int32_t stepper_limit_hi_Lpulse;


////////////
//helpers

static const uint32_t CCMR = TIM_CCMR1_OC1PE | (TIM_CCMR1_OC1M_0 * 7); //Ch1 set to PWM mode 2

static void set_period(TIM_TypeDef* TIM, uint32_t period_Tclk) {
    uint16_t prescaler = period_Tclk / (1u<<16);
    uint16_t reload = period_Tclk / (prescaler + 1) - 1;
    TIM->PSC = prescaler;
    TIM->ARR = reload;
}

static inline bool overtravel() {
    if (stepper_dir == +1) {
        return stepper_pos_Lpulse > stepper_limit_hi_Lpulse;
    } else {
        return stepper_pos_Lpulse < stepper_limit_lo_Lpulse;
    }
}


////////////
// basic methods

void stepper_init() {
    RCC_ENR(APB1, TIM4EN) = 1; RCC_RSTR(APB1, TIM4RST) = 1; RCC_RSTR(APB1, TIM4RST) = 0;
    NVIC_EnableIRQ(TIM4_IRQn);
    stepper_pos_Lpulse = 0;
    stepper_running = false;
}

void stepper_run(float Vmil, bool blind) {
    float Vmil_abs = fabs(Vmil);
    if (Vmil_abs <= 1e-6) return;
    if (Vmil_abs > stepper_speed_max_Vmil) Vmil_abs = stepper_speed_max_Vmil;
    if (Vmil > 0) {
        stepper_dir = +1;
        O_SIGN = 1;
    } else {
        stepper_dir = -1;
        O_SIGN = 0;
    }
    if (overtravel()) return;
    Vmil = Vmil_abs;

    float period_Tclk = SystemCoreClock / (Vmil * Lpulse_Lmil);
    if (period_Tclk < period_min_Tclk) period_Tclk = period_min_Tclk;
    if (period_Tclk > period_max_Tclk) period_Tclk = period_max_Tclk;
    set_period(TIM4, ceil(period_Tclk));
    TIM4->CNT = 0;

    TIM4->CR1 = TIM_CR1_ARPE;
    TIM4->CCER = TIM_CCER_CC1E;
    TIM4->CCMR1 = CCMR;
    TIM4->CCR1 = TIM4->ARR - pulse_width_min_Tclk;

    TIM4->DIER = 0;
    TIM4->EGR = TIM_EGR_UG;
    TIM4->SR = 0;
    if (!blind) {
        TIM4->DIER = TIM_DIER_CC1IE;
    }

    TIM4->CR1 |= TIM_CR1_CEN;
    stepper_running = true;
}

//delayed stop -- synchronize to pulse end
void stepper_stop() {
    stepper_running = false;
}
//actually stop the timer
void stepper_force_stop() {
    stepper_running = false;
    TIM4->CR1 = 0;
    TIM4->DIER = 0;
    NVIC_ClearPendingIRQ(TIM4_IRQn);
}

IRQ_DECL(TIM4_IRQn, TIM4_IRQHandler, 1, 0);
void TIM4_IRQHandler() {
    stepper_pos_Lpulse += stepper_dir;
    TIM4->SR = 0;
    if (!stepper_running || overtravel()) stepper_force_stop();
}


////////////
// homing sequence

static float homing_coarse_Vmil, homing_fine_Vmil;

enum homing_state_t {
    HOMING_INACTIVE = 0,
    HOMING_COARSE = 1,
    HOMING_BACKOFF = 2,
    HOMING_FINE = 3,
};
static homing_state_t volatile homing_state;

enum homing_flag_t{
    HOMING_FLAG_SWITCH = 1<<0,
};

static OS_TID stepper_home_tid;
static OS_SEM stepper_home_sem;
static __task void stepper_home_task() {

    //debounce home signal
#define EXPECT(x) do {                      \
    os_dly_wait(10);                        \
    while (E_nHOME != x) os_dly_wait(1);    \
} while (0)

    //if not home yet, move towards home
    if (E_nHOME == 1) {
        O_D(1, 0, 0, 0);
        homing_state = HOMING_COARSE;
        E_nHOME_EXTI::set_rising(false);
        E_nHOME_EXTI::set_falling(true);
        E_nHOME_EXTI::enable();
        stepper_run(stepper_home_dir * homing_coarse_Vmil, true);
        os_evt_wait_or(HOMING_FLAG_SWITCH, FOREVER);
        os_evt_clr(HOMING_FLAG_SWITCH, os_tsk_self());
        E_nHOME_EXTI::disable();
        O_D(0, 0, 0, 0);

        EXPECT(0);
        os_dly_wait(100);
    }

    //now at home, back off until out of home (clear hysterisis)
    O_D(0, 1, 0, 0);
    homing_state = HOMING_BACKOFF;
    E_nHOME_EXTI::set_rising(true);
    E_nHOME_EXTI::set_falling(false);
    E_nHOME_EXTI::enable();
    stepper_run(-stepper_home_dir * homing_fine_Vmil, true);
    os_evt_wait_or(HOMING_FLAG_SWITCH, FOREVER);
    os_evt_clr(HOMING_FLAG_SWITCH, os_tsk_self());
    E_nHOME_EXTI::disable();
    O_D(0, 0, 0, 0);

    EXPECT(1);
    os_dly_wait(100);

    //with hysterisis cleared, finally move slowly towards home
    O_D(0, 0, 1, 0);
    homing_state = HOMING_FINE;
    E_nHOME_EXTI::set_rising(false);
    E_nHOME_EXTI::set_falling(true);
    E_nHOME_EXTI::enable();
    stepper_run(stepper_home_dir * homing_fine_Vmil, true);
    os_evt_wait_or(HOMING_FLAG_SWITCH, FOREVER);
    os_evt_clr(HOMING_FLAG_SWITCH, os_tsk_self());
    E_nHOME_EXTI::disable();
    O_D(0, 0, 0, 0);

    EXPECT(0);
    os_dly_wait(100);

    //finish up
    homing_state = HOMING_INACTIVE;
    stepper_pos_Lpulse = 0;

    //set default limits
    if (stepper_home_dir == +1) {
        stepper_limit_lo_Lpulse = -stepper_travel_Lpulse;
        stepper_limit_hi_Lpulse = 0;
    } else {
        stepper_limit_lo_Lpulse = 0;
        stepper_limit_hi_Lpulse = +stepper_travel_Lpulse;
    }

    //completed
    os_sem_send(stepper_home_sem);
    os_tsk_delete_self();
}

void stepper_home(float coarse_Vmil, float fine_Vmil) {
    homing_coarse_Vmil = coarse_Vmil;
    homing_fine_Vmil = fine_Vmil;
    os_sem_init(stepper_home_sem, 0);
    stepper_home_tid = os_tsk_create(stepper_home_task, 0xC0);
    os_sem_wait(stepper_home_sem, FOREVER);
}

void E_nHOME_IRQHandler() {
    stepper_force_stop();
    if (homing_state != HOMING_INACTIVE) {
        isr_evt_set(HOMING_FLAG_SWITCH, stepper_home_tid);
    }
}
