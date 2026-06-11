#include "board_gpio.h"
#include "board_pins.h"

void BOARD_GPIO_Init(void)
{
    const rgpio_pin_config_t output_high = {
        .pinDirection = kRGPIO_DigitalOutput,
        .outputLogic = 1U,
    };

    const rgpio_pin_config_t input_no_irq = {
        .pinDirection = kRGPIO_DigitalInput,
        .outputLogic = 0U,
    };

    RGPIO_PinInit(BOARD_L9966_CS_GPIO, BOARD_L9966_CS_PIN, &output_high);
    RGPIO_PinInit(BOARD_L9966_RESET_GPIO, BOARD_L9966_RESET_PIN, &output_high);
    RGPIO_PinInit(BOARD_L9966_FAULT_GPIO, BOARD_L9966_FAULT_PIN, &input_no_irq);
    RGPIO_PinInit(BOARD_GPS_RESET_GPIO, BOARD_GPS_RESET_PIN, &output_high);
    RGPIO_PinInit(BOARD_GPS_PPS_GPIO, BOARD_GPS_PPS_PIN, &input_no_irq);
}

void BOARD_GPIO_Write(RGPIO_Type *base, uint32_t pin, bool value)
{
    RGPIO_PinWrite(base, pin, value ? 1U : 0U);
}

bool BOARD_GPIO_Read(RGPIO_Type *base, uint32_t pin)
{
    return (RGPIO_PinRead(base, pin) != 0U);
}