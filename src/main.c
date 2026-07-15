#include <limine.h>
#include <stdbool.h>
#include <stdint.h>
#include "terminal.h"
#include "shell.h"
#include "fs.h"

struct limine_framebuffer *terminal_fb;
extern volatile uint64_t limine_base_revision[];
extern volatile struct limine_framebuffer_request framebuffer_request;

static void hcf(void) {
    for (;;) {
        asm ("hlt");
    }
}

void kmain(void)
{
    if (LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == false)
        hcf();

    if (!framebuffer_request.response ||
        framebuffer_request.response->framebuffer_count < 1)
        hcf();

    terminal_fb =
        framebuffer_request.response->framebuffers[0];

    struct limine_framebuffer *fb = terminal_fb;

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

    fs_init();

    shell(fb);
}
