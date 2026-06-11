#pragma once

#include <stdint.h>

/* ============================================================
 * Status / identification registers
 * ============================================================ */
#define L9966_REG_GEN_STATUS  0x01u
#define L9966_REG_DEV_V       0x02u
#define L9966_REG_HW_REV      0x03u
#define L9966_REG_DEV_ID      0x04u


/* ============================================================
 * Current source / comparator / channel configuration
 *
 * CURR_SRC_CTRL_1 through CURR_SRC_CTRL_15 correspond to IO1
 * through IO15.
 * ============================================================ */
#define L9966_REG_CURR_SRC_CTRL_1   0x21u
#define L9966_REG_CURR_SRC_CTRL_2   0x22u
#define L9966_REG_CURR_SRC_CTRL_3   0x23u
#define L9966_REG_CURR_SRC_CTRL_4   0x24u
#define L9966_REG_CURR_SRC_CTRL_5   0x25u
#define L9966_REG_CURR_SRC_CTRL_6   0x26u
#define L9966_REG_CURR_SRC_CTRL_7   0x27u
#define L9966_REG_CURR_SRC_CTRL_8   0x28u
#define L9966_REG_CURR_SRC_CTRL_9   0x29u
#define L9966_REG_CURR_SRC_CTRL_10  0x2Au
#define L9966_REG_CURR_SRC_CTRL_11  0x2Bu
#define L9966_REG_CURR_SRC_CTRL_12  0x2Cu
#define L9966_REG_CURR_SRC_CTRL_13  0x2Du
#define L9966_REG_CURR_SRC_CTRL_14  0x2Eu
#define L9966_REG_CURR_SRC_CTRL_15  0x2Fu


/* ============================================================
 * Routing / special-function configuration
 * ============================================================ */
#define L9966_REG_SWITCH_ROUTE               0x30u
#define L9966_REG_DWT_VOLT_SRC_LSF_CTRL      0x31u
#define L9966_REG_DIG_IN_STAT_LTC            0x33u
#define L9966_REG_GTM_TO_SENT_ROUTE_1_2      0x34u
#define L9966_REG_GTM_TO_SENT_ROUTE_3_4      0x35u
#define L9966_REG_ACTIVE_DISCHARGE_LSF_CTRL  0x36u


/* ============================================================
 * Wake / sleep / reset
 * ============================================================ */
#define L9966_REG_WAK_MSK       0x40u
#define L9966_REG_SLEEP_CONFIG  0x41u
#define L9966_REG_WAK_CONFIG    0x42u
#define L9966_REG_SOFT_RST_CMD  0x43u


/* ============================================================
 * VRS
 * ============================================================ */
#define L9966_REG_VRS  0x51u


/* ============================================================
 * ADC / single conversion / sequencer control
 * ============================================================ */
#define L9966_REG_SQNCR_INT_MSK_FLG  0x80u
#define L9966_REG_SC_CONF            0x81u
#define L9966_REG_ADC_TIMING         0x82u
#define L9966_REG_SC_RESULT          0x83u


/* ============================================================
 * Sequencer command registers
 * ============================================================ */
#define L9966_REG_SQNCR_CMD_1   0xC1u
#define L9966_REG_SQNCR_CMD_2   0xC2u
#define L9966_REG_SQNCR_CMD_3   0xC3u
#define L9966_REG_SQNCR_CMD_4   0xC4u
#define L9966_REG_SQNCR_CMD_5   0xC5u
#define L9966_REG_SQNCR_CMD_6   0xC6u
#define L9966_REG_SQNCR_CMD_7   0xC7u
#define L9966_REG_SQNCR_CMD_8   0xC8u
#define L9966_REG_SQNCR_CMD_9   0xC9u
#define L9966_REG_SQNCR_CMD_10  0xCAu
#define L9966_REG_SQNCR_CMD_11  0xCBu
#define L9966_REG_SQNCR_CMD_12  0xCCu
#define L9966_REG_SQNCR_CMD_13  0xCDu
#define L9966_REG_SQNCR_CMD_14  0xCEu
#define L9966_REG_SQNCR_CMD_15  0xCFu


/* ============================================================
 * Sequencer control / result-copy
 * ============================================================ */
#define L9966_REG_SQNCR_CTRL           0xD0u
#define L9966_REG_SQNCR_RSLT_COPY_CMD  0xDFu


/* ============================================================
 * Digital input status
 * ============================================================ */
#define L9966_REG_DIG_IN_STAT  0xE0u


/* ============================================================
 * Sequencer result registers
 * ============================================================ */
#define L9966_REG_SQNCR_RESULT_1   0xE1u
#define L9966_REG_SQNCR_RESULT_2   0xE2u
#define L9966_REG_SQNCR_RESULT_3   0xE3u
#define L9966_REG_SQNCR_RESULT_4   0xE4u
#define L9966_REG_SQNCR_RESULT_5   0xE5u
#define L9966_REG_SQNCR_RESULT_6   0xE6u
#define L9966_REG_SQNCR_RESULT_7   0xE7u
#define L9966_REG_SQNCR_RESULT_8   0xE8u
#define L9966_REG_SQNCR_RESULT_9   0xE9u
#define L9966_REG_SQNCR_RESULT_10  0xEAu
#define L9966_REG_SQNCR_RESULT_11  0xEBu
#define L9966_REG_SQNCR_RESULT_12  0xECu
#define L9966_REG_SQNCR_RESULT_13  0xEDu
#define L9966_REG_SQNCR_RESULT_14  0xEEu
#define L9966_REG_SQNCR_RESULT_15  0xEFu


/* ============================================================
 * Known values / masks
 * ============================================================ */
#define L9966_EXPECTED_DEV_ID  0x005Au

/*
 * L9966 register payloads are 15 useful bits.
 *
 * In the 16-bit data word:
 *   bit 15    = parity
 *   bits 14:0 = payload
 */
#define L9966_DATA_PAYLOAD_MASK  0x7FFFu
#define L9966_DATA_PARITY_MASK   0x8000u


/* ============================================================
 * Known current-source / comparator config presets
 *
 * These are early bench-tested / current working preset values.
 * Keep them here for now because they are simple named register values.
 * Later, if desired, these can be replaced by field-building macros.
 * ============================================================ */
#define L9966_CURR_CFG_UTH1   0x0030u
#define L9966_CURR_CFG_UTH2   0x0230u
#define L9966_CURR_CFG_UTH3   0x0430u
#define L9966_CURR_CFG_RATIO  0x0630u
