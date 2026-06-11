#include "neo_m9v_m33_board_uart.h"

neo_m9v_result_t neo_m9v_m33_board_uart_init(
    neo_m9v_m33_board_uart_t *uart,
    neo_m9v_m33_board_uart_write_fn write_fn,
    neo_m9v_m33_board_uart_read_fn read_fn,
    void *user
)
{
    if (uart == 0 || write_fn == 0 || read_fn == 0) {
        return NEO_M9V_ERROR_NULL_ARGUMENT;
    }

    uart->write = write_fn;
    uart->read = read_fn;
    uart->user = user;
    uart->initialized = true;

    return NEO_M9V_OK;
}

bool neo_m9v_m33_board_uart_write(
    const uint8_t *data,
    size_t len,
    void *user
)
{
    neo_m9v_m33_board_uart_t *uart =
        (neo_m9v_m33_board_uart_t *)user;

    if (uart == 0) {
        return false;
    }

    if (!uart->initialized || uart->write == 0) {
        return false;
    }

    if (len == 0u) {
        return true;
    }

    if (data == 0) {
        return false;
    }

    return uart->write(
        data,
        len,
        uart->user
    );
}

bool neo_m9v_m33_board_uart_read(
    uint8_t *data,
    size_t max_len,
    size_t *bytes_read,
    uint32_t timeout_ms,
    void *user
)
{
    neo_m9v_m33_board_uart_t *uart =
        (neo_m9v_m33_board_uart_t *)user;

    if (bytes_read == 0) {
        return false;
    }

    *bytes_read = 0u;

    if (uart == 0) {
        return false;
    }

    if (!uart->initialized || uart->read == 0) {
        return false;
    }

    if (max_len == 0u) {
        return true;
    }

    if (data == 0) {
        return false;
    }

    return uart->read(
        data,
        max_len,
        bytes_read,
        timeout_ms,
        uart->user
    );
}