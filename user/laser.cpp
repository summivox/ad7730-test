#include "stdafx.h"
#pragma hdrstop
using namespace std;

#include "laser.hpp"


bool laser_is_on = false;
int laser_tone = 0;

void laser_init() {
    RCC_ENR(APB2, TIM8EN) = 1;
    RCC_RSTR(APB2, TIM8RST) = 1;
    RCC_RSTR(APB2, TIM8RST) = 0;
    //TODO
}

void laser_on() {
    //TODO
    laser_is_on = true;
}

void laser_off() {
    //TODO
    laser_is_on = false;
}

void laser_set_tone(int t) {
    if (t < 0) t = 0;
    if (t > 100) t = 100;
    laser_tone = t;
    //TODO
}
