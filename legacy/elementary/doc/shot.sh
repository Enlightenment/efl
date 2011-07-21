#!/bin/sh

DIR=$1
shift
FILE=$1
shift

export ELM_THEME=default
export ELM_SCALE=1.0
export ELM_ENGINE=shot:delay=0.5:file=$DIR/$FILE
export ILLUME_KBD=0,0,240,123
export ILLUME_IND=0,0,240,32
export ILLUME_STK=0,288,240,32

mkdir -p $DIR || true

exec $@
