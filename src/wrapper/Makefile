CC := gcc
CFLAGS := -g -O0 -fpic -I./

all: libdyad_sync.so

libdyad_sync.so: wrapper.o
	$(CC) $(CFLAGS) -Wall -Wl,-init,dyad_sync_init $^ -shared -o $@ -ldl

wrapper.o: wrapper.c wrapper.h
	$(CC) $(CFLAGS) -Wall -DDYAD_CHECK=1 $< -c -o $@

.PHONY: clean

clean:
	rm -f *.o *~ libdyad_sync.so
