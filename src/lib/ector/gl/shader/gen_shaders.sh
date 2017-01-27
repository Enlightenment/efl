#!/bin/sh

# This script will generate a C file containing all the shaders used by Evas

DIR=`dirname $0`

OUTPUT="$DIR/ector_gl_shaders.x"

# Skip generation if there is no diff (or no git)
if ! git rev-parse 2>> /dev/null >> /dev/null ; then exit 0 ; fi
if git diff --quiet --exit-code -- "$DIR"
then
  for f in gen_shaders.sh fragment.glsl vertex.glsl include.glsl; do
    if [ "$DIR/$f" -nt "$OUTPUT" ]; then
      touch "$OUTPUT"
      break
    fi
  done
  exit 0
fi

# Write header
rm -f "$OUTPUT.tmp"
cat <<EOF >> $OUTPUT.tmp
/* DO NOT MODIFY THIS FILE AS IT IS AUTO-GENERATED */
/* IF IT IS CHANGED PLEASE COMMIT THE CHANGES */

EOF

for SHD in fragment vertex ; do
  m4 "$DIR/include.glsl" "$DIR/$SHD.glsl" > "$SHD.tmp"

  OIFS="$IFS"
  IFS=$'\n'
  echo -n "static const char ${SHD}_glsl[] =" >> "$OUTPUT.tmp"
  for LINE in `cat "$SHD.tmp"` ; do
      printf "\n   \"$LINE\\\n\"" >> "$OUTPUT.tmp"
  done
  printf ";\n\n" >> "$OUTPUT.tmp"
  IFS="$OIFS"

  rm "$SHD.tmp"
done

if ! cmp "$OUTPUT" "$OUTPUT.tmp" >/dev/null 2>/dev/null; then
    echo "$OUTPUT updated, please 'git commit' it."
    rm -f "$OUTPUT"
    mv "$OUTPUT.tmp" "$OUTPUT"
else
    rm -f "$OUTPUT.tmp"
fi

