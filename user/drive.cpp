#include "stdafx.h"
#pragma hdrstop
using namespace std;

#include "drive.hpp"

#include "conf.hpp"


IRQ_DECL(TIM1_UP_IRQn, TIM1_UP_IRQHandler, 2, 0);
IRQ_DECL(TIM4_IRQn, TIM4_IRQHandler, 1, 0);
IRQ_DECL(TIM8_CC_IRQn, TIM8_CC_IRQHandler, 1, 0);


struct Axis {
    TIM_TypeDef* TIMX; //associated timer
    IRQn_Type TIMX_IRQn;

    //direction: controlled by TIMX->CCMR1 value
    //              |   OC1M(PULS)          |   OC2M(SIGN)
    //  ----------- |   ------------------- |   ------------------
    //  -  MINUS    |   PWM mode 1 (3'b110) |   force 0 (3'b100)
    //  0  STOP     |   force 0    (3'b100) |   force 0 (3'b100)
    //  +  PLUS     |   PWM mode 1 (3'b110) |   force 1 (3'b101)
    enum dir_t {
        COMMON  =   TIM_CCMR1_OC1PE | TIM_CCMR1_OC2PE,
        MINUS   =   COMMON | (TIM_CCMR1_OC1M_0 * 6) | (TIM_CCMR1_OC2M_0 * 4),
        STOP    =   COMMON | (TIM_CCMR1_OC1M_0 * 4) | (TIM_CCMR1_OC2M_0 * 4),
        PLUS    =   COMMON | (TIM_CCMR1_OC1M_0 * 6) | (TIM_CCMR1_OC2M_0 * 5),
    };

    //double buffer for axis motion parameters
    //data_curr: currently in use
    //data_next: preloaded data
    struct Data {
        dir_t dir;

        //timer reload(period) == dTclk/dNpulse - 1
        //plus == ceil, minus == floor
        U16 period_plus_Tclk;
        U16 period_minus_Tclk;
        I32 total_Npulse; //==dNpulse
        I32 rem_Npulse; //countdown from dNpulse

        //leap pulse generation:
        //  l==kplus*dNpulse-dTclk (constant)
        //  s==(n*l)%dNpulse (updated and compared each pulse)
        I32 l_Npulse;
        I32 s_Npulse;
    } data[2];
    Data * volatile data_curr;
    Data * volatile data_next;

#ifdef QEP_MOCKUP
    uint16_t volatile qep;
#endif

    void init();
    //NOTE: Won't work when running from RAM. Cause unknown.
    //__forceinline void update() IN_RAM;
    void update() NO_INLINE;
};

void Axis::init() {
    data_curr = data;
    data_next = data + 1;

    //TIMX: tick precision
    TIMX->PSC = 0;
    TIMX->CNT = 0;
    TIMX->CR1 = TIM_CR1_ARPE;
    TIMX->CCER = TIM_CCER_CC1E | TIM_CCER_CC2E;
    TIMX->CCMR1 = Axis::STOP;
    TIMX->CCR1 = TIMX->CCR2 = 0;
    TIMX->BDTR = TIM_BDTR_AOE; //needed when TIMX == TIM1/8

    TIMX->EGR = TIM_EGR_UG;
    TIMX->SR = ~TIM_SR_UIF;

    //interrupt on pulse finish
    TIMX->DIER = TIM_DIER_CC1IE;

    NVIC_DisableIRQ(TIMX_IRQn);
    NVIC_ClearPendingIRQ(TIMX_IRQn);

#ifdef QEP_MOCKUP
    qep = 0;
#endif
}

//NOTE: critical
void Axis::update() {
    Data* D=data_curr;

#ifdef QEP_MOCKUP
    switch (D->dir) {
        case PLUS : ++qep; break;
        case MINUS: --qep; break;
    }
#endif

    if (--D->rem_Npulse != 0) {
        D->s_Npulse += D->l_Npulse;
        if (D->s_Npulse >= D->total_Npulse) {
            D->s_Npulse -= D->total_Npulse;
            TIMX->ARR = D->period_minus_Tclk;
        } else {
            TIMX->ARR = D->period_plus_Tclk;
        }
    } else {
        swap(data_curr, data_next);
        D=data_curr;

        TIMX->ARR = D->period_minus_Tclk;
        TIMX->CCMR1 = D->dir;
        assert(D->total_Npulse == D->rem_Npulse);

        //special case: prevent dNpulse==1 from screwing up ISR timing by
        //delaying CC, allowing TIM1 to finish before TIMX CC.
        if (D->total_Npulse == 1) {
            TIMX->CCR1 = D->period_minus_Tclk/2;
        } else {
            TIMX->CCR1 = min_pulse_width_Tclk;
        }
    }
}

