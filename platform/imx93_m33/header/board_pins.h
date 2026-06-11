#ifndef BOARD_PINS_IMX93_M33_H
#define BOARD_PINS_IMX93_M33_H

#include <stdbool.h>
#include <stdint.h>

#include "fsl_common.h"
#include "fsl_rgpio.h"
#include "fsl_lpspi.h"
#include "fsl_lpuart.h"

#define BOARD_L9966_SPI_BASE             LPSPI2
#define BOARD_L9966_SPI_BAUDRATE_HZ      (1000000U)
#define BOARD_L9966_SPI_SRC_CLOCK_HZ     (24000000U)
#define BOARD_L9966_SPI_PCS              kLPSPI_Pcs0

#define BOARD_GPS_UART_BASE              LPUART7
#define BOARD_GPS_UART_BAUDRATE          (38400U)
#define BOARD_GPS_UART_SRC_CLOCK_HZ      (24000000U)
#define BOARD_GPS_UART_IRQn              LPUART7_IRQn
#define BOARD_GPS_UART_IRQHandler        LPUART7_IRQHandler

#define BOARD_L9966_CS_GPIO              GPIO1
#define BOARD_L9966_CS_PIN               (3U)

#define BOARD_L9966_RESET_GPIO           GPIO1
#define BOARD_L9966_RESET_PIN            (4U)

#define BOARD_L9966_FAULT_GPIO           GPIO1
#define BOARD_L9966_FAULT_PIN            (5U)

#define BOARD_GPS_RESET_GPIO             GPIO1
#define BOARD_GPS_RESET_PIN              (6U)

#define BOARD_GPS_PPS_GPIO               GPIO1
#define BOARD_GPS_PPS_PIN                (7U)

void BOARD_PINS_DeviceSignalsInit(void);

void BOARD_L9966_SetCs(bool inactive);
void BOARD_L9966_SetReset(bool released);
bool BOARD_L9966_IsFaultActive(void);

void BOARD_GPS_SetReset(bool released);
bool BOARD_GPS_ReadPps(void);

#endif