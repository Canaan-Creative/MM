/**
 * memcpy - Copies one area of memory to another
 * @dest: Destination
 * @src: Source
 * @n: The size to copy.
 */
void *memcpy(void *to, const void *from, unsigned int n)
{
	void *xto = to;
	unsigned int temp;

	if(!n)
		return xto;
	if((long)to & 1) {
		char *cto = to;
		const char *cfrom = from;
		*cto++ = *cfrom++;
		to = cto;
		from = cfrom;
		n--;
	}
	if((long)from & 1) {
		char *cto = to;
		const char *cfrom = from;
		for (; n; n--)
			*cto++ = *cfrom++;
		return xto;
	}
	if(n > 2 && (long)to & 2) {
		short *sto = to;
		const short *sfrom = from;
		*sto++ = *sfrom++;
		to = sto;
		from = sfrom;
		n -= 2;
	}
	if((long)from & 2) {
		short *sto = to;
		const short *sfrom = from;
		temp = n >> 1;
		for (; temp; temp--)
			*sto++ = *sfrom++;
		to = sto;
		from = sfrom;
		if(n & 1) {
			char *cto = to;
			const char *cfrom = from;
			*cto = *cfrom;
		}
		return xto;
	}
	temp = n >> 2;
	if(temp) {
		long *lto = to;
		const long *lfrom = from;
		for(; temp; temp--)
			*lto++ = *lfrom++;
		to = lto;
		from = lfrom;
	}
	if(n & 2) {
		short *sto = to;
		const short *sfrom = from;
		*sto++ = *sfrom++;
		to = sto;
		from = sfrom;
	}
	if(n & 1) {
		char *cto = to;
		const char *cfrom = from;
		*cto = *cfrom;
	}
	return xto;
}
