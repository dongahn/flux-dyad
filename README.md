## DYAD Synchronizer

DYAD component to intercept file open/close calls and synchronize producers and consumers using Flux APIs

#### Prerequisite:
Build and install flux-core. Please see https://github.com/flux-framework/flux-core.
 

#### To build `libdyad_sync.so`:

```
export PKG_CONFIG_PATH=<FLUX_INSTALL_ROOT>/lib/pkgconfig
cd src/wrapper
make
```

#### To build `libtap.a`:

```
cd src/libtap
make
```

#### To build test cases:

```
cd src/wrapper/test
make
```

#### To run the test:

Frist, start a flux instance 

```
<FLUX_INSTALL_ROOT>/bin/flux start -s 1 -o,-S,log-filename=out
```
This will create a new shell in which you can communicate with the instance. 


```
LD_PRELOAD=<path to libdyad_sync.so> DYAD_SYNC_CHECK=1 consumer_test &
LD_PRELOAD=<path to libdyad_sync.so> DYAD_SYNC_CHECK=1 producer_test
```

To enable debug trace for DYAD synchronizer, set the `DYAD_SYNC_DEBUG` environment variable to 1 as well.
