#ifndef BOARD_TIME_IMX93_M33_H
#define BOARD_TIME_IMX93_M33_H

#include <stdint.h>

void BOARD_TIME_Init(void);
uint32_t BOARD_TIME_GetMs(void);
void BOARD_TIME_DelayMs(uint32_t delay_ms);
void BOARD_TIME_DelayUs(uint32_t delay_us);

#endif