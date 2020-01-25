#!/bin/sh -e
meson -Dfb=true -Dsystemd=false -Devas-loaders-disabler=json \
 $@ . build
