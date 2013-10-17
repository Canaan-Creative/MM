#
# Author: Xiangfu Liu <xiangfu@openmobilefree.net>
# Bitcoin:	1CanaaniJzgps8EV6Sfmpb7T8RutpaeyFn
#
# This is free and unencumbered software released into the public domain.
# For details see the UNLICENSE file at the root of the source tree.
#

-include ../config.mk

.PHONY: all clean

all: libminilibc.a

libminilibc.a: memcpy.o memset.o printf.o strncmp.o
	$(AR) rc $@ $^

%.o: %.c minilibc.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

clean:
	rm -f memcpy.o memset.o printf.o strncmp.o
	rm -f libminilibc.a


