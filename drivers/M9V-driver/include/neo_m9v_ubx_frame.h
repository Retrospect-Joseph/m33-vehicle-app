#pragma once
#include <stddef.h>
#include <stdint.h>

#include "neo_m9v.h"

#define NEO_M9V_UBX_SYNC_LENGTH 2u
#define NEO_M9V_UBX_HEADER_LENGTH 6u
#define NEO_M9V_UBX_CHECKSUM_LENGTH 2u
#define NEO_M9V_UBX_FRAME_OVERHEAD 8u
#define NEO_M9V_UBX_MAX_FRAME_LENGTH (NEO_M9V_UBX_FRAME_OVERHEAD + NEO_M9V_MAX_PAYLOAD_LENGTH)

neo_m9v_result_t neo_m9v_ubx_checksum_calculate(
    uint8_t msg_class,
    uint8_t msg_id,
    const uint8_t *payload,
    uint16_t payload_len,
    uint8_t *ck_a_out,
    uint8_t *ck_b_out
);

neo_m9v_result_t neo_m9v_ubx_frame_encoded_length(
    uint16_t payload_len,
    size_t *frame_len_out
);

neo_m9v_result_t neo_m9v_ubx_frame_encode(
    uint8_t msg_class,
    uint8_t msg_id,
    const uint8_t *payload,
    uint16_t payload_len,
    uint8_t *frame_out,
    size_t frame_out_size,
    size_t *frame_len_out
);

neo_m9v_result_t neo_m9v_ubx_frame_decode(
    const uint8_t *frame,
    size_t frame_len,
    neo_m9v_message_t *message_out
);

