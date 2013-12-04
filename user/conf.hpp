//conf
#ifndef _CONF_HPP_
#define _CONF_HPP_

#include "misc.hpp"
#include "pinout.hpp"


////////////
// Unit conversions:
//   [A] * A_in_B == B_from_A * [A] == equivalent [B]
//   e.g. USD_in_CNY == CNY_from_USD == 6.3

#define CONV(A, type, A_in_B, B) \
    static const type TOKEN_PASTE(TOKEN_PASTE(A, _in_), B) = (A_in_B); \
    static const type TOKEN_PASTE(TOKEN_PASTE(B, _from_), A) = (A_in_B)

//Time: Ts, Tms, Thms(half-ms), Tus, Tk(Tick==1/Clock Freq)
CONV(Ts, U32, 1000, Tms);
CONV(Tms, U32, 2, Thms);    CONV(Ts, U32, 2000, Thms);
CONV(Thms, U32, 500, Tus);  CONV(Tms, U32, 1000, Tus);  CONV(Ts, U32, 1000000, Tus);
CONV(Tus, U32, 72, Tk);     CONV(Thms, U32, 36000, Tk); CONV(Tms, U32, 72000, Tk);  CONV(Ts, U32, 72000000, Tk);

#undef CONV


////////////
// buffer size

static const int N_drive_cmd_max = 20; //drive command


////////////
// axis

static const int N_axis_count = 2;
static const U32 Tk_min_pulse_width = 50; //minimal pulse width
static const U32 Tk_min_period = 300; //min period (in ticks) <=> max pulse frequency

#define FOR_EACH_AXIS(N_axis) for (int N_axis = N_axis_count ; N_axis --> 0 ; )


#endif//_CONF_HPP_
