#! /bin/bash

S="../src/lib/eo/*.c"
I="-I../src/lib/eo -I../build -I../ -Ibuild"
D="-DHAVE_CONFIG_H=1"
L=`pkg-config --cflags --libs eina`

CC=${CC:-gcc}

$CC $CFLAGS eo_simple.c eo2_simple.c eo2-bench.c -DNOMAIN $S $I $D $L -std=gnu99 -lrt -o eo2-bench && ./eo2-bench || exit 1

g++ $CFLAGS --std=c++0x simplesignal.cc `pkg-config --cflags --libs eina eo` -o simplesignal && ./simplesignal || exit 1
