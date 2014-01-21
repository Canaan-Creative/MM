#
# Author: Minux
# Author: Xiangfu Liu <xiangfu@openmobilefree.net>
# Bitcoin:	1CanaaniJzgps8EV6Sfmpb7T8RutpaeyFn
#
# This is free and unencumbered software released into the public domain.
# For details see the UNLICENSE file at the root of the source tree.
#

BIN_DIR:=$(shell date +%Y-%m-%d)
GIT_VERSION:=$(shell git rev-parse HEAD | cut -c 1-7)
GIT_STATUS:=$(shell ([ -z "`git status -s -uno`" ] && echo 0) || echo +)

CFLAGS += -DMM_VERSION='"201401-$(GIT_VERSION)$(GIT_STATUS)"'

-include config.mk
# ----- Customer ----------------------------------------------------------
INCLUDES   = -I. -Isdk -Iminilibc
LDFLAGS    += -nostartfiles -nodefaultlibs -nostdlib -Wl,--gc-sections -Wl,-T,linker.ld -Wl,-Map=$(PRJ_NAME).map
LDLIBS     += -Bminilibc -lminilibc -lgcc

SSRCS      = sdk/crt0ram.S sdk/intr.c
CSRCS      = main.c uart.c sha256.c alink.c twipwm.c miner.c crc16.c shifter.c timer.c
OBJS       := $(addsuffix .o,$(basename $(CSRCS) $(SSRCS)))

BATCHFILE	:= $(shell mktemp)
HARDWARE_NAME	= mm
PRJ_NAME	= mm

isedir  ?= /home/Xilinx/14.6/ISE_DS/
xil_env ?= . $(isedir)/settings$(shell getconf LONG_BIT).sh &>/dev/null


.PHONY: all clean distclean load reflash

# C rules
all: $(PRJ_NAME).objdump $(PRJ_NAME).hexdump $(HARDWARE_NAME).bit $(HARDWARE_NAME).mcs
	mkdir -p bin/${BIN_DIR}
	cp $(PRJ_NAME).elf $^ bin/${BIN_DIR}
	cd bin/${BIN_DIR} && md5sum -b * > md5sums

$(PRJ_NAME).objdump: $(PRJ_NAME).elf
	$(CROSS)objdump -D $^ > $@

$(PRJ_NAME).hexdump: $(PRJ_NAME).elf
	hexdump -v -e '1 "%08x"' -e '"\n"' $^ > $@

$(PRJ_NAME).elf: $(OBJS) minilibc/libminilibc.a
	$(LD) $(LDFLAGS) $(OBJS) -o $@ $(LDLIBS)
	$(SIZE) $(PRJ_NAME).elf

minilibc/libminilibc.a:
	make -C `dirname $@`

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ -c $<
	$(MKDEP)


# Bitstream rules
# TODO: how to setCable -baud 12000000
reflash: $(HARDWARE_NAME).mcs
	echo setmode -bs	>> $(BATCHFILE)
	echo setcable -p auto	>> $(BATCHFILE)
	echo identify		>> $(BATCHFILE)
	echo attachFlash -p 1 -spi W25Q80BV		>> $(BATCHFILE)
	echo assignfiletoattachedflash -p 1 -file $<	>> $(BATCHFILE)
	echo program -p 1 -dataWidth 4 -spionly -erase -loadfpga >> $(BATCHFILE)
	echo exit		>> $(BATCHFILE)
	$(xil_env) && impact -batch $(BATCHFILE)
	@rm -f $(BATCHFILE)

load: $(HARDWARE_NAME).bit
	echo setmode -bs	>> $(BATCHFILE)
	echo setcable -p auto	>> $(BATCHFILE)
	echo identify		>> $(BATCHFILE)
	echo assignfile -p 1 -file $^ >> $(BATCHFILE)
	echo program -p 1	>> $(BATCHFILE)
	echo exit		>> $(BATCHFILE)
	$(xil_env) && impact -batch $(BATCHFILE)
	@rm -f $(BATCHFILE)

$(HARDWARE_NAME).mcs: $(HARDWARE_NAME).bit
	cp $^ ../synth
	make -C ../synth $@
	cp ../synth/$@ $@

$(HARDWARE_NAME).bit: $(PRJ_NAME).elf ../synth/$(HARDWARE_NAME).bit ../synth/$(HARDWARE_NAME)_bd.bmm
	$(xil_env) && data2mem -bm ../synth/$(HARDWARE_NAME)_bd.bmm -bd $(PRJ_NAME).elf -bt ../synth/$(HARDWARE_NAME).bit -o b $@

../synth/$(HARDWARE)_bd.bmm: ../synth/$(HARDWARE_NAME).bit

../synth/$(HARDWARE_NAME).bit:
	make -C ../synth


# Clean rules
clean:
	rm -f $(OBJS) $(OBJS:.o=.d) $(TARGET) $(PRJ_NAME).objdump $(PRJ_NAME).hexdump
	make -C minilibc clean

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
