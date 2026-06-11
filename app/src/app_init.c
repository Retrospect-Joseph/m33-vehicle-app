#include "app.h"
#include "app_l9966.h"
#include "app_gps.h"
#include "board_time.h"
#include <stdint.h>

typedef struct
{
    uint32_t last_l9966_poll_ms;
    uint32_t last_gps_poll_ms;
} app_runtime_t;

static app_runtime_t g_app_runtime;

void APP_Init(void)
{
    g_app_runtime.last_l9966_poll_ms = 0U;
    g_app_runtime.last_gps_poll_ms = 0U;

    APP_L9966_Init();
    APP_GPS_Init();
}

void APP_Run(void)
{
    const uint32_t now_ms = BOARD_TIME_GetMs();

    if ((now_ms - g_app_runtime.last_l9966_poll_ms) >= 100U)
    {
        g_app_runtime.last_l9966_poll_ms = now_ms;
        APP_L9966_Poll();
    }

    if ((now_ms - g_app_runtime.last_gps_poll_ms) >= 20U)
    {
        g_app_runtime.last_gps_poll_ms = now_ms;
        APP_GPS_Poll();
    }
}