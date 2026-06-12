#include "board_uart.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "fsl_common.h"
#include "fsl_lpuart.h"

/*
 * Proven Linux-side mapping before handoff:
 *
 *   /dev/ttyLP6
 *   /soc@0/bus@42000000/serial@42690000
 *   alias serial6
 *   baud 38400
 *
 * Expected M33-side peripheral:
 *
 *   LPUART7
 */
#define BOARD_GPS_UART_BASE              LPUART7
#define BOARD_GPS_UART_BAUDRATE          (38400U)
#define BOARD_GPS_UART_SRC_CLOCK_HZ      (24000000U)
#define BOARD_GPS_UART_IRQn              LPUART7_IRQn
#define BOARD_GPS_UART_IRQHandler        LPUART7_IRQHandler

#define BOARD_GPS_RX_RING_SIZE           (4096U)

typedef struct
{
    uint8_t data[BOARD_GPS_RX_RING_SIZE];
    volatile uint16_t head;
    volatile uint16_t tail;
} board_uart_ring_t;

static board_uart_ring_t g_gps_rx_ring;
static volatile uint32_t g_gps_rx_byte_count;
static volatile uint32_t g_gps_rx_overflow_count;

static uint16_t BOARD_UART_RingNext(uint16_t index)
{
    return (uint16_t)((index + 1U) % BOARD_GPS_RX_RING_SIZE);
}

static bool BOARD_UART_RingPush(board_uart_ring_t *ring, uint8_t value)
{
    const uint16_t next = BOARD_UART_RingNext(ring->head);

    if (next == ring->tail)
    {
        return false;
    }

    ring->data[ring->head] = value;
    ring->head = next;
    return true;
}

static bool BOARD_UART_RingPop(board_uart_ring_t *ring, uint8_t *value)
{
    if (ring->head == ring->tail)
    {
        return false;
    }

    *value = ring->data[ring->tail];
    ring->tail = BOARD_UART_RingNext(ring->tail);
    return true;
}

void BOARD_UART_Init(void)
{
    lpuart_config_t config;

    memset(&g_gps_rx_ring, 0, sizeof(g_gps_rx_ring));
    g_gps_rx_byte_count = 0U;
    g_gps_rx_overflow_count = 0U;

    LPUART_GetDefaultConfig(&config);

    config.baudRate_Bps = BOARD_GPS_UART_BAUDRATE;
    config.enableTx = true;
    config.enableRx = true;

    LPUART_Init(
        BOARD_GPS_UART_BASE,
        &config,
        BOARD_GPS_UART_SRC_CLOCK_HZ);

    LPUART_EnableInterrupts(
        BOARD_GPS_UART_BASE,
        (uint32_t)kLPUART_RxDataRegFullInterruptEnable);

    EnableIRQ(BOARD_GPS_UART_IRQn);
}

size_t BOARD_UART_GPS_Write(const uint8_t *data, size_t length)
{
    if ((data == NULL) || (length == 0U))
    {
        return 0U;
    }

    if (LPUART_WriteBlocking(BOARD_GPS_UART_BASE, data, length) != kStatus_Success)
    {
        return 0U;
    }

    return length;
}

size_t BOARD_UART_GPS_Read(uint8_t *data, size_t max_length)
{
    size_t count = 0U;

    if ((data == NULL) || (max_length == 0U))
    {
        return 0U;
    }

    while (count < max_length)
    {
        uint8_t value;

        if (!BOARD_UART_RingPop(&g_gps_rx_ring, &value))
        {
            break;
        }

        data[count++] = value;
    }

    return count;
}

size_t BOARD_UART_GPS_Available(void)
{
    if (g_gps_rx_ring.head >= g_gps_rx_ring.tail)
    {
        return (size_t)(g_gps_rx_ring.head - g_gps_rx_ring.tail);
    }

    return (size_t)(BOARD_GPS_RX_RING_SIZE - g_gps_rx_ring.tail + g_gps_rx_ring.head);
}

uint32_t BOARD_UART_GPS_RxByteCount(void)
{
    return g_gps_rx_byte_count;
}

uint32_t BOARD_UART_GPS_RxOverflowCount(void)
{
    return g_gps_rx_overflow_count;
}

void BOARD_GPS_UART_IRQHandler(void)
{
    const uint32_t flags = LPUART_GetStatusFlags(BOARD_GPS_UART_BASE);

    if ((flags & (uint32_t)kLPUART_RxOverrunFlag) != 0U)
    {
        g_gps_rx_overflow_count++;
        (void)LPUART_ClearStatusFlags(
            BOARD_GPS_UART_BASE,
            (uint32_t)kLPUART_RxOverrunFlag);
    }

    if ((flags & (uint32_t)kLPUART_RxDataRegFullFlag) != 0U)
    {
        const uint8_t value = LPUART_ReadByte(BOARD_GPS_UART_BASE);

        g_gps_rx_byte_count++;

        if (!BOARD_UART_RingPush(&g_gps_rx_ring, value))
        {
            g_gps_rx_overflow_count++;
        }
    }

    SDK_ISR_EXIT_BARRIER;
}