#include "ui.h"

#include <stdio.h>
#include <string.h>

#include "config.h"
#include "font5x7.h"

#define UI_Q8(value) ((uint16_t)((value) << 8))

typedef struct {
    int16_t x;
    int16_t y;
    int16_t w;
    int16_t h;
    uint16_t scale_q8;
    uint8_t highlight_q8;
    bool focused;
} ui_target_t;

static bool ui_status_changed(const app_status_t *a, const app_status_t *b) {
    return
        (a->state != b->state) ||
        (a->fault_reason != b->fault_reason) ||
        (a->frequency_locked != b->frequency_locked) ||
        (a->leaf_active != b->leaf_active) ||
        (a->inverter_enabled != b->inverter_enabled) ||
        (a->tuning_live != b->tuning_live) ||
        (a->selected_frequency_editable != b->selected_frequency_editable) ||
        (a->selected_frequency_hz != b->selected_frequency_hz) ||
        (a->active_frequency_hz != b->active_frequency_hz) ||
        (a->displayed_frequency_hz != b->displayed_frequency_hz) ||
        (a->pot_frequency_hz != b->pot_frequency_hz) ||
        (a->pot_normalized_permille != b->pot_normalized_permille) ||
        (a->focused_widget != b->focused_widget) ||
        (a->editable_widget != b->editable_widget);
}

static bool ui_chase_q8(int32_t *current, int32_t target, uint8_t shift) {
    int32_t delta = target - *current;
    if (delta == 0) {
        return false;
    }

    int32_t step = delta >> shift;
    if (step == 0) {
        step = (delta > 0) ? 1 : -1;
    }

    *current += step;
    if (((step > 0) && (*current > target)) || ((step < 0) && (*current < target))) {
        *current = target;
    }
    return true;
}

static void ui_draw_centered_text(
    sh1106_t *display,
    int x,
    int y,
    int w,
    const char *text,
    int scale,
    bool invert
) {
    const int text_w = sh1106_measure_text(text, scale);
    const int draw_x = x + ((w - text_w) / 2);
    sh1106_draw_text(display, draw_x, y, text, scale, invert);
}

static void ui_draw_lock_icon(sh1106_t *display, int x, int y, bool locked, bool invert) {
    if (locked) {
        sh1106_draw_rect(display, x + 1, y, 6, 5, invert ? false : true);
        sh1106_draw_rect(display, x, y + 4, 8, 6, invert ? false : true);
    } else {
        sh1106_draw_rect(display, x + 2, y + 1, 5, 4, invert ? false : true);
        sh1106_draw_rect(display, x, y + 4, 8, 6, invert ? false : true);
        sh1106_draw_line(display, x + 1, y + 1, x + 1, y + 4, invert ? false : true);
    }
}

static const char *ui_state_text(const app_status_t *status) {
    switch (status->state) {
        case APP_STATE_BOOT:    return "BOOT";
        case APP_STATE_OFF:     return "OFF";
        case APP_STATE_TUNING:  return "TUNING";
        case APP_STATE_LOCKED:  return "LOCKED";
        case APP_STATE_READY:   return "READY";
        case APP_STATE_ACTIVE:  return "ACTIVE";
        case APP_STATE_BLOCKED: return "BLOCKED";
        case APP_STATE_FAULT:   return "FAULT";
        default:                return "STATE";
    }
}

static const char *ui_action_text(const app_status_t *status) {
    if (status->inverter_enabled) {
        return "STOP";
    }
    if (status->leaf_active) {
        return "RUN";
    }
    return "WAIT";
}

static const char *ui_hint_text(const app_status_t *status) {
    switch (status->state) {
        case APP_STATE_READY:
            return "2x run   hold off";
        case APP_STATE_ACTIVE:
            return "2x stop  hold off";
        case APP_STATE_BLOCKED:
            return "leaf reqd  hold off";
        case APP_STATE_FAULT:
            return (status->fault_reason == APP_FAULT_INTERLOCK_OPEN)
                ? "interlock open"
                : "emergency stop";
        case APP_STATE_LOCKED:
            return "2x run when docked";
        default:
            return "1x lock  hold off";
    }
}

static void ui_format_frequency(char *buffer, size_t buffer_len, uint32_t frequency_hz) {
    const uint32_t tenths_khz = (frequency_hz + 50u) / 100u;
    const uint32_t whole = tenths_khz / 10u;
    const uint32_t tenths = tenths_khz % 10u;
    snprintf(buffer, buffer_len, "%lu.%1lu", (unsigned long)whole, (unsigned long)tenths);
}

