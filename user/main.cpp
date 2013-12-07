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

    drive_start();

    os_dly_wait(100);

    SS in("M 10 10  30 10 L 20 30 z");
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
}

int main(){
    os_sys_init_prio(main_task, 0xFE);
}
