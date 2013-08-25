HARDWARE_NAME := mm
PRJ_NAME := main

CPU_CONFIG := -mmultiply-enabled -mbarrel-shift-enabled -muser-enabled #-mdivide-enabled -msign-extend-enabled 
CC:= lm32-elf-gcc
LD:= lm32-elf-gcc
SIZE:= lm32-elf-size
CFLAGS:= -std=gnu99 -Os -ffunction-sections -ffreestanding -ggdb -Wall -Werror -I. -Isdk -Iminilibc $(CPU_CONFIG)
LDFLAGS:= -Wl,--gc-sections -Wl,-T,linker.ld -nostartfiles -nodefaultlibs -nostdlib -Wl,-Map=$(PRJ_NAME).map
LDLIBS:= -lgcc 
TARGET:=$(PRJ_NAME).elf

isedir ?= /opt/xilinx/13.3/ISE_DS/
xil_env ?= . $(isedir)/settings32.sh &>/dev/null

CSRCS:= \
	main.c\


########
HEADERS:=$(wildcard *.h */*.h)
SDK_SRCS:= \
	sdk/crt0ram.S \
	sdk/utils.S \
	sdk/intr.c \
	minilibc/memcpy.c \
	minilibc/memset.c \
	minilibc/printf.c \

all: $(TARGET)
	$(SIZE) $(TARGET)

OBJS := $(addsuffix .o,$(basename $(CSRCS) $(SDK_SRCS)))
$(TARGET): $(OBJS) | ../synth/$(HARDWARE_NAME)_bd.bmm ../synth/$(HARDWARE_NAME).bit
	$(LD) $(LDFLAGS) $(OBJS) -o $@ $(LDLIBS)
	$(xil_env); data2mem -bm ../synth/$(HARDWARE_NAME)_bd.bmm -bd $(TARGET) -bt ../synth/$(HARDWARE_NAME).bit -o b temp.bit
	mv temp.bit ../synth/$(HARDWARE_NAME).bit
#	$(MAKE) -C ../synth $(HARDWARE_NAME).mcs
%.o: %.S
	$(CC) $(CFLAGS) $< -c -o $@
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) $< -c -o $@
clean:
	rm -f *.o sdk/*.o minilibc/*.o $(TARGET)
.PHONY: all clean

../synth/$(HARDWARE)_bd.bmm ../synth/$(HARDWARE_NAME).bit:
	@echo "please do make in ../synth first"
	@false
