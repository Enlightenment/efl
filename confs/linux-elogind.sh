#!/bin/sh -e
meson -Dfb=true -Ddrm=true -Dwl=true -Devas-loaders-disabler=json \
 -Dsystemd=false -Delogind=true \
 $@ . build
