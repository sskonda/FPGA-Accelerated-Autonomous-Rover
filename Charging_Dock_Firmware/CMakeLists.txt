cmake_minimum_required(VERSION 3.16)

set(PICO_BOARD pico2_w CACHE STRING "Target Raspberry Pi Pico board")

# The Pico SDK chooses its own toolchain before project() runs, so the
# important setting here is PICO_TOOLCHAIN_PATH rather than CMAKE_C_COMPILER.
if(WIN32 AND NOT DEFINED PICO_TOOLCHAIN_PATH)
    set(_pico_toolchain_roots
        "$ENV{USERPROFILE}/.pico-sdk/toolchain/14_2_Rel1"
        "C:/Program Files/Raspberry Pi/Pico SDK v1.5.1/gcc-arm-none-eabi"
        "$ENV{LOCALAPPDATA}/stm32cube/bundles/gnu-tools-for-stm32/13.3.1+st.9"
        "$ENV{USERPROFILE}/OneDrive - University of Florida/Miscellaneous/software/7.0/toolchain/arm/arm-gnu-toolchain"
    )

    foreach(_toolchain_root IN LISTS _pico_toolchain_roots)
        if(EXISTS "${_toolchain_root}/bin/arm-none-eabi-gcc.exe")
            set(PICO_TOOLCHAIN_PATH "${_toolchain_root}" CACHE PATH "ARM GNU toolchain root" FORCE)
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
