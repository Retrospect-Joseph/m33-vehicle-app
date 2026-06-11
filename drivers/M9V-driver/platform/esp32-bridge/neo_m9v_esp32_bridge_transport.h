#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "esp32_bridge_serial.h"

bool neo_m9v_esp32_bridge_write(
    const uint8_t *data,
    size_t len,
    void *user
);

bool neo_m9v_esp32_bridge_read(
    uint8_t *data,
    size_t max_len,
    size_t *bytes_read,
    uint32_t timeout_ms,
    void *user
);
