#
# Author: Mikeqin <Fengling.Qin@gmail.com>
#
# This is free and unencumbered software released into the public domain.
# For details see the UNLICENSE file at the root of the source tree.
#

include ../../defaultc.mk

$(SLIB): ${OBJS}
	mkdir -p $(LIBDIR)
	$(AR) -r -o $@ $(OBJS)
	mv $(SLIB) $(LIBDIR)

all: $(SLIB)

clean:
	rm -f $(OBJS) $(SLIB) $(LIBDIR)/$(SLIB)
