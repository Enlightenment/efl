#!/bin/sh

for x in "$@" ; do
	dir="$(dirname $x)"
	mv "$DESTDIR"/"$x" "$DESTDIR"/"$dir"/module.so
done
