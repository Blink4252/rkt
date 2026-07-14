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



static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    asm volatile ("inb %1, %0"
                  : "=a"(value)
                  : "Nd"(port));
    return value;
}

static inline void io_wait(void) {
    asm volatile ("outb %%al, $0x80" : : "a"(0));
}

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

int strcmp(const char *a, const char *b)
{
    while (*a && (*a == *b))
    {
        a++;
        b++;
    }

    return *(unsigned char *)a - *(unsigned char *)b;
}

static size_t cursor_x = 10;
static size_t cursor_y = 10;

static bool shift_pressed = false;
static bool caps_lock = false;

#define LINE_MAX 128

static char line_buffer[LINE_MAX];
static size_t line_length = 0;

void putchar(struct limine_framebuffer *fb, char c);
void print(struct limine_framebuffer *fb, const char *str);
char keyboard_getchar(void);

void read_line(struct limine_framebuffer *fb);
void print_prompt(struct limine_framebuffer *fb);
void shell(struct limine_framebuffer *fb);

static const char scancode_to_ascii[128] = {
    [0x02] = '1',
    [0x03] = '2',
    [0x04] = '3',
    [0x05] = '4',
    [0x06] = '5',
    [0x07] = '6',
    [0x08] = '7',
    [0x09] = '8',
    [0x0A] = '9',
    [0x0B] = '0',

    [0x0C] = '-',
    [0x0D] = '=',

    [0x0E] = '\b',
    [0x0F] = '\t',

    [0x10] = 'q',
    [0x11] = 'w',
    [0x12] = 'e',
    [0x13] = 'r',
    [0x14] = 't',
    [0x15] = 'y',
    [0x16] = 'u',
    [0x17] = 'i',
    [0x18] = 'o',
    [0x19] = 'p',

    [0x1A] = '[',
    [0x1B] = ']',

    [0x1C] = '\n',

    [0x1E] = 'a',
    [0x1F] = 's',
    [0x20] = 'd',
    [0x21] = 'f',
    [0x22] = 'g',
    [0x23] = 'h',
    [0x24] = 'j',
    [0x25] = 'k',
    [0x26] = 'l',

    [0x27] = ';',
    [0x28] = '\'',
    [0x29] = '`',

    [0x2B] = '\\',

    [0x2C] = 'z',
    [0x2D] = 'x',
    [0x2E] = 'c',
    [0x2F] = 'v',
    [0x30] = 'b',
    [0x31] = 'n',
    [0x32] = 'm',

    [0x33] = ',',
    [0x34] = '.',
    [0x35] = '/',

    [0x39] = ' ',
};

static const char scancode_to_ascii_shift[128] = {
    [0x02] = '!',
    [0x03] = '@',
    [0x04] = '#',
    [0x05] = '$',
    [0x06] = '%',
    [0x07] = '^',
    [0x08] = '&',
    [0x09] = '*',
    [0x0A] = '(',
    [0x0B] = ')',

    [0x0C] = '_',
    [0x0D] = '+',

    [0x0E] = '\b',
    [0x0F] = '\t',

    [0x10] = 'Q',
    [0x11] = 'W',
    [0x12] = 'E',
    [0x13] = 'R',
    [0x14] = 'T',
    [0x15] = 'Y',
    [0x16] = 'U',
    [0x17] = 'I',
    [0x18] = 'O',
    [0x19] = 'P',

    [0x1A] = '{',
    [0x1B] = '}',

    [0x1C] = '\n',

    [0x1E] = 'A',
    [0x1F] = 'S',
    [0x20] = 'D',
    [0x21] = 'F',
    [0x22] = 'G',
    [0x23] = 'H',
    [0x24] = 'J',
    [0x25] = 'K',
    [0x26] = 'L',

    [0x27] = ':',
    [0x28] = '"',

    [0x2B] = '|',

    [0x2C] = 'Z',
    [0x2D] = 'X',
    [0x2E] = 'C',
    [0x2F] = 'V',
    [0x30] = 'B',
    [0x31] = 'N',
    [0x32] = 'M',

    [0x33] = '<',
    [0x34] = '>',
    [0x35] = '?',

    [0x39] = ' ',
};

