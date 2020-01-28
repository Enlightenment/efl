#!/bin/sh -e
meson -Dsystemd=false -Dv4l2=false -Davahi=false -Deeze=false -Dx11=false \
-Dopengl=full -Dcocoa=true -Dnls=false \
-Demotion-loaders-disabler=gstreamer1,libvlc,xine \
-Decore-imf-loaders-disabler=scim,ibus \
 $@ . build
