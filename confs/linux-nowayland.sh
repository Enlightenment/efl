#!/bin/sh -e
meson -Dfb=true \
 $@ . build
