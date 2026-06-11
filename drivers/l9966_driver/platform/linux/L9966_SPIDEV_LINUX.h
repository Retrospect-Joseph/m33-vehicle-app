#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define L9966_LINUX_SPIDEV_DEFAULT_SPEED_HZ      100000u
#define L9966_LINUX_SPIDEV_DEFAULT_BITS_PER_WORD 8u
#define L9966_LINUX_SPIDEV_DEFAULT_DELAY_USECS   1u

typedef struct {
    const char *device_path;
    uint32_t speed_hz;
    uint8_t bits_per_word;
    uint16_t delay_usecs;

} l9966_linux_spidev_config_t;

typedef struct {
    int fd;
    uint32_t speed_hz;
    uint8_t bits_per_word;
    uint16_t delay_usecs;
    bool opened;
} l9966_linux_spidev_t;

bool l9966_linux_spidev_open(
    l9966_linux_spidev_t *transport,
    const l9966_linux_spidev_config_t *config
);

void l9966_linux_spidev_close(
    l9966_linux_spidev_t *transport
);

bool l9966_linux_spidev_transfer(
    const uint8_t *tx,
    uint8_t *rx,
    size_t len,
    void *user
);

bool l9966_linux_spidev_is_open(
    const l9966_linux_spidev_t *transport
);