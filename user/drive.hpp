#ifndef _DRIVE_HPP_
#define _DRIVE_HPP_

#include "conf.hpp"
#include "units.hpp"


struct DriveCmd {
    U32 dTstep;
    I32 dNpulse[N_axis_count];
};

extern Pool<DriveCmd, N_drive_cmd_max+5> drive_cmd_pool;
extern os_mbx_declare(drive_cmd_mbx, N_drive_cmd_max);

static inline bool is_idle() {
    return os_mbx_check(drive_cmd_mbx) == N_drive_cmd_max;
}

void drive_init() AUTORUN;

void drive_start();
void drive_stop();

bool drive_push(U32 dt, I32 dx, I32 dy, U16 timeout = FOREVER); //true: success


#endif//_DRIVE_HPP_
