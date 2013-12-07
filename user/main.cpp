#include "stdafx.h"
#pragma hdrstop
using namespace std;

#include <iostream>
#include <sstream>
#include <string>

#include "conf.hpp"
#include "misc.hpp"

#include "drive.hpp"
#include "vel_gen.hpp"
#include "parser.hpp"

typedef stringstream SS;

bool exec_path(istream& in) {
    parser p(in);
    for (Seg* seg ; p(seg) ; ) {
        if (!seg) return false;
        seg->exec(V_feed_start, V_feed_max, V_feed_start);
        delete seg;
    }
    return true;
}

__task void main_task(){

    printf(
        "\r\n\r\n"
        "### proto-nc-main %s %s\r\n"
        "\r\n",
        __DATE__, __TIME__
    );

    drive_start();

    os_dly_wait(100);

    //SS in("M 10 10  30 10 L 20 30 z");
    //SS in("M30,20 h-15 a15,15 0 1,0 15,-15 z");
    string path;
    while (1) {
        //read from serial
        getline(cin, path);
        SS in(path);
        if (exec_path(in)) {
            printf("### done\r\n");
        } else {
            printf("!!! error\r\n");
        }
    }
}

int main(){
    os_sys_init_prio(main_task, 0xFE);
}
