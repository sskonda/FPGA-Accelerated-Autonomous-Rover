#ifndef SSH1106_H
#define SSH1106_H

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include <stdbool.h>
#include <stdint.h>

#define SSH1106_WIDTH   128
#define SSH1106_HEIGHT  64
#define SSH1106_PAGES   (SSH1106_HEIGHT / 8)

// SPI1 pins
#define SSH1106_SPI_PORT spi1
#define SSH1106_PIN_CS   13
#define SSH1106_PIN_DC   8
#define SSH1106_PIN_RST  9
#define SSH1106_PIN_SCK  10
#define SSH1106_PIN_MOSI 11

#ifndef SSH1106_FONT_FILE
#define SSH1106_FONT_FILE "font5x7.inc"
#endif

#define SSH1106_FONT_WIDTH   5
#define SSH1106_FONT_HEIGHT  8

// --- Public API ---
void SSH1106_Init(void);
void SSH1106_Clear(void);
void SSH1106_Update(void);
void SSH1106_SetRotation(bool upside_down);

void SSH1106_DrawPixel(int x, int y, bool color);
void SSH1106_DrawLine(int x0, int y0, int x1, int y1);
void SSH1106_DrawRect(int x, int y, int w, int h);
void SSH1106_FillRect(int x, int y, int w, int h);
void SSH1106_DrawBall(int x, int y, int size, bool color);

void SSH1106_DrawChar(int page, int col, char c);
void SSH1106_DrawString(int page, int start_col, const char *str);

#endif
