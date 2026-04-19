#include "input.h"

#include <string.h>

#include "config.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"

typedef struct {
    bool stable_active;
    uint8_t integrator;
} debounced_input_t;

typedef struct {
    debounced_input_t button;
    debounced_input_t leaf;
    uint32_t last_scan_ms;
    uint32_t event_bits;
    uint32_t button_press_ms;
    uint32_t first_release_ms;
    uint32_t last_pot_change_ms;
    uint32_t filtered_q8;
    uint16_t raw_counts;
    uint16_t filtered_counts;
    uint16_t stable_counts;
    uint16_t pot_normalized_permille;
    uint32_t pot_frequency_hz;
    bool waiting_second_click;
    bool long_fired;
    bool pot_changed_pending;
    bool adc_initialized;
    input_leaf_trip_callback_t leaf_trip_callback;
} input_state_t;

static input_state_t s_input;
static volatile bool s_leaf_trip_pending;

static bool input_logic_level(bool gpio_level, bool active_level) {
    return gpio_level == active_level;
}

static bool input_update_debounced(debounced_input_t *debounced, bool raw_active) {
    const uint8_t debounce_ticks = 3u;
    bool next_state = debounced->stable_active;

    if (raw_active) {
        if (debounced->integrator < debounce_ticks) {
            debounced->integrator++;
        }
    } else if (debounced->integrator > 0u) {
        debounced->integrator--;
    }

    if (debounced->integrator == 0u) {
        next_state = false;
    } else if (debounced->integrator >= debounce_ticks) {
        next_state = true;
    }

    if (next_state == debounced->stable_active) {
        return false;
    }

    debounced->stable_active = next_state;
    return true;
}

static uint32_t input_map_adc_to_freq(uint16_t counts) {
    const uint32_t span_hz = g_freq_max_hz - g_freq_min_hz;
    const uint32_t scaled =
        (uint32_t)(((uint64_t)counts * span_hz + 2047u) / 4095u);
    return g_freq_min_hz + scaled;
}

static void input_leaf_gpio_irq(uint gpio, uint32_t events) {
    (void)gpio;
    (void)events;

    if (!input_logic_level(gpio_get(HW_PIN_LEAF_SWITCH) != 0, g_leaf_active_level)) {
        s_leaf_trip_pending = true;
        if (s_input.leaf_trip_callback != NULL) {
            s_input.leaf_trip_callback();
        }
    }
}

void input_init(void) {
    memset(&s_input, 0, sizeof(s_input));
    s_input.last_scan_ms = (uint32_t)(0u - g_input_scan_ms);

    gpio_init(HW_PIN_BUTTON);
    gpio_set_dir(HW_PIN_BUTTON, GPIO_IN);
    gpio_pull_up(HW_PIN_BUTTON);

    gpio_init(HW_PIN_LEAF_SWITCH);
    gpio_set_dir(HW_PIN_LEAF_SWITCH, GPIO_IN);
    gpio_pull_up(HW_PIN_LEAF_SWITCH);

    s_input.button.stable_active =
        input_logic_level(gpio_get(HW_PIN_BUTTON) != 0, g_button_active_level);
    s_input.button.integrator = s_input.button.stable_active ? 3u : 0u;

    s_input.leaf.stable_active =
        input_logic_level(gpio_get(HW_PIN_LEAF_SWITCH) != 0, g_leaf_active_level);
    s_input.leaf.integrator = s_input.leaf.stable_active ? 3u : 0u;

    adc_init();
    adc_gpio_init(HW_PIN_POT_ADC_GPIO);
    adc_select_input(HW_POT_ADC_INPUT);

    gpio_set_irq_enabled_with_callback(
        HW_PIN_LEAF_SWITCH,
        GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL,
        true,
        &input_leaf_gpio_irq
    );
}

void input_set_leaf_trip_callback(input_leaf_trip_callback_t callback) {
    s_input.leaf_trip_callback = callback;
}

