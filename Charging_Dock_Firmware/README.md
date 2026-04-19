# Charging Dock Firmware

Production-style Raspberry Pi Pico SDK firmware for a wireless charging dock controller with a HIP4080A full bridge, potentiometer tuning, one-button UI, leaf-switch safety interlock, and an SH1106 128x64 SPI OLED front panel.

## Pin map

### Inverter / bridge control

- `HEN` -> `GP10`
- `DIS` -> `GP11`
- `IN+` -> `GP12`
- `IN-` -> `GP13`

### SH1106 OLED over SPI

- `CLK` -> `GP18`
- `MOSI` -> `GP19`
- `DC` -> `GP20`
- `RES` -> `GP21`
- `CS` -> `GP17`

### Inputs

- Pushbutton -> `GP16`
- Potentiometer -> `GP26` / `ADC0`
- Leaf switch -> `GP15`

## Button actions

- Single press: lock or unlock frequency
- Double press: start when allowed, stop when active
- Long press: immediate shutdown

## Application states

- `BOOT`: splash screen, inverter forced off
- `OFF`: unlocked idle state
- `TUNING`: unlocked and pot recently moving
- `LOCKED`: frequency locked, robot absent
- `READY`: frequency locked, robot present, ready to run
- `ACTIVE`: inverter enabled and driven from hardware PWM
- `BLOCKED`: start request denied because the leaf interlock is open
- `FAULT`: emergency shutdown or interlock loss while active

## Global configuration variables in `config.c`

These are the primary board and behavior settings exposed as globals:

- `g_freq_min_hz`
- `g_freq_max_hz`
- `g_default_freq_hz`
- `g_hen_active_level`
- `g_dis_active_level`
- `g_button_active_level`
- `g_leaf_active_level`
- `g_sh1106_col_offset`
- `g_ui_redraw_ms`
- `g_input_scan_ms`
- `g_double_click_ms`
- `g_long_press_ms`
- `g_pot_filter_shift`
- `g_pot_hysteresis_counts`
- `g_ui_anim_tick_ms`

## Remaining compile-time constants in `config.h`

These are still centralized in one place but left as compile-time constants:

- GPIO pin assignments
- SH1106 SPI port and baud rate
- splash, blocked, and fault hold durations
- the SH1106 framebuffer geometry
- `IN+` / `IN-` active and idle levels

`HEN` and `DIS` polarity often needs board-specific adjustment depending on the HIP4080A front-end wiring. `IN+` and `IN-` idle and active levels are also centralized in `config.h` for the same reason.

The SH1106 visible column alignment may vary slightly by panel. If text looks shifted, tune `g_sh1106_col_offset` in `config.c`.

## Build with Pico SDK

Requirements:

- a working Pico SDK checkout that contains `pico_sdk_init.cmake`
- GNU Arm Embedded toolchain with `arm-none-eabi-gcc`
- CMake
- a generator such as Ninja or a configured VS Code Pico environment

Build:

```powershell
cmake -S . -B build -DPICO_BOARD=pico2_w
cmake --build build
```

Expected firmware image:

```text
build/charging_dock_firmware.uf2
```

## Flashing

1. Hold the Pico `BOOTSEL` button while connecting USB.
2. Wait for the `RPI-RP2` mass-storage device to appear.
3. Copy `build/charging_dock_firmware.uf2` onto that drive.
4. The board reboots automatically and starts the dock controller firmware.
