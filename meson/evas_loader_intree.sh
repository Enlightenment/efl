#!/bin/sh
# The per-extension links the generic loaders are found by, made in the build
# tree as well as in the install.
#
# Installed, evas_loader_conf.sh makes them next to the installed helper. A
# build tree never had them, so running uninstalled there was no
# evas_image_loader.mp4 for the generic loader to run, and a video or a
# document had no thumbnail and no preview -- while the very same tree passed
# its tests, because nothing in them looks at a video.
#
# The links are made before the helpers are built, and point at names that
# appear later. That is fine: a link is followed when it is used.
dir="$1"
shift
original="$1"
shift

mkdir -p "$dir" || exit 1
for ext in "$@"; do
  # A loader named after one of the extensions it handles is already at the
  # name the link would take -- evas_image_loader.pdf is both. Linking that
  # to itself replaces the program with a loop.
  if [ "evas_image_loader.$ext" = "$original" ]; then
    continue
  fi
  ln -sf "$original" "$dir/evas_image_loader.$ext" || exit 1
done
