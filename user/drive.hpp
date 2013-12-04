//drive: linearly-interpolated pulse train generator for generic servomotor drive modules
#ifndef _DRIVE_HPP_
#define _DRIVE_HPP_

#include "conf.hpp"
#include "misc.hpp"




struct DriveCmd{
    U32 dThms;
    I32 dAp[N_axis_count];
};

extern Pool<DriveCmd, N_drive_cmd_max+5> drive_cmd_pool;
extern os_mbx_declare(drive_cmd_mbx, N_drive_cmd_max);

static bool is_idle(){
    return os_mbx_check(drive_cmd_mbx)==N_drive_cmd_max;
}

void drive_init() AUTORUN;
void drive_start();
void drive_stop();




#endif//_DRIVE_HPP_
