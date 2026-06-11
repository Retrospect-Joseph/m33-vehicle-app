#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define NEO_M9V_MAX_PAYLOAD_LENGTH 1024u

typedef enum {
    NEO_M9V_OK = 0,
    NEO_M9V_ERROR_NULL_ARGUMENT,
    NEO_M9V_ERROR_NOT_INITIALIZED,
    NEO_M9V_ERROR_TRANSPORT_WRITE_FAILED,
    NEO_M9V_ERROR_TRANSPORT_READ_FAILED,
    NEO_M9V_ERROR_TIMEOUT,
    NEO_M9V_ERROR_PAYLOAD_TOO_LARGE,
    NEO_M9V_ERROR_BAD_SYNC,
    NEO_M9V_ERROR_BAD_CHECKSUM,
    NEO_M9V_ERROR_UNEXPECTED_MESSAGE,
    NEO_M9V_ERROR_BAD_PAYLOAD_LENGTH,
    NEO_M9V_ERROR_ACK_NAK
} neo_m9v_result_t;

typedef bool (*neo_m9v_write_fn)(
    const uint8_t *data,
    size_t len,
    void *user
);

typedef bool (*neo_m9v_read_fn)(
    uint8_t *data,
    size_t max_len,
    size_t *bytes_read,
    uint32_t timeout_ms,
    void *user
);

typedef struct {
    neo_m9v_write_fn write;
    neo_m9v_read_fn read;
    void *user;
    bool initialized;
} neo_m9v_t;

typedef struct {
    uint8_t msg_class;
    uint8_t msg_id;
    uint16_t length;
    uint8_t payload[NEO_M9V_MAX_PAYLOAD_LENGTH];
} neo_m9v_message_t;

neo_m9v_result_t neo_m9v_init(
    neo_m9v_t *dev,
    neo_m9v_write_fn write_fn,
    neo_m9v_read_fn read_fn,
    void *user
);

neo_m9v_result_t neo_m9v_send_ubx(
    neo_m9v_t *dev,
    uint8_t msg_class,
    uint8_t msg_id,
    const uint8_t *payload,
    uint16_t payload_len
);

neo_m9v_result_t neo_m9v_read_ubx(
    neo_m9v_t *dev,
    neo_m9v_message_t *message_out,
    uint32_t timeout_ms
);

neo_m9v_result_t neo_m9v_poll_ubx(
    neo_m9v_t *dev,
    uint8_t msg_class,
    uint8_t msg_id,
    neo_m9v_message_t *message_out,
    uint32_t timeout_ms
);

const char *neo_m9v_result_to_string(neo_m9v_result_t result);