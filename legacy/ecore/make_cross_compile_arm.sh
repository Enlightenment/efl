#!/bin/sh

make clean distclean
./configure \
--enable-ecore-fb \
--enable-ecore-evas \
--enable-ecore-job \
--enable-ecore-con \
--enable-ecore-ipc

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
mkdir $DST"/share/ecore";

pushd src

 pushd lib

 for I in ecore ecore_fb ecore_job ecore_evas ecore_con ecore_ipc; do
  LIB=$I
  pushd $LIB
   $CC \
   *.c \
   $CFLAGS \
   -I. -I../ecore -I../ecore_x -I../ecore_fb -I../ecore_job -I../ecore_evas -I../ecore_con -I../ecore_ipc \
   -I../../.. \
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
   cp -a Ecore*.h $DST"/include";
  popd
 done

 popd

 pushd bin
  BIN="ecore_test"
   $CC $BIN".c" \
   -I../.. -I../lib \
   -I. -I../lib/ecore -I../lib/ecore_x -I../lib/ecore_fb -I../lib/ecore_job -I../lib/ecore_evas -I../lib/ecore_con -I../lib/ecore_ipc \
   -I/skiff/local/include \
   -L. -L../lib/ecore -L../lib/ecore_x -L../lib/ecore_fb -L../lib/ecore_job -L../lib/ecore_evas -L../lib/ecore_con -L../lib/ecore_ipc \
   -lecore -lecore_evas -lecore_fb -lecore_job -lecore_con -lecore_ipc -levas -lfreetype -ljpeg -lpng -lz -lm \
   -o $BIN
   $ST $BIN
   cp -a $BIN $DST"/bin";
  BIN="ecore_evas_test"
   $CC \
   ecore_evas_test.c \
   ecore_evas_test_app.c \
   ecore_evas_test_bg.c \
   ecore_evas_test_calibrate.c \
   -I../.. -I../lib \
   -I. -I../lib/ecore -I../lib/ecore_x -I../lib/ecore_fb -I../lib/ecore_job -I../lib/ecore_evas -I../lib/ecore_con -I../lib/ecore_ipc \
   -I/skiff/local/include \
   -L. -L../lib/ecore -L../lib/ecore_x -L../lib/ecore_fb -L../lib/ecore_job -L../lib/ecore_evas -L../lib/ecore_con -L../lib/ecore_ipc \
   -lecore -lecore_evas -lecore_fb -lecore_con -lecore_con -lecore_ipc -levas -lfreetype -ljpeg -lpng -lz -lm \
   -o $BIN
   $ST $BIN
   cp -a $BIN $DST"/bin";
 popd
popd

cp -ar data $DST"/share/ecore"

PD=`pwd`
pushd "build"
 tar zcvf $PD"/data.tar.gz" *
 pushd /skiff/local/
  sudo tar zxvf $PD"/data.tar.gz"
 popd
popd
