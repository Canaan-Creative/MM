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


