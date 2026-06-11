#include "app_selftest.h"

#include "board_pins.h"
#include "board_spi.h"
#include "board_time.h"
#include "board_uart.h"

static app_selftest_result_t s_selftestResult;

enum
{
    APP_SELFTEST_OK = 0U,
    APP_SELFTEST_L9966_SPI_TRANSFER_FAILED = 1U,
    APP_SELFTEST_L9966_RESPONSE_INVALID = 2U,
    APP_SELFTEST_GPS_NO_BYTES_RECEIVED = 3U,
};

static bool APP_SelfTest_IsLikelyInvalidSpiResponse(const uint8_t *rx, uint32_t len)
{
    uint32_t i;
    bool allZero = true;
    bool allFF = true;

    for (i = 0U; i < len; i++)
    {
        if (rx[i] != 0x00U)
        {
            allZero = false;
        }

        if (rx[i] != 0xFFU)
        {
            allFF = false;
        }
    }

    return (allZero || allFF);
}

bool APP_SelfTest_RunL9966(void)
{
    uint8_t tx[2] = {0x00U, 0x00U};
    uint8_t rx[2] = {0x00U, 0x00U};
    bool ok;

    s_selftestResult.l9966_ran = true;
    s_selftestResult.l9966_pass = false;
    s_selftestResult.l9966_error = APP_SELFTEST_OK;

    BOARD_L9966_SetCs(true);
    BOARD_L9966_SetReset(false);
    BOARD_TIME_DelayMs(2U);
    BOARD_L9966_SetReset(true);
    BOARD_TIME_DelayMs(10U);

    ok = BOARD_SPI_L9966_Transfer(tx, rx, sizeof(tx));
    if (!ok)
    {
        s_selftestResult.l9966_error = APP_SELFTEST_L9966_SPI_TRANSFER_FAILED;
        return false;
    }

    if (APP_SelfTest_IsLikelyInvalidSpiResponse(rx, sizeof(rx)))
    {
        s_selftestResult.l9966_error = APP_SELFTEST_L9966_RESPONSE_INVALID;
        return false;
    }

    s_selftestResult.l9966_pass = true;
    return true;
}

bool APP_SelfTest_RunGPS(void)
{
    uint8_t sync[2] = {0xB5U, 0x62U};
    uint8_t rxBuf[64];
    uint32_t startMs;
    uint32_t bytesRead;
    uint32_t totalBytes = 0U;

    s_selftestResult.gps_ran = true;
    s_selftestResult.gps_pass = false;
    s_selftestResult.gps_error = APP_SELFTEST_OK;
    s_selftestResult.gps_bytes_seen = 0U;

    BOARD_GPS_SetReset(false);
    BOARD_TIME_DelayMs(10U);
    BOARD_GPS_SetReset(true);
    BOARD_TIME_DelayMs(200U);

    (void)BOARD_UART_GPS_Write(sync, sizeof(sync));

    startMs = BOARD_TIME_GetMs();
    while ((BOARD_TIME_GetMs() - startMs) < 1000U)
    {
        bytesRead = BOARD_UART_GPS_Read(rxBuf, sizeof(rxBuf), 10U);
        totalBytes += bytesRead;

        if (totalBytes > 0U)
        {
            s_selftestResult.gps_bytes_seen = totalBytes;
            s_selftestResult.gps_pass = true;
            return true;
        }
    }

    s_selftestResult.gps_bytes_seen = totalBytes;
    s_selftestResult.gps_error = APP_SELFTEST_GPS_NO_BYTES_RECEIVED;
    return false;
}

void APP_SelfTest_RunAll(void)
{
    s_selftestResult.l9966_ran = false;
    s_selftestResult.l9966_pass = false;
    s_selftestResult.l9966_error = APP_SELFTEST_OK;

    s_selftestResult.gps_ran = false;
    s_selftestResult.gps_pass = false;
    s_selftestResult.gps_error = APP_SELFTEST_OK;
    s_selftestResult.gps_bytes_seen = 0U;

    (void)APP_SelfTest_RunL9966();
    (void)APP_SelfTest_RunGPS();
}

const app_selftest_result_t *APP_SelfTest_GetResult(void)
{
    return &s_selftestResult;
}