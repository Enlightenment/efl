#!/bin/bash

DIR=`dirname $0`
MAKESTR="sh $DIR/make-c-str.sh"

function compile()
{
  F="$DIR/$1"

  if [ -f $F".shd" ]; then
    $MAKESTR $F".shd" > $F".h"
  fi
  if [ -f $F"_frag.shd" ]; then
    $MAKESTR $F"_frag.shd" > $F"_frag.h"
  fi
  if [ -f $F"_vert.shd" ]; then
    $MAKESTR $F"_vert.shd" > $F"_vert.h"
  fi
}

compile rect
compile font

compile img
compile img_nomul
compile img_bgra
compile img_bgra_nomul
compile tex
compile tex_nomul
compile tex_afill
compile tex_nomul_afill

compile img_21
compile img_21_nomul
compile img_21_bgra
compile img_21_bgra_nomul
compile tex_21
compile tex_21_nomul
compile tex_21_afill
compile tex_21_nomul_afill

compile img_12
compile img_12_nomul
compile img_12_bgra
compile img_12_bgra_nomul
compile tex_12
compile tex_12_nomul
compile tex_12_afill
compile tex_12_nomul_afill

compile img_22
compile img_22_nomul
compile img_22_bgra
compile img_22_bgra_nomul
compile tex_22
compile tex_22_nomul
compile tex_22_afill
compile tex_22_nomul_afill

## above section must have 21, 22 and 12 versions

compile yuv
compile yuv_nomul
compile nv12
compile nv12_nomul
compile yuy2
compile yuy2_nomul

compile filter_blur_bgra
compile filter_blur_bgra_nomul
compile filter_blur
compile filter_blur_nomul
compile filter_greyscale_bgra
compile filter_greyscale_bgra_nomul
compile filter_greyscale
compile filter_greyscale_nomul
compile filter_invert_bgra
compile filter_invert_bgra_nomul
compile filter_invert
compile filter_invert_nomul
compile filter_sepia_bgra
compile filter_sepia_bgra_nomul
compile filter_sepia
compile filter_sepia_nomul
