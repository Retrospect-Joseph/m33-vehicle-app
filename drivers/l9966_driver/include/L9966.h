#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "L9966_FRAME.h"

typedef enum {
    L9966_OK = 0,

    L9966_ERROR_NULL_ARGUMENT,
    L9966_ERROR_NOT_INITIALIZED,
    L9966_ERROR_BAD_LENGTH,

    L9966_ERROR_SPI_TRANSFER_FAILED,

    L9966_ERROR_FRAME_BUILD_FAILED,
    L9966_ERROR_FRAME_PARSE_FAILED,

    L9966_ERROR_RESPONSE_FIXED_PATTERN,
    L9966_ERROR_RESPONSE_TRANSFER_FAILED,
    L9966_ERROR_RESPONSE_REGISTER_ECHO,
    L9966_ERROR_RESPONSE_DATA_PARITY,

    L9966_ERROR_VERIFY_FAILED
} l9966_result_t;


typedef bool (*l9966_spi_transfer_fn)(
    const uint8_t *tx,
    uint8_t *rx,
    size_t len,
    void *user
);

typedef struct {
    l9966_chip_addr_t chip_addr;
    l9966_spi_transfer_fn spi_transfer;
    void *user;
    bool initialized;
} l9966_t;

l9966_result_t l9966_init(
    l9966_t *dev,
    l9966_chip_addr_t chip_addr,
    l9966_spi_transfer_fn spi_transfer,
    void *user
);

l9966_result_t l9966_transfer_raw(
    l9966_t *dev,
    const uint8_t *tx,
    uint8_t *rx,
    size_t len
);

l9966_result_t l9966_read_register(
    l9966_t *dev,
    uint8_t register_addr,
    l9966_response_t *response_out
);

l9966_result_t l9966_write_register(
    l9966_t *dev,
    uint8_t register_addr,
    uint16_t data_15bit,
    l9966_response_t *response_out
);

l9966_result_t l9966_write_register_verify(
    l9966_t *dev,
    uint8_t register_addr,
    uint16_t data_15bit,
    uint16_t verify_mask
);

const char *l9966_result_to_string(l9966_result_t result);