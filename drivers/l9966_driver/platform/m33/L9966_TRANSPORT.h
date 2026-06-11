#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef bool (*l9966_m33_spi_transfer_fn)(
    const uint8_t *tx,
    uint8_t *rx,
    size_t len,
    void *user
);

typedef void (*l9966_m33_set_cs_active_fn)(
    bool active,
    void *user
);

typedef void (*l9966_m33_set_reset_released_fn)(
    bool released,
    void *user
);

typedef void (*l9966_m33_delay_us_fn)(
    uint32_t delay_us,
    void *user
);

typedef struct {
    l9966_m33_spi_transfer_fn spi_transfer;
    l9966_m33_set_cs_active_fn set_cs_active;
    l9966_m33_set_reset_released_fn set_reset_released;
    l9966_m33_delay_us_fn delay_us;
    void *user;
} l9966_m33_transport_config_t;

typedef struct {
    l9966_m33_spi_transfer_fn spi_transfer;
    l9966_m33_set_cs_active_fn set_cs_active;
    l9966_m33_set_reset_released_fn set_reset_released;
    l9966_m33_delay_us_fn delay_us;

    void *user;

    bool initialized;

} l9966_m33_transport_t;

bool l9966_m33_transport_init(
    l9966_m33_transport_t *transport,
    const l9966_m33_transport_config_t *config
);

bool l9966_m33_transport_transfer(
    const uint8_t *tx,
    uint8_t *rx,
    size_t len,
    void *user
);

bool l9966_m33_transport_hardware_reset(
    l9966_m33_transport_t *transport,
    uint32_t reset_hold_us,
    uint32_t startup_delay_us
);