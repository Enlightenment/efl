#!/bin/sh -e
meson -Dfb=true -Dsystemd=false\
 $@ . build
