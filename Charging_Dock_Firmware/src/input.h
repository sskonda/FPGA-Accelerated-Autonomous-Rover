#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    INPUT_EVENT_NONE = 0u,
    INPUT_EVENT_BUTTON_SINGLE = 1u << 0,
    INPUT_EVENT_BUTTON_DOUBLE = 1u << 1,
    INPUT_EVENT_BUTTON_LONG = 1u << 2,
    INPUT_EVENT_LEAF_LOST = 1u << 3,
    INPUT_EVENT_LEAF_CHANGED = 1u << 4
} input_event_bits_t;

typedef void (*input_leaf_trip_callback_t)(void);

typedef struct {
    uint32_t events;
    bool button_pressed;
    bool leaf_active;
    uint16_t pot_raw_counts;
    uint16_t pot_filtered_counts;
    uint16_t pot_stable_counts;
    uint16_t pot_normalized_permille;
    uint32_t pot_frequency_hz;
    bool pot_changed;
    uint32_t last_pot_change_ms;
} input_report_t;

void input_init(void);
void input_set_leaf_trip_callback(input_leaf_trip_callback_t callback);
void input_poll(uint32_t now_ms);
void input_take_report(input_report_t *report);

#endif
