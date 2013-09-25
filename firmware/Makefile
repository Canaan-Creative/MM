#
# Author: Minux
# Author: Xiangfu Liu <xiangfu@openmobilefree.net>
# Bitcoin:	1CanaaniJzgps8EV6Sfmpb7T8RutpaeyFn
#
# This is free and unencumbered software released into the public domain.
# For details see the UNLICENSE file at the root of the source tree.
#

CROSS ?= /opt/lm32/bin/lm32-rtems4.11-

CC    := $(CROSS)gcc
LD    := $(CROSS)gcc
SIZE  := $(CROSS)size

# ----- Quiet code ----------------------------------------------------------
SHELL=/bin/bash
CPP := $(CPP)   # make sure changing CC won't affect CPP

CC_normal	:= $(CC)
AR_normal	:= $(AR) rsc
DEPEND_normal	:= $(CPP) $(CFLAGS) -D__OPTIMIZE__ -MM -MG
RANLIB_normal	:= ranlib

CC_quiet	= @echo "  CC       " $@ && $(CC_normal)
AR_quiet	= @echo "  AR       " $@ && $(AR_normal)
DEPEND_quiet	= @$(DEPEND_normal)
RANLIB_quiet	= @$(RANLIB_normal)

ifeq ($(V),1)
    CC		= $(CC_normal)
    AR		= $(AR_normal)
    DEPEND	= $(DEPEND_normal)
    RANLIB	= $(RANLIB_normal)
else
    CC		= $(CC_quiet)
    AR		= $(AR_quiet)
    DEPEND	= $(DEPEND_quiet)
    RANLIB	= $(RANLIB_quiet)
endif

# ----- Customer ----------------------------------------------------------
RTEMS_MAKEFILE_PATH ?= /opt/lm32/lm32-rtems$(RTEMS_VERSION)/mm

INCLUDES   = -I. -Isdk -Iminilibc -I $(RTEMS_MAKEFILE_PATH)/lib/include -B $(RTEMS_MAKEFILE_PATH)/lib
CPU_CONFIG = -mmultiply-enabled -mbarrel-shift-enabled -muser-enabled #-mdivide-enabled -msign-extend-enabled 
CPPFLAGS   += -std=gnu99 -Os -ffunction-sections -ffreestanding -ggdb -Wall -Werror $(CPU_CONFIG) $(INCLUDES)

LDFLAGS    += -Wl,--gc-sections -Wl,-T,linker.ld -nostartfiles -nodefaultlibs -nostdlib -Wl,-Map=$(PRJ_NAME).map
LDLIBS     += -lgcc

CSRCS      = main.c
SDK_SRCS   = \
	sdk/crt0ram.S \
	sdk/utils.S \
	sdk/intr.c \
	minilibc/memcpy.c \
	minilibc/memset.c \
	minilibc/printf.c
OBJS       := $(addsuffix .o,$(basename $(CSRCS) $(SDK_SRCS)))

BATCHFILE	:= $(shell mktemp)
HARDWARE_NAME	= mm
PRJ_NAME	= main

isedir  ?= /home/Xilinx/14.6/ISE_DS/
xil_env ?= . $(isedir)/settings$(shell getconf LONG_BIT).sh &>/dev/null


.PHONY: all clean load

# C rules
all: $(PRJ_NAME).objdump $(PRJ_NAME).hexdump $(HARDWARE_NAME).bit

$(PRJ_NAME).objdump: $(PRJ_NAME).elf
	$(CROSS)objdump -D $^ > $@

$(PRJ_NAME).hexdump: $(PRJ_NAME).elf
	hexdump -v -e '1 "%08x"' -e '"\n"' $^ > $@

$(PRJ_NAME).elf: $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $@ $(LDLIBS)
	$(SIZE) $(PRJ_NAME).elf

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ -c $<
	$(MKDEP)


# Bitstream rules
load: $(HARDWARE_NAME).bit
	echo setmode -bs	>> $(BATCHFILE)
	echo setcable -p auto	>> $(BATCHFILE)
	echo identify		>> $(BATCHFILE)
	echo assignfile -p 1 -file $^ >> $(BATCHFILE)
	echo program -p 1	>> $(BATCHFILE)
	echo exit		>> $(BATCHFILE)
	$(xil_env) && impact -batch $(BATCHFILE)
	@rm -f $(BATCHFILE)

$(HARDWARE_NAME).bit: $(PRJ_NAME).elf ../synth/$(HARDWARE_NAME).bit ../synth/$(HARDWARE_NAME)_bd.bmm
	$(xil_env) && data2mem -bm ../synth/$(HARDWARE_NAME)_bd.bmm -bd $(PRJ_NAME).elf -bt ../synth/$(HARDWARE_NAME).bit -o b $@

../synth/$(HARDWARE)_bd.bmm: ../synth/$(HARDWARE_NAME).bit

../synth/$(HARDWARE_NAME).bit:
	make -C ../synth


# Clean rules
clean:
	rm -f $(OBJS) $(OBJS:.o=.d) $(TARGET) $(PRJ_NAME).objdump $(PRJ_NAME).hexdump

distclean: clean
	make -C ../synth distclean

# ----- Dependencies ----------------------------------------------------------
MKDEP =									\
	$(DEPEND) $< |							\
	  sed 								\
	    -e 's|^$(basename $(notdir $<)).o:|$@:|'			\
	    -e '/^\(.*:\)\? */{p;s///;s/ *\\\?$$/ /;s/  */:\n/g;H;}'	\
	    -e '$${g;p;}'						\
	    -e d >$(basename $@).d;					\
	  [ "$${PIPESTATUS[*]}" = "0 0" ] ||				\
	  { rm -f $(basename $@).d; exit 1; }

-include $(OBJS:.o=.d)
