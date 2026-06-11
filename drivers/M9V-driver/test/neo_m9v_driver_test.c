#include "neo_m9v.h"
#include "neo_m9v_cfg_keys.h"
#include "neo_m9v_config.h"
#include "neo_m9v_messages.h"
#include "neo_m9v_ubx_frame.h"
#include "neo_m9v_ubx_ids.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define TEST_BUFFER_SIZE 8192u

#define CHECK(condition)                                                       \
    do {                                                                       \
        if (!(condition)) {                                                    \
            printf("FAIL: %s:%d: %s\n", __FILE__, __LINE__, #condition);       \
            return false;                                                      \
        }                                                                      \
    } while (0)

typedef struct {
    uint8_t rx[TEST_BUFFER_SIZE];
    size_t rx_len;
    size_t rx_pos;

    uint8_t tx[TEST_BUFFER_SIZE];
    size_t tx_len;
} fake_transport_t;

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

static uint32_t read_u32_le(const uint8_t *p)
{
    return (uint32_t)p[0] |
           ((uint32_t)p[1] << 8u) |
           ((uint32_t)p[2] << 16u) |
           ((uint32_t)p[3] << 24u);
}

static void write_i32_le(uint8_t *p, int32_t value)
{
    write_u32_le(p, (uint32_t)value);
}

static bool fake_write(
    const uint8_t *data,
    size_t len,
    void *user
)
{
    fake_transport_t *fake = (fake_transport_t *)user;

    if (fake == 0 || data == 0) {
        return false;
    }

    if ((fake->tx_len + len) > TEST_BUFFER_SIZE) {
        return false;
    }

    memcpy(&fake->tx[fake->tx_len], data, len);
    fake->tx_len += len;

    return true;
}

static bool fake_read(
    uint8_t *data,
    size_t max_len,
    size_t *bytes_read,
    uint32_t timeout_ms,
    void *user
)
{
    fake_transport_t *fake = (fake_transport_t *)user;

    (void)timeout_ms;

    if (fake == 0 || data == 0 || bytes_read == 0) {
        return false;
    }

    *bytes_read = 0u;

    if (max_len == 0u) {
        return true;
    }

    if (fake->rx_pos >= fake->rx_len) {
        return true;
    }

    size_t available = fake->rx_len - fake->rx_pos;
    size_t to_copy = available < max_len ? available : max_len;

    memcpy(data, &fake->rx[fake->rx_pos], to_copy);

    fake->rx_pos += to_copy;
    *bytes_read = to_copy;

    return true;
}

static void fake_clear(fake_transport_t *fake)
{
    memset(fake, 0, sizeof(*fake));
}

static bool fake_append_bytes(
    fake_transport_t *fake,
    const uint8_t *data,
    size_t len
)
{
    if (fake == 0 || data == 0) {
        return false;
    }

    if ((fake->rx_len + len) > TEST_BUFFER_SIZE) {
        return false;
    }

    memcpy(&fake->rx[fake->rx_len], data, len);
    fake->rx_len += len;

    return true;
}

static bool fake_append_ubx(
    fake_transport_t *fake,
    uint8_t msg_class,
    uint8_t msg_id,
    const uint8_t *payload,
    uint16_t payload_len
)
{
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
        return false;
    }

    return fake_append_bytes(fake, frame, frame_len);
}

static void make_ack_payload(
    uint8_t *payload,
    uint8_t acked_msg_class,
    uint8_t acked_msg_id
)
{
    payload[0] = acked_msg_class;
    payload[1] = acked_msg_id;
}

static void make_nav_pvt_payload(uint8_t *payload)
{
    memset(payload, 0, NEO_M9V_NAV_PVT_LENGTH);

    write_u32_le(&payload[0], 123456u);
    write_u16_le(&payload[4], 2026u);

    payload[6] = 6u;
    payload[7] = 4u;
    payload[8] = 12u;
    payload[9] = 34u;
    payload[10] = 56u;
    payload[11] = 0x03u;

    payload[20] = 3u;
    payload[21] = 0x01u;
    payload[23] = 12u;

    write_i32_le(&payload[24], -844000000);
    write_i32_le(&payload[28], 337500000);
    write_i32_le(&payload[32], 250000);
    write_i32_le(&payload[36], 240000);

    write_u32_le(&payload[40], 1500u);
    write_u32_le(&payload[44], 2500u);

    write_i32_le(&payload[60], 2682);
    write_i32_le(&payload[64], 9000000);
}

