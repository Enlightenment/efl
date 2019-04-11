#!/bin/sh

ext="${ext##*.}"
# skip trailing dot if any
case "$1" in 
    *.) original_loader=${1%?};;
    *) original_loader=$1;;
esac
original_name="$(basename "$original_loader")"
loader_dir="$(dirname "$original_loader")"
loader="$(basename "$original_loader")"
loader_name="$(echo "$loader" | cut -f 1 -d '.')"

original_ext="$(echo "$loader" | tail -c 4)"
if test "$original_ext" = "$ext" ; then
    mv "$DESTDIR"/"$original_loader" "$DESTDIR"/"$loader_dir"/"$original_name"
fi

shift
for x in "$@"; do
  ln -sf "$original_name" "$DESTDIR"/"$loader_dir"/"$loader_name"."$x"
done
