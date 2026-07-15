#ifndef RKTFS_H
#define RKTFS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void rktfs_init(void);

bool rktfs_exists(const char *path);

bool rktfs_create(const char *path);

int rktfs_read(
    const char *path,
    void *buffer,
    size_t buffer_size
);

bool rktfs_write(
    const char *path,
    const void *data,
    size_t size
);

bool rktfs_delete(const char *path);

void rktfs_list(void);

#define RKTFS_MAX_FILES 64
#define RKTFS_NAME_LEN 16
#define RKTFS_MAX_FILE_SIZE 4096

typedef struct
{
    char name[RKTFS_NAME_LEN];

    uint32_t size;

    bool used;

    uint8_t data[RKTFS_MAX_FILE_SIZE];
} rktfs_entry_t;

typedef struct
{
    char magic[4];
    uint32_t version;

    rktfs_entry_t files[RKTFS_MAX_FILES];
} rktfs_header_t;

#endif