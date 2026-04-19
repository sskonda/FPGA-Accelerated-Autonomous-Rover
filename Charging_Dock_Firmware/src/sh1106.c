#include "sh1106.h"

#include <string.h>

#include "font5x7.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"

static void sh1106_write(const uint8_t *bytes, size_t len, bool data_mode) {
    gpio_put(HW_PIN_OLED_DC, data_mode);
    gpio_put(HW_PIN_OLED_CS, 0);
    spi_write_blocking(HW_SH1106_SPI_PORT, bytes, len);
    gpio_put(HW_PIN_OLED_CS, 1);
}

static void sh1106_write_command(uint8_t command) {
    sh1106_write(&command, 1u, false);
}

static void sh1106_draw_char(sh1106_t *display, int x, int y, char c, int scale, bool invert) {
    if (scale < 1) {
        scale = 1;
    }

    uint8_t glyph_index;
    if ((c < 32) || (c > 127)) {
        glyph_index = (uint8_t)('?' - 32);
    } else {
        glyph_index = (uint8_t)(c - 32);
    }

    const int cell_width = (FONT5X7_WIDTH + 1) * scale;
    const int cell_height = FONT5X7_HEIGHT * scale;

    if (invert) {
        sh1106_fill_rect(display, x, y, cell_width, cell_height, true);
    }

    for (int col = 0; col < FONT5X7_WIDTH; ++col) {
        const uint8_t line = k_font5x7[glyph_index][col];
        for (int row = 0; row < FONT5X7_HEIGHT; ++row) {
            bool pixel_on = ((line >> row) & 0x01u) != 0u;
            if (invert) {
                pixel_on = !pixel_on;
            }
            if (!pixel_on) {
                continue;
            }
            sh1106_fill_rect(
                display,
                x + (col * scale),
                y + (row * scale),
                scale,
                scale,
                true
            );
        }
    }
}

