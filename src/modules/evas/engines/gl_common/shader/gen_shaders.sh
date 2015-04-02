#!/bin/bash

set -e

DIR=`dirname $0`
OUTPUT=${DIR}/evas_gl_shaders.x
OUTPUT_ENUM=${DIR}/evas_gl_enum.x
CPP="cpp -P"

VTX_HEADER="#ifdef GL_ES
precision highp float;
#endif
"

FGM_HEADER="#ifdef GL_ES
#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif
#endif
"

# Hack
FGM_HEADER_OES="#ifdef GL_ES
# extension GL_OES_EGL_image_external : require
# ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
# else
precision mediump float;
# endif
uniform samplerExternalOES tex;
#else
uniform sampler2D tex;
#endif
"

function upper() {
  echo $@ |tr '[:lower:]' '[:upper:]'
}

function run() {
  [ "$V" = "1" ] && echo "$@"
  "$@"
}

# Urgh
OIFS=$IFS
IFS=$'\n' SHADERS=(`cat ${DIR}/shaders.txt`)
IFS=$OIFS

# Write header
printf "/* DO NOT MODIFY THIS FILE AS IT IS AUTO-GENERATED\n * See: $0 */\n\n" > ${OUTPUT}

# Including private for hilights and stuff :)
printf "#include \"../evas_gl_private.h\"\n\n" >> ${OUTPUT}

# Prepare list of shaders:
shaders_source=""
shaders_enum=""
shaders_type=(frag vert)

# Generate SHD files
LIST=""
for (( i = 0; i < ${#SHADERS[@]} ; i++ )) ; do
  s=${SHADERS[$i]}
  if [[ $s == \#* ]] ; then continue ; fi

  name=`echo $s |cut -d ':' -f 1`
  opts=`echo $s |cut -d ':' -f 2 |tr ',' '\n'`

  echo "  SHADER   $name"
  UNAME=`echo ${name} |tr "[:lower:]" "[:upper:]"`
  OPTS=""
  FGM=${FGM_HEADER}
  TYPE=`echo ${name} |cut -d '_' -f 1 |tr '[:lower:]' '[:upper:]'`
  bgra=0
  mask=0
  nomul=0
  afill=0
  sam="SAM11"

  # Urgh. Some fixups
  case $TYPE in
   IMG) TYPE="IMAGE";;
   RGB) TYPE="RGB_A_PAIR";;
   TEX) TYPE="TEX_EXTERNAL";;
  esac

  for opt in $opts ; do
    UP=`upper $opt`
    OPTS="$OPTS -DSHD_${UP}"
    case $opt in
     bgra) bgra=1;;
     mask) mask=1;;
     nomul) nomul=1;;
     afill) afill=1;;
     external) FGM=${FGM_HEADER_OES};;
     sam12) sam="SAM12";;
     sam21) sam="SAM21";;
     sam22) sam="SAM22";;
    esac
  done

  # vertex shader
  filev=${DIR}/${name}_vert.shd
  printf "${VTX_HEADER}" >| ${filev}
  run $CPP $OPTS ${DIR}/vertex.glsl >> ${filev}

  # fragment shader
  filef=${DIR}/${name}_frag.shd
  printf "${FGM}" >| ${filef}
  run $CPP $OPTS ${DIR}/fragment.glsl >> ${filef}

  k=0
  for shd in ${filef} ${filev} ; do
    shdname=${name}_${shaders_type[$k]}
    k=$(($k + 1))

    OIFS=$IFS
    IFS=$'\n'
    printf "static const char const ${shdname}_glsl[] =" >> ${OUTPUT}
    for line in `cat ${shd}` ; do
      printf "\n   \"${line}\\\n\"" >> ${OUTPUT}
    done
    printf ";\n" >> ${OUTPUT}
    IFS=${OIFS}

    printf "Evas_GL_Program_Source shader_${shdname}_src =\n{\n   ${shdname}_glsl,\n   NULL, 0\n};\n\n" >> ${OUTPUT}
  done

  shaders_source="${shaders_source}   { SHADER_${UNAME}, &(shader_${name}_vert_src), &(shader_${name}_frag_src), \"${name}\", SHD_${TYPE}, SHD_${sam}, ${bgra}, ${mask}, ${nomul}, ${afill} },\n"
  shaders_enum="${shaders_enum}   SHADER_${UNAME},\n"
done

printf "
static const struct {
   Evas_GL_Shader id;
   Evas_GL_Program_Source *vert;
   Evas_GL_Program_Source *frag;
   const char *name;
   Shader_Type type;
   Shader_Sampling sam;
   Eina_Bool bgra : 1;
   Eina_Bool mask : 1;
   Eina_Bool nomul : 1;
   Eina_Bool afill : 1;
} _shaders_source[] = {\n" >> ${OUTPUT}

printf "${shaders_source}};\n\n" >> ${OUTPUT}

printf "/* DO NOT MODIFY THIS FILE AS IT IS AUTO-GENERATED\n * See: $0 */

typedef enum {
${shaders_enum}   SHADER_LAST
} Evas_GL_Shader;
" >| ${OUTPUT_ENUM}

# You can remove the files now
#rm -f ${LIST}

