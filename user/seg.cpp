#include "stdafx.h"
#pragma hdrstop
using namespace std;

#include "seg.hpp"

#include "conf.hpp"
#include "math.hpp"

#include "drive.hpp"
#include "laser.hpp"

#include "vel_gen.hpp"


MoveSeg::MoveSeg(float x1, float y1, float x2, float y2)
                :Seg(x1, y1, x2, y2) {
    length = -1; //not drawn
}

void MoveSeg::exec(float v1, float v2, float v3) {
    printf("<<< M(%8.3f, %8.3f)\r\n", x2, y2); //DEBUG

    int x1n = round(x1*Lmm_Lpulse);
    int y1n = round(y1*Lmm_Lpulse);
    int x2n = round(x2*Lmm_Lpulse);
    int y2n = round(y2*Lmm_Lpulse);
    int dxn = x2n - x1n;
    int dyn = y2n - y1n;
    int dxn_a = abs(dxn);
    int dyn_a = abs(dyn);
    int diag_x, diag_y, diag_a;
    if (dxn_a < dyn_a) {
        diag_x = dxn;
        diag_y = dyn_a ? dxn_a * (dyn/dyn_a) : 0;
        diag_a = dxn_a;
    } else {
        diag_x = dxn_a ? dyn_a * (dxn/dxn_a) : 0;
        diag_y = dyn;
        diag_a = dyn_a;
    }
    dxn -= diag_x;
    dyn -= diag_y;
    dxn_a -= diag_a; //at least one is zero
    dyn_a -= diag_a;

#define T(n) (floor(CONV((n)/(V_move_max*Lmm_Lpulse), Ts, Tstep)))
#define D(tt, xx, yy) do { \
    drive_push(tt, xx, yy); \
    os_dly_wait(CONV(tt, Tstep, Tms)); \
} while (false);

    D(T(diag_a), diag_x, diag_y);
    D(T(dxn_a + dyn_a), dxn, dyn);

#undef T
#undef D
}


LineSeg::LineSeg(float x1, float y1, float x2, float y2)
                :Seg(x1, y1, x2, y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    theta1 = theta2 = atan2(dy, dx);
    float l2 = dx*dx + dy*dy;
    if (l2 <= 1e-9) {
        length = 0; //denormalized
    } else {
        arm_sqrt_f32(l2, &length);
    }
}

void LineSeg::exec(float v1, float v2, float v3) {
    printf("<<< L(%8.3f, %8.3f) %8.3f -> %8.3f -> %8.3f\r\n",
           x2, y2, v1, v2, v3); //DEBUG

    if (length == 0) return;

    vel_gen vg(Acc_line_max, v1, v2, v3, length);
    float xvec = arm_cos_f32(theta1);
    float yvec = arm_sin_f32(theta1);
    int xlast = round(x1*Lmm_Lpulse);
    int ylast = round(y1*Lmm_Lpulse);
    for (float s ; vg(s) ; ) {
        int x = round((x1 + xvec*s)*Lmm_Lpulse);
        int y = round((y1 + yvec*s)*Lmm_Lpulse);
        drive_push(1, x - xlast, y - ylast);
        xlast = x;
        ylast = y;
    }
}


ArcSeg::ArcSeg(float x1, float y1, float x2, float y2,
               float r1, float r2, float phi,
               bool is_large, bool is_sweep)
              :Seg(x1, y1, x2, y2),
               r1(r1), r2(r2), phi(phi),
               is_large(is_large), is_sweep(is_sweep) {
    //TODO: support ellipsis
    float r = r2 = r1;
    phi = 0;

    //convert to parametric form
    float r_2 = r*r;
    float r_4 = r_2*r_2;
    int sign  = ((is_large ^ is_sweep) * 2) - 1;

    float x1_ = (x1 - x2)*.5;
    float y1_ = (y1 - y2)*.5;

    float k1  = r_2*(x1_*x1_ + y1_*y1_);
    float k2  = (r_4 - k1)/k1;
    float k3; arm_sqrt_f32(k2, &k3); k3 *= sign;

    float cx_ =  k3*y1_;
    float cy_ = -k3*x1_;

    cx  = cx_ + (x1 + x2)*.5;
    cy  = cy_ + (y1 + y2)*.5;
    a1 = atan2( y1_ - cy_,  x1_ - cx_);
    a2 = atan2(-y1_ - cy_, -x1_ - cx_);

    //fix the angles
    if (is_sweep) {
        theta1 = wrap_angle(a1 + HALF_PI);
        theta2 = wrap_angle(a2 + HALF_PI);

        if (a2 < a1) a2 += TWO_PI;
        length = r*(a2 - a1);
    } else {
        theta1 = wrap_angle(a1 - HALF_PI);
        theta2 = wrap_angle(a2 - HALF_PI);

        if (a2 > a1) a2 -= TWO_PI;
        length = r*(a1 - a2);
    }
};

void ArcSeg::exec(float v1, float v2, float v3) {
    //limit max speed according to radius
    float vm_2 = Acc_arc_max * r1;
    float vm; arm_sqrt_f32(vm_2, &vm);
    if (vm < v2) v2 = vm;

    printf("<<< A(%8.3f, %8.3f) %8.3f -> %8.3f -> %8.3f\r\n"
           "    C(%8.3f, %8.3f) R%7.3f  A %8.3f -> %8.3f\r\n",
           x2, y2, v1, v2, v3,
           cx, cy, r1,
           CONV(a1, Arad, Adeg),
           CONV(a2, Arad, Adeg)); //DEBUG

    vel_gen wg(Acc_arc_max/r1, v1/r1, v2/r1, v3/r1, length/r1);
    int xlast = round(x1*Lmm_Lpulse);
    int ylast = round(y1*Lmm_Lpulse);
    int sign  = (is_sweep << 1) - 1;
    for (float a ; wg(a) ; ) {
        a = a1 + sign*a;
        float xx = cx + r1*arm_cos_f32(a);
        float yy = cy + r1*arm_sin_f32(a);
        int x = round(xx*Lmm_Lpulse);
        int y = round(yy*Lmm_Lpulse);
        drive_push(1, x - xlast, y - ylast);
        //printf("     (%8.3f, %8.3f) [%8.3f]\r\n", xx, yy, a); //DEBUG
        xlast = x;
        ylast = y;
    }
}
