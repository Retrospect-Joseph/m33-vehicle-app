#include "board.h"
#include "pin_mux.h"
#include "app_selftest.h"

void BOARD_InitHardware(void);
void APP_Init(void);
void APP_Run(void);

int main(void)
{
    BOARD_InitHardware();
    APP_Init();
    APP_SelfTest_RunAll();

    for (;;)
    {
        APP_Run();
    }
}