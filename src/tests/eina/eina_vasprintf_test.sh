#!/bin/sh

# clean and update
\rm -f vasprintf_test eina_stdio.[hc]
\cp ../../lib/eina/eina_stdio.[hc] . &&

# build
clang-cl -o eina_vasprintf_test     \
    -Wno-include-next-absolute-path \
    eina_vasprintf_test.c           \
    eina_stdio.c                    &&

# test
./eina_vasprintf_test

# clean again
\rm -f eina_vasprintf_test eina_stdio.[hc]
