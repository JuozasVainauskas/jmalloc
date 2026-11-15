# jmalloc

Implementation of memory allocator

## Setup

```bash
./scripts/setup-toolchain.sh
```

## Build

```bash
make build
```

## Run example

```bash
make run
```

## Test

```bash
make test
```

## Run program with the allocator

```bash
make build
LD_PRELOAD=$PWD/build/debug/libjmalloc.so htop
```
