#!/bin/sh

set -e

if [ -d /usr/local/lib64 ] ; then
  export PKG_CONFIG_PATH=/usr/local/lib64/pkgconfig
else
  export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig
fi
cd .ci/example

#build the example
mkdir build
meson . build
ninja -C build all

