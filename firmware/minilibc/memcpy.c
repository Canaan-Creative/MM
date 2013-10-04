void *memcpy(void *restrict dst, const void *restrict src, int n)
{
	char *q = (char *)dst;
	const char *p = (const char *)src;
	while (n--)
		*q++ = *p++;
	return  dst;
}
