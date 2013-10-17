/*
 * Author: Xiangfu Liu <xiangfu@openmobilefree.net>
 * Author: Xiangfu Liu <xiangfu@openmobilefree.net>
 * Bitcoin:	1CanaaniJzgps8EV6Sfmpb7T8RutpaeyFn
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#include <stdarg.h>
#include "minilibc.h"

#define PAD_RIGHT       1
#define PAD_ZERO        2

static void m_putchar(char c)
{
}

static void prints(char **dest, const char *string, int width, int pad)
{
	int padchar = ' ';

	if (width > 0) {
		int len = 0;
		const char *ptr;

		for (ptr = string; *ptr; ++ptr)
			++len;

		if (len >= width)
			width = 0;
		else
			width -= len;

		if (pad & PAD_ZERO)
			padchar = '0';
	}

	if (!(pad & PAD_RIGHT)) {
		for (; width > 0; --width) {
			if (dest)
				*(*dest)++ = padchar;
			else
				m_putchar(padchar);
		}
	}

	for (; *string ; ++string) {
		if (dest)
			*(*dest)++ = *string;
		else
			m_putchar(*string);
	}

	for (; width > 0; --width) {
		if (dest)
			*(*dest)++ = padchar;
		else
			m_putchar(padchar);
	}

	return;
}

#define PRINT_BUF_LEN 12
/* the following should be enough for 32 bit int */

static void printi(char **dest, int i, int b, int sg, int width, int pad, int letbase)
{
	char           print_buf[PRINT_BUF_LEN];
	char*          s;
	int            t, neg = 0;
	unsigned int   u = i;

	if (i == 0) {
		print_buf[0] = '0';
		print_buf[1] = '\0';
		return prints(dest, print_buf, width, pad);
	}

	if (sg && b == 10 && i < 0) {
		neg = 1;
		u = -i;
	}

	s  = print_buf + PRINT_BUF_LEN-1;
	*s = '\0';

	while (u) {
		t = u % b;
		if( t >= 10 )
			t += letbase - '0' - 10;
		*--s = t + '0';
		u /= b;
	}

	if (neg) {
		if (width && (pad & PAD_ZERO)) {
			if (dest)
				*(*dest)++ = '-';
			else
				m_putchar('-');
			--width;
		} else
			*--s = '-';
	}

	prints(dest, s, width, pad);
	return;
}

char *m_sprintf(char *dest, const char *format, ...)
{
	int width, pad;
	char scr[2];
	va_list args;
	char *const dest_bk = dest;

	va_start(args, format);

	for (; *format != 0; ++format) {
		if (*format == '%') {
			++format;
			width = pad = 0;
			if (*format == '\0') break;
			if (*format == '%') goto out;
			if (*format == '-') {
				++format;
				pad = PAD_RIGHT;
			}

			while (*format == '0') {
				++format;
				pad |= PAD_ZERO;
			}

			for (; *format >= '0' && *format <= '9'; ++format) {
				width *= 10;
				width += *format - '0';
			}

			if (*format == 's') {
				char *s = (char *)va_arg(args, int);
				prints(&dest, s ? s:"(null)", width, pad);
				continue;
			}

			if (*format == 'd' || *format == 'i') {
				printi(&dest, va_arg(args, int), 10, 1, width, pad, 'a');
				continue;
			}

			if (*format == 'x' || *format == 'p') {
				printi(&dest, va_arg(args, int), 16, 0, width, pad, 'a');
				continue;
			}

			if (*format == 'X') {
				printi(&dest, va_arg(args, int), 16, 0, width, pad, 'A');
				continue;
			}

			if (*format == 'u') {
				printi(&dest, va_arg(args, int), 10, 0, width, pad, 'a');
				continue;
			}

			if (*format == 'c') {
				/* char are converted to int then pushed on the stack */
				scr[0] = (char)va_arg(args, int);
				scr[1] = '\0';
				prints(&dest, scr, width, pad);
				continue;
			}
		} else {
		out:
			if (dest)
				*dest++ = *format;
			else
				m_putchar(*format);
		}
	}
	if (dest)
		*dest++ = '\0';

	va_end(args);
	return dest_bk;
}

/* vim: set ts=4 sw=4 noet fdm=marker : */
