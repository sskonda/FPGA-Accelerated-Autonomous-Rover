#include "config.h"

uint32_t g_freq_min_hz = 110000;
uint32_t g_freq_max_hz = 130000;
uint32_t g_default_freq_hz = 120000;

bool g_hen_active_level = true;
bool g_dis_active_level = true;
bool g_button_active_level = false;
bool g_leaf_active_level = false;

uint8_t g_sh1106_col_offset = 2;

uint32_t g_ui_redraw_ms = 50;
uint32_t g_input_scan_ms = 10;
uint32_t g_double_click_ms = 300;
uint32_t g_long_press_ms = 700;
uint8_t g_pot_filter_shift = 3;
uint16_t g_pot_hysteresis_counts = 12;
uint32_t g_ui_anim_tick_ms = 33;
