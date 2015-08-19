#
# Author: Mikeqin <Fengling.Qin@gmail.com>
#
# This is free and unencumbered software released into the public domain.
# For details see the UNLICENSE file at the root of the source tree.
#

FIRMWARE = mcu
FW_RELEASE = RELEASE

DATE:=$(shell date +%Y-%m-%d)
OS:=$(shell uname -s)

NXPLIBS_PATH = ../nxplib/

SRCS	= $(wildcard ./code/src/*.c)
include mydefaultc.mk

NXP_PARAMETERS = -g -2 -vendor=NXP -pLPC11U14/201 -wire=winUSB -s50 -flash-driver=LPC11_12_13_32K_4K.cfx
LPCLINK_FIRM ?= LPCXpressoWIN.enc

GIT_VERSION:=$(shell git rev-parse HEAD | cut -c 1-7)
GIT_STATUS:=$(shell ([ -z "`git status -s -uno`" ] && echo 0) || echo +)

CFLAGS	+= -DAVAM_VERSION='"6C1508-$(GIT_VERSION)$(GIT_STATUS)"' \
	-I./code/inc -I$(NXPLIBS_PATH)/lpc_chip_11uxx_lib/inc \
	-I$(NXPLIBS_PATH)/nxp_lpcxpresso_11u14_board_lib/inc \
	-I$(NXPLIBS_PATH)/lpc_chip_11uxx_lib/inc/usbd

ifeq ($(DEBUG),1)
CFLAGS += -DDEBUG
endif

LDCFLAGS += -nostdlib -Xlinker -Map=$(FIRMWARE).map \
	-Xlinker --gc-sections -mcpu=cortex-m0 -mthumb \
	-T $(FIRMWARE).ld -lcr_c -lcr_eabihelpers

ifeq "$(FW_RELEASE)" "DEBUG"
LDCFLAGS += -lcr_semihost
else
LDCFLAGS += -lcr_nohost
endif

LDLIBS   += -lnxp_lpcxpresso_11u14_board_lib -llpc_chip_11uxx_lib \
	-lusbd_11uxx_lib
LIBPATH  += -L$(NXPLIBS_PATH)/lpc_chip_11uxx_lib/libs \
	-L$(NXPLIBS_PATH)/nxp_lpcxpresso_11u14_board_lib/libs

.PHONY: reflash_ulink2 erase_ulink2 reflash_lpclink erase_lpclink

all: $(FIRMWARE).axf
	mkdir -p bin/$(DATE)
	$(OBJCOPY) -O binary $^ $(FIRMWARE).bin
	$(OBJCOPY) -O ihex $^ $(FIRMWARE).hex
	cp $(FIRMWARE).bin $(FIRMWARE).hex $^ bin/$(DATE)
ifeq "$(OS)" "Darwin"
	cd bin/$(DATE) && md5 $(FIRMWARE).bin $(FIRMWARE).hex $^ > md5sums
else
	cd bin/$(DATE) && md5sum -b $(FIRMWARE).bin $(FIRMWARE).hex $^ > md5sums
endif
	$(SIZE) $^

$(FIRMWARE).axf: $(OBJS) lpc_chip_11uxx_lib nxp_lpcxpresso_11u14_board_lib
	$(CC) -Wl,--start-group $(LDCFLAGS) $(LIBPATH) $(OBJS) $(LDLIBS) -Wl,--end-group -o $@

lpc_chip_11uxx_lib:
	make -C $(NXPLIBS_PATH)/lpc_chip_11uxx_lib FW_RELEASE=$(FW_RELEASE)

nxp_lpcxpresso_11u14_board_lib:
	make -C $(NXPLIBS_PATH)/nxp_lpcxpresso_11u14_board_lib FW_RELEASE=$(FW_RELEASE)

reflash_ulink2: $(FIRMWARE).axf erase_ulink2
	(while ! (sleep 0.5 && crt_emu_cm_redlink -flash-load-exec $< $(NXP_PARAMETERS)); do : ; done;)

erase_ulink2:
	-(killall -s 9 redlinkserv && sleep 1)
	(while ! (sleep 0.5 && crt_emu_cm_redlink -flash-erase $(NXP_PARAMETERS)); do : ; done;)

reflash_lpclink: $(FIRMWARE).axf erase_lpclink
	(while !(sleep 0.5 && crt_emu_lpc11_13_nxp -flash-load-exec $< $(NXP_PARAMETERS)) do : ; done;)

erase_lpclink:
	-(dfu-util -d 0x0471:0xdf55 -c 0 -t 2048 -R -D $(LPCLINK_FIRM) && sleep 1)
	(while ! (sleep 0.5 && crt_emu_lpc11_13_nxp -flash-erase $(NXP_PARAMETERS)); do : ; done;)

clean:
	make -C $(NXPLIBS_PATH)/lpc_chip_11uxx_lib clean
	make -C $(NXPLIBS_PATH)/nxp_lpcxpresso_11u14_board_lib clean
	rm -f $(OBJS) $(FIRMWARE).axf $(FIRMWARE).map $(FIRMWARE).bin $(FIRMWARE).hex
