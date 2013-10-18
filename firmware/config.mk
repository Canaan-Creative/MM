#
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
AR    := $(CROSS)ar

# ----- Quiet code ----------------------------------------------------------
SHELL=/bin/bash
CPP := $(CPP)   # make sure changing CC won't affect CPP

CC_normal	:= $(CC)
AR_normal	:= $(AR)
LD_normal	:= $(LD)
DEPEND_normal	:= $(CPP) $(CFLAGS) -D__OPTIMIZE__ -MM -MG
RANLIB_normal	:= ranlib

CC_quiet	= @echo "  CC       " $@ && $(CC_normal)
AR_quiet	= @echo "  AR       " $@ && $(AR_normal)
LD_quiet	= @echo "  LD       " $@ && $(LD_normal)
DEPEND_quiet	= @$(DEPEND_normal)
RANLIB_quiet	= @$(RANLIB_normal)

ifeq ($(V),1)
    CC		= $(CC_normal)
    AR		= $(AR_normal)
    LD		= $(LD_normal)
    DEPEND	= $(DEPEND_normal)
    RANLIB	= $(RANLIB_normal)
else
    CC		= $(CC_quiet)
    AR		= $(AR_quiet)
    LD		= $(LD_quiet)
    DEPEND	= $(DEPEND_quiet)
    RANLIB	= $(RANLIB_quiet)
endif

CPU_CONFIG = -mmultiply-enabled -mbarrel-shift-enabled -muser-enabled
CPPFLAGS   += -std=gnu99 -Os -ffunction-sections -ffreestanding -Wall -Werror $(CPU_CONFIG) $(INCLUDES)
