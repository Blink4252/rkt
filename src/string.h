#ifndef STRING_H
#define STRING_H

#include <stddef.h>
#include <stdbool.h>

bool starts_with(const char *str, const char *prefix);
char *strchr(const char *str, int c);
size_t strlen(const char *str);

void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
void *memmove(void *dest, const void *src, size_t n);

int memcmp(const void *s1, const void *s2, size_t n);
int strcmp(const char *a, const char *b);

#endif
