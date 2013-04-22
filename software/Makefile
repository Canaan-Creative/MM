CC := gcc #mips-openwrt-linux-gcc
CFLAGS += -Wall -Werror -I. -Ilibev -Ijansson \
	  -ggdb -O0 \
	  -Wno-unused-function \

SRCS := ev.c \
	jansson/dump.c \
	jansson/error.c \
	jansson/hashtable.c \
	jansson/load.c \
	jansson/memory.c \
	jansson/pack_unpack.c \
	jansson/strbuffer.c \
	jansson/strconv.c \
	jansson/utf.c \
	jansson/value.c \
	main.c

TARGET := xminer
all: $(TARGET)

OBJS := $(patsubst %.c,%.o,$(SRCS))

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o "$@" $^

.PHONY: clean
clean:
	rm -f $(OBJS) $(TARGET)
