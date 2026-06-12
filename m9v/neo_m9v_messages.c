#include "neo_m9v_messages.h"

#include "neo_m9v_ubx_ids.h"

#include <string.h>

static uint16_t read_u16_le(const uint8_t *p)
{
    return (uint16_t)p[0] |
           ((uint16_t)p[1] << 8u);
}

static uint32_t read_u32_le(const uint8_t *p)
{
    return (uint32_t)p[0] |
           ((uint32_t)p[1] << 8u) |
           ((uint32_t)p[2] << 16u) |
           ((uint32_t)p[3] << 24u);
}

static int32_t read_i32_le(const uint8_t *p)
{
    return (int32_t)read_u32_le(p);
}

static void copy_fixed_string(
    char *dst,
    size_t dst_size,
    const uint8_t *src,
    size_t src_len
)
{
    if (dst == 0 || dst_size == 0u || src == 0) {
        return;
    }

    size_t copy_len = src_len;

    if (copy_len > dst_size - 1u) {
        copy_len = dst_size - 1u;
    }

    memcpy(dst, src, copy_len);
    dst[copy_len] = '\0';
}

neo_m9v_result_t neo_m9v_poll_nav_pvt(
    neo_m9v_t *dev,
    neo_m9v_nav_pvt_t *pvt_out,
    uint32_t timeout_ms
)
{
    if (dev == 0 || pvt_out == 0) {
        return NEO_M9V_ERROR_NULL_ARGUMENT;
    }

    neo_m9v_message_t message;

    neo_m9v_result_t result = neo_m9v_poll_ubx(
        dev,
        NEO_M9V_UBX_CLASS_NAV,
        NEO_M9V_UBX_ID_NAV_PVT,
        &message,
        timeout_ms
    );

    if (result != NEO_M9V_OK) {
        return result;
    }

    return neo_m9v_parse_nav_pvt(&message, pvt_out);
}

neo_m9v_result_t neo_m9v_parse_nav_pvt(
    const neo_m9v_message_t *message,
    neo_m9v_nav_pvt_t *pvt_out
)
{
    if (message == 0 || pvt_out == 0) {
        return NEO_M9V_ERROR_NULL_ARGUMENT;
    }

    if (message->msg_class != NEO_M9V_UBX_CLASS_NAV ||
        message->msg_id != NEO_M9V_UBX_ID_NAV_PVT) {
        return NEO_M9V_ERROR_UNEXPECTED_MESSAGE;
    }

    if (message->length < NEO_M9V_NAV_PVT_LENGTH) {
        return NEO_M9V_ERROR_BAD_PAYLOAD_LENGTH;
    }

    const uint8_t *p = message->payload;

    pvt_out->i_tow_ms = read_u32_le(&p[0]);
    pvt_out->year = read_u16_le(&p[4]);
    pvt_out->month = p[6];
    pvt_out->day = p[7];
    pvt_out->hour = p[8];
    pvt_out->minute = p[9];
    pvt_out->second = p[10];
    pvt_out->valid = p[11];

    pvt_out->fix_type = p[20];
    pvt_out->flags = p[21];
    pvt_out->num_sv = p[23];

    pvt_out->lon_1e7_deg = read_i32_le(&p[24]);
    pvt_out->lat_1e7_deg = read_i32_le(&p[28]);
    pvt_out->height_mm = read_i32_le(&p[32]);
    pvt_out->height_msl_mm = read_i32_le(&p[36]);

    pvt_out->h_acc_mm = read_u32_le(&p[40]);
    pvt_out->v_acc_mm = read_u32_le(&p[44]);

    pvt_out->ground_speed_mm_s = read_i32_le(&p[60]);
    pvt_out->heading_motion_1e5_deg = read_i32_le(&p[64]);

    return NEO_M9V_OK;
}

