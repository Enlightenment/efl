#!/bin/sh

PROJ="edje"

SKIFF="/skiff/local"
HOSTARCH="i686-pc-linux-gnu"
TARGETCPU="arm"
TARGETARCH=$TARGETCPU"-pc-linux-gnu"

export EDB_CONFIG=$SKIFF"/"$TARGETCPU"-linux/bin/edb-config"
export EET_CONFIG=$SKIFF"/"$TARGETCPU"-linux/bin/eet-config"
export FREETYPE_CONFIG=$SKIFF"/"$TARGETCPU"-linux/bin/freetype-config"
export EVAS_CONFIG=$SKIFF"/"$TARGETCPU"-linux/bin/evas-config"
export ECORE_CONFIG=$SKIFF"/"$TARGETCPU"-linux/bin/ecore-config"

make clean distclean
export CC=/skiff/local/bin/arm-linux-gcc
export CFLAGS=-O9
./configure \
--host=$HOSTARCH \
--build=$TARGETARCH \
--target=$TARGETARCH \
--disable-edje-cc \
--enable-fb-only

INST="/tmp/"$PROJ"-instroot"
sudo rm -rf $INST

make

for I in  `find . -name "*.la" -print`; do
  sed s:"/usr/local":$INST:g < $I > "/tmp/.sed.tmp"
  sudo cp "/tmp/.sed.tmp" $I
  rm -f "/tmp/.sed.tmp"
done

sudo \
make \
prefix=$INST \
exec_prefix=$INST \
bindir=$INST"/bin" \
sbindir=$INST"/sbin" \
sysconfdir=$INST"/etc" \
datadir=$INST"/share" \
includedir=$INST"/include" \
libdir=$INST"/lib" \
libexecdir=$INST"/libexec" \
localstatedir=$INST"/var/run" \
mandir=$INST"/share/man" \
infodir=$INST"/share/info" \
install

## FIXUPS
for I in $INST"/bin/"* $INST"/sbin/"* $INST"/libexec/"*; do
  J=`echo $I | sed s:$TARGETARCH"-"::g`
  sudo mv $I $J
done

CF=$INST"/bin/"$PROJ"-config"
sed s:"/usr/local":$SKIFF"/"$TARGETCPU"-linux":g < $CF > "/tmp/.sed.tmp"
sudo cp "/tmp/.sed.tmp" $CF
rm -f "/tmp/.sed.tmp"

for I in  $INST"/lib/"*.la; do
  sed s:"/usr/local":$SKIFF"/"$TARGETCPU"-linux":g < $I > "/tmp/.sed.tmp"
  sudo cp "/tmp/.sed.tmp" $I
  rm -f "/tmp/.sed.tmp"
done
      
## package it all up
PACK=$PROJ"-"$TARGETCPU"-inst.tar.gz"

DIR=$PWD
cd $INST
sudo tar zcvf $DIR"/"$PACK *
sudo chown $USER $DIR"/"$PACK
cd $DIR
sudo rm -rf $INST

## install it in our skiff tree
cd $SKIFF"/"$TARGETCPU"-linux"
sudo tar zxvf $DIR"/"$PACK
  
