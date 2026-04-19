#include "app_state.h"

#include <string.h>

#include "config.h"
#include "input.h"
#include "inverter.h"

static void app_enter_state(app_t *app, app_state_id_t next_state, uint32_t now_ms) {
    if (app->status.state != next_state) {
        app->status.state = next_state;
        app->state_entered_ms = now_ms;
    }
}

static void app_latch_fault(app_t *app, app_fault_reason_t reason, uint32_t now_ms) {
    inverter_disable();
    app->latched_fault = reason;
    app->fault_until_ms = now_ms + APP_FAULT_HOLD_MS;
    app->blocked_until_ms = 0u;
}

static app_widget_id_t app_compute_focus(const app_t *app, const input_report_t *input) {
    (void)input;

    switch (app->status.state) {
        case APP_STATE_BOOT:
        case APP_STATE_BLOCKED:
        case APP_STATE_FAULT:
        case APP_STATE_LOCKED:
            return APP_WIDGET_STATE;
        case APP_STATE_READY:
            return APP_WIDGET_ACTION;
        case APP_STATE_ACTIVE:
            return app->frequency_locked ? APP_WIDGET_ACTION : APP_WIDGET_FREQUENCY;
        case APP_STATE_OFF:
        case APP_STATE_TUNING:
        default:
            return APP_WIDGET_FREQUENCY;
    }
}

static app_widget_id_t app_compute_editable(const app_t *app) {
    if (!app->frequency_locked) {
        return APP_WIDGET_FREQUENCY;
    }

    if (app->status.state == APP_STATE_READY || app->status.state == APP_STATE_ACTIVE) {
        return APP_WIDGET_ACTION;
    }

    return APP_WIDGET_NONE;
}

static void app_fill_status(app_t *app, const input_report_t *input, uint32_t now_ms) {
    const bool inverter_enabled = inverter_is_enabled();

    app->status.frequency_locked = app->frequency_locked;
    app->status.leaf_active = input->leaf_active;
    app->status.inverter_enabled = inverter_enabled;
    app->status.fault_reason = app->latched_fault;
    app->status.now_ms = now_ms;
    app->status.state_age_ms = now_ms - app->state_entered_ms;
    app->status.selected_frequency_hz = app->selected_frequency_hz;
    app->status.active_frequency_hz = inverter_enabled ? inverter_get_frequency_hz() : 0u;
    app->status.displayed_frequency_hz =
        inverter_enabled ? app->status.active_frequency_hz : app->selected_frequency_hz;
    app->status.pot_frequency_hz = input->pot_frequency_hz;
    app->status.pot_normalized_permille = input->pot_normalized_permille;
    app->status.tuning_live =
        (!app->frequency_locked) && ((now_ms - app->last_tune_ms) <= APP_TUNING_LINGER_MS);
    app->status.selected_frequency_editable = !app->frequency_locked;
    app->status.focused_widget = app_compute_focus(app, input);
    app->status.editable_widget = app_compute_editable(app);
}

void app_init(app_t *app, uint32_t now_ms) {
    memset(app, 0, sizeof(*app));

    app->boot_started_ms = now_ms;
    app->state_entered_ms = now_ms;
    app->selected_frequency_hz = g_default_freq_hz;
    app->frequency_locked = false;
    app->latched_fault = APP_FAULT_NONE;
    app->last_tune_ms = now_ms - APP_TUNING_LINGER_MS;
    app->status.state = APP_STATE_BOOT;

    input_poll(now_ms);

    input_report_t input = {0};
    input_take_report(&input);
    app_fill_status(app, &input, now_ms);
}

void app_update(app_t *app, uint32_t now_ms) {
    input_report_t input = {0};

    input_poll(now_ms);
    input_take_report(&input);

    if ((input.events & INPUT_EVENT_BUTTON_LONG) != 0u) {
        app_latch_fault(app, APP_FAULT_EMERGENCY_STOP, now_ms);
    }

    if (((input.events & INPUT_EVENT_LEAF_LOST) != 0u) && inverter_is_enabled()) {
        app_latch_fault(app, APP_FAULT_INTERLOCK_OPEN, now_ms);
    }

    if (inverter_is_enabled() && !input.leaf_active) {
        app_latch_fault(app, APP_FAULT_INTERLOCK_OPEN, now_ms);
    }

    if ((now_ms - app->boot_started_ms) < APP_BOOT_SPLASH_MS) {
        app_enter_state(app, APP_STATE_BOOT, now_ms);
        app_fill_status(app, &input, now_ms);
        return;
    }

    if ((app->latched_fault != APP_FAULT_NONE) && (now_ms >= app->fault_until_ms)) {
        app->latched_fault = APP_FAULT_NONE;
    }

    if (input.pot_changed && !app->frequency_locked) {
        app->selected_frequency_hz = input.pot_frequency_hz;
        app->last_tune_ms = now_ms;
        if (inverter_is_enabled()) {
            inverter_set_frequency(app->selected_frequency_hz);
        }
    }

    if ((input.events & INPUT_EVENT_BUTTON_SINGLE) != 0u) {
        app->frequency_locked = !app->frequency_locked;
    }

    if ((input.events & INPUT_EVENT_BUTTON_DOUBLE) != 0u) {
        if (inverter_is_enabled()) {
            inverter_disable();
        } else if (input.leaf_active) {
            inverter_set_frequency(app->selected_frequency_hz);
            inverter_enable();
        } else {
            app->blocked_until_ms = now_ms + APP_BLOCKED_HOLD_MS;
        }
    }

    if (app->latched_fault != APP_FAULT_NONE) {
        app_enter_state(app, APP_STATE_FAULT, now_ms);
    } else if (now_ms < app->blocked_until_ms) {
        app_enter_state(app, APP_STATE_BLOCKED, now_ms);
    } else if (inverter_is_enabled()) {
        app_enter_state(app, APP_STATE_ACTIVE, now_ms);
    } else if (!app->frequency_locked) {
        if ((now_ms - app->last_tune_ms) <= APP_TUNING_LINGER_MS) {
            app_enter_state(app, APP_STATE_TUNING, now_ms);
        } else {
            app_enter_state(app, APP_STATE_OFF, now_ms);
        }
    } else if (input.leaf_active) {
        app_enter_state(app, APP_STATE_READY, now_ms);
    } else {
        app_enter_state(app, APP_STATE_LOCKED, now_ms);
    }

    app_fill_status(app, &input, now_ms);
}

const app_status_t *app_get_status(const app_t *app) {
    return &app->status;
}
