#!/bin/sh

for x in "$@" ; do
  dir="$(dirname $x)"
  filename="$(basename $x)"
  ext="${filename##*.}"
  mv "$DESTDIR""$x" "$DESTDIR""$dir"/module."$ext"
done