static void ui_seed_targets(ui_t *ui) {
    ui_anim_seed(&ui->widgets[APP_WIDGET_HEADER], 2, 0, 124, 8, UI_Q8(1), 32u);
    ui_anim_seed(&ui->widgets[APP_WIDGET_STATE], 4, 9, 120, 12, UI_Q8(1), 72u);
    ui_anim_seed(&ui->widgets[APP_WIDGET_FREQUENCY], 4, 23, 120, 18, UI_Q8(2), 120u);
    ui_anim_seed(&ui->widgets[APP_WIDGET_LOCK], 4, 42, 38, 10, UI_Q8(1), 70u);
    ui_anim_seed(&ui->widgets[APP_WIDGET_ROBOT], 44, 42, 38, 10, UI_Q8(1), 70u);
    ui_anim_seed(&ui->widgets[APP_WIDGET_ACTION], 86, 41, 38, 12, UI_Q8(1), 110u);
    ui_anim_seed(&ui->widgets[APP_WIDGET_POT], 4, 54, 120, 5, UI_Q8(1), 80u);
    ui_anim_seed(&ui->widgets[APP_WIDGET_HINTS], 2, 57, 124, 7, UI_Q8(1), 32u);
}

static void ui_apply_targets(ui_t *ui, const app_status_t *status) {
    ui_target_t targets[APP_WIDGET_COUNT] = {
        [APP_WIDGET_HEADER] = {2, 0, 124, 8, UI_Q8(1), 24u, false},
        [APP_WIDGET_STATE] = {4, 9, 120, 12, UI_Q8(1), 72u, false},
        [APP_WIDGET_FREQUENCY] = {4, 23, 120, 18, UI_Q8(2), 112u, false},
        [APP_WIDGET_LOCK] = {4, 42, 38, 10, UI_Q8(1), status->frequency_locked ? 136u : 64u, false},
        [APP_WIDGET_ROBOT] = {44, 42, 38, 10, UI_Q8(1), status->leaf_active ? 136u : 48u, false},
        [APP_WIDGET_ACTION] = {86, 41, 38, 12, UI_Q8(1), status->leaf_active ? 128u : 80u, false},
        [APP_WIDGET_POT] = {4, 54, 120, 5, UI_Q8(1), 80u, false},
        [APP_WIDGET_HINTS] = {2, 57, 124, 7, UI_Q8(1), 24u, false}
    };

    switch (status->focused_widget) {
        case APP_WIDGET_STATE:
            targets[APP_WIDGET_STATE] = (ui_target_t){2, 8, 124, 16, UI_Q8(2), 220u, true};
            targets[APP_WIDGET_FREQUENCY] = (ui_target_t){6, 27, 116, 16, UI_Q8(2), 90u, false};
            targets[APP_WIDGET_ACTION] = (ui_target_t){86, 42, 38, 10, UI_Q8(1), 92u, false};
            break;
        case APP_WIDGET_FREQUENCY:
            targets[APP_WIDGET_STATE] = (ui_target_t){4, 8, 120, 11, UI_Q8(1), 88u, false};
            targets[APP_WIDGET_FREQUENCY] = (ui_target_t){2, 20, 124, 24, UI_Q8(3), 220u, true};
            targets[APP_WIDGET_LOCK] = (ui_target_t){4, 44, 38, 10, UI_Q8(1), status->frequency_locked ? 136u : 64u, false};
            targets[APP_WIDGET_ROBOT] = (ui_target_t){44, 44, 38, 10, UI_Q8(1), status->leaf_active ? 136u : 48u, false};
            targets[APP_WIDGET_ACTION] = (ui_target_t){86, 43, 38, 11, UI_Q8(1), status->leaf_active ? 128u : 80u, false};
            break;
        case APP_WIDGET_ACTION:
            targets[APP_WIDGET_STATE] = (ui_target_t){4, 8, 120, 11, UI_Q8(1), 88u, false};
            targets[APP_WIDGET_FREQUENCY] = (ui_target_t){4, 21, 120, 18, UI_Q8(2), 124u, false};
            targets[APP_WIDGET_LOCK] = (ui_target_t){4, 42, 30, 10, UI_Q8(1), status->frequency_locked ? 136u : 64u, false};
            targets[APP_WIDGET_ROBOT] = (ui_target_t){36, 42, 34, 10, UI_Q8(1), status->leaf_active ? 136u : 48u, false};
            targets[APP_WIDGET_ACTION] = (ui_target_t){72, 40, 52, 14, UI_Q8(2), 228u, true};
            break;
        default:
            break;
    }

    if (status->state == APP_STATE_ACTIVE) {
        targets[APP_WIDGET_ACTION].highlight_q8 = 220u;
        targets[APP_WIDGET_STATE].highlight_q8 = 180u;
    } else if (status->state == APP_STATE_BLOCKED) {
        targets[APP_WIDGET_STATE].highlight_q8 = 240u;
        targets[APP_WIDGET_ACTION].highlight_q8 = 170u;
    } else if (status->state == APP_STATE_FAULT) {
        targets[APP_WIDGET_STATE].highlight_q8 = 255u;
        targets[APP_WIDGET_ACTION].highlight_q8 = 110u;
    } else if (status->state == APP_STATE_READY) {
        targets[APP_WIDGET_STATE].highlight_q8 = 150u;
    }

    for (int i = 0; i < APP_WIDGET_COUNT; ++i) {
        ui_anim_set_target(
            &ui->widgets[i],
            targets[i].x,
            targets[i].y,
            targets[i].w,
            targets[i].h,
            targets[i].scale_q8,
            targets[i].highlight_q8,
            targets[i].focused
        );
    }
}

