#include "app_gps.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "board.h"

#include "board_pins.h"
#include "board_time.h"
#include "board_uart.h"

typedef struct
{
    bool initialized;
    uint32_t bytes_seen;
    uint32_t last_log_ms;
} app_gps_state_t;

static app_gps_state_t g_gps;

static void APP_GPS_HardwareReset(void)
{
    BOARD_GPS_SetReset(false);
    BOARD_TIME_DelayMs(10U);
    BOARD_GPS_SetReset(true);
    BOARD_TIME_DelayMs(200U);
}

void APP_GPS_Init(void)
{
    g_gps.initialized = false;
    g_gps.bytes_seen = 0U;
    g_gps.last_log_ms = 0U;

    APP_GPS_HardwareReset();

    /*
     * TODO:
     * Replace this optional UBX config write with your real GPS driver init.
     * This is only a harmless starter example.
     */
    {
        static const uint8_t ubx_ping[] = {0xB5U, 0x62U};
        (void)BOARD_UART_GPS_Write(ubx_ping, sizeof(ubx_ping));
    }

    g_gps.initialized = true;
}

void APP_GPS_Poll(void)
{
    uint8_t buffer[64];
    const size_t bytes_read = BOARD_UART_GPS_Read(buffer, sizeof(buffer), 0U);

    if (bytes_read > 0U)
    {
        g_gps.bytes_seen += (uint32_t)bytes_read;

        if ((BOARD_TIME_GetMs() - g_gps.last_log_ms) >= 1000U)
        {
            g_gps.last_log_ms = BOARD_TIME_GetMs();
        }
    }

    /*
     * TODO:
     * Replace the raw-byte polling with your real NEO-M9V driver calls:
     * - read UBX frames
     * - parse fix/navigation data
     * - update application state
     */
}