#include "L9966_FRAME.h"

#define L9966_RW_READ              0u
#define L9966_RW_WRITE             1u
#define L9966_CLK_MON_32BIT_FRAME  1u
#define L9966_MISO_FIXED_PATTERN_SHIFT 25u
#define L9966_MISO_FIXED_PATTERN_MASK  0x1Fu
#define L9966_MISO_FIXED_PATTERN_VALUE 0x15u

static bool is_valid_chip_addr(l9966_chip_addr_t chip_addr)
{
    return chip_addr == L9966_CHIP_ADDR_10 || 
           chip_addr == L9966_CHIP_ADDR_11;
}

static bool is_valid_register_addr(uint8_t register_addr)
{
    return register_addr != 0x00u;
}

static bool has_odd_parity_u16(uint16_t value)
{
    unsigned int ones = 0u;

    for (unsigned int bit = 0u; bit < 16u; ++bit)
    {
        if ((value & (uint16_t)(1u << bit)) != 0u)
        {
            ++ones;
        }
    }

    return (ones % 2u) == 1u;
}

static uint16_t add_odd_parity_bit15_to_data(uint16_t data_15bit)
{
    uint16_t data_word = (uint16_t)(data_15bit & 0x7FFFu);

    if (!has_odd_parity_u16(data_word))
    {
        data_word |= 0x8000u;
    }

    return data_word;
}

static uint16_t add_odd_parity_bit0_to_instruction(uint16_t instruction_without_parity)
{
    uint16_t instruction = (uint16_t)(instruction_without_parity & 0xFFFEu);

    if (!has_odd_parity_u16(instruction))
    {
        instruction |= 0x0001u;
    }

    return instruction;
}

static l9966_frame_result_t build_instruction_word(
    l9966_chip_addr_t chip_addr,
    uint8_t read_write,
    uint8_t register_addr,
    uint16_t *instruction_out
)
{
    if (instruction_out == 0)
    {
        return L9966_FRAME_ERROR_NULL_ARGUMENT;
    }

    if (!is_valid_chip_addr(chip_addr))
    {
        return L9966_FRAME_ERROR_BAD_CHIP_ADDRESS;
    }

    if (!is_valid_register_addr(register_addr))
    {
        return L9966_FRAME_ERROR_BAD_REGISTER_ADDRESS;
    }
    uint16_t instruction_no_parity = 0u;

    instruction_no_parity |= (uint16_t)(((uint16_t)chip_addr & 0x03u) << 14u);
    instruction_no_parity |= (uint16_t)((read_write & 0x01u) << 13u);
    instruction_no_parity |= (uint16_t)(L9966_CLK_MON_32BIT_FRAME << 12u);
    instruction_no_parity |= (uint16_t)(((uint16_t)register_addr) << 4u);

    *instruction_out = add_odd_parity_bit0_to_instruction(instruction_no_parity);

    return L9966_FRAME_OK;
}

l9966_frame_result_t l9966_build_read_frame(
    l9966_chip_addr_t chip_addr,
    uint8_t register_addr,
    uint32_t *frame_out
)
{
    if (frame_out == 0)
    {
        return L9966_FRAME_ERROR_NULL_ARGUMENT;
    }

    uint16_t instruction = 0u;

    l9966_frame_result_t result = build_instruction_word(
        chip_addr,
        L9966_RW_READ,
        register_addr,
        &instruction
    );

    if (result != L9966_FRAME_OK)
    {
        return result;
    }
    uint16_t data_word = add_odd_parity_bit15_to_data(0u);

    *frame_out = ((uint32_t)instruction << 16u) | (uint32_t)data_word;

    return L9966_FRAME_OK;
}

l9966_frame_result_t l9966_build_write_frame(
    l9966_chip_addr_t chip_addr,
    uint8_t register_addr,
    uint16_t data_15bit,
    uint32_t *frame_out
)
{
    if (frame_out == 0)
    {
        return L9966_FRAME_ERROR_NULL_ARGUMENT;
    }

    if ((data_15bit & 0x8000u) != 0u)
    {
        return L9966_FRAME_ERROR_BAD_WRITE_VALUE;
    }

    uint16_t instruction = 0u;

    l9966_frame_result_t result = build_instruction_word(
        chip_addr,
        L9966_RW_WRITE,
        register_addr,
        &instruction
    );

    if (result != L9966_FRAME_OK)
    {
        return result;
    }

    uint16_t data_word = add_odd_parity_bit15_to_data(data_15bit);

    *frame_out = ((uint32_t)instruction << 16u) | (uint32_t)data_word;

    return L9966_FRAME_OK;
}

l9966_frame_result_t l9966_parse_response(uint32_t raw_response, l9966_response_t *response_out)
{
    if (response_out == 0)
    {
        return L9966_FRAME_ERROR_NULL_ARGUMENT;
    }

    response_out->raw = raw_response;

    response_out->fixed_pattern = (uint8_t)(
        (raw_response >> L9966_MISO_FIXED_PATTERN_SHIFT) &
        L9966_MISO_FIXED_PATTERN_MASK
    );

    response_out->fixed_pattern_ok =
        response_out->fixed_pattern == L9966_MISO_FIXED_PATTERN_VALUE;

    response_out->transfer_failed =
        ((raw_response >> 24u) & 0x01u) != 0u;

    response_out->register_echo =
        (uint8_t)((raw_response >> 16u) & 0xFFu);

    response_out->data_word =
        (uint16_t)(raw_response & 0xFFFFu);

    response_out->data =
        (uint16_t)(response_out->data_word & 0x7FFFu);

    response_out->data_parity_ok =
        has_odd_parity_u16(response_out->data_word);

    return L9966_FRAME_OK;
}