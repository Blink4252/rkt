#include "fs.h"
#include "rktfs.h"

void fs_init(void)
{
    rktfs_init();
}

bool fs_exists(const char *path)
{
    return rktfs_exists(path);
}

int fs_read(const char *path, void *buffer, size_t buffer_size)
{
    return rktfs_read(path, buffer, buffer_size);
}

bool fs_write(const char *path, const void *data, size_t size)
{
    return rktfs_write(path, data, size);
}

bool fs_delete(const char *path)
{
    return rktfs_delete(path);
}

void fs_list(void)
{
    rktfs_list();
}

bool fs_create(const char *path)
{
    return rktfs_create(path);
}