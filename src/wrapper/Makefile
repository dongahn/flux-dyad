CC := gcc
CFLAGS := -g -O0 -fpic -I./ $(shell pkg-config --cflags flux-core)
FLUXLIBS := $(shell pkg-config --libs flux-core)
LDFLAGS := $(FLUXLIBS) -ldl

all: libdyad_sync.so

libdyad_sync.so: wrapper.o
	$(CC) $(CFLAGS) -Wall $^ -shared -o $@ $(LDFLAGS)

wrapper.o: wrapper.c wrapper.h
	$(CC) $(CFLAGS) -Wall -DDYAD_CHECK=1 $< -c -o $@

.PHONY: clean

clean:
	rm -f *.o *~ libdyad_sync.so
