# =========================================
# File: mcuxsdk/examples/_boards/mcimx93evk/user_apps/m33_vehicle_app/CMakeLists.txt
# =========================================
cmake_minimum_required(VERSION 3.22)

project(m33_vehicle_app LANGUAGES C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# repo root = mcuxsdk/
get_filename_component(MCUXSDK_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../../.." ABSOLUTE)
set(MCU_DEVICE_ROOT "${MCUXSDK_ROOT}/devices/i.MX/i.MX93/MIMX9352")
set(LINKER_SCRIPT   "${MCU_DEVICE_ROOT}/gcc/MIMX9352xxxxM_cm33_ram_ns.ld")

add_executable(${PROJECT_NAME}.elf
    main.c
    hardware_init.c
    pin_mux.c

    # add these once you create them
    board.c
    clock_config.c
    syscalls.c

    app/src/app_init.c
    app/src/app_l9966.c
    app/src/app_gps.c
    app/src/app_selftest.c

    platform/imx93_m33/src/board_init.c
    platform/imx93_m33/src/board_gpio.c
    platform/imx93_m33/src/board_pins.c
    platform/imx93_m33/src/board_spi.c
    platform/imx93_m33/src/board_time.c
    platform/imx93_m33/src/board_uart.c

    drivers/l9966_driver/src/L9966.c
    drivers/l9966_driver/src/L9966_FRAME.c
    drivers/l9966_driver/src/L9966_HELPERS.c
    drivers/l9966_driver/platform/m33/L9966_TRANSPORT.c

    drivers/M9V-driver/src/neo_m9v.c
    drivers/M9V-driver/src/neo_m9v_config.c
    drivers/M9V-driver/src/neo_m9v_messages.c
    drivers/M9V-driver/src/neo_m9v_ubx_frame.c

    ${MCU_DEVICE_ROOT}/gcc/startup_MIMX9352_cm33.S
    ${MCU_DEVICE_ROOT}/system_MIMX9352_cm33.c

    ${MCUXSDK_ROOT}/drivers/common/fsl_common.c
    ${MCUXSDK_ROOT}/drivers/rgpio/fsl_rgpio.c
    ${MCUXSDK_ROOT}/drivers/lpspi/fsl_lpspi.c
    ${MCUXSDK_ROOT}/drivers/lpuart/fsl_lpuart.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syscalls.c
    ${CMAKE_CURRENT_SOURCE_DIR}/app/src/app_selftest.c
)

target_include_directories(${PROJECT_NAME}.elf PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/app/header
    ${CMAKE_CURRENT_SOURCE_DIR}/platform/imx93_m33/header
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/l9966_driver/include
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/l9966_driver/platform/m33
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/M9V-driver/include
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/M9V-driver/platform/m33/board
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/M9V-driver/platform/m33/transport

    ${MCUXSDK_ROOT}/arch/arm/CMSIS/Core/Include
    ${MCU_DEVICE_ROOT}
    ${MCU_DEVICE_ROOT}/drivers

    ${MCUXSDK_ROOT}/drivers/common
    ${MCUXSDK_ROOT}/drivers/rgpio
    ${MCUXSDK_ROOT}/drivers/lpspi
    ${MCUXSDK_ROOT}/drivers/lpuart

    ${MCUXSDK_ROOT}/devices/i.MX/i.MX93/periph
)

target_compile_definitions(${PROJECT_NAME}.elf PRIVATE
    CPU_MIMX9352CVVXM_cm33
)

target_compile_options(${PROJECT_NAME}.elf PRIVATE
    -mcpu=cortex-m33
    -mthumb
    -ffunction-sections
    -fdata-sections
)

target_link_options(${PROJECT_NAME}.elf PRIVATE
    -T${LINKER_SCRIPT}
    -mcpu=cortex-m33
    -mthumb
    -Wl,--gc-sections
    -Wl,-Map=${CMAKE_BINARY_DIR}/${PROJECT_NAME}.map
    --specs=nosys.specs
    --specs=nano.specs
)

add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}.elf> ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.bin
    COMMAND ${CMAKE_OBJCOPY} -O ihex   $<TARGET_FILE:${PROJECT_NAME}.elf> ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.hex
    VERBATIM
)