void input_poll(uint32_t now_ms) {
    if ((now_ms - s_input.last_scan_ms) < g_input_scan_ms) {
        return;
    }
    s_input.last_scan_ms = now_ms;

    if (s_input.waiting_second_click &&
        !s_input.button.stable_active &&
        (now_ms - s_input.first_release_ms > g_double_click_ms)) {
        s_input.event_bits |= INPUT_EVENT_BUTTON_SINGLE;
        s_input.waiting_second_click = false;
    }

    const bool raw_button =
        input_logic_level(gpio_get(HW_PIN_BUTTON) != 0, g_button_active_level);
    const bool raw_leaf =
        input_logic_level(gpio_get(HW_PIN_LEAF_SWITCH) != 0, g_leaf_active_level);

    const bool button_changed = input_update_debounced(&s_input.button, raw_button);
    const bool leaf_changed = input_update_debounced(&s_input.leaf, raw_leaf);

    if (leaf_changed) {
        s_input.event_bits |= INPUT_EVENT_LEAF_CHANGED;
        if (!s_input.leaf.stable_active) {
            s_input.event_bits |= INPUT_EVENT_LEAF_LOST;
        }
    }

    if (s_leaf_trip_pending) {
        s_input.event_bits |= INPUT_EVENT_LEAF_LOST;
        s_leaf_trip_pending = false;
    }

    if (button_changed) {
        if (s_input.button.stable_active) {
            if (s_input.waiting_second_click &&
                (now_ms - s_input.first_release_ms > g_double_click_ms)) {
                s_input.event_bits |= INPUT_EVENT_BUTTON_SINGLE;
                s_input.waiting_second_click = false;
            }
            s_input.button_press_ms = now_ms;
            s_input.long_fired = false;
        } else if (!s_input.long_fired) {
            if (s_input.waiting_second_click &&
                (now_ms - s_input.first_release_ms <= g_double_click_ms)) {
                s_input.event_bits |= INPUT_EVENT_BUTTON_DOUBLE;
                s_input.waiting_second_click = false;
            } else {
                s_input.waiting_second_click = true;
                s_input.first_release_ms = now_ms;
            }
        } else {
            s_input.waiting_second_click = false;
        }
    }

    if (s_input.button.stable_active &&
        !s_input.long_fired &&
        (now_ms - s_input.button_press_ms >= g_long_press_ms)) {
        s_input.event_bits |= INPUT_EVENT_BUTTON_LONG;
        s_input.long_fired = true;
        s_input.waiting_second_click = false;
    }

    adc_select_input(HW_POT_ADC_INPUT);
    s_input.raw_counts = adc_read() & 0x0FFFu;

    if (!s_input.adc_initialized) {
        s_input.filtered_q8 = (uint32_t)s_input.raw_counts << 8;
        s_input.filtered_counts = s_input.raw_counts;
        s_input.stable_counts = s_input.raw_counts;
        s_input.adc_initialized = true;
        s_input.pot_changed_pending = true;
        s_input.last_pot_change_ms = now_ms;
    } else {
        const uint32_t target_q8 = (uint32_t)s_input.raw_counts << 8;
        s_input.filtered_q8 +=
            (target_q8 - s_input.filtered_q8) >> g_pot_filter_shift;
        s_input.filtered_counts = (uint16_t)(s_input.filtered_q8 >> 8);

        const uint16_t delta = (s_input.filtered_counts > s_input.stable_counts)
            ? (uint16_t)(s_input.filtered_counts - s_input.stable_counts)
            : (uint16_t)(s_input.stable_counts - s_input.filtered_counts);

        if (delta >= g_pot_hysteresis_counts) {
            s_input.stable_counts = s_input.filtered_counts;
            s_input.pot_changed_pending = true;
            s_input.last_pot_change_ms = now_ms;
        }
    }

    s_input.pot_normalized_permille =
        (uint16_t)(((uint32_t)s_input.stable_counts * 1000u + 2047u) / 4095u);
    s_input.pot_frequency_hz = input_map_adc_to_freq(s_input.stable_counts);
}

void input_take_report(input_report_t *report) {
    memset(report, 0, sizeof(*report));

    report->events = s_input.event_bits;
    report->button_pressed = s_input.button.stable_active;
    report->leaf_active = s_input.leaf.stable_active;
    report->pot_raw_counts = s_input.raw_counts;
    report->pot_filtered_counts = s_input.filtered_counts;
    report->pot_stable_counts = s_input.stable_counts;
    report->pot_normalized_permille = s_input.pot_normalized_permille;
    report->pot_frequency_hz = s_input.pot_frequency_hz;
    report->pot_changed = s_input.pot_changed_pending;
    report->last_pot_change_ms = s_input.last_pot_change_ms;

    s_input.event_bits = INPUT_EVENT_NONE;
    s_input.pot_changed_pending = false;
}
