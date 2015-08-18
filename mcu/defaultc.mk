#
# Author: Mikeqin <Fengling.Qin@gmail.com>
#
# This is free and unencumbered software released into the public domain.
# For details see the UNLICENSE file at the root of the source tree.
#

SHELL = /bin/bash

CROSS_COMPILE ?= arm-none-eabi-

CC	:= $(CROSS_COMPILE)gcc
LD	:= $(CROSS_COMPILE)ld
SIZE	:= $(CROSS_COMPILE)size
AR	:= $(CROSS_COMPILE)ar
OBJCOPY	:= $(CROSS_COMPILE)objcopy
OBJDUMP	:= $(CROSS_COMPILE)objdump

# ----- Verbosity control -----------------------------------------------------

CC_normal       := $(CC)
CPP_normal      := $(CPP)
DEPEND_normal   = $(CPP_normal) $(CFLAGS) -MM -MG

ifeq ($(V),1)
    CC          = $(CC_normal)
    BUILD       =
    DEPEND      = $(DEPEND_normal)
else
    CC          = @echo "  CC       " $@ && $(CC_normal)
    BUILD       = @echo "  BUILD    " $@ &&
    DEPEND      = @$(DEPEND_normal)
endif

LIBNAME	 = $(shell pwd |sed 's/^\(.*\)[/]//' )

SRCS    ?= $(wildcard src/*.c)
OBJS	 = $(patsubst %.c,%.o,$(SRCS))

LIBDIR	 = ./libs
SLIB	 = lib$(LIBNAME).a

INCLUDES	+= -I./inc

CFLAGS_WARN = -Wall -Wextra -Wshadow -Wmissing-prototypes \
		-Wmissing-declarations -Wno-format-zero-length \
		-Wno-unused-parameter

CFLAGS_PLATFORM = -D__REDLIB__ -D__CODE_RED -DCORE_M0 -D__REDLIB__ \
		-fmessage-length=0 -fno-builtin -mcpu=cortex-m0 -mthumb  \
		-specs=redlib.specs

# -Os -Og will not work with spi, it has a side effect
ifeq "$(FW_RELEASE)" "DEBUG" 
    CFLAGS_DEBUG = -g -ffunction-sections -fdata-sections -DDEBUG -DDEBUG_ENABLE -DDEBUG_SEMIHOSTING
endif

CFLAGS += -Os -ffunction-sections -fdata-sections -Wl,--gc-sections
CFLAGS += $(CFLAGS_WARN) $(CFLAGS_PLATFORM) $(CFLAGS_DEBUG) $(INCLUDES)