static void make_mon_ver_payload(
    uint8_t *payload,
    uint16_t *payload_len_out
)
{
    const char sw[] = "EXT CORE 3.01";
    const char hw[] = "00190000";
    const char ext0[] = "PROTVER=35.16";
    const char ext1[] = "MOD=NEO-M9V";

    memset(payload, 0, 100u);

    memcpy(&payload[0], sw, sizeof(sw));
    memcpy(&payload[NEO_M9V_MON_VER_SW_VERSION_LENGTH], hw, sizeof(hw));

    memcpy(
        &payload[NEO_M9V_MON_VER_MIN_LENGTH],
        ext0,
        sizeof(ext0)
    );

    memcpy(
        &payload[NEO_M9V_MON_VER_MIN_LENGTH +
                 NEO_M9V_MON_VER_EXTENSION_LENGTH],
        ext1,
        sizeof(ext1)
    );

    *payload_len_out =
        NEO_M9V_MON_VER_MIN_LENGTH +
        (2u * NEO_M9V_MON_VER_EXTENSION_LENGTH);
}

static bool test_frame_encode_decode(void)
{
    uint8_t frame[NEO_M9V_UBX_MAX_FRAME_LENGTH];
    size_t frame_len = 0u;

    neo_m9v_result_t result = neo_m9v_ubx_frame_encode(
        NEO_M9V_UBX_CLASS_MON,
        NEO_M9V_UBX_ID_MON_VER,
        0,
        0u,
        frame,
        sizeof(frame),
        &frame_len
    );

    CHECK(result == NEO_M9V_OK);
    CHECK(frame_len == NEO_M9V_UBX_FRAME_OVERHEAD);
    CHECK(frame[0] == NEO_M9V_UBX_SYNC_1);
    CHECK(frame[1] == NEO_M9V_UBX_SYNC_2);
    CHECK(frame[2] == NEO_M9V_UBX_CLASS_MON);
    CHECK(frame[3] == NEO_M9V_UBX_ID_MON_VER);

    neo_m9v_message_t message;

    result = neo_m9v_ubx_frame_decode(
        frame,
        frame_len,
        &message
    );

    CHECK(result == NEO_M9V_OK);
    CHECK(message.msg_class == NEO_M9V_UBX_CLASS_MON);
    CHECK(message.msg_id == NEO_M9V_UBX_ID_MON_VER);
    CHECK(message.length == 0u);

    return true;
}

static bool test_read_ubx_skips_nmea(void)
{
    fake_transport_t fake;
    fake_clear(&fake);

    const char nmea[] =
        "$GNTXT,01,01,02,u-blox test sentence*00\r\n";

    CHECK(fake_append_bytes(
        &fake,
        (const uint8_t *)nmea,
        strlen(nmea)
    ));

    uint8_t ack_payload[NEO_M9V_ACK_PAYLOAD_LENGTH];
    make_ack_payload(
        ack_payload,
        NEO_M9V_UBX_CLASS_CFG,
        NEO_M9V_UBX_ID_CFG_VALSET
    );

    CHECK(fake_append_ubx(
        &fake,
        NEO_M9V_UBX_CLASS_ACK,
        NEO_M9V_UBX_ID_ACK_ACK,
        ack_payload,
        sizeof(ack_payload)
    ));

    neo_m9v_t dev;

    neo_m9v_result_t result = neo_m9v_init(
        &dev,
        fake_write,
        fake_read,
        &fake
    );

    CHECK(result == NEO_M9V_OK);

    neo_m9v_message_t message;

    result = neo_m9v_read_ubx(
        &dev,
        &message,
        1000u
    );

    CHECK(result == NEO_M9V_OK);
    CHECK(message.msg_class == NEO_M9V_UBX_CLASS_ACK);
    CHECK(message.msg_id == NEO_M9V_UBX_ID_ACK_ACK);
    CHECK(message.length == NEO_M9V_ACK_PAYLOAD_LENGTH);

    return true;
}

