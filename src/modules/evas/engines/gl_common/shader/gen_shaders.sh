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
# define SAMPLER_EXTERNAL_OES samplerExternalOES
#else
# define SAMPLER_EXTERNAL_OES sampler2D
#endif
"

# Skip generation if there is no diff (or no git)
if ! git rev-parse 2>> /dev/null >> /dev/null ; then exit 0 ; fi
if git diff --quiet --exit-code -- "$DIR"
then
  touch "${OUTPUT}" "${OUTPUT_ENUM}"
  exit 0
fi

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
printf "/* DO NOT MODIFY THIS FILE AS IT IS AUTO-GENERATED */\n\n" > ${OUTPUT}

# Including private for hilights and stuff :)
printf "#include \"../evas_gl_private.h\"\n\n" >> ${OUTPUT}

# Prepare list of shaders:
shaders_source=""
shaders_enum=""
shaders_type=(frag vert)
shaders_textures=""

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
  tex=""

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
     tex) tex="${tex}tex ";;
     mask) tex="${tex}texm " ; mask=1;;
     texa) tex="${tex}texa ";;
     yuv) tex="${tex}texu texv ";;
     nv12) tex="${tex}texuv ";;
     yuy2) tex="${tex}texuv ";;
     bgra) bgra=1;;
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
    printf "static const char ${shdname}_glsl[] =" >> ${OUTPUT}
    for line in `cat ${shd}` ; do
      printf "\n   \"${line}\\\n\"" >> ${OUTPUT}
    done
    printf ";\n" >> ${OUTPUT}
    IFS=${OIFS}

    printf "Evas_GL_Program_Source shader_${shdname}_src =\n{\n   ${shdname}_glsl,\n   NULL, 0\n};\n\n" >> ${OUTPUT}
  done

  shaders_source="${shaders_source}   { SHADER_${UNAME}, &(shader_${name}_vert_src), &(shader_${name}_frag_src), \"${name}\", SHD_${TYPE}, SHD_${sam}, ${bgra}, ${mask}, ${nomul}, ${afill} },\n"
  shaders_enum="${shaders_enum}   SHADER_${UNAME},\n"

  # Bind textures to the programs. Only if there is more than 1 texture.
  textures=(${tex})
  if [ ${#textures[@]} -ge 2 ] ; then
    for tname in ${tex} ; do
      shaders_textures="${shaders_textures}   { SHADER_${UNAME}, \"${tname}\" },\n"
    done
  fi
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

printf "/* DO NOT MODIFY THIS FILE AS IT IS AUTO-GENERATED */

typedef enum {
${shaders_enum}   SHADER_LAST
} Evas_GL_Shader;

#ifdef _EVAS_GL_CONTEXT_C

static struct {
   Evas_GL_Shader id;
   const char *tname;
} _shaders_textures[] = {
${shaders_textures}   { SHADER_LAST, NULL }
};

#endif // _EVAS_GL_CONTEXT_C
" >| ${OUTPUT_ENUM}

# You can remove the files now
#rm -f ${LIST}

