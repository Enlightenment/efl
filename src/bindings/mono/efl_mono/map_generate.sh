#!/bin/sh

find ${MESON_SOURCE_ROOT}/src/lib/ -iname "*\.eo" | sed -n 's/.*\/src\/lib\/\([a-z0-9_]*\)[\/a-z]*\/\([a-z0-9\._]*\)/\2,\1/pg'
