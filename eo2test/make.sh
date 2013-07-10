#! /bin/bash

S="../src/lib/eo/*.c"
I="-I../src/lib/eo -I../build -I../ -Ibuild"
D="-DHAVE_CONFIG_H=1"
L=`pkg-config --cflags --libs eina`

CC=${CC:-gcc}

$CC $CFLAGS eo2test.c $S $I $D $L -std=c99 -o eo2test && ./eo2test

$CC $CFLAGS eo_simple.c eo2test.c eo2-bench.c -DNOMAIN $S $I $D $L -std=gnu99 -lrt -o eo2test-bench && ./eo2test-bench
