#!/bin/sh

cd $EXACTNESS_DATA_PATH
if [[ "$MAKEFLAGS" =~ -j([0-9]+) ]]
then
   JOBS=${BASH_REMATCH[1]}
else
   JOBS=1
fi
exactness -f fonts -j $JOBS -p -b recordings tests.txt

