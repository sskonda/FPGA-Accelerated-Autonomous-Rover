#ifndef APP_STATE_H
#define APP_STATE_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    APP_STATE_BOOT = 0,
    APP_STATE_OFF,
    APP_STATE_TUNING,
    APP_STATE_LOCKED,
    APP_STATE_READY,
    APP_STATE_ACTIVE,
    APP_STATE_BLOCKED,
    APP_STATE_FAULT
} app_state_id_t;

typedef enum {
    APP_FAULT_NONE = 0,
    APP_FAULT_INTERLOCK_OPEN,
    APP_FAULT_EMERGENCY_STOP
} app_fault_reason_t;

typedef enum {
    APP_WIDGET_HEADER = 0,
    APP_WIDGET_STATE,
    APP_WIDGET_FREQUENCY,
    APP_WIDGET_LOCK,
    APP_WIDGET_ROBOT,
    APP_WIDGET_ACTION,
    APP_WIDGET_POT,
    APP_WIDGET_HINTS,
    APP_WIDGET_COUNT,
    APP_WIDGET_NONE = 0xFF
} app_widget_id_t;

typedef struct {
    app_state_id_t state;
    app_fault_reason_t fault_reason;
    bool frequency_locked;
    bool leaf_active;
    bool inverter_enabled;
    bool tuning_live;
    bool selected_frequency_editable;
    uint32_t now_ms;
    uint32_t state_age_ms;
    uint32_t selected_frequency_hz;
    uint32_t active_frequency_hz;
    uint32_t displayed_frequency_hz;
    uint32_t pot_frequency_hz;
    uint16_t pot_normalized_permille;
    app_widget_id_t focused_widget;
    app_widget_id_t editable_widget;
} app_status_t;

typedef struct {
    app_status_t status;
    uint32_t boot_started_ms;
    uint32_t state_entered_ms;
    uint32_t blocked_until_ms;
    uint32_t fault_until_ms;
    uint32_t last_tune_ms;
    uint32_t selected_frequency_hz;
    bool frequency_locked;
    app_fault_reason_t latched_fault;
} app_t;

void app_init(app_t *app, uint32_t now_ms);
void app_update(app_t *app, uint32_t now_ms);
const app_status_t *app_get_status(const app_t *app);

#endif
