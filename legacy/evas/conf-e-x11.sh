#!/bin/sh

#make clean distclean
rm config.cache
./autogen.sh \
--enable-software-x11 \
--enable-gl-x11 \
--enable-image-loader-png \
--enable-image-loader-jpeg \
--enable-image-loader-eet \
--enable-image-loader-edb \
--enable-font-loader-eet \
--enable-cpu-mmx \
--enable-cpu-sse \
--enable-cpu-c \
--enable-scale-smooth \
--enable-scale-sample \
--enable-convert-8-rgb-332 \
--enable-convert-8-rgb-666 \
--enable-convert-8-rgb-232 \
--enable-convert-8-rgb-222 \
--enable-convert-8-rgb-221 \
--enable-convert-8-rgb-121 \
--enable-convert-8-rgb-111 \
--enable-convert-16-rgb-565 \
--enable-convert-16-rgb-555 \
--enable-convert-16-rgb-rot-0 \
--enable-convert-16-rgb-rot-90 \
--enable-convert-16-rgb-rot-270 \
--enable-convert-32-rgb-8888 \
--enable-convert-32-rgbx-8888 \
--enable-convert-32-bgr-8888 \
--enable-convert-32-bgrx-8888 \
--enable-convert-32-rgb-rot-0 \
--enable-convert-32-rgb-rot-90 \
--enable-convert-32-rgb-rot-270 \
--enable-convert-yuv \
$@
