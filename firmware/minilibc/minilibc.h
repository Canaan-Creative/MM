#ifndef MINILIBC_H_
#define MINILIBC_H_

typedef unsigned int size_t;

void *memcpy(void *restrict dst, const void *restrict src, int n);
void *memset(void *dst, int c, size_t n);
int strncmp(register const char *s1, register const char *s2, register size_t n);

char *m_sprintf(char *dest, const char *format, ...); /* Return dest not length of the string */

#endif