static bool test_poll_nav_pvt(void)
{
    fake_transport_t fake;
    fake_clear(&fake);

    uint8_t nav_payload[NEO_M9V_NAV_PVT_LENGTH];
    make_nav_pvt_payload(nav_payload);

    CHECK(fake_append_ubx(
        &fake,
        NEO_M9V_UBX_CLASS_NAV,
        NEO_M9V_UBX_ID_NAV_PVT,
        nav_payload,
        sizeof(nav_payload)
    ));

    neo_m9v_t dev;

    neo_m9v_result_t result = neo_m9v_init(
        &dev,
        fake_write,
        fake_read,
        &fake
    );

    CHECK(result == NEO_M9V_OK);

    neo_m9v_nav_pvt_t pvt;

    result = neo_m9v_poll_nav_pvt(
        &dev,
        &pvt,
        1000u
    );

    CHECK(result == NEO_M9V_OK);

    CHECK(fake.tx_len == NEO_M9V_UBX_FRAME_OVERHEAD);
    CHECK(fake.tx[2] == NEO_M9V_UBX_CLASS_NAV);
    CHECK(fake.tx[3] == NEO_M9V_UBX_ID_NAV_PVT);

    CHECK(pvt.i_tow_ms == 123456u);
    CHECK(pvt.year == 2026u);
    CHECK(pvt.month == 6u);
    CHECK(pvt.day == 4u);
    CHECK(pvt.fix_type == 3u);
    CHECK(pvt.num_sv == 12u);
    CHECK(pvt.lon_1e7_deg == -844000000);
    CHECK(pvt.lat_1e7_deg == 337500000);
    CHECK(pvt.ground_speed_mm_s == 2682);
    CHECK(pvt.heading_motion_1e5_deg == 9000000);

    return true;
}

static bool test_poll_mon_ver(void)
{
    fake_transport_t fake;
    fake_clear(&fake);

    uint8_t mon_payload[128];
    uint16_t mon_payload_len = 0u;

    make_mon_ver_payload(mon_payload, &mon_payload_len);

    CHECK(fake_append_ubx(
        &fake,
        NEO_M9V_UBX_CLASS_MON,
        NEO_M9V_UBX_ID_MON_VER,
        mon_payload,
        mon_payload_len
    ));

    neo_m9v_t dev;

    neo_m9v_result_t result = neo_m9v_init(
        &dev,
        fake_write,
        fake_read,
        &fake
    );

    CHECK(result == NEO_M9V_OK);

    neo_m9v_mon_ver_t ver;

    result = neo_m9v_poll_mon_ver(
        &dev,
        &ver,
        1000u
    );

    CHECK(result == NEO_M9V_OK);
    CHECK(strcmp(ver.sw_version, "EXT CORE 3.01") == 0);
    CHECK(strcmp(ver.hw_version, "00190000") == 0);
    CHECK(ver.extension_count == 2u);
    CHECK(strcmp(ver.extensions[0], "PROTVER=35.16") == 0);
    CHECK(strcmp(ver.extensions[1], "MOD=NEO-M9V") == 0);

    return true;
}

