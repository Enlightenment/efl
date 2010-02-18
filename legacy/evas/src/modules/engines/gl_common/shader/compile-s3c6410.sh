#!/bin/sh
ORIONEXE=/home/raster/Data/orion/orion.exe
OPTS="-O --nolodcalc -lp"

function compile()
{
  F=$1

  make-c-str.sh $F"_frag.shd" > $F"_frag.h"
  if test -f $F"_frag_s3c6410.asm"; then
    wine $ORIONEXE -a $OPTS -f $F"_frag_s3c6410.asm"
    make-c-bin.sh $F"_frag_s3c6410.bin" > $F"_frag_bin_s3c6410.h"
    rm -f   $F"_frag_s3c6410.bin"   $F"_frag_s3c6410.h"
  else
    wine $ORIONEXE $OPTS -f $F"_frag.shd"
    make-c-bin.sh $F"_frag.shd.bin" > $F"_frag_bin_s3c6410.h"
    rm -f   $F"_frag.shd.bin"   $F"_frag.shd.asm"   $F"_frag.shd.h"
  fi

  make-c-str.sh $F"_vert.shd" > $F"_vert.h"
  if test -f $F"_vert_s3c6410.asm"; then
    wine $ORIONEXE -a $OPTS -v $F"_vert_s3c6410.asm"
    make-c-bin.sh $F"_vert_s3c6410.bin" > $F"_vert_bin_s3c6410.h"
    rm -f   $F"_vert_s3c6410.bin"   $F"_vert_s3c6410.h"
  else
    wine $ORIONEXE $OPTS -v $F"_vert.shd"
    make-c-bin.sh $F"_vert.shd.bin" > $F"_vert_bin_s3c6410.h"
    rm -f   $F"_vert.shd.bin"   $F"_vert.shd.asm"   $F"_vert.shd.h"
  fi
}

compile rect
compile img
compile img_nomul
compile img_solid
compile img_solid_nomul
compile img_bgra
compile img_bgra_nomul
compile img_bgra_solid
compile img_bgra_solid_nomul
compile font
compile yuv
compile tex
