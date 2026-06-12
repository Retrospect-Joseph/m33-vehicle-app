/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rpmsg_lite.h"
#include "rpmsg_queue.h"
#include "rpmsg_ns.h"

#include "pin_mux.h"
#include "board.h"
#include "fsl_debug_console.h"

#include "FreeRTOS.h"
#include "task.h"

#include "board_uart.h"
#include "rsc_table.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define RPMSG_LITE_LINK_ID            (RL_PLATFORM_IMX93_M33_A55_USER_LINK_ID)
#define RPMSG_LITE_SHMEM_BASE         (VDEV1_VRING_BASE)
#define RPMSG_LITE_NS_ANNOUNCE_STRING "rpmsg-virtual-tty-channel"
#define RPMSG_LITE_MASTER_IS_LINUX

#define APP_DEBUG_UART_BAUDRATE       (115200U)
#define APP_TASK_STACK_SIZE           (1024U)

#ifndef LOCAL_EPT_ADDR
#define LOCAL_EPT_ADDR                (30U)
#endif

#define GPS_BRIDGE_BUFFER_SIZE        (128U)

/*******************************************************************************
 * Globals
 ******************************************************************************/

static TaskHandle_t app_task_handle = NULL;

static struct rpmsg_lite_instance *volatile my_rpmsg = NULL;
static struct rpmsg_lite_endpoint *volatile my_ept = NULL;
static volatile rpmsg_queue_handle my_queue = NULL;

/*******************************************************************************
 * Code
 ******************************************************************************/

void app_destroy_task(void)
{
    if (app_task_handle != NULL)
    {
        vTaskDelete(app_task_handle);
        app_task_handle = NULL;
    }

    if (my_ept != NULL)
    {
        rpmsg_lite_destroy_ept(my_rpmsg, my_ept);
        my_ept = NULL;
    }

    if (my_queue != NULL)
    {
        rpmsg_queue_destroy(my_rpmsg, my_queue);
        my_queue = NULL;
    }

    if (my_rpmsg != NULL)
    {
        rpmsg_lite_deinit(my_rpmsg);
        my_rpmsg = NULL;
    }
}

void app_task(void *param)
{
    volatile uint32_t remote_addr;
    void *rx_buf;
    uint32_t len;
    int32_t result;
    uint8_t gps_buf[GPS_BRIDGE_BUFFER_SIZE];

    (void)param;

    PRINTF("\r\nGNSS RPMsg bridge starting...\r\n");

#ifdef MCMGR_USED
    uint32_t startupData;

    (void)MCMGR_GetStartupData(kMCMGR_Core1, &startupData);

    my_rpmsg = rpmsg_lite_remote_init(
        (void *)startupData,
        RPMSG_LITE_LINK_ID,
        RL_NO_FLAGS);

    (void)MCMGR_SignalReady(kMCMGR_Core1);
#else
    my_rpmsg = rpmsg_lite_remote_init(
        (void *)RPMSG_LITE_SHMEM_BASE,
        RPMSG_LITE_LINK_ID,
        RL_NO_FLAGS);
#endif

    rpmsg_lite_wait_for_link_up(my_rpmsg, RL_BLOCK);

    my_queue = rpmsg_queue_create(my_rpmsg);
    assert(my_queue != NULL);

    my_ept = rpmsg_lite_create_ept(
        my_rpmsg,
        LOCAL_EPT_ADDR,
        rpmsg_queue_rx_cb,
        my_queue);

    assert(my_ept != NULL);

    result = rpmsg_ns_announce(
        my_rpmsg,
        my_ept,
        RPMSG_LITE_NS_ANNOUNCE_STRING,
        RL_NS_CREATE);

    assert(result == 0);

    BOARD_UART_Init();

    PRINTF("\r\nNameservice sent.\r\n");
    PRINTF("Write any byte/string to /dev/ttyRPMSG30 from Linux to start GNSS streaming.\r\n");

    /*
     * Learn the Linux-side endpoint address.
     *
     * On Linux, after /dev/ttyRPMSG30 appears:
     *
     *   echo start > /dev/ttyRPMSG30
     *
     * This first message tells the M33 which Linux RPMsg endpoint address
     * to stream GNSS UART bytes back to.
     */
    result = rpmsg_queue_recv_nocopy(
        my_rpmsg,
        my_queue,
        (uint32_t *)&remote_addr,
        (char **)&rx_buf,
        &len,
        RL_BLOCK);

    if (result != 0)
    {
        assert(false);
    }

    result = rpmsg_queue_nocopy_free(my_rpmsg, rx_buf);
    if (result != 0)
    {
        assert(false);
    }

    PRINTF("Linux endpoint learned. Streaming GNSS UART to RPMsg.\r\n");

    for (;;)
    {
        size_t n = BOARD_UART_GPS_Read(gps_buf, sizeof(gps_buf));

        if (n > 0U)
        {
            void *tx_buf;
            uint32_t size;

            tx_buf = rpmsg_lite_alloc_tx_buffer(my_rpmsg, &size, RL_BLOCK);
            assert(tx_buf != NULL);

            if (n > size)
            {
                n = size;
            }

            memcpy(tx_buf, gps_buf, n);

            result = rpmsg_lite_send_nocopy(
                my_rpmsg,
                my_ept,
                remote_addr,
                tx_buf,
                (uint32_t)n);

            if (result != 0)
            {
                assert(false);
            }
        }
        else
        {
            vTaskDelay(1);
        }
    }
}

void app_create_task(void)
{
    if ((app_task_handle == NULL) &&
        (xTaskCreate(
             app_task,
             "APP_TASK",
             APP_TASK_STACK_SIZE,
             NULL,
             tskIDLE_PRIORITY + 1,
             &app_task_handle) != pdPASS))
    {
        PRINTF("\r\nFailed to create application task\r\n");
        for (;;)
        {
            ;
        }
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    /*
     * Keep the stock Variscite/NXP init order.
     *
     * For the first GNSS bridge test, do not change pin_mux.c yet.
     * Linux will configure the /dev/ttyLP6 pins first, then Linux will unbind
     * 42690000.serial before this firmware is started.
     */
    BOARD_InitBootPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    /* Copy resource table to destination address. */
    copyResourceTable();

#ifdef MCMGR_USED
    (void)MCMGR_Init();
#endif

    app_create_task();
    vTaskStartScheduler();

    PRINTF("Failed to start FreeRTOS on core0.\n");

    for (;;)
    {
        ;
    }
}