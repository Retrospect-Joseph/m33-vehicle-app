#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "neo_m9v.h"
// MON-VER
// ACK-ACK / ACK-NAK
// NAV-PVT
// NAV-STATUS
// NAV-SAT
// NAV-SIG
// NAV-ATT
// NAV-PVAT
// ESF-INS
// ESF-STATUS

#define NEO_M9V_NAV_PVT_LENGTH 92u
#define NEO_M9V_ACK_PAYLOAD_LENGTH 2u
#define NEO_M9V_MON_VER_SW_VERSION_LENGTH 30u
#define NEO_M9V_MON_VER_HW_VERSION_LENGTH 10u
#define NEO_M9V_MON_VER_EXTENSION_LENGTH 30u

#define NEO_M9V_MON_VER_SW_VERSION_STRING_SIZE (NEO_M9V_MON_VER_SW_VERSION_LENGTH + 1u)
#define NEO_M9V_MON_VER_HW_VERSION_STRING_SIZE (NEO_M9V_MON_VER_HW_VERSION_LENGTH + 1u)
#define NEO_M9V_MON_VER_EXTENSION_STRING_SIZE (NEO_M9V_MON_VER_EXTENSION_LENGTH + 1u)

#define NEO_M9V_MON_VER_MIN_LENGTH 40u
#define NEO_M9V_MON_VER_MAX_EXTENSIONS 10u

typedef struct 
{
    uint32_t i_tow_ms;
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t valid;
    uint8_t fix_type;
    uint8_t flags;
    uint8_t num_sv;
    int32_t lon_1e7_deg;
    int32_t lat_1e7_deg;
    int32_t height_mm;
    int32_t height_msl_mm;
    uint32_t h_acc_mm;
    uint32_t v_acc_mm;
    int32_t ground_speed_mm_s;
    int32_t heading_motion_1e5_deg;
} neo_m9v_nav_pvt_t;

typedef struct 
{
    char sw_version[32];
    char hw_version[16];
    char extensions[NEO_M9V_MON_VER_MAX_EXTENSIONS][NEO_M9V_MON_VER_EXTENSION_LENGTH + 1u];
    size_t extension_count;
} neo_m9v_mon_ver_t;

typedef struct 
{
    bool acked;
    uint8_t acked_msg_class;
    uint8_t acked_msg_id;
} neo_m9v_ack_t;

neo_m9v_result_t neo_m9v_poll_nav_pvt(
    neo_m9v_t *dev,
    neo_m9v_nav_pvt_t *pvt_out,
    uint32_t timeout_ms
);

neo_m9v_result_t neo_m9v_parse_nav_pvt(
    const neo_m9v_message_t *message,
    neo_m9v_nav_pvt_t *pvt_out
);

neo_m9v_result_t neo_m9v_poll_mon_ver(
    neo_m9v_t *dev,
    neo_m9v_mon_ver_t *ver_out,
    uint32_t timeout_ms
);

neo_m9v_result_t neo_m9v_parse_mon_ver(
    const neo_m9v_message_t *message,
    neo_m9v_mon_ver_t *ver_out
);

neo_m9v_result_t neo_m9v_parse_ack(
    const neo_m9v_message_t *message,
    neo_m9v_ack_t *ack_out
);