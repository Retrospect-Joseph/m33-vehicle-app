#include "board_time.h"

#include "fsl_common.h"

static volatile uint32_t g_board_ms_ticks;

void BOARD_TIME_Init(void)
{
    g_board_ms_ticks = 0U;
    (void)SysTick_Config(SystemCoreClock / 1000U);
}

uint32_t BOARD_TIME_GetMs(void)
{
    return g_board_ms_ticks;
}

void BOARD_TIME_DelayMs(uint32_t delay_ms)
{
    const uint32_t start = BOARD_TIME_GetMs();

    while ((BOARD_TIME_GetMs() - start) < delay_ms)
    {
    }
}

void BOARD_TIME_DelayUs(uint32_t delay_us)
{
    SDK_DelayAtLeastUs(delay_us, SystemCoreClock);
}

void SysTick_Handler(void)
{
    g_board_ms_ticks++;
}