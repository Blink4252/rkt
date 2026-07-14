#include <stdint.h>
#include <stddef.h>
#include <limine.h>

#include "terminal.h"
#include "framebuffer.h"
#include "font.h"

static size_t cursor_x = 10;
static size_t cursor_y = 10;

static void newline(struct limine_framebuffer *fb) {
    cursor_x = 10;
    cursor_y += 8;

    if (cursor_y + 7 >= fb->height)
    {
        scroll_screen(fb);
        cursor_y -= 8;
    }
}

void draw_char(struct limine_framebuffer *fb, char c, size_t x, size_t y) {
    unsigned char ch = (unsigned char)c;

    for (size_t row = 0; row < 7; row++)
    {
        for (size_t col = 0; col < 5; col++)
        {
            uint32_t color;

            if (font[ch][row] & (1 << (4 - col)))
                color = 0xffffffff; // white
            else
                color = 0x00000000; // black

            put_pixel(fb, x + col, y + row, color);
        }
    }
}


void putchar(struct limine_framebuffer *fb, char c) {
    switch (c)
    {
    case '\n':
        newline(fb);
        return;

    case '\b':
        if (cursor_x > 10)
        {
            cursor_x -= 6;
            draw_char(fb, ' ', cursor_x, cursor_y);
        }
        return;
    }

    // Wrap BEFORE drawing the character.
    if (cursor_x + 5 >= fb->width - 10) {
        newline(fb);
    }

    draw_char(fb, c, cursor_x, cursor_y);
    cursor_x += 6;
}

void print(struct limine_framebuffer *fb, const char *str) {
    while (*str)
    {
        if (*str == '\n')
        {
            cursor_x = 10;
            cursor_y += 8;
        }
        else
        {
            putchar(fb, *str);
        }

        str++;
    }
}

void reset_cursor(void)
{
    cursor_x = 10;
    cursor_y = 10;
}
