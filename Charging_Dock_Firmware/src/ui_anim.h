#ifndef UI_ANIM_H
#define UI_ANIM_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    int16_t start_x;
    int16_t start_y;
    int16_t start_w;
    int16_t start_h;
    int16_t current_x;
    int16_t current_y;
    int16_t current_w;
    int16_t current_h;
    int16_t target_x;
    int16_t target_y;
    int16_t target_w;
    int16_t target_h;
    uint16_t start_scale_q8;
    uint16_t current_scale_q8;
    uint16_t target_scale_q8;
    uint8_t start_highlight_q8;
    uint8_t current_highlight_q8;
    uint8_t target_highlight_q8;
    uint8_t anim_progress_q8;
    bool focused;
    bool highlight_enabled;
    bool animating;
} ui_widget_anim_t;

void ui_anim_seed(
    ui_widget_anim_t *widget,
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h,
    uint16_t scale_q8,
    uint8_t highlight_q8
);
void ui_anim_set_target(
    ui_widget_anim_t *widget,
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h,
    uint16_t scale_q8,
    uint8_t highlight_q8,
    bool focused
);
bool ui_anim_step(ui_widget_anim_t *widget, uint32_t dt_ms, uint32_t duration_ms);
uint8_t ui_anim_ease_in_out(uint8_t t_q8);
uint8_t ui_anim_triangle_wave(uint32_t now_ms, uint32_t period_ms);
int ui_anim_quantize_scale(uint16_t scale_q8);

#endif
