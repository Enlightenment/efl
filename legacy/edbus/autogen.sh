#!/bin/sh

autoreconf -f -i

if [ -z "$NOCONFIGURE" ]; then
    exec ./configure "$@"
fi
