#!/bin/sh

make clean distclean
./configure

CC="/skiff/local/bin/arm-linux-gcc"
ST="/skiff/local/bin/arm-linux-strip"
CFLAGS="-O2"

rm -rf "build"
mkdir "build"
DST=`pwd`"/build";

mkdir $DST"/lib";
mkdir $DST"/bin";
mkdir $DST"/include";
mkdir $DST"/share";
mkdir $DST"/share/edje";

pushd src

 pushd lib
  LIB="edje"
  $CC \
  *.c \
  -DEDJE_FB_ONLY \
  $CFLAGS \
  -I. \
  -I../.. \
  -I/skiff/local/include \
  -shared -fPIC -DPIC \
  -Wl,-soname -Wl,"lib"$LIB".so.0" \
  -o "lib"$LIB".so.0.0.1"
  $ST -g "lib"$LIB".so.0.0.1"
  rm -f "lib"$LIB".so"
  ln -s "lib"$LIB".so.0.0.1" "lib"$LIB".so"
  rm -f "lib"$LIB".so.0"
  ln -s "lib"$LIB".so.0.0.1" "lib"$LIB".so.0"
  rm -f "lib"$LIB".so.0.0"
  ln -s "lib"$LIB".so.0.0.1" "lib"$LIB".so.0.0"
  cp -a "lib"$LIB".so"* $DST"/lib";
  cp -a Edje.h $DST"/include";
 popd

 pushd bin
  BIN="edje"
   $CC edje_main.c \
   -DEDJE_FB_ONLY \
   -I../.. -I../lib \
   -I. \
   -I/skiff/local/include \
   -L. -L../lib \
   -L/skiff/local/lib \
   -ledje \
   -lecore -lecore_evas -lecore_fb -lecore_ipc -lecore_con -lecore_job \
   -levas -leet -ljpeg -lpng -lfreetype -lm -lz \
   -o $BIN
   $ST $BIN
   cp -a $BIN $DST"/bin";
 popd
popd

mkdir $DST"/share/edje/data"
cp -ar data/test data/e_logo.eet $DST"/share/edje/data"

PD=`pwd`
pushd "build"
 tar zcvf $PD"/data.tar.gz" *
 pushd "/skiff/local"
  sudo tar zxvf $PD"/data.tar.gz"
 popd
popd
