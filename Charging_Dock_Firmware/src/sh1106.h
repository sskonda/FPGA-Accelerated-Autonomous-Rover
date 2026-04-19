#ifndef SH1106_H
#define SH1106_H

#include <stdbool.h>
#include <stdint.h>

#include "config.h"

typedef struct {
    bool initialized;
    uint8_t framebuffer[SH1106_FRAMEBUFFER_SIZE];
} sh1106_t;

void sh1106_init(sh1106_t *display);
void sh1106_clear(sh1106_t *display, bool color);
void sh1106_flush(sh1106_t *display);
void sh1106_draw_pixel(sh1106_t *display, int x, int y, bool color);
void sh1106_draw_line(sh1106_t *display, int x0, int y0, int x1, int y1, bool color);
void sh1106_draw_rect(sh1106_t *display, int x, int y, int w, int h, bool color);
void sh1106_fill_rect(sh1106_t *display, int x, int y, int w, int h, bool color);
void sh1106_invert_rect(sh1106_t *display, int x, int y, int w, int h);
void sh1106_draw_text(sh1106_t *display, int x, int y, const char *text, int scale, bool invert);
int sh1106_measure_text(const char *text, int scale);

#endif
