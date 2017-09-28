#!/bin/sh

COPTS="--with-tests=regular"
PARALLEL_JOBS=10

# Normal build test of all targets
./autogen.sh $COPTS $@
make -j $PARALLEL_JOBS
make -j $PARALLEL_JOBS examples
make -j $PARALLEL_JOBS benchmark
