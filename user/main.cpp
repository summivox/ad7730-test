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
    
    drive_init();
    drive_start();

    os_dly_wait(100);

    DriveCmd *cmd;

#define SEND(dt, dx, dy) do { \
    cmd = drive_cmd_pool.allocate(); \
    cmd->dThms = dt; \
    cmd->dAp[0] = dx; \
    cmd->dAp[1] = dy; \
    os_mbx_send(drive_cmd_mbx, cmd, FOREVER); \
} while (false)

    SEND(2, 10, 10);
    SEND(4, 10, -10);
    SEND(2, -10, -10);
    SEND(4, -10, 10);

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
