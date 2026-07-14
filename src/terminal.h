#ifndef TERMINAL_H
#define TERMINAL_H

#include <limine.h>

void putchar(struct limine_framebuffer *fb, char c);
void print(struct limine_framebuffer *fb, const char *str);
void reset_cursor(void);

#endif