static void ui_draw_header(ui_t *ui, const app_status_t *status) {
    sh1106_t *display = &ui->display;
    sh1106_draw_text(display, 2, 0, "WIRELESS DOCK", 1, false);
    sh1106_draw_line(display, 0, 8, 127, 8, true);

    if (status->leaf_active) {
        sh1106_fill_rect(display, 113, 1, 6, 6, true);
    } else {
        sh1106_draw_rect(display, 113, 1, 6, 6, true);
    }

    if (status->inverter_enabled) {
        sh1106_fill_rect(display, 121, 1, 6, 6, true);
    } else {
        sh1106_draw_rect(display, 121, 1, 6, 6, true);
    }
}

static void ui_draw_state_banner(ui_t *ui, const app_status_t *status, uint32_t now_ms) {
    sh1106_t *display = &ui->display;
    ui_widget_anim_t *widget = &ui->widgets[APP_WIDGET_STATE];
    const int x = widget->current_x;
    const int y = widget->current_y;
    const int w = widget->current_w;
    const int h = widget->current_h;
    const int scale = ui_anim_quantize_scale(widget->current_scale_q8);
    const char *label = ui_state_text(status);

    bool invert_text = false;
    if (status->state == APP_STATE_ACTIVE) {
        const uint8_t pulse = ui_anim_triangle_wave(now_ms, 900u);
        if (pulse > 104u) {
            sh1106_fill_rect(display, x, y, w, h, true);
            invert_text = true;
        }
    } else if (status->state == APP_STATE_BLOCKED) {
        if (((now_ms / 140u) & 0x01u) != 0u) {
            sh1106_fill_rect(display, x, y, w, h, true);
            invert_text = true;
        }
    } else if (status->state == APP_STATE_FAULT) {
        if (((now_ms / 100u) & 0x01u) == 0u) {
            sh1106_fill_rect(display, x, y, w, h, true);
            invert_text = true;
        }
    }

    sh1106_draw_rect(display, x, y, w, h, true);

    const int underline_w = (w * widget->current_highlight_q8) / 255;
    const int underline_x = x + ((w - underline_w) / 2);
    sh1106_fill_rect(display, underline_x, y + h - 2, underline_w, 2, !invert_text);

    ui_draw_centered_text(display, x, y + ((h - (scale * FONT5X7_HEIGHT)) / 2), w, label, scale, invert_text);
}