void sh1106_init(sh1106_t *display) {
    static const uint8_t init_sequence[] = {
        0xAE,
        0xD5, 0x80,
        0xA8, 0x3F,
        0xD3, 0x00,
        0x40,
        0xAD, 0x8B,
        0xA1,
        0xC8,
        0xDA, 0x12,
        0x81, 0x8F,
        0xD9, 0x1F,
        0xDB, 0x40,
        0xA4,
        0xA6,
        0xAF
    };

    memset(display, 0, sizeof(*display));

    spi_init(HW_SH1106_SPI_PORT, HW_SH1106_SPI_BAUD_HZ);
    spi_set_format(HW_SH1106_SPI_PORT, 8u, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    gpio_set_function(HW_PIN_OLED_CLK, GPIO_FUNC_SPI);
    gpio_set_function(HW_PIN_OLED_MOSI, GPIO_FUNC_SPI);

    gpio_init(HW_PIN_OLED_CS);
    gpio_set_dir(HW_PIN_OLED_CS, GPIO_OUT);
    gpio_put(HW_PIN_OLED_CS, 1);

    gpio_init(HW_PIN_OLED_DC);
    gpio_set_dir(HW_PIN_OLED_DC, GPIO_OUT);

    gpio_init(HW_PIN_OLED_RES);
    gpio_set_dir(HW_PIN_OLED_RES, GPIO_OUT);

    gpio_put(HW_PIN_OLED_RES, 0);
    sleep_ms(5);
    gpio_put(HW_PIN_OLED_RES, 1);
    sleep_ms(10);

    for (size_t i = 0; i < sizeof(init_sequence); ++i) {
        sh1106_write_command(init_sequence[i]);
    }

    sh1106_clear(display, false);
    sh1106_flush(display);
    display->initialized = true;
}

void sh1106_clear(sh1106_t *display, bool color) {
    memset(display->framebuffer, color ? 0xFF : 0x00, sizeof(display->framebuffer));
}

void sh1106_flush(sh1106_t *display) {
    for (uint8_t page = 0; page < (SH1106_HEIGHT / 8u); ++page) {
        const uint8_t low_col = g_sh1106_col_offset & 0x0Fu;
        const uint8_t high_col = 0x10u | ((g_sh1106_col_offset >> 4) & 0x0Fu);
        sh1106_write_command((uint8_t)(0xB0u + page));
        sh1106_write_command(low_col);
        sh1106_write_command(high_col);
        sh1106_write(
            &display->framebuffer[page * SH1106_WIDTH],
            SH1106_WIDTH,
            true
        );
    }
}

void sh1106_draw_pixel(sh1106_t *display, int x, int y, bool color) {
    if ((x < 0) || (x >= (int)SH1106_WIDTH) || (y < 0) || (y >= (int)SH1106_HEIGHT)) {
        return;
    }

    const uint32_t index = (uint32_t)x + (((uint32_t)y >> 3) * SH1106_WIDTH);
    const uint8_t mask = (uint8_t)(1u << (y & 0x7));

    if (color) {
        display->framebuffer[index] |= mask;
    } else {
        display->framebuffer[index] &= (uint8_t)~mask;
    }
}

void sh1106_draw_line(sh1106_t *display, int x0, int y0, int x1, int y1, bool color) {
    int dx = (x1 > x0) ? (x1 - x0) : (x0 - x1);
    int sx = (x0 < x1) ? 1 : -1;
    int dy = -((y1 > y0) ? (y1 - y0) : (y0 - y1));
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx + dy;

    while (true) {
        sh1106_draw_pixel(display, x0, y0, color);
        if ((x0 == x1) && (y0 == y1)) {
            break;
        }
        const int e2 = err << 1;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void sh1106_fill_rect(sh1106_t *display, int x, int y, int w, int h, bool color) {
    if ((w <= 0) || (h <= 0)) {
        return;
    }

    if (x < 0) {
        w += x;
        x = 0;
    }
    if (y < 0) {
        h += y;
        y = 0;
    }
    if ((x + w) > (int)SH1106_WIDTH) {
        w = (int)SH1106_WIDTH - x;
    }
    if ((y + h) > (int)SH1106_HEIGHT) {
        h = (int)SH1106_HEIGHT - y;
    }
    if ((w <= 0) || (h <= 0)) {
        return;
    }

    for (int iy = y; iy < (y + h); ++iy) {
        for (int ix = x; ix < (x + w); ++ix) {
            sh1106_draw_pixel(display, ix, iy, color);
        }
    }
}

void sh1106_draw_rect(sh1106_t *display, int x, int y, int w, int h, bool color) {
    if ((w <= 0) || (h <= 0)) {
        return;
    }

    sh1106_draw_line(display, x, y, x + w - 1, y, color);
    sh1106_draw_line(display, x, y + h - 1, x + w - 1, y + h - 1, color);
    sh1106_draw_line(display, x, y, x, y + h - 1, color);
    sh1106_draw_line(display, x + w - 1, y, x + w - 1, y + h - 1, color);
}

void sh1106_invert_rect(sh1106_t *display, int x, int y, int w, int h) {
    if ((w <= 0) || (h <= 0)) {
        return;
    }

    for (int iy = y; iy < (y + h); ++iy) {
        for (int ix = x; ix < (x + w); ++ix) {
            if ((ix < 0) || (ix >= (int)SH1106_WIDTH) ||
                (iy < 0) || (iy >= (int)SH1106_HEIGHT)) {
                continue;
            }
            const uint32_t index = (uint32_t)ix + (((uint32_t)iy >> 3) * SH1106_WIDTH);
            const uint8_t mask = (uint8_t)(1u << (iy & 0x7));
            display->framebuffer[index] ^= mask;
        }
    }
}

void sh1106_draw_text(sh1106_t *display, int x, int y, const char *text, int scale, bool invert) {
    if (scale < 1) {
        scale = 1;
    }

    while ((*text != '\0') && (x < (int)SH1106_WIDTH)) {
        sh1106_draw_char(display, x, y, *text, scale, invert);
        x += (FONT5X7_WIDTH + 1) * scale;
        ++text;
    }
}

int sh1106_measure_text(const char *text, int scale) {
    int count = 0;

    if (scale < 1) {
        scale = 1;
    }

    while (text[count] != '\0') {
        ++count;
    }

    if (count == 0) {
        return 0;
    }

    return ((FONT5X7_WIDTH + 1) * scale * count) - scale;
}
