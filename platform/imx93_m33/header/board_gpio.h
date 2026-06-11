#ifndef BOARD_GPIO_IMX93_M33_H
#define BOARD_GPIO_IMX93_M33_H

#include <stdbool.h>
#include <stdint.h>

#include "fsl_rgpio.h"

void BOARD_GPIO_Init(void);
void BOARD_GPIO_Write(RGPIO_Type *base, uint32_t pin, bool value);
bool BOARD_GPIO_Read(RGPIO_Type *base, uint32_t pin);

#endif