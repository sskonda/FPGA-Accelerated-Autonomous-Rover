#include "ui_anim.h"

static int16_t ui_anim_lerp_i16(int16_t a, int16_t b, uint8_t t_q8) {
    const int32_t delta = (int32_t)b - (int32_t)a;
    return (int16_t)(a + ((delta * (int32_t)t_q8) / 255));
}

static uint16_t ui_anim_lerp_u16(uint16_t a, uint16_t b, uint8_t t_q8) {
    const int32_t delta = (int32_t)b - (int32_t)a;
    return (uint16_t)((int32_t)a + ((delta * (int32_t)t_q8) / 255));
}

static uint8_t ui_anim_lerp_u8(uint8_t a, uint8_t b, uint8_t t_q8) {
    const int16_t delta = (int16_t)b - (int16_t)a;
    return (uint8_t)((int16_t)a + ((delta * (int16_t)t_q8) / 255));
}

void ui_anim_seed(
    ui_widget_anim_t *widget,
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h,
    uint16_t scale_q8,
    uint8_t highlight_q8
) {
    widget->start_x = x;
    widget->start_y = y;
    widget->start_w = w;
    widget->start_h = h;
    widget->current_x = x;
    widget->current_y = y;
    widget->current_w = w;
    widget->current_h = h;
    widget->target_x = x;
    widget->target_y = y;
    widget->target_w = w;
    widget->target_h = h;
    widget->start_scale_q8 = scale_q8;
    widget->current_scale_q8 = scale_q8;
    widget->target_scale_q8 = scale_q8;
    widget->start_highlight_q8 = highlight_q8;
    widget->current_highlight_q8 = highlight_q8;
    widget->target_highlight_q8 = highlight_q8;
    widget->anim_progress_q8 = 255u;
    widget->focused = false;
    widget->highlight_enabled = highlight_q8 > 0u;
    widget->animating = false;
}

void ui_anim_set_target(
    ui_widget_anim_t *widget,
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h,
    uint16_t scale_q8,
    uint8_t highlight_q8,
    bool focused
) {
    const bool changed =
        (widget->target_x != x) ||
        (widget->target_y != y) ||
        (widget->target_w != w) ||
        (widget->target_h != h) ||
        (widget->target_scale_q8 != scale_q8) ||
        (widget->target_highlight_q8 != highlight_q8);

    widget->focused = focused;
    widget->highlight_enabled = highlight_q8 > 0u;

    if (!changed) {
        return;
    }

    widget->start_x = widget->current_x;
    widget->start_y = widget->current_y;
    widget->start_w = widget->current_w;
    widget->start_h = widget->current_h;
    widget->start_scale_q8 = widget->current_scale_q8;
    widget->start_highlight_q8 = widget->current_highlight_q8;

    widget->target_x = x;
    widget->target_y = y;
    widget->target_w = w;
    widget->target_h = h;
    widget->target_scale_q8 = scale_q8;
    widget->target_highlight_q8 = highlight_q8;
    widget->anim_progress_q8 = 0u;
    widget->animating = true;
}

uint8_t ui_anim_ease_in_out(uint8_t t_q8) {
    const uint32_t t = t_q8;
    const uint32_t t2 = (t * t) / 255u;
    const uint32_t t3 = (t2 * t) / 255u;
    const uint32_t eased = (3u * t2) - (2u * t3);
    return (uint8_t)eased;
}

bool ui_anim_step(ui_widget_anim_t *widget, uint32_t dt_ms, uint32_t duration_ms) {
    if (!widget->animating) {
        return false;
    }

    uint32_t step_q8 = (dt_ms * 255u) / duration_ms;
    if (step_q8 == 0u) {
        step_q8 = 1u;
    }

    if ((uint32_t)widget->anim_progress_q8 + step_q8 >= 255u) {
        widget->anim_progress_q8 = 255u;
    } else {
        widget->anim_progress_q8 = (uint8_t)((uint32_t)widget->anim_progress_q8 + step_q8);
    }

    const uint8_t eased = ui_anim_ease_in_out(widget->anim_progress_q8);

    widget->current_x = ui_anim_lerp_i16(widget->start_x, widget->target_x, eased);
    widget->current_y = ui_anim_lerp_i16(widget->start_y, widget->target_y, eased);
    widget->current_w = ui_anim_lerp_i16(widget->start_w, widget->target_w, eased);
    widget->current_h = ui_anim_lerp_i16(widget->start_h, widget->target_h, eased);
    widget->current_scale_q8 =
        ui_anim_lerp_u16(widget->start_scale_q8, widget->target_scale_q8, eased);
    widget->current_highlight_q8 =
        ui_anim_lerp_u8(widget->start_highlight_q8, widget->target_highlight_q8, eased);

    if (widget->anim_progress_q8 >= 255u) {
        widget->current_x = widget->target_x;
        widget->current_y = widget->target_y;
        widget->current_w = widget->target_w;
        widget->current_h = widget->target_h;
        widget->current_scale_q8 = widget->target_scale_q8;
        widget->current_highlight_q8 = widget->target_highlight_q8;
        widget->animating = false;
    }

    return true;
}

uint8_t ui_anim_triangle_wave(uint32_t now_ms, uint32_t period_ms) {
    if (period_ms == 0u) {
        return 0u;
    }

    const uint32_t phase = now_ms % period_ms;
    const uint32_t half = period_ms / 2u;
    if (half == 0u) {
        return 0u;
    }

    if (phase < half) {
        return (uint8_t)((phase * 255u) / half);
    }

    return (uint8_t)(((period_ms - phase) * 255u) / half);
}

int ui_anim_quantize_scale(uint16_t scale_q8) {
    if (scale_q8 >= 640u) {
        return 3;
    }
    if (scale_q8 >= 384u) {
        return 2;
    }
    return 1;
}
