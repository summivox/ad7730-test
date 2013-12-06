#include "stdafx.h"
#pragma hdrstop
using namespace std;

#include "rtl.h"

#include "conf.hpp"
#include "misc.hpp"

#include "drive.hpp"

__task void main_task(){

    printf(
        "\r\n\r\n"
        "### proto-nc-main %s %s\r\n"
        "\r\n",
        __DATE__, __TIME__
    );
    
    drive_start();

    os_dly_wait(100);

    drive_push(2, 10, 10);
    drive_push(4, 10, -10);
    drive_push(2, -10, -10);
    drive_push(4, -10, 10);

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
