#!/bin/sh

# This script will generate a C file containing all the shaders used by Evas_3D

OUTPUT="$1"
OUTDIR=$(dirname ${OUTPUT})
INDIR=`dirname $0`

mkdir -p $OUTDIR

# Skip generation during make distcheck
if [ "${top_distdir}" != "" ] ; then exit 0; fi

# Skip generation if file can not be written to
if [ -e ${OUTPUT} ] && [ ! -w ${OUTPUT} ] ; then exit 0; fi

# Skip generation if there is no diff (or no git)
if ! git rev-parse 2>> /dev/null >> /dev/null ; then exit 0 ; fi
if git diff --quiet --exit-code -- "$OUTDIR"
then
  touch "$OUTPUT"
  exit 0
fi

exec 1<&-
exec 1>"$OUTPUT"

SHADERS=${@:2}
VERT_SHADERS_SOURCE=""
FRAG_SHADERS_SOURCE=""

# Write header
printf "/* DO NOT MODIFY THIS FILE AS IT IS AUTO-GENERATED */\n"
printf "/* IF IT IS CHANGED PLEASE COMMIT THE CHANGES */\n\n"

for SHD in $SHADERS ; do
  LNAME=`basename "$SHD" .shd`

  if echo "$LNAME" | grep _vert 2>&1 >> /dev/null ; then
    VERT_SHADERS_SOURCE="$VERT_SHADERS_SOURCE    "$LNAME"_glsl,\n"
  fi
  if echo "$LNAME" | grep _frag 2>&1 >> /dev/null ; then
    FRAG_SHADERS_SOURCE="$FRAG_SHADERS_SOURCE    "$LNAME"_glsl,\n"
  fi

  m4 "$INDIR/include.shd" "$SHD" > "$SHD.tmp"

  OIFS="$IFS"
  IFS=`printf '\n+'`
  IFS=${IFS%+}
  printf "static const char "$LNAME"_glsl[] ="
  for LINE in `cat "$SHD.tmp"` ; do
      printf "\n   \"$LINE\\\n\""
  done
  printf ";\n\n"
  IFS="$OIFS"

  rm "$SHD.tmp"
done

printf "static const char *vertex_shaders[] =
{\n"
      printf "$VERT_SHADERS_SOURCE"
printf "};\n\n"

printf "static const char *fragment_shaders[] =
{\n"
      printf "$FRAG_SHADERS_SOURCE"
printf "};\n"
