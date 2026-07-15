#ifndef FS_H
#define FS_H

#include <stddef.h>
#include <stdbool.h>

// Initialize the filesystem.
void fs_init(void);

// Returns true if the file exists.
bool fs_exists(const char *path);

bool fs_create(const char *path);

// Read a file into a buffer.
// Returns the number of bytes read, or -1 on error.
int fs_read(
    const char *path,
    void *buffer,
    size_t buffer_size
);

// Write a file.
// Returns true on success.
bool fs_write(
    const char *path,
    const void *data,
    size_t size
);

// Delete a file.
bool fs_delete(const char *path);

// List files in the root directory.
void fs_list(void);

#endif