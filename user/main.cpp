#include "stdafx.h"
#pragma hdrstop
using namespace std;

#include <iostream>
#include <sstream>

#include "conf.hpp"
#include "misc.hpp"

#include "drive.hpp"
#include "vel_gen.hpp"
#include "parser.hpp"

typedef stringstream SS;

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

    /*
    vel_gen vg(Acc_line_max, V_feed_start, V_feed_max, V_feed_start, 50);

    float xvec = arm_cos_f32(CONV(30, Adeg, Arad));
    float yvec = arm_sin_f32(CONV(30, Adeg, Arad));
    int xlast = 0, ylast = 0;
    for (float s ; vg(s) ; ) {
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
    */
    
    SS in("M 10 10 L 30 10 L 20 30 z");
    parser p(in);
    for (Seg* seg ; p(seg) ; ) {
        if (!seg) {
            printf("!!! parse error!");
            break;
        }
        seg->exec(V_feed_start, V_feed_max, V_feed_start);
        delete seg;
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
