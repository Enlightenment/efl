#!/bin/sh

mkdir build-eolian-bootstrap
meson --prefix=/usr/ --libdir=/usr/lib -Deolian-bootstrap=true build-eolian-bootstrap
ninja -C build-eolian-bootstrap install
rm -rf build-eolian-bootstrap
ldconfig
