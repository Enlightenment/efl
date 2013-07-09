#! /bin/bash

S="../src/lib/eo/*.c"
I="-I../src/lib/eo -I../build -I../ -Ibuild"
D="-DHAVE_CONFIG_H=1"
L=`pkg-config --cflags --libs eina`


gcc $CFLAGS $I $D $L $S -std=c99 eo2test.c -o eo2test && ./eo2test

gcc $CFLAGS $I $D $L $S -std=gnu99 eo2test-bench.c -lrt -o eo2test-bench && ./eo2test-bench
