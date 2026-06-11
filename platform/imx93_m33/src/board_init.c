#include "board_init.h"

#include "board_pins.h"
#include "board_gpio.h"
#include "board_spi.h"
#include "board_uart.h"
#include "board_time.h"

void BOARD_PlatformInit(void)
{
    BOARD_PINS_DeviceSignalsInit();
    BOARD_GPIO_Init();
    BOARD_TIME_Init();
    BOARD_SPI_Init();
    BOARD_UART_Init();
}
