#!/bin/sh

DIR=$1
shift
FILE=$1
shift

export ELM_THEME=default
export ELM_SCALE=1.0
export ELM_ENGINE=shot:delay=3:file=$DIR/$FILE
export ELM_DISPLAY=shot
export ILLUME_KBD=0,0,240,123
export ILLUME_IND=0,0,240,32
export ILLUME_STK=0,288,240,32
export EFL_RUN_IN_TREE=1

mkdir -p $DIR || true

exec $@
