#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"

#include "board_init.h"

void BOARD_InitHardware(void)
{
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
    BOARD_PlatformInit();
}