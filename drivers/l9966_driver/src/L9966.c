#include "L9966.h"

#define L9966_FRAME_BYTE_LENGTH 4u

static void pack_u32_be(uint32_t value, uint8_t bytes[4])
{
    bytes[0] = (uint8_t)((value >> 24u) & 0xFFu);
    bytes[1] = (uint8_t)((value >> 16u) & 0xFFu);
    bytes[2] = (uint8_t)((value >> 8u) & 0xFFu);
    bytes[3] = (uint8_t)(value & 0xFFu);
}

static uint32_t unpack_u32_be(const uint8_t bytes[4])
{
    return ((uint32_t)bytes[0] << 24u) |
           ((uint32_t)bytes[1] << 16u) |
           ((uint32_t)bytes[2] << 8u) |
           ((uint32_t)bytes[3]);
}

static l9966_result_t validate_response(
    const l9966_response_t *response,
    uint8_t expected_register_addr
)
{
    if (response == 0) {
        return L9966_ERROR_NULL_ARGUMENT;
    }

    if (!response->fixed_pattern_ok) {
        return L9966_ERROR_RESPONSE_FIXED_PATTERN;
    }

    if (response->transfer_failed) {
        return L9966_ERROR_RESPONSE_TRANSFER_FAILED;
    }

    if (response->register_echo != expected_register_addr) {
        return L9966_ERROR_RESPONSE_REGISTER_ECHO;
    }

    if (!response->data_parity_ok) {
        return L9966_ERROR_RESPONSE_DATA_PARITY;
    }

    return L9966_OK;
}

l9966_result_t l9966_init(
    l9966_t *dev,
    l9966_chip_addr_t chip_addr,
    l9966_spi_transfer_fn spi_transfer,
    void *user
)
{
    if (dev == 0 || spi_transfer == 0) {
        return L9966_ERROR_NULL_ARGUMENT;
    }

    dev->chip_addr = chip_addr;
    dev->spi_transfer = spi_transfer;
    dev->user = user;
    dev->initialized = true;

    return L9966_OK;
}

l9966_result_t l9966_transfer_raw(
    l9966_t *dev,
    const uint8_t *tx,
    uint8_t *rx,
    size_t len
)
{
    if (dev == 0 || tx == 0 || rx == 0) {
        return L9966_ERROR_NULL_ARGUMENT;
    }

    if (!dev->initialized || dev->spi_transfer == 0) {
        return L9966_ERROR_NOT_INITIALIZED;
    }

    if (len == 0u) {
        return L9966_ERROR_BAD_LENGTH;
    }

    bool ok = dev->spi_transfer(tx, rx, len, dev->user);

    if (!ok) {
        return L9966_ERROR_SPI_TRANSFER_FAILED;
    }

    return L9966_OK;
}

l9966_result_t l9966_read_register(
    l9966_t *dev,
    uint8_t register_addr,
    l9966_response_t *response_out
)
{
    if (dev == 0 || response_out == 0) {
        return L9966_ERROR_NULL_ARGUMENT;
    }

    if (!dev->initialized || dev->spi_transfer == 0) {
        return L9966_ERROR_NOT_INITIALIZED;
    }

    uint32_t frame = 0u;

    l9966_frame_result_t frame_result = l9966_build_read_frame(
        dev->chip_addr,
        register_addr,
        &frame
    );

    if (frame_result != L9966_FRAME_OK) {
        return L9966_ERROR_FRAME_BUILD_FAILED;
    }

    uint8_t tx[L9966_FRAME_BYTE_LENGTH] = {0u};
    uint8_t rx[L9966_FRAME_BYTE_LENGTH] = {0u};

    pack_u32_be(frame, tx);

    l9966_result_t transfer_result = l9966_transfer_raw(
        dev,
        tx,
        rx,
        L9966_FRAME_BYTE_LENGTH
    );

    if (transfer_result != L9966_OK) {
        return transfer_result;
    }

    uint32_t raw_response = unpack_u32_be(rx);

    frame_result = l9966_parse_response(raw_response, response_out);

    if (frame_result != L9966_FRAME_OK) {
        return L9966_ERROR_FRAME_PARSE_FAILED;
    }

    return validate_response(response_out, register_addr);
}

