#include "L9966_TRANSPORT.h"

bool l9966_m33_transport_init(
    l9966_m33_transport_t *transport,
    const l9966_m33_transport_config_t *config
)
{
    if (transport == 0 || config == 0) {
        return false;
    }

    if (config->spi_transfer == 0) {
        return false;
    }

    transport->spi_transfer = config->spi_transfer;
    transport->set_cs_active = config->set_cs_active;
    transport->set_reset_released = config->set_reset_released;
    transport->delay_us = config->delay_us;
    transport->user = config->user;
    transport->initialized = true;

    return true;
}

bool l9966_m33_transport_transfer(
    const uint8_t *tx,
    uint8_t *rx,
    size_t len,
    void *user
)
{
    l9966_m33_transport_t *transport = (l9966_m33_transport_t *)user;

    if (transport == 0 || tx == 0 || rx == 0) {
        return false;
    }

    if (!transport->initialized || transport->spi_transfer == 0) {
        return false;
    }

    if (len == 0u) {
        return false;
    }

    if (transport->set_cs_active != 0) {
        transport->set_cs_active(true, transport->user);
    }

    bool ok = transport->spi_transfer(
        tx,
        rx,
        len,
        transport->user
    );

    if (transport->set_cs_active != 0) {
        transport->set_cs_active(false, transport->user);
    }

    return ok;
}

bool l9966_m33_transport_hardware_reset(
    l9966_m33_transport_t *transport,
    uint32_t reset_hold_us,
    uint32_t startup_delay_us
)
{
    if (transport == 0) {
        return false;
    }

    if (!transport->initialized) {
        return false;
    }

    if (transport->set_reset_released == 0 || transport->delay_us == 0) {
        return false;
    }

    transport->set_reset_released(false, transport->user);
    transport->delay_us(reset_hold_us, transport->user);

    transport->set_reset_released(true, transport->user);
    transport->delay_us(startup_delay_us, transport->user);

    return true;
}