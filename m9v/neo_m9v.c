#include "neo_m9v.h"

#include "neo_m9v_ubx_frame.h"
#include "neo_m9v_ubx_ids.h"

#define NEO_M9V_SYNC_SEARCH_LIMIT_BYTES 4096u
#define NEO_M9V_POLL_MAX_ATTEMPTS 20u

static uint16_t read_u16_le(const uint8_t *p)
{
    return (uint16_t)p[0] |
           ((uint16_t)p[1] << 8u);
}

static neo_m9v_result_t read_some(
    neo_m9v_t *dev,
    uint8_t *data,
    size_t max_len,
    size_t *bytes_read,
    uint32_t timeout_ms
)
{
    if (dev == 0 || data == 0 || bytes_read == 0) {
        return NEO_M9V_ERROR_NULL_ARGUMENT;
    }

    if (!dev->initialized || dev->read == 0) {
        return NEO_M9V_ERROR_NOT_INITIALIZED;
    }

    bool ok = dev->read(
        data,
        max_len,
        bytes_read,
        timeout_ms,
        dev->user
    );

    if (!ok) {
        return NEO_M9V_ERROR_TRANSPORT_READ_FAILED;
    }

    return NEO_M9V_OK;
}

static neo_m9v_result_t read_exact(
    neo_m9v_t *dev,
    uint8_t *data,
    size_t len,
    uint32_t timeout_ms
)
{
    size_t total = 0u;

    while (total < len) {
        size_t got = 0u;

        neo_m9v_result_t result = read_some(
            dev,
            &data[total],
            len - total,
            &got,
            timeout_ms
        );

        if (result != NEO_M9V_OK) {
            return result;
        }

        if (got == 0u) {
            return NEO_M9V_ERROR_TIMEOUT;
        }

        total += got;
    }

    return NEO_M9V_OK;
}

neo_m9v_result_t neo_m9v_init(
    neo_m9v_t *dev,
    neo_m9v_write_fn write_fn,
    neo_m9v_read_fn read_fn,
    void *user
)
{
    if (dev == 0 || write_fn == 0 || read_fn == 0) {
        return NEO_M9V_ERROR_NULL_ARGUMENT;
    }

    dev->write = write_fn;
    dev->read = read_fn;
    dev->user = user;
    dev->initialized = true;

    return NEO_M9V_OK;
}

neo_m9v_result_t neo_m9v_send_ubx(
    neo_m9v_t *dev,
    uint8_t msg_class,
    uint8_t msg_id,
    const uint8_t *payload,
    uint16_t payload_len
)
{
    if (dev == 0) {
        return NEO_M9V_ERROR_NULL_ARGUMENT;
    }

    if (!dev->initialized || dev->write == 0) {
        return NEO_M9V_ERROR_NOT_INITIALIZED;
    }

    uint8_t frame[NEO_M9V_UBX_MAX_FRAME_LENGTH];
    size_t frame_len = 0u;

    neo_m9v_result_t result = neo_m9v_ubx_frame_encode(
        msg_class,
        msg_id,
        payload,
        payload_len,
        frame,
        sizeof(frame),
        &frame_len
    );

    if (result != NEO_M9V_OK) {
        return result;
    }

    bool ok = dev->write(
        frame,
        frame_len,
        dev->user
    );

    if (!ok) {
        return NEO_M9V_ERROR_TRANSPORT_WRITE_FAILED;
    }

    return NEO_M9V_OK;
}

