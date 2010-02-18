#!/bin/bash
function compile()
{
  F=$1

  make-c-str.sh $F"_frag.shd" > $F"_frag.h"
  make-c-str.sh $F"_vert.shd" > $F"_vert.h"
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