static void ui_draw_frequency(ui_t *ui, const app_status_t *status) {
    sh1106_t *display = &ui->display;
    ui_widget_anim_t *widget = &ui->widgets[APP_WIDGET_FREQUENCY];
    const int x = widget->current_x;
    const int y = widget->current_y;
    const int w = widget->current_w;
    const int h = widget->current_h;
    const int scale = ui_anim_quantize_scale(widget->current_scale_q8);

    char freq_text[16];
    const uint32_t displayed_frequency_hz = (uint32_t)(ui->displayed_frequency_hz_q8 >> 8);
    ui_format_frequency(freq_text, sizeof(freq_text), displayed_frequency_hz);

    const int pad = widget->current_highlight_q8 / 64;
    sh1106_draw_rect(display, x - pad, y - 1, w + (pad * 2), h + 2, true);

    if (status->state == APP_STATE_ACTIVE && status->state_age_ms < 260u) {
        const int pulse_pad = 1 + (int)((260u - status->state_age_ms) / 40u);
        sh1106_draw_rect(
            display,
            x - pad - pulse_pad,
            y - 2 - pulse_pad,
            w + ((pad + pulse_pad) * 2),
            h + 4 + (pulse_pad * 2),
            true
        );
    }

    sh1106_draw_text(display, x + 2, y + 1, status->inverter_enabled ? "LIVE" : "SET", 1, false);
    sh1106_draw_text(display, x + w - 20, y + 1, "kHz", 1, false);

    if (status->frequency_locked) {
        ui_draw_lock_icon(display, x + 2, y + h - 11, true, false);
        sh1106_draw_line(display, x + 12, y + h - 5, x + w - 4, y + h - 5, true);
    } else {
        ui_draw_lock_icon(display, x + 2, y + h - 11, false, false);
    }

    ui_draw_centered_text(
        display,
        x,
        y + ((h - (scale * FONT5X7_HEIGHT)) / 2) - 1,
        w,
        freq_text,
        scale,
        false
    );
}

static void ui_draw_chip(
    sh1106_t *display,
    const ui_widget_anim_t *widget,
    const char *label,
    bool filled,
    bool add_border
) {
    if (filled) {
        sh1106_fill_rect(display, widget->current_x, widget->current_y, widget->current_w, widget->current_h, true);
    }
    if (add_border || !filled) {
        sh1106_draw_rect(display, widget->current_x, widget->current_y, widget->current_w, widget->current_h, true);
    }
    ui_draw_centered_text(
        display,
        widget->current_x,
        widget->current_y + ((widget->current_h - FONT5X7_HEIGHT) / 2),
        widget->current_w,
        label,
        1,
        filled
    );
}

static void ui_draw_action(ui_t *ui, const app_status_t *status, uint32_t now_ms) {
    sh1106_t *display = &ui->display;
    ui_widget_anim_t *widget = &ui->widgets[APP_WIDGET_ACTION];
    const int scale = ui_anim_quantize_scale(widget->current_scale_q8);
    const char *label = ui_action_text(status);

    bool fill = false;
    if (status->state == APP_STATE_ACTIVE) {
        fill = ui_anim_triangle_wave(now_ms, 780u) > 88u;
    } else if (status->state == APP_STATE_READY) {
        fill = widget->current_highlight_q8 > 180u;
    } else if (status->state == APP_STATE_BLOCKED) {
        fill = ((now_ms / 150u) & 0x01u) != 0u;
    }

    if (fill) {
        sh1106_fill_rect(display, widget->current_x, widget->current_y, widget->current_w, widget->current_h, true);
    }
    sh1106_draw_rect(display, widget->current_x, widget->current_y, widget->current_w, widget->current_h, true);

    ui_draw_centered_text(
        display,
        widget->current_x,
        widget->current_y + ((widget->current_h - (scale * FONT5X7_HEIGHT)) / 2),
        widget->current_w,
        label,
        scale,
        fill
    );
}

static void ui_draw_pot_bar(ui_t *ui, const app_status_t *status) {
    sh1106_t *display = &ui->display;
    ui_widget_anim_t *widget = &ui->widgets[APP_WIDGET_POT];
    const int x = widget->current_x;
    const int y = widget->current_y;
    const int w = widget->current_w;
    const int h = widget->current_h;
    const int inner_w = w - 2;
    const int fill_w = (int)((((int32_t)ui->displayed_pot_permille_q8 >> 8) * inner_w) / 1000);
    const int default_x =
        x + 1 + (int)(((uint64_t)(g_default_freq_hz - g_freq_min_hz) * inner_w) /
        (g_freq_max_hz - g_freq_min_hz));

    sh1106_draw_rect(display, x, y, w, h, true);
    sh1106_fill_rect(display, x + 1, y + 1, fill_w, h - 2, true);
    sh1106_draw_line(display, default_x, y, default_x, y + h - 1, true);

    if (status->frequency_locked) {
        sh1106_draw_line(display, x + 3, y, x + 7, y + h - 1, false);
        sh1106_draw_line(display, x + 8, y, x + 12, y + h - 1, false);
    }
}

