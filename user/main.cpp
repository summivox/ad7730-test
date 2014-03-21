#include "stdafx.h"
#pragma hdrstop
using namespace std;

#include <iostream>
#include <sstream>
#include <string>

#include "conf.hpp"
#include "misc.hpp"
#include "math.hpp"

#include "drive.hpp"
#include "ad7730.hpp"


__task void main_task(){
    printf(
        "\r\n\r\n"
        "### ad7730-test %s %s\r\n"
        "\r\n",
        __DATE__, __TIME__
    );
    os_dly_wait(500);

    ad7730_reset();
    uint8_t status = ad7730_get_status();
    printf("<<< status: 0x%02X\r\n", status);
    uint32_t offset = ad7730_get_offset();
    printf("<<< offset: 0x%06X\r\n", offset);

    //string cmd;
    while (1) {
        /*
        cin >> cmd;
        if (0) {
        } else if (cmd == "path") {
        } else if (cmd == "feed") {
        }
        */
    }
}

int main(){
    os_sys_init_prio(main_task, 0x80);
}
