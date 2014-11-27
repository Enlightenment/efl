#!/bin/bash

# This script will generate a C file containing all the shaders used by Evas.
# This C file should be checked-in Git and not modified.

DIR=`dirname $0`

OUTPUT=${DIR}/evas_gl_shaders.x
OUTPUT_ENUM=${DIR}/evas_gl_enum.x

#SHADERS=${DIR}/*.shd
SHADERS="$@"

# Redirect output to ${OUTPUT}
exec 1<&-
exec 1>${OUTPUT}

# Write header
printf "/* DO NOT MODIFY THIS FILE AS IT IS AUTO-GENERATED\n * See: $0 */\n\n"

# Including private for hilights and stuff :)
printf "#include \"../evas_gl_private.h\"\n\n"

# Prepare list of shaders:
shaders_source=""
shaders_enum=""

# Reading with the following code breaks indents:
#  while read -r line ; do
#  done < ${shd}
# So we use some cat & IFS hack instead.

for shd in ${SHADERS} ; do
  lname=`basename ${shd} .shd`
  UNAME=`echo ${lname} |tr "[:lower:]" "[:upper:]"`

  printf "/* Source: $shd */\n"

  OIFS=$IFS
  IFS=$'\n'
  printf "static const char const ${lname}_glsl[] ="
  for line in `cat ${shd}` ; do
    printf "\n   \"${line}\\\n\""
  done
  printf ";\n"
  IFS=${OIFS}

  printf "Evas_GL_Program_Source shader_${lname}_src =\n{\n   ${lname}_glsl,\n   NULL, 0\n};\n\n"

  if echo ${lname} |grep vert 2>&1 >> /dev/null ; then
    name=`echo ${lname} |sed -e 's/_vert//'`
    SHADER=`echo ${UNAME} |sed -e 's/_VERT//'`
    shaders_source="${shaders_source}   { SHADER_${SHADER}, &(shader_${name}_vert_src), &(shader_${name}_frag_src), \"${name}\" },\n"
    shaders_enum="${shaders_enum}   SHADER_${SHADER},\n"
  fi
done

printf "static const struct {
   Evas_GL_Shader id;
   Evas_GL_Program_Source *vert;
   Evas_GL_Program_Source *frag;
   const char *name;
} _shaders_source[] = {\n"

printf "${shaders_source}};\n\n"

printf "/* DO NOT MODIFY THIS FILE AS IT IS AUTO-GENERATED\n * See: $0 */

typedef enum {
${shaders_enum}   SHADER_LAST
} Evas_GL_Shader;
" >| ${OUTPUT_ENUM}

