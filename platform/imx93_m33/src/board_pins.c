#include "board_pins.h"

#include "board_gpio.h"

void BOARD_PINS_DeviceSignalsInit(void)
{
    /* Pin mux belongs in pin_mux.c; keep device signal helpers here. */
}

void BOARD_L9966_SetCs(bool inactive)
{
    BOARD_GPIO_Write(BOARD_L9966_CS_GPIO, BOARD_L9966_CS_PIN, inactive);
}

void BOARD_L9966_SetReset(bool released)
{
    BOARD_GPIO_Write(BOARD_L9966_RESET_GPIO, BOARD_L9966_RESET_PIN, released);
}

bool BOARD_L9966_IsFaultActive(void)
{
    return BOARD_GPIO_Read(BOARD_L9966_FAULT_GPIO, BOARD_L9966_FAULT_PIN);
}

void BOARD_GPS_SetReset(bool released)
{
    BOARD_GPIO_Write(BOARD_GPS_RESET_GPIO, BOARD_GPS_RESET_PIN, released);
}

bool BOARD_GPS_ReadPps(void)
{
    return BOARD_GPIO_Read(BOARD_GPS_PPS_GPIO, BOARD_GPS_PPS_PIN);
}
