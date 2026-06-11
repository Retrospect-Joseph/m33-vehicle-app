#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "neo_m9v.h"

typedef bool (*neo_m9v_m33_uart_write_fn)(
    const uint8_t *data,
    size_t len,
    void *user
);

typedef bool (*neo_m9v_m33_uart_read_fn)(
    uint8_t *data,
    size_t max_len,
    size_t *bytes_read,
    uint32_t timeout_ms,
    void *user
);

typedef struct {
    neo_m9v_m33_uart_write_fn uart_write;
    neo_m9v_m33_uart_read_fn uart_read;
    void *uart_user;
    bool initialized;
} neo_m9v_m33_transport_t;

neo_m9v_result_t neo_m9v_m33_transport_init(
    neo_m9v_m33_transport_t *transport,
    neo_m9v_m33_uart_write_fn uart_write,
    neo_m9v_m33_uart_read_fn uart_read,
    void *uart_user
);

neo_m9v_result_t neo_m9v_m33_attach(
    neo_m9v_t *dev,
    neo_m9v_m33_transport_t *transport
);

bool neo_m9v_m33_transport_write(
    const uint8_t *data,
    size_t len,
    void *user
);

bool neo_m9v_m33_transport_read(
    uint8_t *data,
    size_t max_len,
    size_t *bytes_read,
    uint32_t timeout_ms,
    void *user
);