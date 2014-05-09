#ifndef _ADC_HPP_
#define _ADC_HPP_


#include "conf.hpp"

#include "ad7730.hpp"
#include "ad7686.hpp"


void adc_init();
void adc_start();
void adc_stop();

extern void adc_sample_handler();


#endif//_ADC_HPP_
