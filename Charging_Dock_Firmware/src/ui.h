#ifndef UI_H
#define UI_H

#include <stdbool.h>
#include <stdint.h>

#include "app_state.h"
#include "sh1106.h"
#include "ui_anim.h"

typedef struct {
    sh1106_t display;
    ui_widget_anim_t widgets[APP_WIDGET_COUNT];
    app_status_t last_status;
    bool have_status;
    uint32_t last_anim_ms;
    uint32_t last_draw_ms;
    int32_t displayed_frequency_hz_q8;
    int32_t displayed_pot_permille_q8;
} ui_t;

void ui_init(ui_t *ui);
void ui_render_splash(ui_t *ui, uint32_t now_ms);
void ui_render_status(ui_t *ui, const app_status_t *status, uint32_t now_ms);

#endif
