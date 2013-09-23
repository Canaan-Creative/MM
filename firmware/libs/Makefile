#
# Author: Xiangfu Liu <xiangfu@openmobilefree.net>
# Bitcoin:	1CanaaniJzgps8EV6Sfmpb7T8RutpaeyFn
#
# This is free and unencumbered software released into the public domain.
# For details see the UNLICENSE file at the root of the source tree.
#

WGET=wget -c -O 

RTEMS_VERSION=4.11
RTEMS_MAKEFILE_PATH ?= \
    /opt/lm32/lm32-rtems$(RTEMS_VERSION)/mm

JANSSON_VERSION   = 2.3
JANSSON   = jansson-$(JANSSON_VERSION).tar.gz

DL        = $(if $(wildcard ../dl/.),../dl,dl)
BUILD_DIR = ./build_dir

$(BUILD_DIR)/jansson: $(DL)/$(JANSSON).ok build_dir/.prepare
	tar xf $(DL)/$(JANSSON) -C $(BUILD_DIR)
	cd $(BUILD_DIR)/jansson-$(JANSSON_VERSION) && \
	    patch -Np1 < ../../patches/jansson-0001-for-milkymist-one.patch.patch
	cd $(BUILD_DIR)/jansson-$(JANSSON_VERSION) && \
	    $(CONFIGURE_VARS) ./configure --host=lm32-rtems4.11 \
	    --disable-shared --prefix=$(RTEMS_MAKEFILE_PATH) && \
	    make all install
	mkdir -p /opt/lm32/lm32-rtems4.11/mm/lib/include
	cp -rf $(RTEMS_MAKEFILE_PATH)/include/* \
	    $(RTEMS_MAKEFILE_PATH)/lib/include
	rm -rf $(RTEMS_MAKEFILE_PATH)/include/*
	touch $@

$(DL)/$(JANSSON).ok:
	mkdir -p dl
	$(WGET) $(DL)/$(JANSSON) "http://www.digip.org/jansson/releases/$(JANSSON)"
	touch $@

build_dir/.prepare:
	mkdir -p build_dir
	touch $@


clean:
	rm -rf build_dir/*
