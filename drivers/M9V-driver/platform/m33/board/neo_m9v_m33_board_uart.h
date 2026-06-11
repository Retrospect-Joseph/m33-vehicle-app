#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "neo_m9v.h"

typedef bool (*neo_m9v_m33_board_uart_write_fn)(
    const uint8_t *data,
    size_t len,
    void *user
);

typedef bool (*neo_m9v_m33_board_uart_read_fn)(
    uint8_t *data,
    size_t max_len,
    size_t *bytes_read,
    uint32_t timeout_ms,
    void *user
);

typedef struct {
    neo_m9v_m33_board_uart_write_fn write;
    neo_m9v_m33_board_uart_read_fn read;
    void *user;
    bool initialized;
} neo_m9v_m33_board_uart_t;

neo_m9v_result_t neo_m9v_m33_board_uart_init(
    neo_m9v_m33_board_uart_t *uart,
    neo_m9v_m33_board_uart_write_fn write_fn,
    neo_m9v_m33_board_uart_read_fn read_fn,
    void *user
);

bool neo_m9v_m33_board_uart_write(
    const uint8_t *data,
    size_t len,
    void *user
);

bool neo_m9v_m33_board_uart_read(
    uint8_t *data,
    size_t max_len,
    size_t *bytes_read,
    uint32_t timeout_ms,
    void *user
);