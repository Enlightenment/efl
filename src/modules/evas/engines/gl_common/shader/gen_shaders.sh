#!/bin/sh

# This script will generate a C file containing all the shaders used by Evas

DIR=`dirname $0`
cd $DIR/../../../../../

OUTPUT="$DIR/evas_gl_shaders.x"

# Skip generation during make distcheck
if [ "${top_distdir}" != "" ] ; then exit 0; fi

# Skip generation if file can not be written to
if [ -e ${OUTPUT} ] && [ ! -w ${OUTPUT} ] ; then exit 0; fi

# Skip generation if there is no diff (or no git)
if ! git rev-parse 2>> /dev/null >> /dev/null ; then exit 0 ; fi
if git diff --quiet --exit-code -- "$DIR"
then
  touch "$OUTPUT"
  exit 0
fi

exec 1<&-
exec 1>"$OUTPUT"

# Write header
printf "/* DO NOT MODIFY THIS FILE AS IT IS AUTO-GENERATED */\n"
printf "/* IF IT IS CHANGED PLEASE COMMIT THE CHANGES */\n\n"

for SHD in fragment vertex ; do
  m4 "$DIR/include.glsl" "$DIR/$SHD.glsl" > "$SHD.tmp"

  OIFS="$IFS"
  IFS=`printf '\n+'`
  IFS=${IFS%+}
  printf "static const char "$SHD"_glsl[] ="
  for LINE in `cat "$SHD.tmp"` ; do
      printf "\n   \"$LINE\\\n\""
  done
  printf ";\n\n"
  IFS="$OIFS"

  rm "$SHD.tmp"
done

