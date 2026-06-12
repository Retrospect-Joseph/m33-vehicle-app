#include "neo_m9v_config.h"

#include "neo_m9v_messages.h"
#include "neo_m9v_ubx_ids.h"

#define NEO_M9V_CFG_VALSET_VERSION 0u
#define NEO_M9V_CFG_VALGET_VERSION 0u

#define NEO_M9V_CFG_VALSET_TRANSACTION_NONE 0u

#define NEO_M9V_CFG_VALSET_HEADER_LENGTH 4u
#define NEO_M9V_CFG_VALGET_HEADER_LENGTH 4u
#define NEO_M9V_CFG_KEY_LENGTH 4u

#define NEO_M9V_ACK_WAIT_MAX_MESSAGES 20u
#define NEO_M9V_CFG_VALGET_MAX_MESSAGES 20u

static void write_u16_le(uint8_t *p, uint16_t value)
{
    p[0] = (uint8_t)(value & 0xFFu);
    p[1] = (uint8_t)((value >> 8u) & 0xFFu);
}

static void write_u32_le(uint8_t *p, uint32_t value)
{
    p[0] = (uint8_t)(value & 0xFFu);
    p[1] = (uint8_t)((value >> 8u) & 0xFFu);
    p[2] = (uint8_t)((value >> 16u) & 0xFFu);
    p[3] = (uint8_t)((value >> 24u) & 0xFFu);
}

static neo_m9v_result_t cfg_valset_one(
    neo_m9v_t *dev,
    uint32_t key,
    const uint8_t *value,
    uint8_t value_len,
    uint8_t layers,
    uint32_t timeout_ms
)
{
    if (dev == 0 || value == 0) {
        return NEO_M9V_ERROR_NULL_ARGUMENT;
    }

    if (layers == 0u) {
        return NEO_M9V_ERROR_NULL_ARGUMENT;
    }

    uint8_t payload[
        NEO_M9V_CFG_VALSET_HEADER_LENGTH +
        NEO_M9V_CFG_KEY_LENGTH +
        4u
    ];

    payload[0] = NEO_M9V_CFG_VALSET_VERSION;
    payload[1] = layers;
    payload[2] = NEO_M9V_CFG_VALSET_TRANSACTION_NONE;
    payload[3] = 0u;

    write_u32_le(&payload[4], key);

    for (uint8_t i = 0u; i < value_len; ++i) {
        payload[8u + i] = value[i];
    }

    uint16_t payload_len =
        (uint16_t)(NEO_M9V_CFG_VALSET_HEADER_LENGTH +
                   NEO_M9V_CFG_KEY_LENGTH +
                   value_len);

    neo_m9v_result_t result = neo_m9v_send_ubx(
        dev,
        NEO_M9V_UBX_CLASS_CFG,
        NEO_M9V_UBX_ID_CFG_VALSET,
        payload,
        payload_len
    );

    if (result != NEO_M9V_OK) {
        return result;
    }

    return neo_m9v_wait_for_ack(
        dev,
        NEO_M9V_UBX_CLASS_CFG,
        NEO_M9V_UBX_ID_CFG_VALSET,
        timeout_ms
    );
}

neo_m9v_result_t neo_m9v_wait_for_ack(
    neo_m9v_t *dev,
    uint8_t acked_msg_class,
    uint8_t acked_msg_id,
    uint32_t timeout_ms
)
{
    if (dev == 0) {
        return NEO_M9V_ERROR_NULL_ARGUMENT;
    }

    for (uint32_t i = 0u; i < NEO_M9V_ACK_WAIT_MAX_MESSAGES; ++i) {
        neo_m9v_message_t message;

        neo_m9v_result_t result = neo_m9v_read_ubx(
            dev,
            &message,
            timeout_ms
        );

        if (result != NEO_M9V_OK) {
            return result;
        }

        if (message.msg_class != NEO_M9V_UBX_CLASS_ACK) {
            continue;
        }

        neo_m9v_ack_t ack;

        result = neo_m9v_parse_ack(&message, &ack);

        if (result != NEO_M9V_OK) {
            return result;
        }

        if (ack.acked_msg_class != acked_msg_class ||
            ack.acked_msg_id != acked_msg_id) {
            continue;
        }

        if (ack.acked) {
            return NEO_M9V_OK;
        }

        return NEO_M9V_ERROR_ACK_NAK;
    }

    return NEO_M9V_ERROR_TIMEOUT;
}

