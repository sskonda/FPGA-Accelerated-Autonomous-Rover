#ifndef INVERTER_H
#define INVERTER_H

#include <stdbool.h>
#include <stdint.h>

void inverter_init(void);
void inverter_set_frequency(uint32_t frequency_hz);
void inverter_enable(void);
void inverter_disable(void);
void inverter_emergency_disable_isr(void);
bool inverter_is_enabled(void);
uint32_t inverter_get_frequency_hz(void);

#endif