static void ui_draw_footer(ui_t *ui, const app_status_t *status) {
    ui_widget_anim_t *widget = &ui->widgets[APP_WIDGET_HINTS];
    sh1106_draw_line(&ui->display, 0, widget->current_y - 1, 127, widget->current_y - 1, true);
    sh1106_draw_text(&ui->display, widget->current_x, widget->current_y, ui_hint_text(status), 1, false);
}

void ui_init(ui_t *ui) {
    memset(ui, 0, sizeof(*ui));
    sh1106_init(&ui->display);
    ui_seed_targets(ui);
}

void ui_render_splash(ui_t *ui, uint32_t now_ms) {
    if (ui->last_draw_ms != 0u && (now_ms - ui->last_draw_ms) < g_ui_redraw_ms) {
        return;
    }

    sh1106_clear(&ui->display, false);

    const uint8_t pulse = ui_anim_triangle_wave(now_ms, 700u);
    const int bar_width = 24 + ((pulse * 72) / 255);
    const int bar_x = (128 - bar_width) / 2;

    sh1106_draw_rect(&ui->display, 4, 4, 120, 56, true);
    sh1106_draw_rect(&ui->display, 18, 12, 92, 22, true);
    ui_draw_centered_text(&ui->display, 18, 16, 92, "ERN DOCK", 2, false);
    ui_draw_centered_text(&ui->display, 12, 39, 104, "wireless charger", 1, false);
    sh1106_draw_rect(&ui->display, 16, 52, 96, 6, true);
    sh1106_fill_rect(&ui->display, bar_x, 53, bar_width, 4, true);

    sh1106_flush(&ui->display);
    ui->last_draw_ms = now_ms;
}

void ui_render_status(ui_t *ui, const app_status_t *status, uint32_t now_ms) {
    bool changed = !ui->have_status || ui_status_changed(&ui->last_status, status);
    bool stepped = false;

    ui_apply_targets(ui, status);

    if (!ui->have_status) {
        ui->displayed_frequency_hz_q8 = (int32_t)status->displayed_frequency_hz << 8;
        ui->displayed_pot_permille_q8 = (int32_t)status->pot_normalized_permille << 8;
        ui->last_anim_ms = now_ms;
    }

    if ((now_ms - ui->last_anim_ms) >= g_ui_anim_tick_ms) {
        const uint32_t dt_ms = now_ms - ui->last_anim_ms;
        ui->last_anim_ms = now_ms;

        for (int i = 0; i < APP_WIDGET_COUNT; ++i) {
            stepped |= ui_anim_step(&ui->widgets[i], dt_ms, UI_FOCUS_ANIM_MS);
        }
        stepped |= ui_chase_q8(
            &ui->displayed_frequency_hz_q8,
            (int32_t)status->displayed_frequency_hz << 8,
            UI_NUMERIC_SMOOTH_SHIFT
        );
        stepped |= ui_chase_q8(
            &ui->displayed_pot_permille_q8,
            (int32_t)status->pot_normalized_permille << 8,
            UI_NUMERIC_SMOOTH_SHIFT
        );
    }

    if (ui->have_status && !changed && !stepped &&
        ((now_ms - ui->last_draw_ms) < g_ui_redraw_ms)) {
        return;
    }

    sh1106_clear(&ui->display, false);

    ui_draw_header(ui, status);
    ui_draw_state_banner(ui, status, now_ms);
    ui_draw_frequency(ui, status);
    ui_draw_chip(
        &ui->display,
        &ui->widgets[APP_WIDGET_LOCK],
        status->frequency_locked ? "LOCK" : "FREE",
        status->frequency_locked,
        true
    );
    ui_draw_chip(
        &ui->display,
        &ui->widgets[APP_WIDGET_ROBOT],
        status->leaf_active ? "DOCK" : "OPEN",
        status->leaf_active,
        true
    );
    ui_draw_action(ui, status, now_ms);
    ui_draw_pot_bar(ui, status);
    ui_draw_footer(ui, status);

    sh1106_flush(&ui->display);
    ui->last_draw_ms = now_ms;
    ui->last_status = *status;
    ui->have_status = true;
}
