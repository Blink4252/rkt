#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <limine.h>
#include <stdint.h>
#include <stddef.h>

void put_pixel(
    struct limine_framebuffer *fb,
    size_t x,
    size_t y,
    uint32_t color
);

void clear_screen(struct limine_framebuffer *fb);
void scroll_screen(struct limine_framebuffer *fb);

#endif
