#!/bin/sh

make clean distclean
./configure \
--disable-software-x11 \
--disable-directfb \
 --enable-fb \
--disable-buffer \
--disable-software-qtopia \
--disable-gl-x11 \
 --enable-image-loader-png \
 --enable-image-loader-jpeg \
 --enable-image-loader-eet \
--disable-image-loader-edb \
 --enable-font-loader-eet \
--disable-cpu-mmx \
--disable-cpu-sse \
--disable-cpu-altivec \
 --enable-cpu-c \
 --enable-scale-sample \
 --enable-scale-smooth \
 --enable-convert-yuv \
--disable-small-dither-mask \
--disable-convert-8-rgb-332 \
--disable-convert-8-rgb-666 \
--disable-convert-8-rgb-232 \
--disable-convert-8-rgb-222 \
--disable-convert-8-rgb-221 \
--disable-convert-8-rgb-121 \
--disable-convert-8-rgb-111 \
--disable-convert-16-rgb-565 \
--disable-convert-16-rgb-555 \
--disable-convert-16-rgb-444 \
 --enable-convert-16-rgb-ipq \
--disable-convert-16-rgb-rot-0 \
 --enable-convert-16-rgb-rot-270 \
--disable-convert-16-rgb-rot-90 \
--disable-convert-24-rgb-888 \
--disable-convert-24-bgr-888 \
--disable-convert-32-rgb-8888 \
--disable-convert-32-rgbx-8888 \
--disable-convert-32-bgr-8888 \
--disable-convert-32-bgrx-8888 \
--disable-convert-32-rgb-rot-0 \
--disable-convert-32-rgb-rot-270 \
--disable-convert-32-rgb-rot-90

CC="/skiff/local/bin/arm-linux-gcc"
ST="/skiff/local/bin/arm-linux-strip"
CFLAGS="-O9"

rm -rf "build"
mkdir "build"
DST=`pwd`"/build";

mkdir $DST"/lib";
mkdir $DST"/bin";
mkdir $DST"/include";
mkdir $DST"/share";
mkdir $DST"/share/evas";

pushd src

 pushd lib
  LIB="evas"
  $CC \
  main.c canvas/*.c data/*.c engines/fb/*.c file/*.c \
  engines/common/evas_blend_alpha_color_pixel.c \
  engines/common/evas_blend_color_pixel.c \
  engines/common/evas_blend_main.c \
  engines/common/evas_blend_pixel_cmod_pixel.c \
  engines/common/evas_blend_pixel_mul_pixel.c \
  engines/common/evas_blend_pixel_pixel.c \
  engines/common/evas_blit_main.c \
  engines/common/evas_convert_color.c \
  engines/common/evas_convert_gry_1.c \
  engines/common/evas_convert_gry_4.c \
  engines/common/evas_convert_gry_8.c \
  engines/common/evas_convert_main.c \
  engines/common/evas_convert_rgb_16.c \
  engines/common/evas_convert_rgb_24.c \
  engines/common/evas_convert_rgb_32.c \
  engines/common/evas_convert_rgb_8.c \
  engines/common/evas_cpu.c \
  engines/common/evas_draw_main.c \
  engines/common/evas_font_draw.c \
  engines/common/evas_font_load.c \
  engines/common/evas_font_main.c \
  engines/common/evas_font_query.c \
  engines/common/evas_gradient_main.c \
  engines/common/evas_image_load.c \
  engines/common/evas_image_main.c \
  engines/common/evas_line_main.c \
  engines/common/evas_polygon_main.c \
  engines/common/evas_rectangle_main.c \
  engines/common/evas_scale_main.c \
  engines/common/evas_scale_sample.c \
  engines/common/evas_scale_smooth.c \
  engines/common/evas_tiler.c \
  $CFLAGS \
  -I. -Icanvas -Idata -Iengines/common -Iengines/fb -Ifile -Iinclude \
  -I/skiff/local/include/freetype2 \
  -I../.. \
  -I/skiff/local/include \
  -shared -fPIC -DPIC \
  -Wl,-soname -Wl,"lib"$LIB".so.1" \
  -o "lib"$LIB".so.1.0.0"
  $ST -g "lib"$LIB".so.1.0.0"
  rm -f "lib"$LIB".so"
  ln -s "lib"$LIB".so.1.0.0" "lib"$LIB".so"
  rm -f "lib"$LIB".so.1"
  ln -s "lib"$LIB".so.1.0.0" "lib"$LIB".so.1"
  rm -f "lib"$LIB".so.1.0"
  ln -s "lib"$LIB".so.1.0.0" "lib"$LIB".so.1.0"
  cp -a "lib"$LIB".so"* $DST"/lib";
  cp -a Evas.h Evas_Engine_FB.h $DST"/include";
 popd

 pushd bin
  BIN="evas_fb_test"
   $CC evas_fb_main.c evas_test_main.c \
   -I../.. -I../lib \
   -I. \
   -I/skiff/local/include \
   -L. -L../lib \
   -levas -leet -ljpeg -lpng -lfreetype -lm -lz \
   -o $BIN
   $ST $BIN
   cp -a $BIN $DST"/bin";
 popd
popd

cp -ar data $DST"/share/evas"

PD=`pwd`
pushd "build"
 tar zcvf $PD"/data.tar.gz" *
 pushd "/skiff/local"
  sudo tar zxvf $PD"/data.tar.gz"
 popd
popd