static bool test_cfg_valset_u1_ack(void)
{
    fake_transport_t fake;
    fake_clear(&fake);

    uint8_t ack_payload[NEO_M9V_ACK_PAYLOAD_LENGTH];

    make_ack_payload(
        ack_payload,
        NEO_M9V_UBX_CLASS_CFG,
        NEO_M9V_UBX_ID_CFG_VALSET
    );

    CHECK(fake_append_ubx(
        &fake,
        NEO_M9V_UBX_CLASS_ACK,
        NEO_M9V_UBX_ID_ACK_ACK,
        ack_payload,
        sizeof(ack_payload)
    ));

    neo_m9v_t dev;

    neo_m9v_result_t result = neo_m9v_init(
        &dev,
        fake_write,
        fake_read,
        &fake
    );

    CHECK(result == NEO_M9V_OK);

    result = neo_m9v_cfg_valset_u1(
        &dev,
        NEO_M9V_CFG_MSGOUT_UBX_NAV_PVT_UART1,
        1u,
        NEO_M9V_CFG_LAYER_RAM,
        1000u
    );

    CHECK(result == NEO_M9V_OK);

    neo_m9v_message_t sent;

    result = neo_m9v_ubx_frame_decode(
        fake.tx,
        fake.tx_len,
        &sent
    );

    CHECK(result == NEO_M9V_OK);
    CHECK(sent.msg_class == NEO_M9V_UBX_CLASS_CFG);
    CHECK(sent.msg_id == NEO_M9V_UBX_ID_CFG_VALSET);

    CHECK(sent.length == 9u);
    CHECK(sent.payload[0] == 0u);
    CHECK(sent.payload[1] == NEO_M9V_CFG_LAYER_RAM);
    CHECK(sent.payload[2] == 0u);
    CHECK(sent.payload[3] == 0u);

    CHECK(read_u32_le(&sent.payload[4]) ==
          NEO_M9V_CFG_MSGOUT_UBX_NAV_PVT_UART1);

    CHECK(sent.payload[8] == 1u);

    return true;
}

static bool test_cfg_valget_response(void)
{
    fake_transport_t fake;
    fake_clear(&fake);

    uint8_t response_payload[9];

    response_payload[0] = 0u;
    response_payload[1] = NEO_M9V_CFG_VALGET_LAYER_RAM;
    response_payload[2] = 0u;
    response_payload[3] = 0u;

    write_u32_le(
        &response_payload[4],
        NEO_M9V_CFG_MSGOUT_UBX_NAV_PVT_UART1
    );

    response_payload[8] = 1u;

    CHECK(fake_append_ubx(
        &fake,
        NEO_M9V_UBX_CLASS_CFG,
        NEO_M9V_UBX_ID_CFG_VALGET,
        response_payload,
        sizeof(response_payload)
    ));

    neo_m9v_t dev;

    neo_m9v_result_t result = neo_m9v_init(
        &dev,
        fake_write,
        fake_read,
        &fake
    );

    CHECK(result == NEO_M9V_OK);

    neo_m9v_message_t response;

    result = neo_m9v_cfg_valget(
        &dev,
        NEO_M9V_CFG_VALGET_LAYER_RAM,
        NEO_M9V_CFG_MSGOUT_UBX_NAV_PVT_UART1,
        &response,
        1000u
    );

    CHECK(result == NEO_M9V_OK);
    CHECK(response.msg_class == NEO_M9V_UBX_CLASS_CFG);
    CHECK(response.msg_id == NEO_M9V_UBX_ID_CFG_VALGET);
    CHECK(response.length == sizeof(response_payload));
    CHECK(read_u32_le(&response.payload[4]) ==
          NEO_M9V_CFG_MSGOUT_UBX_NAV_PVT_UART1);
    CHECK(response.payload[8] == 1u);

    return true;
}

static bool run_test(
    const char *name,
    bool (*test_fn)(void)
)
{
    printf("Running %s...\n", name);

    if (!test_fn()) {
        printf("%s: FAIL\n", name);
        return false;
    }

    printf("%s: PASS\n", name);
    return true;
}

int main(void)
{
    unsigned int passed = 0u;
    unsigned int total = 0u;

#define RUN(test_fn)                                                           \
    do {                                                                       \
        ++total;                                                               \
        if (run_test(#test_fn, test_fn)) {                                     \
            ++passed;                                                          \
        }                                                                      \
    } while (0)

    RUN(test_frame_encode_decode);
    RUN(test_read_ubx_skips_nmea);
    RUN(test_poll_nav_pvt);
    RUN(test_poll_mon_ver);
    RUN(test_cfg_valset_u1_ack);
    RUN(test_cfg_valget_response);

#undef RUN

    printf("\n%u/%u tests passed\n", passed, total);

    return passed == total ? 0 : 1;
}