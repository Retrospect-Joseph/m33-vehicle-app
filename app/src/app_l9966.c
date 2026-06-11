#include "app_l9966.h"

#include <stdbool.h>
#include <stdint.h>

#include "board.h"

#include "board_pins.h"
#include "board_spi.h"
#include "board_time.h"

typedef struct
{
    bool initialized;
    uint32_t fault_count;
} app_l9966_state_t;

static app_l9966_state_t g_l9966;

static void APP_L9966_HardwareReset(void)
{
    BOARD_L9966_SetCs(true);
    BOARD_L9966_SetReset(false);
    BOARD_TIME_DelayMs(2U);
    BOARD_L9966_SetReset(true);
    BOARD_TIME_DelayMs(10U);
}

void APP_L9966_Init(void)
{
    g_l9966.initialized = false;
    g_l9966.fault_count = 0U;

    APP_L9966_HardwareReset();

    /*
     * TODO:
     * Replace this smoke transfer with your actual L9966 transport/driver init.
     * Keep the SPI and CS control in the platform layer.
     */
    {
        uint8_t tx[2] = {0x00U, 0x00U};
        uint8_t rx[2] = {0U, 0U};

        if (BOARD_SPI_L9966_Transfer(tx, rx, sizeof(tx)))
        {
            g_l9966.initialized = true;
        }
        else
        {
        }
    }
}

void APP_L9966_Poll(void)
{
    const bool fault_active = BOARD_L9966_IsFaultActive();

    if (fault_active)
    {
        g_l9966.fault_count++;
    }

    /*
     * TODO:
     * Call your real L9966 driver here, for example:
     * - read status/fault registers
     * - run diagnostics
     * - update application state
     */
}
