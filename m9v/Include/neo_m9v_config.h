#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "neo_m9v.h"

#define NEO_M9V_CFG_LAYER_RAM    0x01u
#define NEO_M9V_CFG_LAYER_BBR    0x02u
#define NEO_M9V_CFG_LAYER_FLASH  0x04u

#define NEO_M9V_CFG_VALGET_LAYER_RAM      0x00u
#define NEO_M9V_CFG_VALGET_LAYER_BBR      0x01u
#define NEO_M9V_CFG_VALGET_LAYER_FLASH    0x02u
#define NEO_M9V_CFG_VALGET_LAYER_DEFAULT  0x07u

neo_m9v_result_t neo_m9v_wait_for_ack(
    neo_m9v_t *dev,
    uint8_t acked_msg_class,
    uint8_t acked_msg_id,
    uint32_t timeout_ms
);

neo_m9v_result_t neo_m9v_cfg_valset_u1(
    neo_m9v_t *dev,
    uint32_t key,
    uint8_t value,
    uint8_t layers,
    uint32_t timeout_ms
);

neo_m9v_result_t neo_m9v_cfg_valset_u2(
    neo_m9v_t *dev,
    uint32_t key,
    uint16_t value,
    uint8_t layers,
    uint32_t timeout_ms
);

neo_m9v_result_t neo_m9v_cfg_valset_u4(
    neo_m9v_t *dev,
    uint32_t key,
    uint32_t value,
    uint8_t layers,
    uint32_t timeout_ms
);

neo_m9v_result_t neo_m9v_cfg_valset_bool(
    neo_m9v_t *dev,
    uint32_t key,
    bool value,
    uint8_t layers,
    uint32_t timeout_ms
);

neo_m9v_result_t neo_m9v_cfg_valget(
    neo_m9v_t *dev,
    uint8_t layer,
    uint32_t key,
    neo_m9v_message_t *message_out,
    uint32_t timeout_ms
);