static Axis axis[N_axis_count] = {
    {TIM4, TIM4_IRQn},
    {TIM8, TIM8_CC_IRQn},
};

#ifdef QEP_MOCKUP
extern uint16_t volatile * const qep_x = &(axis[0].qep);
extern uint16_t volatile * const qep_y = &(axis[1].qep);
#endif

void TIM4_IRQHandler() {
    axis[0].update();
    TIM4->SR = ~TIM_SR_CC1IF;
}
void TIM8_CC_IRQHandler() {
    axis[1].update();
    TIM8->SR = ~TIM_SR_CC1IF;
}


//when command FIFO gets empty, generate "idle" commands(dNpulse[]==0) instead
static DriveCmd cmd_idle = {1, {0}};

Pool<DriveCmd, N_drive_cmd_max+5> drive_cmd_pool;
os_mbx_declare(drive_cmd_mbx, N_drive_cmd_max);

//preload next drive command
DriveCmd *preloaded;
void free_preloaded() {
    if (preloaded && preloaded != &cmd_idle) {
        drive_cmd_pool.deallocate(preloaded);
    }
}
void preload(DriveCmd *cmd) {
    free_preloaded();
    preloaded = cmd;

    I32 dTclk = CONV(preloaded->dTstep, Tstep, Tclk);
    assert(dTclk > 0);

    FOR_EACH_AXIS(N_axis) {
        Axis& A = axis[N_axis];
        Axis::Data* D = A.data_next;

        I32 dNpulse = preloaded->dNpulse[N_axis];
        if (dNpulse != 0) {
            if (dNpulse < 0) {
                dNpulse = -dNpulse;
                D->dir = Axis::MINUS;
            } else { //dNpulse>0
                D->dir = Axis::PLUS;
            }

            I32 kplus = (dTclk - 1) / dNpulse + 1;
            I32 kminus = dTclk / dNpulse;
            I32 l = kplus * dNpulse - dTclk;

            assert(kminus >= min_period_Tclk);
            assert(kplus <= Tstep/Tclk);

            D->period_plus_Tclk = kplus - 1;   //ceil(dTclk/dNpulse) - 1
            D->period_minus_Tclk = kminus - 1; //floor(dTclk/dNpulse) - 1
            D->total_Npulse = D->rem_Npulse = dNpulse;
            D->l_Npulse = l;
            D->s_Npulse = 0;
        } else {
            //stall: dummy pulses, period = Tstep
            D->dir = Axis::STOP;
            D->period_plus_Tclk = Tstep/Tclk - 1;
            D->period_minus_Tclk = Tstep/Tclk - 1;
            D->total_Npulse = D->rem_Npulse = preloaded->dTstep;
            D->l_Npulse = 0;
            D->s_Npulse = 0;
        }
    }
}

void TIM1_UP_IRQHandler() {
    TIM1->ARR = preloaded->dTstep * 2 - 1;

    //preload next command in FIFO
    DriveCmd *cmd;
    if (isr_mbx_receive(drive_cmd_mbx, (void**)&cmd)!=OS_R_MBX) {
        //no command in FIFO, preload idle command instead
        cmd = &cmd_idle;
    }
    preload(cmd);

    TIM1->SR = ~TIM_SR_UIF;
}


