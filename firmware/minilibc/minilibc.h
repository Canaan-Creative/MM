#ifndef MINILIBC_H_
#define MINILIBC_H_

#include "sdk.h"
#include "defines.h"

typedef unsigned int size_t;

void *memcpy(void *restrict dst, const void *restrict src, int n);
void *memset(void *dst, int c, size_t n);

char *m_sprintf(char *dest, const char *format, ...); /* Return dest not length of the string */

#endif
