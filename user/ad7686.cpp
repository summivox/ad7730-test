#include "stdafx.h"
#pragma hdrstop
using namespace std;

#include "ad7686.hpp"

#include "pinout.hpp"
#include "spi.hpp"


uint32_t ad7686_data;

void ad7686_init() {
    SPI1->CR1 =
        SPI_CR1_CPHA    * 0 | //leading edge
        SPI_CR1_CPOL    * 0 | //idle low, active high
        SPI_CR1_MSTR    * 1 |
        SPI_CR1_BR_0    * 4 | //36 MHz / 2^(4+1) = 1.125 MHz
        SPI_CR1_SPE     * 1 |
        SPI_CR1_SSI     * 1 |
        SPI_CR1_SSM     * 1 |
        SPI_CR1_DFF     * 1 ; //16-bit word
}

/*
uint32_t ad7686_read() {
    SPI1->DR = 0;
    SPI_WAIT_UNTIL(SPI1, RXNE, 1);
    return ad7686_data = SPI1->DR;
}
*/
