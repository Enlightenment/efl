#ifndef ECTOR_GL_PRIVATE_H_
# define ECTOR_GL_PRIVATE_H_

#define EVAS_GL_NO_HEADERS

#include "ector_private.h"

#define SHADER_FLAG_SAM_BITSHIFT 3
#define SHADER_FLAG_MASKSAM_BITSHIFT 6

typedef enum {
  SHADER_FLAG_TEX               = (1 << 0),
  SHADER_FLAG_BGRA              = (1 << 1),
  SHADER_FLAG_MASK              = (1 << 2),
  SHADER_FLAG_SAM12             = (1 << (SHADER_FLAG_SAM_BITSHIFT + 0)),
  SHADER_FLAG_SAM21             = (1 << (SHADER_FLAG_SAM_BITSHIFT + 1)),
  SHADER_FLAG_SAM22             = (1 << (SHADER_FLAG_SAM_BITSHIFT + 2)),
  SHADER_FLAG_MASKSAM12         = (1 << (SHADER_FLAG_MASKSAM_BITSHIFT + 0)),
  SHADER_FLAG_MASKSAM21         = (1 << (SHADER_FLAG_MASKSAM_BITSHIFT + 1)),
  SHADER_FLAG_MASKSAM22         = (1 << (SHADER_FLAG_MASKSAM_BITSHIFT + 2)),
  SHADER_FLAG_IMG               = (1 << 9),
  SHADER_FLAG_BIGENDIAN         = (1 << 10),
  SHADER_FLAG_YUV               = (1 << 11),
  SHADER_FLAG_YUY2              = (1 << 12),
  SHADER_FLAG_NV12              = (1 << 13),
  SHADER_FLAG_YUV_709           = (1 << 14),
  SHADER_FLAG_EXTERNAL          = (1 << 15),
  SHADER_FLAG_AFILL             = (1 << 16),
  SHADER_FLAG_NOMUL             = (1 << 17),
  SHADER_FLAG_ALPHA             = (1 << 18),
  SHADER_FLAG_RGB_A_PAIR        = (1 << 19),
} Shader_Flag;
#define SHADER_FLAG_COUNT 20

#define SHAD_VERTEX 0
#define SHAD_COLOR  1
#define SHAD_TEXUV  2
#define SHAD_TEXUV2 3
#define SHAD_TEXUV3 4
#define SHAD_TEXA   5
#define SHAD_TEXSAM 6
#define SHAD_MASK   7
#define SHAD_MASKSAM 8

static inline void
gl_compile_link_error(GLuint target, const char *action, Eina_Bool is_shader)
{
   int loglen = 0, chars = 0;
   char *logtxt;

   if (is_shader)
     /* Shader info log */
     GL.glGetShaderiv(target, GL_INFO_LOG_LENGTH, &loglen);
   else
     /* Program info log */
     GL.glGetProgramiv(target, GL_INFO_LOG_LENGTH, &loglen);

   if (loglen > 0)
     {
        logtxt = calloc(loglen, sizeof(char));
        if (logtxt)
          {
             if (is_shader) GL.glGetShaderInfoLog(target, loglen, &chars, logtxt);
             else GL.glGetProgramInfoLog(target, loglen, &chars, logtxt);
             ERR("Failed to %s: %s", action, logtxt);
             free(logtxt);
          }
     }
}

GLuint ector_gl_shader_compile(uint64_t flags);

#endif
