#!/bin/bash
function compile()
{
  F=$1

  make-c-str.sh $F"_frag.shd" > $F"_frag.h"
  make-c-str.sh $F"_vert.shd" > $F"_vert.h"
}

compile rect
compile font
compile img
compile img_nomul
compile img_bgra
compile img_bgra_nomul
compile yuv
compile yuv_nomul
compile tex
compile tex_nomul
