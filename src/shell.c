#include <stddef.h>
#include <limine.h>

#include "shell.h"
#include "keyboard.h"
#include "terminal.h"
#include "string.h"
#include "framebuffer.h"

#define LINE_MAX 128

static char line_buffer[LINE_MAX];
static size_t line_length = 0;


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
            "    ____  __ ________\n"
            "   / __ \\/ //_/_  __/\n"
            "  / /_/ / ,<   / /\n"
            " / _, _/ /| | / /\n"
            "/_/ |_/_/ |_|/_/\n"
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
        print(fb, "Unknown command: ");
        print(fb, line_buffer);
        putchar(fb, '\n');
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
