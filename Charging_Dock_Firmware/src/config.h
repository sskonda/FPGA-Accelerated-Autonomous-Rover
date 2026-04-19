#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>
#include <stdint.h>

#include "hardware/spi.h"

extern uint32_t g_freq_min_hz;
extern uint32_t g_freq_max_hz;
extern uint32_t g_default_freq_hz;
extern bool g_hen_active_level;
extern bool g_dis_active_level;
extern bool g_button_active_level;
extern bool g_leaf_active_level;
extern uint8_t g_sh1106_col_offset;
extern uint32_t g_ui_redraw_ms;
extern uint32_t g_input_scan_ms;
extern uint32_t g_double_click_ms;
extern uint32_t g_long_press_ms;
extern uint8_t g_pot_filter_shift;
extern uint16_t g_pot_hysteresis_counts;
extern uint32_t g_ui_anim_tick_ms;

#define HW_PIN_HEN               10u
#define HW_PIN_DIS               11u
#define HW_PIN_IN_PLUS           12u
#define HW_PIN_IN_MINUS          13u

#define HW_PIN_OLED_CS           17u
#define HW_PIN_OLED_CLK          18u
#define HW_PIN_OLED_MOSI         19u
#define HW_PIN_OLED_DC           20u
#define HW_PIN_OLED_RES          21u

#define HW_PIN_BUTTON            16u
#define HW_PIN_LEAF_SWITCH       15u
#define HW_PIN_POT_ADC_GPIO      26u
#define HW_POT_ADC_INPUT         0u

#define HW_SH1106_SPI_PORT       spi0
#define HW_SH1106_SPI_BAUD_HZ    10000000u

#define SH1106_WIDTH             128u
#define SH1106_HEIGHT            64u
#define SH1106_FRAMEBUFFER_SIZE  ((SH1106_WIDTH * SH1106_HEIGHT) / 8u)

#define APP_BOOT_SPLASH_MS       1000u
#define APP_TUNING_LINGER_MS     450u
#define APP_BLOCKED_HOLD_MS      900u
#define APP_FAULT_HOLD_MS        1200u

#define UI_FOCUS_ANIM_MS         180u
#define UI_NUMERIC_SMOOTH_SHIFT  2u

#define INVERTER_IN_PLUS_ACTIVE_LEVEL  true
#define INVERTER_IN_MINUS_ACTIVE_LEVEL true
#define INVERTER_IN_PLUS_IDLE_LEVEL    false
#define INVERTER_IN_MINUS_IDLE_LEVEL   false

#endif