neo_m9v_result_t neo_m9v_read_ubx(
    neo_m9v_t *dev,
    neo_m9v_message_t *message_out,
    uint32_t timeout_ms
)
{
    if (dev == 0 || message_out == 0) {
        return NEO_M9V_ERROR_NULL_ARGUMENT;
    }

    if (!dev->initialized || dev->read == 0) {
        return NEO_M9V_ERROR_NOT_INITIALIZED;
    }

    uint8_t frame[NEO_M9V_UBX_MAX_FRAME_LENGTH];

    uint8_t previous = 0u;
    uint8_t current = 0u;
    bool found_sync = false;

    /*
     * The receiver may output NMEA or other bytes before the UBX packet.
     * Search for the UBX sync sequence B5 62, but do not search forever.
     */
    for (uint32_t scanned = 0u;
         scanned < NEO_M9V_SYNC_SEARCH_LIMIT_BYTES;
         ++scanned) {
        neo_m9v_result_t result = read_exact(
            dev,
            &current,
            1u,
            timeout_ms
        );

        if (result != NEO_M9V_OK) {
            return result;
        }

        if (previous == NEO_M9V_UBX_SYNC_1 &&
            current == NEO_M9V_UBX_SYNC_2) {
            frame[0] = NEO_M9V_UBX_SYNC_1;
            frame[1] = NEO_M9V_UBX_SYNC_2;
            found_sync = true;
            break;
        }

        previous = current;
    }

    if (!found_sync) {
        return NEO_M9V_ERROR_TIMEOUT;
    }

    /*
     * We already have the two sync bytes. Read the remaining four bytes of the
     * UBX header:
     *
     *   class, id, length_l, length_h
     */
    neo_m9v_result_t result = read_exact(
        dev,
        &frame[NEO_M9V_UBX_SYNC_LENGTH],
        NEO_M9V_UBX_HEADER_LENGTH - NEO_M9V_UBX_SYNC_LENGTH,
        timeout_ms
    );

    if (result != NEO_M9V_OK) {
        return result;
    }

    uint16_t payload_len = read_u16_le(&frame[4]);

    if (payload_len > NEO_M9V_MAX_PAYLOAD_LENGTH) {
        return NEO_M9V_ERROR_PAYLOAD_TOO_LARGE;
    }

    size_t remaining_len =
        (size_t)payload_len + NEO_M9V_UBX_CHECKSUM_LENGTH;

    result = read_exact(
        dev,
        &frame[NEO_M9V_UBX_HEADER_LENGTH],
        remaining_len,
        timeout_ms
    );

    if (result != NEO_M9V_OK) {
        return result;
    }

    size_t frame_len = NEO_M9V_UBX_FRAME_OVERHEAD + payload_len;

    return neo_m9v_ubx_frame_decode(
        frame,
        frame_len,
        message_out
    );
}

neo_m9v_result_t neo_m9v_poll_ubx(
    neo_m9v_t *dev,
    uint8_t msg_class,
    uint8_t msg_id,
    neo_m9v_message_t *message_out,
    uint32_t timeout_ms
)
{
    if (dev == 0 || message_out == 0) {
        return NEO_M9V_ERROR_NULL_ARGUMENT;
    }

    neo_m9v_result_t result = neo_m9v_send_ubx(
        dev,
        msg_class,
        msg_id,
        0,
        0u
    );

    if (result != NEO_M9V_OK) {
        return result;
    }

    for (uint32_t attempts = 0u;
         attempts < NEO_M9V_POLL_MAX_ATTEMPTS;
         ++attempts) {
        result = neo_m9v_read_ubx(
            dev,
            message_out,
            timeout_ms
        );

        if (result != NEO_M9V_OK) {
            return result;
        }

        if (message_out->msg_class == msg_class &&
            message_out->msg_id == msg_id) {
            return NEO_M9V_OK;
        }

        if (message_out->msg_class == NEO_M9V_UBX_CLASS_ACK &&
            message_out->msg_id == NEO_M9V_UBX_ID_ACK_NAK &&
            message_out->length >= 2u &&
            message_out->payload[0] == msg_class &&
            message_out->payload[1] == msg_id) {
            return NEO_M9V_ERROR_ACK_NAK;
        }
    }

    return NEO_M9V_ERROR_UNEXPECTED_MESSAGE;
}

const char *neo_m9v_result_to_string(neo_m9v_result_t result)
{
    switch (result) {
        case NEO_M9V_OK:
            return "NEO_M9V_OK";

        case NEO_M9V_ERROR_NULL_ARGUMENT:
            return "NEO_M9V_ERROR_NULL_ARGUMENT";

        case NEO_M9V_ERROR_NOT_INITIALIZED:
            return "NEO_M9V_ERROR_NOT_INITIALIZED";

        case NEO_M9V_ERROR_TRANSPORT_WRITE_FAILED:
            return "NEO_M9V_ERROR_TRANSPORT_WRITE_FAILED";

        case NEO_M9V_ERROR_TRANSPORT_READ_FAILED:
            return "NEO_M9V_ERROR_TRANSPORT_READ_FAILED";

        case NEO_M9V_ERROR_TIMEOUT:
            return "NEO_M9V_ERROR_TIMEOUT";

        case NEO_M9V_ERROR_PAYLOAD_TOO_LARGE:
            return "NEO_M9V_ERROR_PAYLOAD_TOO_LARGE";

        case NEO_M9V_ERROR_BAD_SYNC:
            return "NEO_M9V_ERROR_BAD_SYNC";

        case NEO_M9V_ERROR_BAD_CHECKSUM:
            return "NEO_M9V_ERROR_BAD_CHECKSUM";

        case NEO_M9V_ERROR_UNEXPECTED_MESSAGE:
            return "NEO_M9V_ERROR_UNEXPECTED_MESSAGE";

        case NEO_M9V_ERROR_BAD_PAYLOAD_LENGTH:
            return "NEO_M9V_ERROR_BAD_PAYLOAD_LENGTH";

        case NEO_M9V_ERROR_ACK_NAK:
            return "NEO_M9V_ERROR_ACK_NAK";

        default:
            return "NEO_M9V_ERROR_UNKNOWN";
    }
}