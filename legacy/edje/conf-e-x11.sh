#!/bin/sh
if [ -f ./Makefile ]; then
    make clean distclean
fi
if [ -f ./config.cache ]; then
    rm config.cache
fi
./autogen.sh \
$@