neo_m9v_result_t neo_m9v_poll_mon_ver(
    neo_m9v_t *dev,
    neo_m9v_mon_ver_t *ver_out,
    uint32_t timeout_ms
)
{
    if (dev == 0 || ver_out == 0) {
        return NEO_M9V_ERROR_NULL_ARGUMENT;
    }

    neo_m9v_message_t message;

    neo_m9v_result_t result = neo_m9v_poll_ubx(
        dev,
        NEO_M9V_UBX_CLASS_MON,
        NEO_M9V_UBX_ID_MON_VER,
        &message,
        timeout_ms
    );

    if (result != NEO_M9V_OK) {
        return result;
    }

    return neo_m9v_parse_mon_ver(&message, ver_out);
}

neo_m9v_result_t neo_m9v_parse_mon_ver(
    const neo_m9v_message_t *message,
    neo_m9v_mon_ver_t *ver_out
)
{
    if (message == 0 || ver_out == 0) {
        return NEO_M9V_ERROR_NULL_ARGUMENT;
    }

    if (message->msg_class != NEO_M9V_UBX_CLASS_MON ||
        message->msg_id != NEO_M9V_UBX_ID_MON_VER) {
        return NEO_M9V_ERROR_UNEXPECTED_MESSAGE;
    }

    if (message->length < NEO_M9V_MON_VER_MIN_LENGTH) {
        return NEO_M9V_ERROR_BAD_PAYLOAD_LENGTH;
    }

    size_t extension_bytes =
        (size_t)message->length - NEO_M9V_MON_VER_MIN_LENGTH;

    if ((extension_bytes % NEO_M9V_MON_VER_EXTENSION_LENGTH) != 0u) {
        return NEO_M9V_ERROR_BAD_PAYLOAD_LENGTH;
    }

    memset(ver_out, 0, sizeof(*ver_out));

    const uint8_t *p = message->payload;

    copy_fixed_string(
        ver_out->sw_version,
        sizeof(ver_out->sw_version),
        &p[0],
        NEO_M9V_MON_VER_SW_VERSION_LENGTH
    );

    copy_fixed_string(
        ver_out->hw_version,
        sizeof(ver_out->hw_version),
        &p[NEO_M9V_MON_VER_SW_VERSION_LENGTH],
        NEO_M9V_MON_VER_HW_VERSION_LENGTH
    );

    size_t reported_extensions =
        extension_bytes / NEO_M9V_MON_VER_EXTENSION_LENGTH;

    size_t extensions_to_copy = reported_extensions;

    if (extensions_to_copy > NEO_M9V_MON_VER_MAX_EXTENSIONS) {
        extensions_to_copy = NEO_M9V_MON_VER_MAX_EXTENSIONS;
    }

    const uint8_t *extension_base = &p[NEO_M9V_MON_VER_MIN_LENGTH];

    for (size_t i = 0u; i < extensions_to_copy; ++i) {
        copy_fixed_string(
            ver_out->extensions[i],
            sizeof(ver_out->extensions[i]),
            &extension_base[i * NEO_M9V_MON_VER_EXTENSION_LENGTH],
            NEO_M9V_MON_VER_EXTENSION_LENGTH
        );
    }

    ver_out->extension_count = extensions_to_copy;

    return NEO_M9V_OK;
}

neo_m9v_result_t neo_m9v_parse_ack(
    const neo_m9v_message_t *message,
    neo_m9v_ack_t *ack_out
)
{
    if (message == 0 || ack_out == 0) {
        return NEO_M9V_ERROR_NULL_ARGUMENT;
    }

    if (message->msg_class != NEO_M9V_UBX_CLASS_ACK) {
        return NEO_M9V_ERROR_UNEXPECTED_MESSAGE;
    }

    if (message->msg_id != NEO_M9V_UBX_ID_ACK_ACK &&
        message->msg_id != NEO_M9V_UBX_ID_ACK_NAK) {
        return NEO_M9V_ERROR_UNEXPECTED_MESSAGE;
    }

    if (message->length != NEO_M9V_ACK_PAYLOAD_LENGTH) {
        return NEO_M9V_ERROR_BAD_PAYLOAD_LENGTH;
    }

    ack_out->acked = message->msg_id == NEO_M9V_UBX_ID_ACK_ACK;
    ack_out->acked_msg_class = message->payload[0];
    ack_out->acked_msg_id = message->payload[1];

    return NEO_M9V_OK;
}