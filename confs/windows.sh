#!/bin/sh -e
meson -Dsystemd=false -Dv4l2=false -Davahi=false -Deeze=false -Dx11=false \
-Dopengl=none -Dpulseaudio=false -Dlibmount=false -Dfribidi=false \
-Devas-loaders-disabler=pdf,ps,raw,svg,rsvg,json,tga,tgv \
-Decore-imf-loaders-disabler=xim,ibus,scim \
 $@ . build
