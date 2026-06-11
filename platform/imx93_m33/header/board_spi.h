#ifndef BOARD_SPI_IMX93_M33_H
#define BOARD_SPI_IMX93_M33_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void BOARD_SPI_Init(void);
bool BOARD_SPI_L9966_Transfer(const uint8_t *tx, uint8_t *rx, size_t length);

#endif