l9966_result_t l9966_write_register(
    l9966_t *dev,
    uint8_t register_addr,
    uint16_t data_15bit,
    l9966_response_t *response_out
)
{
    if (dev == 0 || response_out == 0) {
        return L9966_ERROR_NULL_ARGUMENT;
    }

    if (!dev->initialized || dev->spi_transfer == 0) {
        return L9966_ERROR_NOT_INITIALIZED;
    }

    uint32_t frame = 0u;

    l9966_frame_result_t frame_result = l9966_build_write_frame(
        dev->chip_addr,
        register_addr,
        data_15bit,
        &frame
    );

    if (frame_result != L9966_FRAME_OK) {
        return L9966_ERROR_FRAME_BUILD_FAILED;
    }

    uint8_t tx[L9966_FRAME_BYTE_LENGTH] = {0u};
    uint8_t rx[L9966_FRAME_BYTE_LENGTH] = {0u};

    pack_u32_be(frame, tx);

    l9966_result_t transfer_result = l9966_transfer_raw(
        dev,
        tx,
        rx,
        L9966_FRAME_BYTE_LENGTH
    );

    if (transfer_result != L9966_OK) {
        return transfer_result;
    }

    uint32_t raw_response = unpack_u32_be(rx);

    frame_result = l9966_parse_response(raw_response, response_out);

    if (frame_result != L9966_FRAME_OK) {
        return L9966_ERROR_FRAME_PARSE_FAILED;
    }

    return validate_response(response_out, register_addr);
}

l9966_result_t l9966_write_register_verify(
    l9966_t *dev,
    uint8_t register_addr,
    uint16_t data_15bit,
    uint16_t verify_mask
)
{
    if (dev == 0) {
        return L9966_ERROR_NULL_ARGUMENT;
    }

    l9966_response_t write_response;

    l9966_result_t result = l9966_write_register(
        dev,
        register_addr,
        data_15bit,
        &write_response
    );

    if (result != L9966_OK) {
        return result;
    }

    l9966_response_t read_response;

    result = l9966_read_register(
        dev,
        register_addr,
        &read_response
    );

    if (result != L9966_OK) {
        return result;
    }

    if ((read_response.data & verify_mask) != (data_15bit & verify_mask)) {
        return L9966_ERROR_VERIFY_FAILED;
    }

    return L9966_OK;
}

const char *l9966_result_to_string(l9966_result_t result)
{
    switch (result)
    {
        case L9966_OK:
            return "L9966_OK";

        case L9966_ERROR_NULL_ARGUMENT:
            return "L9966_ERROR_NULL_ARGUMENT";

        case L9966_ERROR_NOT_INITIALIZED:
            return "L9966_ERROR_NOT_INITIALIZED";

        case L9966_ERROR_BAD_LENGTH:
            return "L9966_ERROR_BAD_LENGTH";

        case L9966_ERROR_SPI_TRANSFER_FAILED:
            return "L9966_ERROR_SPI_TRANSFER_FAILED";

        case L9966_ERROR_FRAME_BUILD_FAILED:
            return "L9966_ERROR_FRAME_BUILD_FAILED";

        case L9966_ERROR_FRAME_PARSE_FAILED:
            return "L9966_ERROR_FRAME_PARSE_FAILED";

        case L9966_ERROR_RESPONSE_FIXED_PATTERN:
            return "L9966_ERROR_RESPONSE_FIXED_PATTERN";

        case L9966_ERROR_RESPONSE_TRANSFER_FAILED:
            return "L9966_ERROR_RESPONSE_TRANSFER_FAILED";

        case L9966_ERROR_RESPONSE_REGISTER_ECHO:
            return "L9966_ERROR_RESPONSE_REGISTER_ECHO";

        case L9966_ERROR_RESPONSE_DATA_PARITY:
            return "L9966_ERROR_RESPONSE_DATA_PARITY";

        case L9966_ERROR_VERIFY_FAILED:
            return "L9966_ERROR_VERIFY_FAILED";

        default:
            return "L9966_ERROR_UNKNOWN";
    }
}