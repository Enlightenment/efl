#!/bin/bash

# This script will generate a C file containing all the shaders used by Evas

DIR=`dirname $0`

OUTPUT=${DIR}/evas_gl_shaders.x

# Skip generation if there is no diff (or no git)
if ! git rev-parse 2>> /dev/null >> /dev/null ; then exit 0 ; fi
if git diff --quiet --exit-code -- "$DIR"
then
  touch "${OUTPUT}"
  exit 0
fi

exec 1<&-
exec 1>${OUTPUT}

# Write header
printf "/* DO NOT MODIFY THIS FILE AS IT IS AUTO-GENERATED */\n"
printf "/* IF IT IS CHANGED PLEASE COMMIT THE CHANGES */\n\n"

for shd in fragment vertex ; do
  m4 ${DIR}/include.glsl ${DIR}/${shd}.glsl > ${shd}.tmp

  OIFS=$IFS
  IFS=$'\n'
  printf "static const char ${shd}_glsl[] ="
  for line in `cat ${shd}.tmp` ; do
      printf "\n   \"${line}\\\n\""
  done
  printf ";\n\n"
  IFS=${OIFS}

  rm ${shd}.tmp
done

