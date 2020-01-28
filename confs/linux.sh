#!/bin/sh -e
meson -Dfb=true -Ddrm=true -Dwl=true \
 $@ . build
