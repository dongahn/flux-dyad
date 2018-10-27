CC := gcc
CFLAGS := -g -O0 -fpic -I./

all: consumer_test

consumer_test: consumer_test.o
	$(CC) $(CFLAGS) -Wall $^ -o $@ -L../../libtap -ltap

consumer_test.o: consumer_test.c
	$(CC) $(CFLAGS) -Wall $< -c -o $@

.PHONY: clean

clean:
	rm -f *.o *~ consumer_test
