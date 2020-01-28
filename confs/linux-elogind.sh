#!/bin/sh -e
meson -Dfb=true -Ddrm=true -Dwl=true -Dsystemd=false -Delogind=true \
 $@ . build
