#include "stdafx.h"
#pragma hdrstop
using namespace std;

#include "seg.hpp"

#include "conf.hpp"
#include "vel_gen.hpp"
#include "drive.hpp"


MoveSeg::MoveSeg(float x1, float y1, float x2, float y2)
                :Seg(x1, y1, x2, y2) {
    length = 0; //special: not actually a segment
}

void MoveSeg::exec(float v1, float v2, float v3) {
    os_dly_wait(100);
    //TODO: shut down laser
    float x = floor((x2 - x1)*Lmm_Lpulse);
    float y = floor((y2 - y1)*Lmm_Lpulse);
    float t = max(x, y) / V_move_max;
    drive_push(t, x, y);
    //TODO: resume laser
    os_dly_wait(100);
}


LineSeg::LineSeg(float x1, float y1, float x2, float y2)
                :Seg(x1, y1, x2, y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    theta1 = theta2 = atan2(dy, dx);
    arm_sqrt_f32(dx*dx + dy*dy, &length);
}

void LineSeg::exec(float v1, float v2, float v3) {
    printf("LineSeg(%8.3f, %8.3f) %8.3f -> %8.3f -> %8.3f\r\n",
           x2, y2, v1, v2, v3); //DEBUG

    vel_gen vg(Acc_line_max, v1, v2, v3, length);
    float xvec = arm_cos_f32(theta1);
    float yvec = arm_sin_f32(theta1);
    float xlast = x1;
    float ylast = y1;
    for (float s ; vg(s) ; ) {
        float d = s*Lmm_Lpulse;
        int x = floor(x1 + xvec*d);
        int y = floor(y1 + yvec*d);
        drive_push(1, x - xlast, y - ylast);
        xlast = x;
        ylast = y;
    }
}
