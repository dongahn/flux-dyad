CC := gcc
CFLAGS := -g -O0 -fpic -I./

all: consumer_test

consumer_test: consumer_test.o
	$(CC) $(CFLAGS) $^ -o $@ -L../../libtap -ltap

consumer_test.o: consumer_test.c
	$(CC) $(CFLAGS) $< -c -o $@

.PHONY: clean

clean:
	rm -f *.o *~ consumer_test
