#!/bin/sh

cmake clean distclean
rm config.cache
./configure \
--enable-ecore-x \
--enable-ecore-evas \
--enable-ecore-evas-gl \
--enable-ecore-job \
--enable-ecore-con \
--enable-ecore-ipc \
$@
