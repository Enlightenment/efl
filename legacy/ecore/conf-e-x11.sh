#!/bin/sh

if [ -f ./Makefile ]; then
    make clean distclean
fi
if [ -f ./config.cache ]; then
    rm config.cache
fi

./autogen.sh \
--enable-ecore-x \
--enable-ecore-evas \
--enable-ecore-evas-gl \
--enable-ecore-job \
--enable-ecore-con \
--enable-ecore-ipc \
$@
