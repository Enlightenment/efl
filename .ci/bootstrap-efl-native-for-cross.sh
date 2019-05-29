#!/bin/sh

mkdir build-bootstrap-native
meson --prefix=/usr/ --libdir=/usr/lib -Dbuild-examples=false -Dbuild-tests=false -Dbindings="" build-bootstrap-native
ninja -C build-bootstrap-native install
rm -rf build-bootstrap-native
ldconfig
