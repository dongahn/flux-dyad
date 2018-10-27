dyad synchronizer: Intercepts file open and close calls and synchronize producers and consumers using Flux APIs

To build `libdyad_sync.so`

```
cd src/wrapper
make
```

To build `libtap.a`

```
cd src/libtap
make
```

To build a test case
```
cd src/wrapper/test
make
```

To run the test

```
LD_PRELOAD=<path to libdyad_sync.so> consumer_test
```

To enable debug trace, set the `DYAD_SYNC_DEBUG` environment variable to 1.
