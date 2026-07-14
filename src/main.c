#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>

// Set the base revision to 6, this is recommended as this is the latest
// base revision described by the Limine boot protocol specification.
// See specification for further info.

__attribute__((used, section(".limine_requests")))
static volatile uint64_t limine_base_revision[] = LIMINE_BASE_REVISION(6);

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent, _and_ they should be accessed at least
// once or marked as used with the "used" attribute as done here.

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0
};

// Finally, define the start and end markers for the Limine requests.
// These can also be moved anywhere, to any .c file, as seen fit.

__attribute__((used, section(".limine_requests_start")))
static volatile uint64_t limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end")))
static volatile uint64_t limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;

// GCC and Clang reserve the right to generate calls to the following
// 4 functions even if they are not directly called.
// Implement them as the C specification mandates.
// DO NOT remove or rename these functions, or stuff will eventually break!
// They CAN be moved to a different .c file.

void *memcpy(void *restrict dest, const void *restrict src, size_t n) {
    uint8_t *restrict pdest = dest;
    const uint8_t *restrict psrc = src;

    for (size_t i = 0; i < n; i++) {
        pdest[i] = psrc[i];
    }

    return dest;
}

void *memset(void *s, int c, size_t n) {
    uint8_t *p = s;

    for (size_t i = 0; i < n; i++) {
        p[i] = (uint8_t)c;
    }

    return s;
}

void *memmove(void *dest, const void *src, size_t n) {
    uint8_t *pdest = dest;
    const uint8_t *psrc = src;

    if ((uintptr_t)src > (uintptr_t)dest) {
        for (size_t i = 0; i < n; i++) {
            pdest[i] = psrc[i];
        }
    } else if ((uintptr_t)src < (uintptr_t)dest) {
        for (size_t i = n; i > 0; i--) {
            pdest[i-1] = psrc[i-1];
        }
    }

    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const uint8_t *p1 = s1;
    const uint8_t *p2 = s2;

    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }

    return 0;
}

// Halt and catch fire function.
static void hcf(void) {
    for (;;) {
        asm ("hlt");
    }
}

static size_t cursor_x = 10;
static size_t cursor_y = 10;

