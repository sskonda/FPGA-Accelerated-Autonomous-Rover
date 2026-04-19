#include "inverter.h"

#include "config.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/sync.h"

typedef struct {
    uint slice_num;
    bool initialized;
    volatile bool enabled;
    volatile uint32_t target_frequency_hz;
    volatile uint32_t active_frequency_hz;
} inverter_state_t;

static inverter_state_t s_inverter;

static uint32_t inverter_clamp_frequency(uint32_t frequency_hz) {
    if (frequency_hz < g_freq_min_hz) {
        return g_freq_min_hz;
    }
    if (frequency_hz > g_freq_max_hz) {
        return g_freq_max_hz;
    }
    return frequency_hz;
}

static void inverter_write_gate_enable(bool enabled) {
    gpio_put(HW_PIN_HEN, enabled ? g_hen_active_level : !g_hen_active_level);
    gpio_put(HW_PIN_DIS, enabled ? !g_dis_active_level : g_dis_active_level);
}

static void inverter_drive_idle_outputs(void) {
    gpio_set_function(HW_PIN_IN_PLUS, GPIO_FUNC_SIO);
    gpio_set_function(HW_PIN_IN_MINUS, GPIO_FUNC_SIO);
    gpio_set_dir(HW_PIN_IN_PLUS, GPIO_OUT);
    gpio_set_dir(HW_PIN_IN_MINUS, GPIO_OUT);
    gpio_put(HW_PIN_IN_PLUS, INVERTER_IN_PLUS_IDLE_LEVEL);
    gpio_put(HW_PIN_IN_MINUS, INVERTER_IN_MINUS_IDLE_LEVEL);
}

static void inverter_program_waveform_locked(uint32_t requested_hz) {
    const uint32_t clock_hz = clock_get_hz(clk_sys);
    const uint32_t clamped_hz = inverter_clamp_frequency(requested_hz);
    uint32_t period_counts = (clock_hz + (clamped_hz / 2u)) / clamped_hz;

    if (period_counts < 2u) {
        period_counts = 2u;
    }
    if (period_counts > 65535u) {
        period_counts = 65535u;
    }

    const uint16_t wrap = (uint16_t)(period_counts - 1u);
    const uint16_t half_level = (uint16_t)(period_counts / 2u);

    gpio_set_function(HW_PIN_IN_PLUS, GPIO_FUNC_PWM);
    gpio_set_function(HW_PIN_IN_MINUS, GPIO_FUNC_PWM);

    pwm_set_clkdiv_int_frac(s_inverter.slice_num, 1u, 0u);
    pwm_set_phase_correct(s_inverter.slice_num, false);
    pwm_set_output_polarity(
        s_inverter.slice_num,
        !INVERTER_IN_PLUS_ACTIVE_LEVEL,
        INVERTER_IN_MINUS_ACTIVE_LEVEL
    );
    pwm_set_wrap(s_inverter.slice_num, wrap);
    pwm_set_chan_level(s_inverter.slice_num, PWM_CHAN_A, half_level);
    pwm_set_chan_level(s_inverter.slice_num, PWM_CHAN_B, half_level);

    s_inverter.active_frequency_hz = clock_hz / period_counts;
}

static void inverter_disable_locked(void) {
    inverter_write_gate_enable(false);
    pwm_set_enabled(s_inverter.slice_num, false);
    inverter_drive_idle_outputs();
    s_inverter.enabled = false;
    s_inverter.active_frequency_hz = 0u;
}

void inverter_init(void) {
    s_inverter.slice_num = pwm_gpio_to_slice_num(HW_PIN_IN_PLUS);
    s_inverter.target_frequency_hz = inverter_clamp_frequency(g_default_freq_hz);
    s_inverter.active_frequency_hz = 0u;
    s_inverter.enabled = false;

    gpio_init(HW_PIN_HEN);
    gpio_set_dir(HW_PIN_HEN, GPIO_OUT);

    gpio_init(HW_PIN_DIS);
    gpio_set_dir(HW_PIN_DIS, GPIO_OUT);

    gpio_init(HW_PIN_IN_PLUS);
    gpio_set_dir(HW_PIN_IN_PLUS, GPIO_OUT);

    gpio_init(HW_PIN_IN_MINUS);
    gpio_set_dir(HW_PIN_IN_MINUS, GPIO_OUT);

    inverter_disable_locked();
    s_inverter.initialized = true;
}

void inverter_set_frequency(uint32_t frequency_hz) {
    const uint32_t clamped_hz = inverter_clamp_frequency(frequency_hz);
    const uint32_t irq_state = save_and_disable_interrupts();

    s_inverter.target_frequency_hz = clamped_hz;
    if (s_inverter.enabled) {
        inverter_program_waveform_locked(clamped_hz);
        pwm_set_enabled(s_inverter.slice_num, true);
    }

    restore_interrupts(irq_state);
}

void inverter_enable(void) {
    const uint32_t irq_state = save_and_disable_interrupts();

    inverter_program_waveform_locked(s_inverter.target_frequency_hz);
    inverter_write_gate_enable(false);
    pwm_set_enabled(s_inverter.slice_num, true);
    inverter_write_gate_enable(true);
    s_inverter.enabled = true;

    restore_interrupts(irq_state);
}

void inverter_disable(void) {
    const uint32_t irq_state = save_and_disable_interrupts();
    inverter_disable_locked();
    restore_interrupts(irq_state);
}

void inverter_emergency_disable_isr(void) {
    if (!s_inverter.initialized) {
        return;
    }
    inverter_disable_locked();
}

bool inverter_is_enabled(void) {
    return s_inverter.enabled;
}

uint32_t inverter_get_frequency_hz(void) {
    return s_inverter.active_frequency_hz;
}
