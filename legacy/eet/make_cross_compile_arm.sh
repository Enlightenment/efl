#!/bin/sh

make clean distclean
./configure

cp config.h config.h.bak
grep -v HAVE_FMEMOPEN config.h.bak > config.h
#cp config.h config.h.bak
#grep -v HAVE_OPEN_MEMSTREAM config.h.bak > config.h
rm config.h.bak

CC="/skiff/local/bin/arm-linux-gcc"
ST="/skiff/local/bin/arm-linux-strip"
CFLAGS="-O2"

rm -rf "build"
mkdir "build"
mkdir "build""/usr"
mkdir "build""/usr/local"
DST=`pwd`"/build""/usr/local";

mkdir $DST"/lib";
mkdir $DST"/bin";
mkdir $DST"/include";
#mkdir $DST"/share";
#mkdir $DST"/share/eet";

pushd src

 pushd lib
  LIB="eet"
  $CC \
  *.c \
  $CFLAGS \
  -I. \
  -I../.. \
  -I/skiff/local/include \
  -shared -fPIC -DPIC \
  -Wl,-soname -Wl,"lib"$LIB".so.0.0.1" \
  -o "lib"$LIB".so.0.0.1"
  $ST -g "lib"$LIB".so.0.0.1"
  rm -f "lib"$LIB".so"
  ln -s "lib"$LIB".so.0.0.1" "lib"$LIB".so"
  rm -f "lib"$LIB".so.0"
  ln -s "lib"$LIB".so.0.0.1" "lib"$LIB".so.0"
  rm -f "lib"$LIB".so.0.0"
  ln -s "lib"$LIB".so.0.0.1" "lib"$LIB".so.0.0"
  cp -a "lib"$LIB".so"* $DST"/lib";
  cp -a "Eet.h" $DST"/include";
 popd

 pushd bin
  BIN="eet"
   $CC "eet_main.c" \
   -I../.. -I../lib \
   -I. \
   -I/skiff/local/include \
   -L. -L../lib \
   -L/skiff/local/lib \
   -leet -ljpeg -lz -lm \
   -o $BIN
   $ST $BIN
   cp -a $BIN $DST"/bin";
 popd
popd

#cp -ar data $DST"/share/eet"

PD=`pwd`
pushd "build"
 tar zcvf $PD"/data.tar.gz" *
 pushd /skiff
  sudo tar zxvf $PD"/data.tar.gz"
 popd
popd