static const uint8_t font[128][7] = {

    // UPPERCASE LETTERS

    ['A'] = {
        0b01110,
        0b10001,
        0b10001,
        0b11111,
        0b10001,
        0b10001,
        0b10001
    },

    ['B'] = {
        0b11110,
        0b10001,
        0b10001,
        0b11110,
        0b10001,
        0b10001,
        0b11110
    },

    ['C'] = {
        0b01110,
        0b10001,
        0b10000,
        0b10000,
        0b10000,
        0b10001,
        0b01110
    },

    ['D'] = {
        0b11110,
        0b10001,
        0b10001,
        0b10001,
        0b10001,
        0b10001,
        0b11110
    },

    ['E'] = {
        0b11111,
        0b10000,
        0b10000,
        0b11110,
        0b10000,
        0b10000,
        0b11111
    },

    ['F'] = {
        0b11111,
        0b10000,
        0b10000,
        0b11110,
        0b10000,
        0b10000,
        0b10000
    },

    ['G'] = {
        0b01110,
        0b10001,
        0b10000,
        0b10111,
        0b10001,
        0b10001,
        0b01110
    },

    ['H'] = {
        0b10001,
        0b10001,
        0b10001,
        0b11111,
        0b10001,
        0b10001,
        0b10001
    },

    ['I'] = {
        0b11111,
        0b00100,
        0b00100,
        0b00100,
        0b00100,
        0b00100,
        0b11111
    },

    ['J'] = {
        0b00010,
        0b00010,
        0b00010,
        0b00010,
        0b00010,
        0b10010,
        0b01100
    },

    ['K'] = {
        0b10001,
        0b10001,
        0b10010,
        0b11100,
        0b10010,
        0b10001,
        0b10001
    },

    ['L'] = {
        0b10000,
        0b10000,
        0b10000,
        0b10000,
        0b10000,
        0b10000,
        0b11111
    },

    ['M'] = {
        0b10001,
        0b11011,
        0b10101,
        0b10101,
        0b10001,
        0b10001,
        0b10001
    },

    ['N'] = {
        0b10001,
        0b10001,
        0b11001,
        0b10101,
        0b10011,
        0b10001,
        0b10001
    },

    ['O'] = {
        0b01110,
        0b10001,
        0b10001,
        0b10001,
        0b10001,
        0b10001,
        0b01110
    },

    ['P'] = {
        0b11110,
        0b10001,
        0b10001,
        0b11110,
        0b10000,
        0b10000,
        0b10000
    },

    ['Q'] = {
        0b01110,
        0b10001,
        0b10001,
        0b10001,
        0b10101,
        0b10011,
        0b01111
    },

    ['R'] = {
        0b11110,
        0b10001,
        0b10001,
        0b11110,
        0b10001,
        0b10001,
        0b10001
    },

    ['S'] = {
        0b01111,
        0b10000,
        0b10000,
        0b01110,
        0b00001,
        0b00001,
        0b11110
    },

    ['T'] = {
        0b11111,
        0b00100,
        0b00100,
        0b00100,
        0b00100,
        0b00100,
        0b00100
    },

    ['U'] = {
        0b10001,
        0b10001,
        0b10001,
        0b10001,
        0b10001,
        0b10001,
        0b01110
    },

    ['V'] = {
        0b10001,
        0b10001,
        0b10001,
        0b10001,
        0b10001,
        0b01010,
        0b00100
    },

    ['W'] = {
        0b10001,
        0b10001,
        0b10001,
        0b10101,
        0b10101,
        0b11011,
        0b10001
    },

    ['X'] = {
        0b10001,
        0b10001,
        0b01010,
        0b00100,
        0b01010,
        0b10001,
        0b10001
    },

    ['Y'] = {
        0b10001,
        0b10001,
        0b01010,
        0b00100,
        0b00100,
        0b00100,
        0b00100
    },

    ['Z'] = {
        0b11111,
        0b00001,
        0b00010,
        0b00100,
        0b01000,
        0b10000,
        0b11111
    },

    // LOWERCASE LETTERS

    ['a'] = {
      0b00000,
      0b00000,
      0b01110,
      0b00001,
      0b01111,
      0b10001,
      0b01111

    },

    ['b'] = {
      0b10000,
      0b10000,
      0b10000,
      0b11110,
      0b10001,
      0b10001,
      0b11110
    },
    
    ['c'] = {
      0b00000,
      0b00000,
      0b01110,
      0b10001,
      0b10000,
      0b10001,
      0b01110
    },
    
    ['d'] = {
      0b00001,
      0b00001,
      0b00001,
      0b01111,
      0b10001,
      0b10001,
      0b01111
    },
    
    ['e'] = {
      0b00000,
      0b00000,
      0b01110,
      0b10001,
      0b11111,
      0b10000,
      0b01111
    },
    
    ['f'] = {
      0b00110,
      0b01001,
      0b01000,
      0b11100,
      0b01000,
      0b01000,
      0b01000
    },
    
    ['g'] = {
      0b00000,
      0b00000,
      0b01110,
      0b10001,
      0b01111,
      0b00001,
      0b01110
    },
    
    ['h'] = {
      0b10000,
      0b10000,
      0b10000,
      0b11110,
      0b10001,
      0b10001,
      0b10001
    },

    ['i'] = {
        0b00100,
        0b00000,
        0b01100,
        0b00100,
        0b00100,
        0b00100,
        0b01110
    },

    ['j'] = {
        0b00010,
        0b00000,
        0b00110,
        0b00010,
        0b00010,
        0b10010,
        0b01100
    },

    ['k'] = {
        0b10000,
        0b10000,
        0b10001,
        0b10010,
        0b11100,
        0b10010,
        0b10001
    },

    ['l'] = {
        0b01100,
        0b00100,
        0b00100,
        0b00100,
        0b00100,
        0b00100,
        0b01110
    },

    ['m'] = {
        0b00000,
        0b00000,
        0b11110,
        0b10101,
        0b10101,
        0b10101,
        0b10101
    },

    ['n'] = {
        0b00000,
        0b00000,
        0b11110,
        0b10001,
        0b10001,
        0b10001,
        0b10001
    },

    ['o'] = {
        0b00000,
        0b00000,
        0b01110,
        0b10001,
        0b10001,
        0b10001,
        0b01110
    },

    ['p'] = {
        0b00000,
        0b00000,
        0b01110,
        0b01001,
        0b01110,
        0b01000,
        0b01000
    },

    ['q'] = {
        0b00000,
        0b00000,
        0b00111,
        0b01001,
        0b00111,
        0b00001,
        0b00001
    },

    ['r'] = {
        0b00000,
        0b00000,
        0b10111,
        0b11000,
        0b10000,
        0b10000,
        0b10000
    },

    ['s'] = {
        0b00000,
        0b00000,
        0b01111,
        0b10000,
        0b01110,
        0b00001,
        0b11110
    },

    ['t'] = {
        0b00100,
        0b00100,
        0b01110,
        0b00100,
        0b00100,
        0b00100,
        0b00011
    },

    ['u'] = {
        0b00000,
        0b00000,
        0b10001,
        0b10001,
        0b10001,
        0b10011,
        0b01101
    },

    ['v'] = {
        0b00000,
        0b00000,
        0b10001,
        0b10001,
        0b10001,
        0b01010,
        0b00100
    },

    ['w'] = {
        0b00000,
        0b00000,
        0b10001,
        0b10001,
        0b10101,
        0b11111,
        0b10101
    },

    ['x'] = {
        0b00000,
        0b00000,
        0b10001,
        0b01010,
        0b00100,
        0b01010,
        0b10001
    },

    ['y'] = {
        0b00000,
        0b00000,
        0b10001,
        0b10001,
        0b01111,
        0b00001,
        0b11110
    },

    ['z'] = {
        0b00000,
        0b00000,
        0b11111,
        0b00010,
        0b00100,
        0b01000,
        0b11111
    },
    
    // NUMBERS
    
    ['0'] = {
        0b01110,
        0b10001,
        0b10011,
        0b10101,
        0b11001,
        0b10001,
        0b01110
    },

    ['1'] = {
        0b00100,
        0b01100,
        0b10100,
        0b00100,
        0b00100,
        0b00100,
        0b11111
    },

    ['2'] = {
        0b01110,
        0b10001,
        0b00001,
        0b00010,
        0b00100,
        0b01000,
        0b11111
    },

    ['3'] = {
        0b11110,
        0b00001,
        0b00001,
        0b11110,
        0b00001,
        0b00001,
        0b11110
    },

    ['4'] = {
        0b00010,
        0b00110,
        0b01010,
        0b10010,
        0b11111,
        0b00010,
        0b00010
    },
    
    ['5'] = {
        0b11111,
        0b10000,
        0b11110,
        0b00001,
        0b00001,
        0b10001,
        0b01110
    },

    ['6'] = {
        0b01110,
        0b10001,
        0b10000,
        0b11110,
        0b10001,
        0b10001,
        0b01110
    },

    ['7'] = {
        0b11111,
        0b00001,
        0b00010,
        0b00100,
        0b00100,
        0b00100,
        0b00100
    },

    ['8'] = {
        0b01110,
        0b10001,
        0b10001,
        0b01110,
        0b10001,
        0b10001,
        0b01110
    },

    ['9'] = {
        0b01110,
        0b10001,
        0b10001,
        0b01111,
        0b00001,
        0b10001,
        0b01110
    },

    // PUNCTUATION

    ['!'] = {
        0b01000,
        0b01000,
        0b01000,
        0b01000,
        0b01000,
        0b00000,
        0b01000
    },

    [' '] = {
      0b00000,
      0b00000,
      0b00000,
      0b00000,
      0b00000,
      0b00000,
      0b00000
    },

    ['&'] = {
      0b01000,
      0b10100,
      0b10100,
      0b01000,
      0b10101,
      0b10010,
      0b01101 
    },

    ['\''] = {
      0b00100,
      0b00100,
      0b00000,
      0b00000,
      0b00000,
      0b00000,
      0b00000
    },

    ['('] = {
      0b00010,
      0b00100,
      0b00100,
      0b00100,
      0b00100,
      0b00100,
      0b00010
    },

    [')'] = {
      0b01000,
      0b00100,
      0b00100,
      0b00100,
      0b00100,
      0b00100,
      0b01000
    },

    ['*'] = {
      0b00000,
      0b10101,
      0b01110,
      0b11111,
      0b01110,
      0b10101,
      0b00000
    },

    ['+'] = {
      0b00000,
      0b00100,
      0b00100,
      0b01110,
      0b00100,
      0b00100,
      0b00000
    },

    ['-'] = {
      0b00000,
      0b00000,
      0b00000,
      0b01110,
      0b00000,
      0b00000,
      0b00000
    },

    ['='] = {
      0b00000,
      0b00000,
      0b01110,
      0b00000,
      0b01110,
      0b00000,
      0b00000
    },

    ['.'] = {
      0b00000,
      0b00000,
      0b00000,
      0b00000,
      0b00000,
      0b00000,
      0b00100
    },

    ['"'] = {
      0b01010,
      0b01010,
      0b00000,
      0b00000,
      0b00000,
      0b00000,
      0b00000
    },

    ['#'] = {
      0b01010,
      0b01010,
      0b11111,
      0b01010,
      0b11111,
      0b01010,
      0b01010
    },

    ['$'] = {
      0b00100,
      0b01111,
      0b10100,
      0b01110,
      0b00101,
      0b11110,
      0b00100
    },

    ['%'] = {
      0b11000,
      0b11001,
      0b00010,
      0b00100,
      0b01000,
      0b10011,
      0b00011
    },

    ['^'] = {
      0b00100,
      0b01010,
      0b10001,
      0b00000,
      0b00000,
      0b00000,
      0b00000
    },

    [','] = {
      0b00000,
      0b00000,
      0b00000,
      0b00000,
      0b00100,
      0b00100,
      0b01000
    },

    [':'] = {
      0b00000,
      0b00000,
      0b00100,
      0b00000,
      0b00100,
      0b00000,
      0b00000
    },

    [';'] = {
      0b00000,
      0b00000,
      0b00100,
      0b00000,
      0b00100,
      0b01000,
      0b00000
    },

    ['?'] = {
      0b01110,
      0b10001,
      0b00001,
      0b00010,
      0b00100,
      0b00000,
      0b00100
    },

    ['@'] = {
      0b01110,
      0b10001,
      0b10111,
      0b10101,
      0b10110,
      0b10000,
      0b01111
    },

    ['/'] = {
      0b00000,
      0b00001,
      0b00010,
      0b00100,
      0b01000,
      0b10000,
      0b00000
    },

    ['<'] = {
      0b00010,
      0b00100,
      0b01000,
      0b10000,
      0b01000,
      0b00100,
      0b00010
    },

    ['>'] = {
      0b01000,
      0b00100,
      0b00010,
      0b00001,
      0b00010,
      0b00100,
      0b01000
    },

    ['|'] = {
      0b00100,
      0b00100,
      0b00100,
      0b00100,
      0b00100,
      0b00100,
      0b00100
    },

    ['\\'] = {
      0b00000,
      0b10000,
      0b01000,
      0b00100,
      0b00010,
      0b00001,
      0b00000
    },

    ['['] = {
      0b00110,
      0b00100,
      0b00100,
      0b00100,
      0b00100,
      0b00100,
      0b00110
    },

    [']'] = {
      0b01100,
      0b00100,
      0b00100,
      0b00100,
      0b00100,
      0b00100,
      0b01100
    },

    ['{'] = {
      0b00010,
      0b00100,
      0b00100,
      0b01000,
      0b00100,
      0b00100,
      0b00010
    },

    ['}'] = {
      0b01000,
      0b00100,
      0b00100,
      0b00010,
      0b00100,
      0b00100,
      0b01000
    }

};


