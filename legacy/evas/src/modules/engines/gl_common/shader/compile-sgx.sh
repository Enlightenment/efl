#!/bin/sh
function compile()
{
  F=$1

  make-c-str.sh $F"_frag.shd" > $F"_frag.h"
  make-c-str.sh $F"_vert.shd" > $F"_vert.h"
}

compile rect
compile img
compile font
