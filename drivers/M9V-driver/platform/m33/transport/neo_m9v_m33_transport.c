#include "neo_m9v_m33_transport.h"

neo_m9v_result_t neo_m9v_m33_transport_init(
    neo_m9v_m33_transport_t *transport,
    neo_m9v_m33_uart_write_fn uart_write,
    neo_m9v_m33_uart_read_fn uart_read,
    void *uart_user
)
{
    if (transport == 0 || uart_write == 0 || uart_read == 0) {
        return NEO_M9V_ERROR_NULL_ARGUMENT;
    }

    transport->uart_write = uart_write;
    transport->uart_read = uart_read;
    transport->uart_user = uart_user;
    transport->initialized = true;

    return NEO_M9V_OK;
}

neo_m9v_result_t neo_m9v_m33_attach(
    neo_m9v_t *dev,
    neo_m9v_m33_transport_t *transport
)
{
    if (dev == 0 || transport == 0) {
        return NEO_M9V_ERROR_NULL_ARGUMENT;
    }

    if (!transport->initialized ||
        transport->uart_write == 0 ||
        transport->uart_read == 0) {
        return NEO_M9V_ERROR_NOT_INITIALIZED;
    }

    return neo_m9v_init(
        dev,
        neo_m9v_m33_transport_write,
        neo_m9v_m33_transport_read,
        transport
    );
}

bool neo_m9v_m33_transport_write(
    const uint8_t *data,
    size_t len,
    void *user
)
{
    neo_m9v_m33_transport_t *transport =
        (neo_m9v_m33_transport_t *)user;

    if (transport == 0 || data == 0) {
        return false;
    }

    if (!transport->initialized || transport->uart_write == 0) {
        return false;
    }

    return transport->uart_write(
        data,
        len,
        transport->uart_user
    );
}

bool neo_m9v_m33_transport_read(
    uint8_t *data,
    size_t max_len,
    size_t *bytes_read,
    uint32_t timeout_ms,
    void *user
)
{
    neo_m9v_m33_transport_t *transport =
        (neo_m9v_m33_transport_t *)user;

    if (transport == 0 || data == 0 || bytes_read == 0) {
        return false;
    }

    *bytes_read = 0u;

    if (max_len == 0u) {
        return true;
    }

    if (!transport->initialized || transport->uart_read == 0) {
        return false;
    }

    return transport->uart_read(
        data,
        max_len,
        bytes_read,
        timeout_ms,
        transport->uart_user
    );
}