void drive_init() {
    //Reset all timers
	RCC_ENR(APB2, TIM1EN) = 1; RCC_RSTR(APB2, TIM1RST) = 1; RCC_RSTR(APB2, TIM1RST) = 0;
    RCC_ENR(APB1, TIM4EN) = 1; RCC_RSTR(APB1, TIM4RST) = 1; RCC_RSTR(APB1, TIM4RST) = 0;
    RCC_ENR(APB2, TIM8EN) = 1; RCC_RSTR(APB2, TIM8RST) = 1; RCC_RSTR(APB2, TIM8RST) = 0;

    //TIM1: time step precision
    //NOTE: Use half time step timebase to avoid setting ARR to 0, which results
    //in no interrupt at all
    TIM1->PSC = CONV(1, Tstep, Tclk)/2 - 1;
    TIM1->CNT = 0;

    TIM1->EGR = TIM_EGR_UG;
    TIM1->SR = ~TIM_SR_UIF;

    TIM1->DIER = TIM_DIER_UIE;

    NVIC_DisableIRQ(TIM1_UP_IRQn);
    NVIC_ClearPendingIRQ(TIM1_UP_IRQn);

    //TIMX: handled by Axis::init()
    FOR_EACH_AXIS(N_axis) {
        axis[N_axis].init();
    }
}

void drive_start() {
    //clear FIFO
    drive_cmd_pool.init();
    os_mbx_init(drive_cmd_mbx, sizeof(drive_cmd_mbx));

    //preload 1st command as idle
    preloaded = 0;
    preload(&cmd_idle);

    //TIM1: 1st command, send bootstrapping trigger on enable
    TIM1->CNT = 0;
    TIM1->CR2 = TIM_CR2_MMS_0 * 1;

    TIM1->EGR = TIM_EGR_UG;
    TIM1->SR = ~TIM_SR_UIF;

    NVIC_ClearPendingIRQ(TIM1_UP_IRQn);
    NVIC_EnableIRQ(TIM1_UP_IRQn);

    //TIMX: bootstrapping:
    //  - set to idle run, 1st period preloaded(due to ARPE)
    //  - TIM1 enable => UEV to all TIMX
    //  - all TIMX synchronized and loaded with correct data
    FOR_EACH_AXIS(N_axis) {
        Axis& A = axis[N_axis];
        TIM_TypeDef* TIMX = A.TIMX;

        TIMX->ARR = 0xFFFF;
        TIMX->CCR1 = 0;
        TIMX->EGR = TIM_EGR_UG;
        TIMX->SR = ~(TIM_SR_UIF|TIM_SR_CC1IF);
        TIMX->CNT = 1;

        //NOTE: CEN will also flush CC preload register
        TIMX->CR1 |= TIM_CR1_CEN;

        A.data_curr->rem_Npulse = 1;
        A.update();

        NVIC_ClearPendingIRQ(A.TIMX_IRQn);
        NVIC_EnableIRQ(A.TIMX_IRQn);
    }
    //NOTE: trigger selection not guaranteed to be the same!
    //for 1->(4,8) it HAPPENS to be the same.
    TIM4->SMCR = (TIM_SMCR_SMS_0*4) | (TIM_SMCR_TS_0*0);
    TIM8->SMCR = (TIM_SMCR_SMS_0*4) | (TIM_SMCR_TS_0*0);

    //Enable TIM1
    //NOTE: Enabling TIM1 won't generate its own UEV, so we need to manually
    //generate it in order to correctly preload the next command and period.
    TIM1->EGR = TIM_EGR_UG;
    TIM1->CR1 |= TIM_CR1_CEN;

    //Finally remove the bootstrapping trigger
    TIM1->CR2 = 0;
}

void drive_stop() {
    __disable_irq();

    //suppress all pulse output before stopping
    FOR_EACH_AXIS(N_axis){
        axis[N_axis].TIMX->CCMR1 = Axis::STOP;
    }

    //then just wipe everything out
    drive_init();

    __enable_irq();
}

bool drive_push(U32 dTstep, I32 dx_Npulse, I32 dy_Npulse, U16 timeout) {
    //validate
    if (dTstep == 0 ||
        (dx_Npulse == 0 && dy_Npulse == 0)) return false;

    //allocate
    DriveCmd* cmd = drive_cmd_pool.allocate();
    if (cmd == NULL) return false;

    //fill
    cmd->dTstep = dTstep;
    cmd->dNpulse[0] = dx_Npulse;
    cmd->dNpulse[1] = dy_Npulse;

    //send
    if (os_mbx_send(drive_cmd_mbx, cmd, timeout) == OS_R_OK) return true;

    //if timeout: cleanup
    drive_cmd_pool.deallocate(cmd);
    return false;
}
