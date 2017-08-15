#
# Author: Xiangfu Liu <xiangfu@openmobilefree.net>
# Bitcoin:	1CanaaniJzgps8EV6Sfmpb7T8RutpaeyFn
#
# This is free and unencumbered software released into the public domain.
# For details see the UNLICENSE file at the root of the source tree.
#

-include ../config.mk

CSRCS      = memcpy.c memset.c strncmp.c printf.c math.c

OBJS       := $(addsuffix .o,$(basename $(CSRCS) $(SSRCS)))

.PHONY: all clean

all: libminilibc.a

libminilibc.a: $(OBJS)
	$(AR) rc $@ $^

%.o: %.c minilibc.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)
	rm -f libminilibc.a


