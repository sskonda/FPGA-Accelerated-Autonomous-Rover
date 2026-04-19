#include "SSH1106.h"
#include <stdlib.h>
#include <string.h>
#include "hardware/gpio.h"
#include "hardware/spi.h"

static uint8_t buffer[SSH1106_WIDTH * SSH1106_HEIGHT / 8];
static bool display_rotated = false;

// ------------------ Internal helpers ------------------
static void ssh1106_send_cmd(uint8_t cmd) {
    gpio_put(SSH1106_PIN_DC, 0);
    gpio_put(SSH1106_PIN_CS, 0);
    spi_write_blocking(SSH1106_SPI_PORT, &cmd, 1);
    gpio_put(SSH1106_PIN_CS, 1);
}

static void ssh1106_send_data(const uint8_t *data, size_t len) {
    gpio_put(SSH1106_PIN_DC, 1);
    gpio_put(SSH1106_PIN_CS, 0);
    spi_write_blocking(SSH1106_SPI_PORT, data, len);
    gpio_put(SSH1106_PIN_CS, 1);
}

// ------------------ Initialization ------------------
void SSH1106_Init(void) {
    spi_init(SSH1106_SPI_PORT, 8000000);
    gpio_set_function(SSH1106_PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(SSH1106_PIN_MOSI, GPIO_FUNC_SPI);

    gpio_init(SSH1106_PIN_CS);
    gpio_set_dir(SSH1106_PIN_CS, GPIO_OUT);
    gpio_put(SSH1106_PIN_CS, 1);

    gpio_init(SSH1106_PIN_DC);
    gpio_set_dir(SSH1106_PIN_DC, GPIO_OUT);

    gpio_init(SSH1106_PIN_RST);
    gpio_set_dir(SSH1106_PIN_RST, GPIO_OUT);

    gpio_put(SSH1106_PIN_RST, 0);
    sleep_ms(20);
    gpio_put(SSH1106_PIN_RST, 1);
    sleep_ms(20);

    uint8_t init_cmds[] = {
    0xAE,          // Display OFF
    0xD5, 0x80,    // Clock divide
    0xA8, 0x3F,    // Multiplex ratio
    0xD3, 0x00,    // Display offset
    0x40,          // Start line
    (display_rotated ? 0xA0 : 0xA1),  // SEG remap
    (display_rotated ? 0xC0 : 0xC8),  // COM scan direction
    0xDA, 0x12,    // COM pins config
    0x81, 0x7F,    // Contrast
    0xD9, 0xF1,    // Precharge
    0xDB, 0x40,    // VCOM detect
    0xA4,          // Resume RAM content
    0xA6,          // Normal display
    0xAF           // Display ON
};

    for (size_t i=0; i<sizeof(init_cmds); i++)
        ssh1106_send_cmd(init_cmds[i]);

    SSH1106_Clear();
    SSH1106_Update();
}

// ------------------ Rotation Function ------------------
void SSH1106_SetRotation(bool upside_down){
    display_rotated = upside_down;

    //re-run remap related commands
    ssh1106_send_cmd(display_rotated ? 0xA0 : 0xA1);
    ssh1106_send_cmd(display_rotated ? 0xC0 : 0xC8);
    SSH1106_Update();
}

// ------------------ Framebuffer ops ------------------
void SSH1106_Clear(void) {
    memset(buffer, 0, sizeof(buffer));
}

void SSH1106_Update(void) {
    for (uint8_t page = 0; page < SSH1106_PAGES; page++) {
        ssh1106_send_cmd(0xB0 + page);
        ssh1106_send_cmd(0x02);
        ssh1106_send_cmd(0x10);
        ssh1106_send_data(&buffer[page * SSH1106_WIDTH], SSH1106_WIDTH);
    }
}

// ------------------ Safe pixel draw ------------------
void SSH1106_DrawPixel(int x, int y, bool color) {
    if (x < 0 || x >= SSH1106_WIDTH || y < 0 || y >= SSH1106_HEIGHT)
        return;

    int page = y >> 3;
    int bit  = 1 << (y & 7);
    int idx  = x + page * SSH1106_WIDTH;

    if (color)
        buffer[idx] |= bit;
    else
        buffer[idx] &= ~bit;
}

// ------------------ Font table ------------------
static const uint8_t font5x7[96][SSH1106_FONT_WIDTH] = {
#include SSH1106_FONT_FILE
};

// ------------------ Text drawing ------------------
void SSH1106_DrawChar(int page, int col, char c) {
    if (page >= SSH1106_PAGES || col >= SSH1106_WIDTH) return;
    if (c < 32 || c > 127) c = '?';

    for (int i = 0; i < SSH1106_FONT_WIDTH; i++) {
        uint8_t line = font5x7[c - 32][i];
        for (int bit = 0; bit < SSH1106_FONT_HEIGHT; bit++) {
            SSH1106_DrawPixel(col, page * 8 + bit, (line >> bit) & 1);
        }
        col++;
        if (col >= SSH1106_WIDTH) return;
    }
    col++;
}

void SSH1106_DrawString(int page, int start_col, const char *str) {
    int col = start_col;
    while (*str && col < SSH1106_WIDTH - (SSH1106_FONT_WIDTH + 1)) {
        SSH1106_DrawChar(page, col, *str++);
        col += SSH1106_FONT_WIDTH + 1;
    }
}

// ------------------ Basic shapes ------------------
void SSH1106_FillRect(int x, int y, int w, int h) {
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x >= SSH1106_WIDTH || y >= SSH1106_HEIGHT) return;
    if (x + w > SSH1106_WIDTH)  w = SSH1106_WIDTH - x;
    if (y + h > SSH1106_HEIGHT) h = SSH1106_HEIGHT - y;
    if (w <= 0 || h <= 0) return;

    for (int i = x; i < x + w; i++)
        for (int j = y; j < y + h; j++)
            SSH1106_DrawPixel(i, j, true);
}

void SSH1106_DrawRect(int x, int y, int w, int h) {
    SSH1106_FillRect(x, y, w, 1);
    SSH1106_FillRect(x, y + h - 1, w, 1);
    SSH1106_FillRect(x, y, 1, h);
    SSH1106_FillRect(x + w - 1, y, 1, h);
}

void SSH1106_DrawLine(int x0, int y0, int x1, int y1) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    while (true) {
        SSH1106_DrawPixel(x0, y0, true);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

// ------------------ Ball / Square Helper ------------------
void SSH1106_DrawBall(int x, int y, int size, bool color) {
    if (x >= SSH1106_WIDTH || y >= SSH1106_HEIGHT || size <= 0) return;

    // Clip to screen boundaries
    if (x + size > SSH1106_WIDTH)  size = SSH1106_WIDTH - x;
    if (y + size > SSH1106_HEIGHT) size = SSH1106_HEIGHT - y;

    for (int dx = 0; dx < size; dx++) {
        for (int dy = 0; dy < size; dy++) {
            SSH1106_DrawPixel(x + dx, y + dy, color);
        }
    }
}
