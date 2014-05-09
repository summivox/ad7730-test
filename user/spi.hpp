#ifndef _SPI_HPP_
#define _SPI_HPP_

#include "conf.hpp"


#define SPI_WAIT_UNTIL(SPI, flag, val) while (!!(SPI->SR & SPI_SR_##flag) != val);

//8-bit & 16-bit
uint32_t SPI_send1(SPI_TypeDef* SPI, uint32_t tx);
uint32_t SPI_send2(SPI_TypeDef* SPI, uint32_t tx);

//8-bit only
uint32_t SPI_send3(SPI_TypeDef* SPI, uint32_t tx);
uint32_t SPI_send4(SPI_TypeDef* SPI, uint32_t tx);
void SPI_send(SPI_TypeDef* SPI, uint8_t* tx, uint8_t* rx, size_t n);

#endif//_SPI_HPP_
