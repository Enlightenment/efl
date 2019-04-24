#!/bin/sh

# Installer script needed to install documentation xml file
# as meson's library() call can't track files other than the generated
# efl_mono.dll

cp "$1" "$DESTDIR$2"
