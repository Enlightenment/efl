#!/bin/sh -e
meson -Dfb=true -Devas-loaders-disabler=json \
 $@ . build
