#ifndef _DAC_RAMP_HPP_
#define _DAC_RAMP_HPP_

#include "conf.hpp"


extern bool volatile dac_ramp_running;
extern uint32_t volatile * dac_ramp_data;

void dac_ramp_init() AUTORUN;
void dac_ramp_start(uint16_t start, uint16_t stop, uint32_t Tms);
void dac_ramp_stop();


#endif//_DAC_RAMP_HPP_
