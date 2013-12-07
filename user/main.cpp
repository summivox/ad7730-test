#include "stdafx.h"
#pragma hdrstop
using namespace std;

#include "conf.hpp"
#include "misc.hpp"

#include "drive.hpp"
#include "vel_gen.hpp"

__task void main_task(){

    printf(
        "\r\n\r\n"
        "### proto-nc-main %s %s\r\n"
        "\r\n",
        __DATE__, __TIME__
    );
    
    int* a = new int[5];
    delete a;

    drive_start();

    os_dly_wait(100);

    vel_gen vg(Acc_line_max, V_feed_start, V_feed_max, V_feed_start, 50);

    float xvec = arm_cos_f32(CONV(30, Adeg, Arad));
    float yvec = arm_sin_f32(CONV(30, Adeg, Arad));
    int xlast = 0, ylast = 0;
    int i = 0;
    for (float s ; vg(s) ; ++i) {
        float d = s*Lmm_Lpulse;
        int x = floor(xvec*d);
        int y = floor(yvec*d);
#if true
        drive_push(1, x - xlast, y - ylast);
#else
        printf("\t%5d, %5d\r\n", x, y);
#endif
        xlast = x;
        ylast = y;
    }

    printf("### done\r\n");

    while (1) ;

	/*
    wait_ms(1000);
    //TODO: Initialize and launch other tasks

    os_tsk_delete_self();
	*/
}

int main(){
    os_sys_init_prio(main_task, 0xFE);
}
