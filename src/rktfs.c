#include "rktfs.h"
#include "terminal.h"
#include "string.h"

extern struct limine_framebuffer *terminal_fb;

static rktfs_header_t fs;

void rktfs_init(void)
{
    fs.magic[0] = 'R';
    fs.magic[1] = 'K';
    fs.magic[2] = 'T';
    fs.magic[3] = '\0';

    fs.version = 1;

    for (int i = 0; i < RKTFS_MAX_FILES; i++)
        fs.files[i].used = false;
}

bool rktfs_exists(const char *path)
{
    for (int i = 0; i < RKTFS_MAX_FILES; i++)
    {
        if (!fs.files[i].used)
            continue;

        if (strcmp(fs.files[i].name, path) == 0)
            return true;
    }

    return false;
}

int rktfs_read(
    const char *path,
    void *buffer,
    size_t buffer_size
)
{
    for (int i = 0; i < RKTFS_MAX_FILES; i++)
    {
        if (!fs.files[i].used)
            continue;

        if (strcmp(fs.files[i].name, path) == 0)
        {
            size_t bytes = fs.files[i].size;

            if (bytes > buffer_size)
                bytes = buffer_size;

            memcpy(
                buffer,
                fs.files[i].data,
                bytes
            );

            return bytes;
        }
    }

    return -1;
}

bool rktfs_write(
    const char *path,
    const void *data,
    size_t size
)
{
    for (int i = 0; i < RKTFS_MAX_FILES; i++)
    {
        if (!fs.files[i].used)
            continue;

        if (strcmp(fs.files[i].name, path) == 0)
        {
            if (size > RKTFS_MAX_FILE_SIZE)
                size = RKTFS_MAX_FILE_SIZE;

            memcpy(
                fs.files[i].data,
                data,
                size
            );

            fs.files[i].size = size;

            return true;
        }
    }

    return false;
}

bool rktfs_delete(const char *path)
{
    (void)path;

    return false;
}

void rktfs_list(void)
{
    bool found = false;

    for (size_t i = 0; i < RKTFS_MAX_FILES; i++)
    {
        if (!fs.files[i].used)
            continue;

        print(terminal_fb, fs.files[i].name);
        putchar(terminal_fb, '\n');

        found = true;
    }

    if (!found)
    {
        print(terminal_fb, "(no files)\n");
    }
}

bool rktfs_create(const char *path)
{
    // Don't allow duplicates.
    if (rktfs_exists(path))
        return false;

    // Find a free entry.
    for (int i = 0; i < RKTFS_MAX_FILES; i++)
    {
        if (!fs.files[i].used)
        {
            fs.files[i].used = true;

            // Copy filename.
            int j;
            for (j = 0; j < RKTFS_NAME_LEN - 1 && path[j]; j++)
                fs.files[i].name[j] = path[j];

            fs.files[i].name[j] = '\0';

            fs.files[i].size = 0;

            return true;
        }
    }

    // No free entries.
    return false;
}