#!/bin/sh

DIR=$1
shift
FILE=$1
shift

export ELM_THEME=default
export ELM_SCALE=1.0
export ELM_ENGINE=shot:delay=0.5:file=$DIR/$FILE

mkdir -p $DIR || true

exec $@
