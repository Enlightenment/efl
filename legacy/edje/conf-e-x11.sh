#!/bin/sh

cmake clean distclean
rm config.cache
./configure \
$@