static const uint8_t font[][7] = {
    // A
    {
        0b01110,
        0b10001,
        0b10001,
        0b11111,
        0b10001,
        0b10001,
        0b10001
    },

    // B
    {
        0b11110,
        0b10001,
        0b10001,
        0b11110,
        0b10001,
        0b10001,
        0b11110
    },

    // C
    {
        0b01110,
        0b10001,
        0b10000,
        0b10000,
        0b10000,
        0b10001,
        0b01110
    },

    // D
    {
        0b11110,
        0b10001,
        0b10001,
        0b10001,
        0b10001,
        0b10001,
        0b11110
    },

    // E
    {
        0b11111,
        0b10000,
        0b10000,
        0b11110,
        0b10000,
        0b10000,
        0b11111
    },

    // F
    {
        0b11111,
        0b10000,
        0b10000,
        0b11110,
        0b10000,
        0b10000,
        0b10000
    },

    // G
    {
        0b01110,
        0b10001,
        0b10000,
        0b10111,
        0b10101,
        0b10001,
        0b01110
    },

    // H
    {
        0b10001,
        0b10001,
        0b10001,
        0b11111,
        0b10001,
        0b10001,
        0b10001
    },

    // I
    {
        0b11111,
        0b00100,
        0b00100,
        0b00100,
        0b00100,
        0b00100,
        0b11111
    },

    // J
    {
        0b00010,
        0b00010,
        0b00010,
        0b00010,
        0b00010,
        0b10010,
        0b01100
    },

    // K
    {
        0b10001,
        0b10001,
        0b10010,
        0b11100,
        0b10010,
        0b10001,
        0b10001
    },

    // L
    {
        0b10000,
        0b10000,
        0b10000,
        0b10000,
        0b10000,
        0b10000,
        0b11111
    },

    // M
    {
        0b10001,
        0b11011,
        0b10101,
        0b10101,
        0b10001,
        0b10001,
        0b10001
    },

    // N
    {
        0b10001,
        0b10001,
        0b11001,
        0b10101,
        0b10011,
        0b10001,
        0b10001
    },

    // O
    {
        0b01110,
        0b10001,
        0b10001,
        0b10001,
        0b10001,
        0b10001,
        0b01110
    },

    // P
    {
        0b11110,
        0b10001,
        0b10001,
        0b11110,
        0b10000,
        0b10000,
        0b10000
    },

    // Q
    {
        0b01110,
        0b10001,
        0b10001,
        0b10001,
        0b10101,
        0b10011,
        0b01111
    },

    // R
    {
        0b11110,
        0b10001,
        0b10001,
        0b11110,
        0b10001,
        0b10001,
        0b10001
    },

    // S
    {
        0b01111,
        0b10000,
        0b10000,
        0b01110,
        0b00001,
        0b00001,
        0b11110
    },

    // T
    {
        0b11111,
        0b00100,
        0b00100,
        0b00100,
        0b00100,
        0b00100,
        0b00100
    },

    // U
    {
        0b10001,
        0b10001,
        0b10001,
        0b10001,
        0b10001,
        0b10001,
        0b01110
    },

    // V
    {
        0b10001,
        0b10001,
        0b10001,
        0b10001,
        0b10001,
        0b01010,
        0b00100
    },

    // W
    {
        0b10001,
        0b10001,
        0b10001,
        0b10101,
        0b10101,
        0b11011,
        0b10001
    },

    // X
    {
        0b10001,
        0b10001,
        0b01010,
        0b00100,
        0b01010,
        0b10001,
        0b10001
    },

    // Y
    {
        0b10001,
        0b10001,
        0b01010,
        0b00100,
        0b00100,
        0b00100,
        0b00100
    },

    // Z
    {
        0b11111,
        0b00001,
        0b00010,
        0b00100,
        0b01000,
        0b10000,
        0b11111
    },

    // a
    {
      0b00000,
      0b00000,
      0b01110,
      0b00001,
      0b01111,
      0b10001,
      0b01111

    },

    // b
    {
      0b10000,
      0b10000,
      0b10000,
      0b11110,
      0b10001,
      0b10001,
      0b11110
    },
    
    // c
    {
      0b00000,
      0b00000,
      0b01110,
      0b10001,
      0b10000,
      0b10001,
      0b01110
    },
    
    // d
    {
      0b00001,
      0b00001,
      0b00001,
      0b01111,
      0b10001,
      0b10001,
      0b01111
    },
    
    // e
    {
      0b00000,
      0b00000,
      0b01110,
      0b10001,
      0b11111,
      0b10000,
      0b01111
    },
    
    // f
    {
      0b00110,
      0b01001,
      0b01000,
      0b11100,
      0b01000,
      0b01000,
      0b01000
    },
    
    // g
    {
      0b00000,
      0b00000,
      0b01110,
      0b10001,
      0b01111,
      0b00001,
      0b01110
    },
    
    // h
    {
      0b10000,
      0b10000,
      0b10000,
      0b11110,
      0b10001,
      0b10001,
      0b10001
    },

    // i
    {
        0b00100,
        0b00000,
        0b01100,
        0b00100,
        0b00100,
        0b00100,
        0b01110
    },

    // j
    {
        0b00010,
        0b00000,
        0b00110,
        0b00010,
        0b00010,
        0b10010,
        0b01100
    },

    // k
    {
        0b10000,
        0b10000,
        0b10001,
        0b10010,
        0b11100,
        0b10010,
        0b10001
    },

    // l
    {
        0b01100,
        0b00100,
        0b00100,
        0b00100,
        0b00100,
        0b00100,
        0b01110
    },

    // m
    {
        0b00000,
        0b00000,
        0b11110,
        0b10101,
        0b10101,
        0b10101,
        0b10101
    },

    // n
    {
        0b00000,
        0b00000,
        0b11110,
        0b10001,
        0b10001,
        0b10001,
        0b10001
    },

    // o
    {
        0b00000,
        0b00000,
        0b01110,
        0b10001,
        0b10001,
        0b10001,
        0b01110
    },

    // p
    {
        0b00000,
        0b00000,
        0b01110,
        0b01001,
        0b01110,
        0b01000,
        0b01000
    },

    // q
    {
        0b00000,
        0b00000,
        0b00111,
        0b01001,
        0b00111,
        0b00001,
        0b00001
    },

    // r
    {
        0b00000,
        0b00000,
        0b10111,
        0b11000,
        0b10000,
        0b10000,
        0b10000
    },

    // s
    {
        0b00000,
        0b00000,
        0b01111,
        0b10000,
        0b01110,
        0b00001,
        0b11110
    },

    // t
    {
        0b00100,
        0b00100,
        0b01110,
        0b00100,
        0b00100,
        0b00100,
        0b00011
    },

    // u
    {
        0b00000,
        0b00000,
        0b10001,
        0b10001,
        0b10001,
        0b10011,
        0b01101
    },

    // v
    {
        0b00000,
        0b00000,
        0b10001,
        0b10001,
        0b10001,
        0b01010,
        0b00100
    },

    // w
    {
        0b00000
        0b00000,
        0b10001,
        0b10001,
        0b10101,
        0b11111,
        0b10101
    },

    // x
    {
        0b00000,
        0b00000,
        0b10001,
        0b01010,
        0b00100,
        0b01010,
        0b10001
    },

    // y
    {
        0b00000,
        0b00000,
        0b10001,
        0b10001,
        0b01111,
        0b00001,
        0b11110
    },

    // z
    {
        0b00000,
        0b00000,
        0b11111,
        0b00010,
        0b00100,
        0b01000,
        0b11111
    },
    
    // 0
    {
        0b01110,
        0b10011,
        0b10101,
        0b10101,
        0b10101,
        0b11001,
        0b01110
    },

    // 1
    {
        0b00100,
        0b01100,
        0b10100,
        0b00100,
        0b00100,
        0b00100,
        0b11111
    },

    // 2
    {
        0b01110,
        0b10001,
        0b00010,
        0b00100,
        0b01000,
        0b11111,
        0b00000
    },

    // 3
    {
        0b11110,
        0b00001,
        0b00001,
        0b11110,
        0b00001,
        0b00001,
        0b11110
    },

    // 4
    {
        0b00010,
        0b00110,
        0b01010,
        0b11111,
        0b00010,
        0b00010,
        0b00010
    },
    
    // 5
    {
        0b11111,
        0b10000,
        0b11110,
        0b00001,
        0b00001,
        0b00001,
        0b11110
    },

    // 6
    {
        0b01110,
        0b10000,
        0b10000,
        0b11110,
        0b10001,
        0b10001,
        0b01110
    },

    // 7
    {
        0b11111,
        0b00010,
        0b00010,
        0b00100,
        0b00100,
        0b01000,
        0b01000
    },

    // 8
    {
        0b01110,
        0b10001,
        0b10001,
        0b01110,
        0b10001,
        0b10001,
        0b01110
    },

    // 9
    {
        0b01110,
        0b10001,
        0b10001,
        0b01111,
        0b00001,
        0b00001,
        0b01110
    },

    // !
    {
        0b01000,
        0b01000,
        0b01000,
        0b01000,
        0b01000,
        0b00000,
        0b01000
    }
};


