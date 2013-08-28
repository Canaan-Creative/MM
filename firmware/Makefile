CROSS ?= lm32-elf-

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
CPU_CONFIG = -mmultiply-enabled -mbarrel-shift-enabled -muser-enabled #-mdivide-enabled -msign-extend-enabled 
CPPFLAGS   += -std=gnu99 -Os -ffunction-sections -ffreestanding -ggdb -Wall -Werror -I. -Isdk -Iminilibc $(CPU_CONFIG)
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

HARDWARE_NAME = mm
PRJ_NAME      = main
TARGET = $(PRJ_NAME).elf

isedir  ?= /opt/xilinx/13.3/ISE_DS/
xil_env ?= . $(isedir)/settings$(shell getconf LONG_BIT).sh &>/dev/null


.PHONY: all clean

all: $(TARGET)
	$(SIZE) $(TARGET)

$(TARGET): $(OBJS) | ../synth/$(HARDWARE_NAME)_bd.bmm ../synth/$(HARDWARE_NAME).bit
	$(LD) $(LDFLAGS) $(OBJS) -o $@ $(LDLIBS)
	$(xil_env)
	data2mem -bm ../synth/$(HARDWARE_NAME)_bd.bmm -bd $(TARGET) -bt ../synth/$(HARDWARE_NAME).bit -o b temp.bit
	mv temp.bit ../synth/$(HARDWARE_NAME).bit
#	$(MAKE) -C ../synth $(HARDWARE_NAME).mcs

../synth/$(HARDWARE)_bd.bmm ../synth/$(HARDWARE_NAME).bit:
	@echo "Please do make in ../synth first"
	@false

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ -c $<
	$(MKDEP)

clean:
	rm -f $(OBJS) $(OBJS:.o=.d) $(TARGET)

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
