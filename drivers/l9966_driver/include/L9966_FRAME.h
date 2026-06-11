#pragma once

#include <stdbool.h>
#include <stdint.h>


typedef enum
{
    L9966_CHIP_ADDR_10 = 0x02u,
    L9966_CHIP_ADDR_11 = 0x03u
} l9966_chip_addr_t;

typedef enum
{
    L9966_FRAME_OK = 0,

    L9966_FRAME_ERROR_NULL_ARGUMENT,
    L9966_FRAME_ERROR_BAD_CHIP_ADDRESS,
    L9966_FRAME_ERROR_BAD_REGISTER_ADDRESS,
    L9966_FRAME_ERROR_BAD_WRITE_VALUE
} l9966_frame_result_t;

typedef struct
{
    uint32_t raw;
    uint8_t fixed_pattern;
    bool fixed_pattern_ok;
    bool transfer_failed;
    uint8_t register_echo;
    uint16_t data_word;
    uint16_t data;
    bool data_parity_ok;
} l9966_response_t;

l9966_frame_result_t l9966_build_read_frame(
    l9966_chip_addr_t chip_addr, 
    uint8_t register_addr, 
    uint32_t *frame_out
);

l9966_frame_result_t l9966_build_write_frame(
    l9966_chip_addr_t chip_addr,
    uint8_t register_addr,
    uint16_t data_15bit,
    uint32_t *frame_out
);

l9966_frame_result_t l9966_parse_response(
    uint32_t raw_response,
    l9966_response_t *response_out
);