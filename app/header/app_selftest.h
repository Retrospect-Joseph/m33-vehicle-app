#ifndef APP_SELFTEST_H
#define APP_SELFTEST_H

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    bool l9966_ran;
    bool l9966_pass;
    uint32_t l9966_error;

    bool gps_ran;
    bool gps_pass;
    uint32_t gps_error;
    uint32_t gps_bytes_seen;
} app_selftest_result_t;

#ifdef __cplusplus
extern "C" {
#endif

void APP_SelfTest_RunAll(void);
bool APP_SelfTest_RunL9966(void);
bool APP_SelfTest_RunGPS(void);

const app_selftest_result_t *APP_SelfTest_GetResult(void);

#ifdef __cplusplus
}
#endif

#endif