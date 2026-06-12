#include "neo_m9v_ubx_frame.h"

#include "neo_m9v_ubx_ids.h"

#include <string.h>

static uint16_t read_u16_le(const uint8_t *p)
{
    return (uint16_t)p[0] |
           ((uint16_t)p[1] << 8u);
}

static void write_u16_le(uint8_t *p, uint16_t value)
{
    p[0] = (uint8_t)(value & 0xFFu);
    p[1] = (uint8_t)((value >> 8u) & 0xFFu);
}

static void checksum_update(
    uint8_t byte,
    uint8_t *ck_a,
    uint8_t *ck_b
)
{
    *ck_a = (uint8_t)(*ck_a + byte);
    *ck_b = (uint8_t)(*ck_b + *ck_a);
}

neo_m9v_result_t neo_m9v_ubx_checksum_calculate(
    uint8_t msg_class,
    uint8_t msg_id,
    const uint8_t *payload,
    uint16_t payload_len,
    uint8_t *ck_a_out,
    uint8_t *ck_b_out
)
{
    if (ck_a_out == 0 || ck_b_out == 0) {
        return NEO_M9V_ERROR_NULL_ARGUMENT;
    }

    if (payload_len > 0u && payload == 0) {
        return NEO_M9V_ERROR_NULL_ARGUMENT;
    }

    if (payload_len > NEO_M9V_MAX_PAYLOAD_LENGTH) {
        return NEO_M9V_ERROR_PAYLOAD_TOO_LARGE;
    }

    uint8_t ck_a = 0u;
    uint8_t ck_b = 0u;

    checksum_update(msg_class, &ck_a, &ck_b);
    checksum_update(msg_id, &ck_a, &ck_b);
    checksum_update((uint8_t)(payload_len & 0xFFu), &ck_a, &ck_b);
    checksum_update((uint8_t)((payload_len >> 8u) & 0xFFu), &ck_a, &ck_b);

    for (uint16_t i = 0u; i < payload_len; ++i) {
        checksum_update(payload[i], &ck_a, &ck_b);
    }

    *ck_a_out = ck_a;
    *ck_b_out = ck_b;

    return NEO_M9V_OK;
}

neo_m9v_result_t neo_m9v_ubx_frame_encoded_length(
    uint16_t payload_len,
    size_t *frame_len_out
)
{
    if (frame_len_out == 0) {
        return NEO_M9V_ERROR_NULL_ARGUMENT;
    }

    if (payload_len > NEO_M9V_MAX_PAYLOAD_LENGTH) {
        return NEO_M9V_ERROR_PAYLOAD_TOO_LARGE;
    }

    *frame_len_out = (size_t)NEO_M9V_UBX_FRAME_OVERHEAD + payload_len;

    return NEO_M9V_OK;
}

neo_m9v_result_t neo_m9v_ubx_frame_encode(
    uint8_t msg_class,
    uint8_t msg_id,
    const uint8_t *payload,
    uint16_t payload_len,
    uint8_t *frame_out,
    size_t frame_out_size,
    size_t *frame_len_out
)
{
    if (frame_out == 0 || frame_len_out == 0) {
        return NEO_M9V_ERROR_NULL_ARGUMENT;
    }

    if (payload_len > 0u && payload == 0) {
        return NEO_M9V_ERROR_NULL_ARGUMENT;
    }

    size_t required_len = 0u;

    neo_m9v_result_t result = neo_m9v_ubx_frame_encoded_length(
        payload_len,
        &required_len
    );

    if (result != NEO_M9V_OK) {
        return result;
    }

    if (frame_out_size < required_len) {
        return NEO_M9V_ERROR_PAYLOAD_TOO_LARGE;
    }

    frame_out[0] = NEO_M9V_UBX_SYNC_1;
    frame_out[1] = NEO_M9V_UBX_SYNC_2;
    frame_out[2] = msg_class;
    frame_out[3] = msg_id;

    write_u16_le(&frame_out[4], payload_len);

    if (payload_len > 0u) {
        memcpy(&frame_out[NEO_M9V_UBX_HEADER_LENGTH], payload, payload_len);
    }

    uint8_t ck_a = 0u;
    uint8_t ck_b = 0u;

    result = neo_m9v_ubx_checksum_calculate(
        msg_class,
        msg_id,
        payload,
        payload_len,
        &ck_a,
        &ck_b
    );

    if (result != NEO_M9V_OK) {
        return result;
    }

    frame_out[NEO_M9V_UBX_HEADER_LENGTH + payload_len] = ck_a;
    frame_out[NEO_M9V_UBX_HEADER_LENGTH + payload_len + 1u] = ck_b;

    *frame_len_out = required_len;

    return NEO_M9V_OK;
}

neo_m9v_result_t neo_m9v_ubx_frame_decode(
    const uint8_t *frame,
    size_t frame_len,
    neo_m9v_message_t *message_out
)
{
    if (frame == 0 || message_out == 0) {
        return NEO_M9V_ERROR_NULL_ARGUMENT;
    }

    if (frame_len < NEO_M9V_UBX_FRAME_OVERHEAD) {
        return NEO_M9V_ERROR_BAD_PAYLOAD_LENGTH;
    }

    if (frame[0] != NEO_M9V_UBX_SYNC_1 ||
        frame[1] != NEO_M9V_UBX_SYNC_2) {
        return NEO_M9V_ERROR_BAD_SYNC;
    }

    uint8_t msg_class = frame[2];
    uint8_t msg_id = frame[3];
    uint16_t payload_len = read_u16_le(&frame[4]);

    if (payload_len > NEO_M9V_MAX_PAYLOAD_LENGTH) {
        return NEO_M9V_ERROR_PAYLOAD_TOO_LARGE;
    }

    size_t expected_frame_len =
        (size_t)NEO_M9V_UBX_FRAME_OVERHEAD + payload_len;

    if (frame_len != expected_frame_len) {
        return NEO_M9V_ERROR_BAD_PAYLOAD_LENGTH;
    }

    const uint8_t *payload = &frame[NEO_M9V_UBX_HEADER_LENGTH];

    uint8_t calc_ck_a = 0u;
    uint8_t calc_ck_b = 0u;

    neo_m9v_result_t result = neo_m9v_ubx_checksum_calculate(
        msg_class,
        msg_id,
        payload,
        payload_len,
        &calc_ck_a,
        &calc_ck_b
    );

    if (result != NEO_M9V_OK) {
        return result;
    }

    uint8_t received_ck_a = frame[NEO_M9V_UBX_HEADER_LENGTH + payload_len];
    uint8_t received_ck_b = frame[NEO_M9V_UBX_HEADER_LENGTH + payload_len + 1u];

    if (received_ck_a != calc_ck_a || received_ck_b != calc_ck_b) {
        return NEO_M9V_ERROR_BAD_CHECKSUM;
    }

    message_out->msg_class = msg_class;
    message_out->msg_id = msg_id;
    message_out->length = payload_len;

    if (payload_len > 0u) {
        memcpy(message_out->payload, payload, payload_len);
    }

    return NEO_M9V_OK;
}