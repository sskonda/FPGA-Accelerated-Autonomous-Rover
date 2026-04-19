#include "pico/stdlib.h"
#include "hardware/clocks.h"

#include "app_state.h"
#include "input.h"
#include "inverter.h"
#include "ui.h"

int main(void) {
    set_sys_clock_khz(125000, true);
    stdio_init_all();

    inverter_init();
    input_init();
    input_set_leaf_trip_callback(inverter_emergency_disable_isr);

    ui_t ui;
    ui_init(&ui);

    const uint32_t boot_ms = to_ms_since_boot(get_absolute_time());

    app_t app;
    app_init(&app, boot_ms);

    while (true) {
        const uint32_t now_ms = to_ms_since_boot(get_absolute_time());

        app_update(&app, now_ms);

        const app_status_t *status = app_get_status(&app);
        if (status->state == APP_STATE_BOOT) {
            ui_render_splash(&ui, now_ms);
        } else {
            ui_render_status(&ui, status, now_ms);
        }

        tight_loop_contents();
    }
}
