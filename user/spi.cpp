#include "stdafx.h"
#pragma hdrstop
using namespace std;

#include "spi.hpp"


//8-bit & 16-bit
uint32_t SPI_send1(SPI_TypeDef* SPI, uint32_t tx) {
    SPI->DR = tx;
    SPI_WAIT_UNTIL(SPI, RXNE, 1);
    return SPI->DR;
}
uint32_t SPI_send2(SPI_TypeDef* SPI, uint32_t tx) {
    uint32_t rx;

    if (SPI->CR1 & SPI_CR1_DFF) {
        //16-bit frame
        SPI->DR = tx >> 16;

        SPI_WAIT_UNTIL(SPI, TXE, 1);
        SPI->DR = tx & 0xffffu;
        SPI_WAIT_UNTIL(SPI, RXNE, 1);
        rx = SPI->DR << 16;

        SPI_WAIT_UNTIL(SPI, RXNE, 1);
        rx |= SPI->DR;
    } else {
        //8-bit frame
        SPI->DR = tx >> 8;

        SPI_WAIT_UNTIL(SPI, TXE, 1);
        SPI->DR = tx & 0xffu;
        SPI_WAIT_UNTIL(SPI, RXNE, 1);
        rx = SPI->DR << 8;

        SPI_WAIT_UNTIL(SPI, RXNE, 1);
        rx |= SPI->DR;
    }

    return rx;
}

//8-bit only
uint32_t SPI_send3(SPI_TypeDef* SPI, uint32_t tx) {
    uint32_t rx = 0;

    SPI->DR = (tx >> 16) & 0xffu;

    SPI_WAIT_UNTIL(SPI, TXE, 1);
    SPI->DR = (tx >> 8) & 0xffu;
    SPI_WAIT_UNTIL(SPI, RXNE, 1);
    rx = (rx << 8) | SPI->DR;
    SPI_WAIT_UNTIL(SPI, TXE, 1);
    SPI->DR = (tx >> 0) & 0xffu;
    SPI_WAIT_UNTIL(SPI, RXNE, 1);
    rx = (rx << 8) | SPI->DR;

    SPI_WAIT_UNTIL(SPI, RXNE, 1);
    rx = (rx << 8) | SPI->DR;

    return rx;
}
uint32_t SPI_send4(SPI_TypeDef* SPI, uint32_t tx) {
    uint32_t rx = 0;

    SPI->DR = (tx >> 24) & 0xffu;

    SPI_WAIT_UNTIL(SPI, TXE, 1);
    SPI->DR = (tx >> 16) & 0xffu;
    SPI_WAIT_UNTIL(SPI, RXNE, 1);
    rx = (rx << 8) | SPI->DR;
    SPI_WAIT_UNTIL(SPI, TXE, 1);
    SPI->DR = (tx >> 8) & 0xffu;
    SPI_WAIT_UNTIL(SPI, RXNE, 1);
    rx = (rx << 8) | SPI->DR;
    SPI_WAIT_UNTIL(SPI, TXE, 1);
    SPI->DR = (tx >> 0) & 0xffu;
    SPI_WAIT_UNTIL(SPI, RXNE, 1);
    rx = (rx << 8) | SPI->DR;

    SPI_WAIT_UNTIL(SPI, RXNE, 1);
    rx = (rx << 8) | SPI->DR;

    return rx;
}
void SPI_send(SPI_TypeDef* SPI, uint8_t* tx, uint8_t* rx, size_t n) {
    SPI->DR = *tx++;
    while (--n > 0) {
        SPI_WAIT_UNTIL(SPI, TXE, 1);
        SPI->DR = *tx++;
        SPI_WAIT_UNTIL(SPI, RXNE, 1);
        *rx++ = SPI->DR;
    }
    SPI_WAIT_UNTIL(SPI, RXNE, 1);
    *rx++ = SPI->DR;
}
