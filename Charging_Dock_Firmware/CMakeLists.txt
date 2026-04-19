cmake_minimum_required(VERSION 3.16)

set(PICO_BOARD pico2_w CACHE STRING "Target Raspberry Pi Pico board")

# Prefer a locally installed Pico GNU Arm toolchain when it is not already on PATH.
if(WIN32 AND NOT DEFINED CMAKE_C_COMPILER)
    file(GLOB _pico_toolchain_bins
        "$ENV{USERPROFILE}/.pico-sdk/toolchain/*/bin"
    )
    list(SORT _pico_toolchain_bins COMPARE NATURAL ORDER DESCENDING)
    foreach(_toolchain_bin IN LISTS _pico_toolchain_bins)
        if(EXISTS "${_toolchain_bin}/arm-none-eabi-gcc.exe")
            set(CMAKE_C_COMPILER "${_toolchain_bin}/arm-none-eabi-gcc.exe" CACHE FILEPATH "ARM GCC" FORCE)
            set(CMAKE_CXX_COMPILER "${_toolchain_bin}/arm-none-eabi-g++.exe" CACHE FILEPATH "ARM G++" FORCE)
            set(CMAKE_ASM_COMPILER "${_toolchain_bin}/arm-none-eabi-gcc.exe" CACHE FILEPATH "ARM GCC ASM" FORCE)
            break()
        endif()
    endforeach()
endif()

include(pico_sdk_import.cmake)

project(charging_dock_firmware C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

pico_sdk_init()

add_executable(charging_dock_firmware
    src/app_state.c
    src/config.c
    src/input.c
    src/inverter.c
    src/main.c
    src/sh1106.c
    src/ui.c
    src/ui_anim.c
)

pico_set_program_name(charging_dock_firmware "charging_dock_firmware")
pico_set_program_version(charging_dock_firmware "1.0.0")

pico_enable_stdio_usb(charging_dock_firmware 0)
pico_enable_stdio_uart(charging_dock_firmware 0)

target_include_directories(charging_dock_firmware
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src
)

target_link_libraries(charging_dock_firmware
    pico_stdlib
    hardware_adc
    hardware_clocks
    hardware_gpio
    hardware_pwm
    hardware_spi
    hardware_sync
)

pico_add_extra_outputs(charging_dock_firmware)
