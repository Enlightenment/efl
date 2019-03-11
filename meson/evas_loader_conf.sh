#!/bin/sh

original_loader=$1
filename="$(basename $original_loader)"
ext="${ext##*.}"
original_name="$(basename $original_loader .$ext)"
loader_dir="$(dirname $original_loader)"
loader="$(basename $original_loader)"
loader_name="$(echo $original_name | cut -f 1 -d '.')"

if [ `echo -n ${original_loader} | tail -c 3` == "$ext" ]
  then
    mv "$DESTDIR"/"$original_loader" "$DESTDIR"/"$loader_dir"/"$original_name"
fi

shift
for x in "$@"; do
  ln -sf "$original_name" "$DESTDIR"/"$loader_dir"/"$loader_name"."$x"
done
