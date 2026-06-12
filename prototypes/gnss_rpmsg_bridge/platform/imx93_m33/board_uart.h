#ifndef BOARD_UART_IMX93_M33_H
#define BOARD_UART_IMX93_M33_H

#include <stddef.h>
#include <stdint.h>

void BOARD_UART_Init(void);

size_t BOARD_UART_GPS_Write(const uint8_t *data, size_t length);
size_t BOARD_UART_GPS_Read(uint8_t *data, size_t max_length);
size_t BOARD_UART_GPS_Available(void);

uint32_t BOARD_UART_GPS_RxByteCount(void);
uint32_t BOARD_UART_GPS_RxOverflowCount(void);

#endif /* BOARD_UART_IMX93_M33_H */