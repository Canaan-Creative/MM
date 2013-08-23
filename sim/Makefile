CFLAGS += -Wall -Wno-pointer-sign \
	-ggdb -O0

all: sim

sim: sha256.o sim.c

.PHONY: clean all
clean:
	rm -f sim *.o
