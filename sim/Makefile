CC := gcc
CFLAGS += -Wall -Wno-pointer-sign \
	-fsanitize=address \
	-ggdb -O0

all: sim

sim: sha256.o sim.c

.PHONY: clean all
clean:
	rm -f sim *.o
