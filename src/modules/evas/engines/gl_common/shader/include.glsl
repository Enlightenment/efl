define(`FRAGMENT_SHADER',`
#ifdef GL_ES
# ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
# else
precision mediump float;
# endif
# ifdef SHD_EXTERNAL
extension GL_OES_EGL_image_external : require
#  define SAMPLER_EXTERNAL_OES samplerExternalOES
# endif
#else
# define SAMPLER_EXTERNAL_OES sampler2D
#endif
')

define(`VERTEX_SHADER',`
#ifdef GL_ES
precision highp float;
#endif
')

