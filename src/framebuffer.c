#include <limine.h>
#include <stdint.h>
#include <stddef.h>

#include "framebuffer.h"
#include "string.h"
#include "terminal.h"

void clear_screen(struct limine_framebuffer *fb)
{
    memset(
        fb->address,
        0,
        fb->pitch * fb->height
    );

    reset_cursor();
}

void put_pixel(struct limine_framebuffer *fb,
               size_t x,
               size_t y,
               uint32_t color){
    if (x >= fb->width || y >= fb->height)
        return;

    uint32_t *pixel = fb->address;
    pixel[y * (fb->pitch / 4) + x] = color;
}

void scroll_screen(struct limine_framebuffer *fb) {
    // Move everything up by one text row (8 pixels).
    memmove(
        fb->address,
        (uint8_t *)fb->address + fb->pitch * 8,
        fb->pitch * (fb->height - 8)
    );

    // Clear the bottom 8 pixel rows.
    memset(
        (uint8_t *)fb->address + fb->pitch * (fb->height - 8),
        0,
        fb->pitch * 8
    );
}