void put_pixel(struct limine_framebuffer *fb, size_t x, size_t y, uint32_t color)
{
    uint32_t *pixel = fb->address;
    pixel[y * (fb->pitch / 4) + x] = color;
}

int char_to_index(char c)
{
    if (c >= 'A' && c <= 'Z')
        return c - 'A';

    if (c >= 'a' && c <= 'z')
        return 26 + (c - 'a');

    if (c >= '0' && c <= '9')
        return 50 + (c - '0');
    

    if (c == '!')
        return 62;

    return -1;
}

void draw_char(struct limine_framebuffer *fb, char c, size_t x, size_t y)
{
    int index = char_to_index(c);

    if (index < 0)
        return;

    for (size_t row = 0; row < 7; row++)
    {
        for (size_t col = 0; col < 5; col++)
        {
            if (font[index][row] & (1 << (4 - col)))
            {
                put_pixel(
                    fb,
                    x + col,
                    y + row,
                    0xffffffff
                );
            }
        }
    }
}


void print(struct limine_framebuffer *fb, const char *str, size_t x, size_t y)
{
    while (*str)
    {
        draw_char(fb, *str, x, y);

        x += 6; // character width + spacing

        str++;
    }
}


// The following will be our kernel's entry point.
// If renaming kmain() to something else, make sure to change the
// linker script accordingly.
void kmain(void)
{
    if (LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == false) {
        hcf();
    }

    if (framebuffer_request.response == NULL ||
        framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    struct limine_framebuffer *framebuffer =
        framebuffer_request.response->framebuffers[0];

    print(framebuffer, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890!", 10, 10);

    hcf();
}