void put_pixel(struct limine_framebuffer *fb,
               size_t x,
               size_t y,
               uint32_t color){
    if (x >= fb->width || y >= fb->height)
        return;

    uint32_t *pixel = fb->address;
    pixel[y * (fb->pitch / 4) + x] = color;
}

static void scroll_screen(struct limine_framebuffer *fb) {
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
char keyboard_getchar(void) {
    for (;;)
    {
        // Wait until the keyboard controller has data.
        while (!(inb(0x64) & 1))
            ;

        uint8_t scancode = inb(0x60);

        if (scancode & 0x80)
        {
            scancode &= 0x7F;

            if (scancode == 0x2A || scancode == 0x36)
                shift_pressed = false;

            continue;
        }

        if (scancode == 0x2A || scancode == 0x36)
        {
            shift_pressed = true;
            continue;
        }

        char c;

        if (shift_pressed)
            c = scancode_to_ascii_shift[scancode];
        else
            c = scancode_to_ascii[scancode];

        if (c == 0)
            continue;

        if (caps_lock ^ shift_pressed)
        {
            if (c >= 'a' && c <= 'z')
                c -= 32;
        }

        return c;
    }
}

void read_line(struct limine_framebuffer *fb) {
    line_length = 0;

    while (1)
    {
        char c = keyboard_getchar();

        if (c == '\n')
        {
            line_buffer[line_length] = '\0';
            putchar(fb, '\n');
            return;
        }

        if (c == '\b')
        {
            if (line_length > 0)
            {
                line_length--;
                putchar(fb, '\b');
            }

            continue;
        }

        // Ignore unknown keys
        if (c == 0)
            continue;

        if (line_length < LINE_MAX - 1)
        {
            line_buffer[line_length++] = c;
            putchar(fb, c);
        }
    }
}

void clear_screen(struct limine_framebuffer *fb) {
    memset(
        fb->address,
        0,
        fb->pitch * fb->height
    );

    cursor_x = 10;
    cursor_y = 10;
}

void print_prompt(struct limine_framebuffer *fb) {
    print(fb, "RKT> ");
}

void execute_command(struct limine_framebuffer *fb) {
    if (strcmp(line_buffer, "about") == 0)
    {
        print(fb,
            "RKT OS\n"
            "Kernel: RKT 0.1\n"
            "Made from scratch\n"
        );
    }
    else if (strcmp(line_buffer, "help") == 0)
    {
        print(fb,
            "Commands:\n"
            " help\n"
            " about\n"
            " clear\n"
            " rktfetch\n"
        );
    }
    else if (strcmp(line_buffer, "clear") == 0)
    {
        clear_screen(fb);
    }
    else if (strcmp(line_buffer, "rktfetch") == 0)
    {
        print(fb,
            " ____  _  _______\n"
            "|  _ \\| |/ /_   _|\n"
            "| |_) | ' /  | |\n"
            "|  _ <| . \\  | |\n"
            "|_| \\_\\_|\\_\\ |_|\n"
            "-----------------\n"
            "OS: RKT\n"
            "Kernel: RKT 0.1\n"
            "CPU: some sort of 64-bit cpu\n"
            "RAM: idk\n"
            "Shell: rktsh 0.1\n"
            "-------------------------------------\n"
        );
    }
    else if (line_buffer[0] != '\0')
    {
        print(fb, "Unknown command\n");
    }
}

void shell(struct limine_framebuffer *fb) {
    while (1)
    {
        print_prompt(fb);

        read_line(fb);

        execute_command(fb);
    }
}

// The following will be our kernel's entry point.
// If renaming kmain() to something else, make sure to change the
// linker script accordingly.
void kmain(void) {
    if (LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == false)
    {
        hcf();
    }

    if (framebuffer_request.response == NULL ||
        framebuffer_request.response->framebuffer_count < 1)
    {
        hcf();
    }

    struct limine_framebuffer *framebuffer =
        framebuffer_request.response->framebuffers[0];

    print(framebuffer,
        " ____  _  _______\n|  _ \\| |/ /_   _|\n| |_) | ' /  | |\n|  _ <| . \\  | |\n|_| \\_\\_|\\_\\ |_|\n-----------------\nOS: RKT\nKernel: RKT 0.1\nCPU: some sort of 64-bit cpu\nRAM: idk\nShell: rktsh 0.1\n-------------------------------------\n"
    );

    shell(framebuffer);

    hcf();
}