neo_m9v_result_t neo_m9v_cfg_valset_u1(
    neo_m9v_t *dev,
    uint32_t key,
    uint8_t value,
    uint8_t layers,
    uint32_t timeout_ms
)
{
    return cfg_valset_one(
        dev,
        key,
        &value,
        1u,
        layers,
        timeout_ms
    );
}

neo_m9v_result_t neo_m9v_cfg_valset_u2(
    neo_m9v_t *dev,
    uint32_t key,
    uint16_t value,
    uint8_t layers,
    uint32_t timeout_ms
)
{
    uint8_t value_bytes[2];

    write_u16_le(value_bytes, value);

    return cfg_valset_one(
        dev,
        key,
        value_bytes,
        sizeof(value_bytes),
        layers,
        timeout_ms
    );
}

neo_m9v_result_t neo_m9v_cfg_valset_u4(
    neo_m9v_t *dev,
    uint32_t key,
    uint32_t value,
    uint8_t layers,
    uint32_t timeout_ms
)
{
    uint8_t value_bytes[4];

    write_u32_le(value_bytes, value);

    return cfg_valset_one(
        dev,
        key,
        value_bytes,
        sizeof(value_bytes),
        layers,
        timeout_ms
    );
}

neo_m9v_result_t neo_m9v_cfg_valset_bool(
    neo_m9v_t *dev,
    uint32_t key,
    bool value,
    uint8_t layers,
    uint32_t timeout_ms
)
{
    uint8_t value_byte = value ? 1u : 0u;

    return cfg_valset_one(
        dev,
        key,
        &value_byte,
        1u,
        layers,
        timeout_ms
    );
}

neo_m9v_result_t neo_m9v_cfg_valget(
    neo_m9v_t *dev,
    uint8_t layer,
    uint32_t key,
    neo_m9v_message_t *message_out,
    uint32_t timeout_ms
)
{
    if (dev == 0 || message_out == 0) {
        return NEO_M9V_ERROR_NULL_ARGUMENT;
    }

    uint8_t payload[
        NEO_M9V_CFG_VALGET_HEADER_LENGTH +
        NEO_M9V_CFG_KEY_LENGTH
    ];

    payload[0] = NEO_M9V_CFG_VALGET_VERSION;
    payload[1] = layer;

    /*
     * Position is used when reading large configuration sets in chunks.
     * For a single-key request, use position 0.
     */
    write_u16_le(&payload[2], 0u);

    write_u32_le(&payload[4], key);

    neo_m9v_result_t result = neo_m9v_send_ubx(
        dev,
        NEO_M9V_UBX_CLASS_CFG,
        NEO_M9V_UBX_ID_CFG_VALGET,
        payload,
        sizeof(payload)
    );

    if (result != NEO_M9V_OK) {
        return result;
    }

    for (uint32_t i = 0u; i < NEO_M9V_CFG_VALGET_MAX_MESSAGES; ++i) {
        result = neo_m9v_read_ubx(
            dev,
            message_out,
            timeout_ms
        );

        if (result != NEO_M9V_OK) {
            return result;
        }

        if (message_out->msg_class == NEO_M9V_UBX_CLASS_CFG &&
            message_out->msg_id == NEO_M9V_UBX_ID_CFG_VALGET) {
            return NEO_M9V_OK;
        }

        if (message_out->msg_class == NEO_M9V_UBX_CLASS_ACK) {
            neo_m9v_ack_t ack;

            result = neo_m9v_parse_ack(message_out, &ack);

            if (result != NEO_M9V_OK) {
                return result;
            }

            if (ack.acked_msg_class == NEO_M9V_UBX_CLASS_CFG &&
                ack.acked_msg_id == NEO_M9V_UBX_ID_CFG_VALGET &&
                !ack.acked) {
                return NEO_M9V_ERROR_ACK_NAK;
            }
        }
    }

    return NEO_M9V_ERROR_TIMEOUT;
}