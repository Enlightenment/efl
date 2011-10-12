#!/bin/bash
function compile()
{
  F=$1

  if [ -f $F".shd" ]; then
    make-c-str.sh $F".shd" > $F".h"
  fi
  if [ -f $F"_frag.shd" ]; then
    make-c-str.sh $F"_frag.shd" > $F"_frag.h"
  fi
  if [ -f $F"_vert.shd" ]; then
    make-c-str.sh $F"_vert.shd" > $F"_vert.h"
  fi
}

compile rect
compile font
compile img
compile img_nomul
compile img_bgra
compile img_bgra_nomul
compile img_mask
compile yuv
compile yuv_nomul
compile tex
compile tex_nomul
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
compile nv12
compile nv12_nomul
compile yuy2
compile yuy2_nomul

