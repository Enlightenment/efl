#include "evas_gl_private.h"

/////////////////////////////////////////////
const char rect_frag_glsl[] =
#include "shader/rect_frag.h"
  ;
Evas_GL_Program_Source shader_rect_frag_src =
{
   rect_frag_glsl,
   NULL, 0
};

const char rect_vert_glsl[] =
#include "shader/rect_vert.h"
  ;
Evas_GL_Program_Source shader_rect_vert_src =
{
   rect_vert_glsl,
   NULL, 0
};

/////////////////////////////////////////////
const char font_frag_glsl[] =
#include "shader/font_frag.h"
  ;
Evas_GL_Program_Source shader_font_frag_src =
{
   font_frag_glsl,
   NULL, 0
};

const char font_vert_glsl[] =
#include "shader/font_vert.h"
  ;
Evas_GL_Program_Source shader_font_vert_src =
{
   font_vert_glsl,
   NULL, 0
};

/////////////////////////////////////////////
const char img_frag_glsl[] =
#include "shader/img_frag.h"
  ;
Evas_GL_Program_Source shader_img_frag_src =
{
   img_frag_glsl,
   NULL, 0
};

const char img_vert_glsl[] =
#include "shader/img_vert.h"
  ;
Evas_GL_Program_Source shader_img_vert_src =
{
   img_vert_glsl,
   NULL, 0
};

/////////////////////////////////////////////
const char img_nomul_frag_glsl[] =
#include "shader/img_nomul_frag.h"
  ;
Evas_GL_Program_Source shader_img_nomul_frag_src =
{
   img_nomul_frag_glsl,
   NULL, 0
};

const char img_nomul_vert_glsl[] =
#include "shader/img_nomul_vert.h"
  ;
Evas_GL_Program_Source shader_img_nomul_vert_src =
{
   img_nomul_vert_glsl,
   NULL, 0
};

/////////////////////////////////////////////
const char img_bgra_frag_glsl[] =
#include "shader/img_bgra_frag.h"
  ;
Evas_GL_Program_Source shader_img_bgra_frag_src =
{
   img_bgra_frag_glsl,
   NULL, 0
};

const char img_bgra_vert_glsl[] =
#include "shader/img_bgra_vert.h"
  ;
Evas_GL_Program_Source shader_img_bgra_vert_src =
{
   img_bgra_vert_glsl,
   NULL, 0
};

/////////////////////////////////////////////
const char img_bgra_nomul_frag_glsl[] =
#include "shader/img_bgra_nomul_frag.h"
  ;
Evas_GL_Program_Source shader_img_bgra_nomul_frag_src =
{
   img_bgra_nomul_frag_glsl,
   NULL, 0
};

const char img_bgra_nomul_vert_glsl[] =
#include "shader/img_bgra_nomul_vert.h"
  ;
Evas_GL_Program_Source shader_img_bgra_nomul_vert_src =
{
   img_bgra_nomul_vert_glsl,
   NULL, 0
};

/////////////////////////////////////////////
const char tex_frag_glsl[] =
#include "shader/tex_frag.h"
  ;
Evas_GL_Program_Source shader_tex_frag_src =
{
   tex_frag_glsl,
   NULL, 0
};

const char tex_vert_glsl[] =
#include "shader/tex_vert.h"
  ;
Evas_GL_Program_Source shader_tex_vert_src =
{
   tex_vert_glsl,
   NULL, 0
};

/////////////////////////////////////////////
const char tex_afill_frag_glsl[] =
#include "shader/tex_afill_frag.h"
  ;
Evas_GL_Program_Source shader_tex_afill_frag_src =
{
   tex_afill_frag_glsl,
   NULL, 0
};

const char tex_afill_vert_glsl[] =
#include "shader/tex_afill_vert.h"
  ;
Evas_GL_Program_Source shader_tex_afill_vert_src =
{
   tex_afill_vert_glsl,
   NULL, 0
};

/////////////////////////////////////////////
const char tex_nomul_frag_glsl[] =
#include "shader/tex_nomul_frag.h"
  ;
Evas_GL_Program_Source shader_tex_nomul_frag_src =
{
   tex_nomul_frag_glsl,
   NULL, 0
};

const char tex_nomul_vert_glsl[] =
#include "shader/tex_nomul_vert.h"
  ;
Evas_GL_Program_Source shader_tex_nomul_vert_src =
{
   tex_nomul_vert_glsl,
   NULL, 0
};

/////////////////////////////////////////////
const char tex_nomul_afill_frag_glsl[] =
#include "shader/tex_nomul_afill_frag.h"
  ;
Evas_GL_Program_Source shader_tex_nomul_afill_frag_src =
{
   tex_nomul_afill_frag_glsl,
   NULL, 0
};

const char tex_nomul_afill_vert_glsl[] =
#include "shader/tex_nomul_afill_vert.h"
  ;
Evas_GL_Program_Source shader_tex_nomul_afill_vert_src =
{
   tex_nomul_afill_vert_glsl,
   NULL, 0
};




/////////////////////////////////////////////
const char img_21_frag_glsl[] =
#include "shader/img_21_frag.h"
  ;
Evas_GL_Program_Source shader_img_21_frag_src =
{
   img_21_frag_glsl,
   NULL, 0
};

const char img_21_vert_glsl[] =
#include "shader/img_21_vert.h"
  ;
Evas_GL_Program_Source shader_img_21_vert_src =
{
   img_21_vert_glsl,
   NULL, 0
};

/////////////////////////////////////////////
const char img_21_nomul_frag_glsl[] =
#include "shader/img_21_nomul_frag.h"
  ;
Evas_GL_Program_Source shader_img_21_nomul_frag_src =
{
   img_21_nomul_frag_glsl,
   NULL, 0
};

const char img_21_nomul_vert_glsl[] =
#include "shader/img_21_nomul_vert.h"
  ;
Evas_GL_Program_Source shader_img_21_nomul_vert_src =
{
   img_21_nomul_vert_glsl,
   NULL, 0
};

/////////////////////////////////////////////
const char img_21_bgra_frag_glsl[] =
#include "shader/img_21_bgra_frag.h"
  ;
Evas_GL_Program_Source shader_img_21_bgra_frag_src =
{
   img_21_bgra_frag_glsl,
   NULL, 0
};

const char img_21_bgra_vert_glsl[] =
#include "shader/img_21_bgra_vert.h"
  ;
Evas_GL_Program_Source shader_img_21_bgra_vert_src =
{
   img_21_bgra_vert_glsl,
   NULL, 0
};

/////////////////////////////////////////////
const char img_21_bgra_nomul_frag_glsl[] =
#include "shader/img_21_bgra_nomul_frag.h"
  ;
Evas_GL_Program_Source shader_img_21_bgra_nomul_frag_src =
{
   img_21_bgra_nomul_frag_glsl,
   NULL, 0
};

const char img_21_bgra_nomul_vert_glsl[] =
#include "shader/img_21_bgra_nomul_vert.h"
  ;
Evas_GL_Program_Source shader_img_21_bgra_nomul_vert_src =
{
   img_21_bgra_nomul_vert_glsl,
   NULL, 0
};

/////////////////////////////////////////////
const char tex_21_frag_glsl[] =
#include "shader/tex_21_frag.h"
  ;
Evas_GL_Program_Source shader_tex_21_frag_src =
{
   tex_21_frag_glsl,
   NULL, 0
};

const char tex_21_vert_glsl[] =
#include "shader/tex_21_vert.h"
  ;
Evas_GL_Program_Source shader_tex_21_vert_src =
{
   tex_21_vert_glsl,
   NULL, 0
};

/////////////////////////////////////////////
const char tex_21_afill_frag_glsl[] =
#include "shader/tex_21_afill_frag.h"
  ;
Evas_GL_Program_Source shader_tex_21_afill_frag_src =
{
   tex_21_afill_frag_glsl,
   NULL, 0
};

const char tex_21_afill_vert_glsl[] =
#include "shader/tex_21_afill_vert.h"
  ;
Evas_GL_Program_Source shader_tex_21_afill_vert_src =
{
   tex_21_afill_vert_glsl,
   NULL, 0
};

/////////////////////////////////////////////
const char tex_21_nomul_frag_glsl[] =
#include "shader/tex_21_nomul_frag.h"
  ;
Evas_GL_Program_Source shader_tex_21_nomul_frag_src =
{
   tex_21_nomul_frag_glsl,
   NULL, 0
};

const char tex_21_nomul_vert_glsl[] =
#include "shader/tex_21_nomul_vert.h"
  ;
Evas_GL_Program_Source shader_tex_21_nomul_vert_src =
{
   tex_21_nomul_vert_glsl,
   NULL, 0
};

/////////////////////////////////////////////
const char tex_21_nomul_afill_frag_glsl[] =
#include "shader/tex_21_nomul_afill_frag.h"
  ;
Evas_GL_Program_Source shader_tex_21_nomul_afill_frag_src =
{
   tex_21_nomul_afill_frag_glsl,
   NULL, 0
};

const char tex_21_nomul_afill_vert_glsl[] =
#include "shader/tex_21_nomul_afill_vert.h"
  ;
Evas_GL_Program_Source shader_tex_21_nomul_afill_vert_src =
{
   tex_21_nomul_afill_vert_glsl,
   NULL, 0
};




/////////////////////////////////////////////
const char img_12_frag_glsl[] =
#include "shader/img_12_frag.h"
  ;
Evas_GL_Program_Source shader_img_12_frag_src =
{
   img_12_frag_glsl,
   NULL, 0
};

const char img_12_vert_glsl[] =
#include "shader/img_12_vert.h"
  ;
Evas_GL_Program_Source shader_img_12_vert_src =
{
   img_12_vert_glsl,
   NULL, 0
};

/////////////////////////////////////////////
const char img_12_nomul_frag_glsl[] =
#include "shader/img_12_nomul_frag.h"
  ;
Evas_GL_Program_Source shader_img_12_nomul_frag_src =
{
   img_12_nomul_frag_glsl,
   NULL, 0
};

const char img_12_nomul_vert_glsl[] =
#include "shader/img_12_nomul_vert.h"
  ;
Evas_GL_Program_Source shader_img_12_nomul_vert_src =
{
   img_12_nomul_vert_glsl,
   NULL, 0
};

/////////////////////////////////////////////
const char img_12_bgra_frag_glsl[] =
#include "shader/img_12_bgra_frag.h"
  ;
Evas_GL_Program_Source shader_img_12_bgra_frag_src =
{
   img_12_bgra_frag_glsl,
   NULL, 0
};

const char img_12_bgra_vert_glsl[] =
#include "shader/img_12_bgra_vert.h"
  ;
Evas_GL_Program_Source shader_img_12_bgra_vert_src =
{
   img_12_bgra_vert_glsl,
   NULL, 0
};

/////////////////////////////////////////////
const char img_12_bgra_nomul_frag_glsl[] =
#include "shader/img_12_bgra_nomul_frag.h"
  ;
Evas_GL_Program_Source shader_img_12_bgra_nomul_frag_src =
{
   img_12_bgra_nomul_frag_glsl,
   NULL, 0
};

const char img_12_bgra_nomul_vert_glsl[] =
#include "shader/img_12_bgra_nomul_vert.h"
  ;
Evas_GL_Program_Source shader_img_12_bgra_nomul_vert_src =
{
   img_12_bgra_nomul_vert_glsl,
   NULL, 0
};

/////////////////////////////////////////////
const char tex_12_frag_glsl[] =
#include "shader/tex_12_frag.h"
  ;
Evas_GL_Program_Source shader_tex_12_frag_src =
{
   tex_12_frag_glsl,
   NULL, 0
};

const char tex_12_vert_glsl[] =
#include "shader/tex_12_vert.h"
  ;
Evas_GL_Program_Source shader_tex_12_vert_src =
{
   tex_12_vert_glsl,
   NULL, 0
};

/////////////////////////////////////////////
const char tex_12_afill_frag_glsl[] =
#include "shader/tex_12_afill_frag.h"
  ;
Evas_GL_Program_Source shader_tex_12_afill_frag_src =
{
   tex_12_afill_frag_glsl,
   NULL, 0
};

const char tex_12_afill_vert_glsl[] =
#include "shader/tex_12_afill_vert.h"
  ;
Evas_GL_Program_Source shader_tex_12_afill_vert_src =
{
   tex_12_afill_vert_glsl,
   NULL, 0
};

/////////////////////////////////////////////
const char tex_12_nomul_frag_glsl[] =
#include "shader/tex_12_nomul_frag.h"
  ;
Evas_GL_Program_Source shader_tex_12_nomul_frag_src =
{
   tex_12_nomul_frag_glsl,
   NULL, 0
};

const char tex_12_nomul_vert_glsl[] =
#include "shader/tex_12_nomul_vert.h"
  ;
Evas_GL_Program_Source shader_tex_12_nomul_vert_src =
{
   tex_12_nomul_vert_glsl,
   NULL, 0
};

/////////////////////////////////////////////
const char tex_12_nomul_afill_frag_glsl[] =
#include "shader/tex_12_nomul_afill_frag.h"
  ;
Evas_GL_Program_Source shader_tex_12_nomul_afill_frag_src =
{
   tex_12_nomul_afill_frag_glsl,
   NULL, 0
};

const char tex_12_nomul_afill_vert_glsl[] =
#include "shader/tex_12_nomul_afill_vert.h"
  ;
Evas_GL_Program_Source shader_tex_12_nomul_afill_vert_src =
{
   tex_12_nomul_afill_vert_glsl,
   NULL, 0
};




/////////////////////////////////////////////
const char img_22_frag_glsl[] =
#include "shader/img_22_frag.h"
  ;
Evas_GL_Program_Source shader_img_22_frag_src =
{
   img_22_frag_glsl,
   NULL, 0
};

const char img_22_vert_glsl[] =
#include "shader/img_22_vert.h"
  ;
Evas_GL_Program_Source shader_img_22_vert_src =
{
   img_22_vert_glsl,
   NULL, 0
};

/////////////////////////////////////////////
const char img_22_nomul_frag_glsl[] =
#include "shader/img_22_nomul_frag.h"
  ;
Evas_GL_Program_Source shader_img_22_nomul_frag_src =
{
   img_22_nomul_frag_glsl,
   NULL, 0
};

const char img_22_nomul_vert_glsl[] =
#include "shader/img_22_nomul_vert.h"
  ;
Evas_GL_Program_Source shader_img_22_nomul_vert_src =
{
   img_22_nomul_vert_glsl,
   NULL, 0
};

/////////////////////////////////////////////
const char img_22_bgra_frag_glsl[] =
#include "shader/img_22_bgra_frag.h"
  ;
Evas_GL_Program_Source shader_img_22_bgra_frag_src =
{
   img_22_bgra_frag_glsl,
   NULL, 0
};

const char img_22_bgra_vert_glsl[] =
#include "shader/img_22_bgra_vert.h"
  ;
Evas_GL_Program_Source shader_img_22_bgra_vert_src =
{
   img_22_bgra_vert_glsl,
   NULL, 0
};

/////////////////////////////////////////////
const char img_22_bgra_nomul_frag_glsl[] =
#include "shader/img_22_bgra_nomul_frag.h"
  ;
Evas_GL_Program_Source shader_img_22_bgra_nomul_frag_src =
{
   img_22_bgra_nomul_frag_glsl,
   NULL, 0
};

const char img_22_bgra_nomul_vert_glsl[] =
#include "shader/img_22_bgra_nomul_vert.h"
  ;
Evas_GL_Program_Source shader_img_22_bgra_nomul_vert_src =
{
   img_22_bgra_nomul_vert_glsl,
   NULL, 0
};

/////////////////////////////////////////////
const char tex_22_frag_glsl[] =
#include "shader/tex_22_frag.h"
  ;
Evas_GL_Program_Source shader_tex_22_frag_src =
{
   tex_22_frag_glsl,
   NULL, 0
};

const char tex_22_vert_glsl[] =
#include "shader/tex_22_vert.h"
  ;
Evas_GL_Program_Source shader_tex_22_vert_src =
{
   tex_22_vert_glsl,
   NULL, 0
};

/////////////////////////////////////////////
const char tex_22_afill_frag_glsl[] =
#include "shader/tex_22_afill_frag.h"
  ;
Evas_GL_Program_Source shader_tex_22_afill_frag_src =
{
   tex_22_afill_frag_glsl,
   NULL, 0
};

const char tex_22_afill_vert_glsl[] =
#include "shader/tex_22_afill_vert.h"
  ;
Evas_GL_Program_Source shader_tex_22_afill_vert_src =
{
   tex_22_afill_vert_glsl,
   NULL, 0
};

/////////////////////////////////////////////
const char tex_22_nomul_frag_glsl[] =
#include "shader/tex_22_nomul_frag.h"
  ;
Evas_GL_Program_Source shader_tex_22_nomul_frag_src =
{
   tex_22_nomul_frag_glsl,
   NULL, 0
};

const char tex_22_nomul_vert_glsl[] =
#include "shader/tex_22_nomul_vert.h"
  ;
Evas_GL_Program_Source shader_tex_22_nomul_vert_src =
{
   tex_22_nomul_vert_glsl,
   NULL, 0
};

/////////////////////////////////////////////
const char tex_22_nomul_afill_frag_glsl[] =
#include "shader/tex_22_nomul_afill_frag.h"
  ;
Evas_GL_Program_Source shader_tex_22_nomul_afill_frag_src =
{
   tex_22_nomul_afill_frag_glsl,
   NULL, 0
};

const char tex_22_nomul_afill_vert_glsl[] =
#include "shader/tex_22_nomul_afill_vert.h"
  ;
Evas_GL_Program_Source shader_tex_22_nomul_afill_vert_src =
{
   tex_22_nomul_afill_vert_glsl,
   NULL, 0
};




/////////////////////////////////////////////
const char yuv_frag_glsl[] =
#include "shader/yuv_frag.h"
  ;
Evas_GL_Program_Source shader_yuv_frag_src =
{
   yuv_frag_glsl,
   NULL, 0
};

const char yuv_vert_glsl[] =
#include "shader/yuv_vert.h"
  ;
Evas_GL_Program_Source shader_yuv_vert_src =
{
   yuv_vert_glsl,
   NULL, 0
};

/////////////////////////////////////////////
const char yuy2_frag_glsl[] =
#include "shader/yuy2_frag.h"
  ;
Evas_GL_Program_Source shader_yuy2_frag_src =
{
   yuy2_frag_glsl,
   NULL, 0
};

const char yuy2_vert_glsl[] =
#include "shader/yuy2_vert.h"
  ;
Evas_GL_Program_Source shader_yuy2_vert_src =
{
   yuy2_vert_glsl,
   NULL, 0
};

/////////////////////////////////////////////
const char yuy2_nomul_frag_glsl[] =
#include "shader/yuy2_nomul_frag.h"
  ;
Evas_GL_Program_Source shader_yuy2_nomul_frag_src =
{
   yuy2_nomul_frag_glsl,
   NULL, 0
};

const char yuy2_nomul_vert_glsl[] =
#include "shader/yuy2_nomul_vert.h"
  ;
Evas_GL_Program_Source shader_yuy2_nomul_vert_src =
{
   yuy2_nomul_vert_glsl,
   NULL, 0
};

/////////////////////////////////////////////
const char nv12_nomul_vert_glsl[] =
#include "shader/nv12_nomul_vert.h"
  ;
Evas_GL_Program_Source shader_nv12_nomul_vert_src =
{
   nv12_nomul_vert_glsl,
   NULL, 0
};

const char nv12_vert_glsl[] =
#include "shader/nv12_vert.h"
  ;
Evas_GL_Program_Source shader_nv12_vert_src =
{
   nv12_vert_glsl,
   NULL, 0
};

const char nv12_frag_glsl[] =
#include "shader/nv12_frag.h"
  ;
Evas_GL_Program_Source shader_nv12_frag_src =
{
   nv12_frag_glsl,
   NULL, 0
};

const char nv12_nomul_frag_glsl[] =
#include "shader/nv12_nomul_frag.h"
  ;
Evas_GL_Program_Source shader_nv12_nomul_frag_src =
{
   nv12_nomul_frag_glsl,
   NULL, 0
};

/////////////////////////////////////////////
const char yuv_nomul_frag_glsl[] =
#include "shader/yuv_nomul_frag.h"
  ;
Evas_GL_Program_Source shader_yuv_nomul_frag_src =
{
   yuv_nomul_frag_glsl,
   NULL, 0
};

const char yuv_nomul_vert_glsl[] =
#include "shader/yuv_nomul_vert.h"
  ;
Evas_GL_Program_Source shader_yuv_nomul_vert_src =
{
   yuv_nomul_vert_glsl,
   NULL, 0
};

/////////////////////////////////////////////
const char rgb_a_pair_frag_glsl[] =
#include "shader/rgb_a_pair_frag.h"
  ;
Evas_GL_Program_Source shader_rgb_a_pair_frag_src =
{
   rgb_a_pair_frag_glsl,
   NULL, 0
};

const char rgb_a_pair_vert_glsl[] =
#include "shader/rgb_a_pair_vert.h"
  ;
Evas_GL_Program_Source shader_rgb_a_pair_vert_src =
{
   rgb_a_pair_vert_glsl,
   NULL, 0
};

const char rgb_a_pair_nomul_frag_glsl[] =
#include "shader/rgb_a_pair_nomul_frag.h"
  ;
Evas_GL_Program_Source shader_rgb_a_pair_nomul_frag_src =
{
   rgb_a_pair_nomul_frag_glsl,
   NULL, 0
};

const char rgb_a_pair_nomul_vert_glsl[] =
#include "shader/rgb_a_pair_nomul_vert.h"
  ;
Evas_GL_Program_Source shader_rgb_a_pair_nomul_vert_src =
{
   rgb_a_pair_nomul_vert_glsl,
   NULL, 0
};


/////////////////////////////////////////////
static void
gl_compile_link_error(GLuint target, const char *action)
{
   int loglen = 0, chars = 0;
   char *logtxt;

   /* Shader info log */
   glGetShaderiv(target, GL_INFO_LOG_LENGTH, &loglen);
   if (loglen > 0)
     {
        logtxt = calloc(loglen, sizeof(char));
        if (logtxt)
          {
             glGetShaderInfoLog(target, loglen, &chars, logtxt);
             ERR("Failed to %s: %s", action, logtxt);
             free(logtxt);
          }
     }

   /* Program info log */
   glGetProgramiv(target, GL_INFO_LOG_LENGTH, &loglen);
   if (loglen > 0)
     {
        logtxt = calloc(loglen, sizeof(char));
        if (logtxt)
          {
             glGetProgramInfoLog(target, loglen, &chars, logtxt);
             ERR("Failed to %s: %s", action, logtxt);
             free(logtxt);
          }
     }
}

static int
_evas_gl_common_shader_program_binary_init(Evas_GL_Program *p,
                                           const char *pname,
                                           Eet_File *ef)
{
   int res = 0, num = 0, length = 0;
   int *formats = NULL;
   void *data = NULL;
   GLint ok = 0;

   if (!ef) return res;

   data = eet_read(ef, pname, &length);
   if ((!data) || (length <= 0)) goto finish;

   glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &num);
   if (num <= 0) goto finish;

   formats = calloc(num, sizeof(int));
   if (!formats) goto finish;

   glGetIntegerv(GL_PROGRAM_BINARY_FORMATS, formats);
   if (!formats[0]) goto finish;

   p->prog = glCreateProgram();

#if 1
   // TODO: invalid rendering error occurs when attempting to use a
   // glProgramBinary. in order to render correctly we should create a dummy
   // vertex shader.
   p->vert = glCreateShader(GL_VERTEX_SHADER);
   glAttachShader(p->prog, p->vert);
   p->frag = glCreateShader(GL_FRAGMENT_SHADER);
   glAttachShader(p->prog, p->frag);
#endif
   glsym_glProgramBinary(p->prog, formats[0], data, length);

   glBindAttribLocation(p->prog, SHAD_VERTEX, "vertex");
   glBindAttribLocation(p->prog, SHAD_COLOR,  "color");
   glBindAttribLocation(p->prog, SHAD_TEXUV,  "tex_coord");
   glBindAttribLocation(p->prog, SHAD_TEXUV2, "tex_coord2");
   glBindAttribLocation(p->prog, SHAD_TEXUV3, "tex_coord3");
   glBindAttribLocation(p->prog, SHAD_TEXM,   "tex_coordm");
   glBindAttribLocation(p->prog, SHAD_TEXSAM, "tex_sample");

   glGetProgramiv(p->prog, GL_LINK_STATUS, &ok);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   if (!ok)
     {
        gl_compile_link_error(p->prog, "load a program object");
        ERR("Abort load of program (%s)", pname);
        goto finish;
     }

   res = 1;

finish:
   if (formats) free(formats);
   if (data) free(data);
   if ((!res) && (p->prog))
     {
        glDeleteProgram(p->prog);
        p->prog = 0;
     }
   return res;
}

static int
_evas_gl_common_shader_program_binary_save(Evas_GL_Program *p,
                                           const char *pname,
                                           Eet_File *ef)
{
   void* data = NULL;
   GLenum format;
   int length = 0, size = 0;

   if (!glsym_glGetProgramBinary) return 0;

   glGetProgramiv(p->prog, GL_PROGRAM_BINARY_LENGTH, &length);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   if (length <= 0) return 0;

   data = malloc(length);
   if (!data) return 0;

   glsym_glGetProgramBinary(p->prog, length, &size, &format, data);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");

   if (length != size)
     {
        free(data);
        return 0;
     }

   if (eet_write(ef, pname, data, length, 1) < 0)
     {
        if (data) free(data);
        return 0;
     }
   if (data) free(data);
   return 1;
}

static int
_evas_gl_common_shader_program_source_init(Evas_GL_Program *p,
                                           Evas_GL_Program_Source *vert,
                                           Evas_GL_Program_Source *frag,
                                           const char *name)
{
   GLint ok;

   p->vert = glCreateShader(GL_VERTEX_SHADER);
   p->frag = glCreateShader(GL_FRAGMENT_SHADER);

   glShaderSource(p->vert, 1,
                  (const char **)&(vert->src), NULL);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glCompileShader(p->vert);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   ok = 0;
   glGetShaderiv(p->vert, GL_COMPILE_STATUS, &ok);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   if (!ok)
     {
        gl_compile_link_error(p->vert, "compile vertex shader");
        ERR("Abort compile of shader vert (%s): %s", name, vert->src);
        return 0;
     }
   glShaderSource(p->frag, 1,
                  (const char **)&(frag->src), NULL);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glCompileShader(p->frag);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   ok = 0;
   glGetShaderiv(p->frag, GL_COMPILE_STATUS, &ok);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   if (!ok)
     {
        gl_compile_link_error(p->frag, "compile fragment shader");
        ERR("Abort compile of shader frag (%s): %s", name, frag->src);
        return 0;
     }

   p->prog = glCreateProgram();
#ifdef GL_GLES
#else
   if ((glsym_glGetProgramBinary) && (glsym_glProgramParameteri))
      glsym_glProgramParameteri(p->prog, GL_PROGRAM_BINARY_RETRIEVABLE_HINT,
                                GL_TRUE);
#endif
   glAttachShader(p->prog, p->vert);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glAttachShader(p->prog, p->frag);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");

   glBindAttribLocation(p->prog, SHAD_VERTEX, "vertex");
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glBindAttribLocation(p->prog, SHAD_COLOR,  "color");
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glBindAttribLocation(p->prog, SHAD_TEXUV,  "tex_coord");
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glBindAttribLocation(p->prog, SHAD_TEXUV2, "tex_coord2");
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glBindAttribLocation(p->prog, SHAD_TEXUV3, "tex_coord3");
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glBindAttribLocation(p->prog, SHAD_TEXM, "tex_coordm");
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glBindAttribLocation(p->prog, SHAD_TEXSAM, "tex_sample");
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");

   glLinkProgram(p->prog);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   ok = 0;
   glGetProgramiv(p->prog, GL_LINK_STATUS, &ok);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   if (!ok)
     {
        gl_compile_link_error(p->prog, "link fragment and vertex shaders");
        ERR("Abort compile of shader frag (%s): %s", name, frag->src);
        ERR("Abort compile of shader vert (%s): %s", name, vert->src);
        return 0;
     }
   return 1;
}

#define SHADER_SOURCE_LINE(Big, Small) \
  { SHADER_##Big, &(shader_##Small##_vert_src), &(shader_##Small##_frag_src), #Small }

#define SHADER_SOURCE_FILTER_LINE(Big, Small) \
  { SHADER_##Big, &(shader_img_vert_src), &(shader_##Small##_frag_src), #Small }

static const struct {
  Evas_GL_Shader id;
  Evas_GL_Program_Source *vert;
  Evas_GL_Program_Source *frag;
  const char *name;
} _shaders_source[] = {
  SHADER_SOURCE_LINE(RECT, rect),
  SHADER_SOURCE_LINE(FONT, font),
   
  SHADER_SOURCE_LINE(IMG, img),
  SHADER_SOURCE_LINE(IMG_NOMUL, img_nomul),
  SHADER_SOURCE_LINE(IMG_BGRA, img_bgra),
  SHADER_SOURCE_LINE(IMG_BGRA_NOMUL, img_bgra_nomul),
  SHADER_SOURCE_LINE(TEX, tex),
  SHADER_SOURCE_LINE(TEX_NOMUL, tex_nomul),
  SHADER_SOURCE_LINE(TEX_AFILL, tex_afill),
  SHADER_SOURCE_LINE(TEX_NOMUL_AFILL, tex_nomul_afill),
   
  SHADER_SOURCE_LINE(IMG_21, img_21),
  SHADER_SOURCE_LINE(IMG_21_NOMUL, img_21_nomul),
  SHADER_SOURCE_LINE(IMG_21_BGRA, img_21_bgra),
  SHADER_SOURCE_LINE(IMG_21_BGRA_NOMUL, img_21_bgra_nomul),
  SHADER_SOURCE_LINE(TEX_21, tex_21),
  SHADER_SOURCE_LINE(TEX_21_NOMUL, tex_21_nomul),
  SHADER_SOURCE_LINE(TEX_21_AFILL, tex_21_afill),
  SHADER_SOURCE_LINE(TEX_21_NOMUL_AFILL, tex_21_nomul_afill),
   
  SHADER_SOURCE_LINE(IMG_12, img_12),
  SHADER_SOURCE_LINE(IMG_12_NOMUL, img_12_nomul),
  SHADER_SOURCE_LINE(IMG_12_BGRA, img_12_bgra),
  SHADER_SOURCE_LINE(IMG_12_BGRA_NOMUL, img_12_bgra_nomul),
  SHADER_SOURCE_LINE(TEX_12, tex_12),
  SHADER_SOURCE_LINE(TEX_12_NOMUL, tex_12_nomul),
  SHADER_SOURCE_LINE(TEX_12_AFILL, tex_12_afill),
  SHADER_SOURCE_LINE(TEX_12_NOMUL_AFILL, tex_12_nomul_afill),
   
  SHADER_SOURCE_LINE(IMG_22, img_22),
  SHADER_SOURCE_LINE(IMG_22_NOMUL, img_22_nomul),
  SHADER_SOURCE_LINE(IMG_22_BGRA, img_22_bgra),
  SHADER_SOURCE_LINE(IMG_22_BGRA_NOMUL, img_22_bgra_nomul),
  SHADER_SOURCE_LINE(TEX_22, tex_22),
  SHADER_SOURCE_LINE(TEX_22_NOMUL, tex_22_nomul),
  SHADER_SOURCE_LINE(TEX_22_AFILL, tex_22_afill),
  SHADER_SOURCE_LINE(TEX_22_NOMUL_AFILL, tex_22_nomul_afill),
   
  SHADER_SOURCE_LINE(YUV, yuv),
  SHADER_SOURCE_LINE(YUV_NOMUL, yuv_nomul),
  SHADER_SOURCE_LINE(YUY2, yuy2),
  SHADER_SOURCE_LINE(YUY2_NOMUL, yuy2_nomul),
  SHADER_SOURCE_LINE(NV12, nv12),
  SHADER_SOURCE_LINE(NV12_NOMUL, nv12_nomul),

  SHADER_SOURCE_LINE(RGB_A_PAIR, rgb_a_pair),
  SHADER_SOURCE_LINE(RGB_A_PAIR_NOMUL, rgb_a_pair_nomul),
};

static int
_evas_gl_common_shader_source_init(Evas_GL_Shared *shared)
{
  unsigned int i;

  for (i = 0; i < sizeof (_shaders_source) / sizeof (_shaders_source[0]); i++)
     {
        if (!_evas_gl_common_shader_program_source_init
            (&(shared->shader[_shaders_source[i].id]),
                _shaders_source[i].vert,
                _shaders_source[i].frag,
                _shaders_source[i].name))
          return 0;
     }
   return 1;
}

static int
_evas_gl_common_shader_binary_init(Evas_GL_Shared *shared)
{
   /* check eet */
   Eet_File *et = NULL;
   char bin_dir_path[PATH_MAX];
   char bin_file_path[PATH_MAX];
   unsigned int i;

   if (!evas_gl_common_file_cache_dir_check(bin_dir_path, sizeof(bin_dir_path)))
      return 0;

   if (!evas_gl_common_file_cache_file_check(bin_dir_path, "binary_shader", bin_file_path,
                                   sizeof(bin_dir_path)))
      return 0;

   /* use eet */
   if (!eet_init()) return 0;
   et = eet_open(bin_file_path, EET_FILE_MODE_READ);
   if (!et) goto error;

   for (i = 0; i < sizeof (_shaders_source) / sizeof (_shaders_source[0]); ++i)
     if (!_evas_gl_common_shader_program_binary_init(&(shared->shader[_shaders_source[i].id]),
                                                     _shaders_source[i].name,
                                                     et))
       goto error;

   if (et) eet_close(et);
   eet_shutdown();
   return 1;

error:
   if (et) eet_close(et);
   eet_shutdown();
   return 0;
}

static int
_evas_gl_common_shader_binary_save(Evas_GL_Shared *shared)
{
   /* check eet */
   Eet_File *et = NULL; //check eet file
   int tmpfd;
   int res = 0;
   char bin_dir_path[PATH_MAX];
   char bin_file_path[PATH_MAX];
   char tmp_file[PATH_MAX];
   unsigned int i;

   if (!evas_gl_common_file_cache_dir_check(bin_dir_path, sizeof(bin_dir_path)))
     {
        res = evas_gl_common_file_cache_mkpath(bin_dir_path);
        if (!res) return 0; /* we can't make directory */
     }

   evas_gl_common_file_cache_file_check(bin_dir_path, "binary_shader", bin_file_path,
                              sizeof(bin_dir_path));

   /* use mkstemp for writing */
   snprintf(tmp_file, sizeof(tmp_file), "%s.XXXXXX", bin_file_path);
   tmpfd = mkstemp(tmp_file);
   if (tmpfd < 0) goto error;
   close(tmpfd);

   /* use eet */
   if (!eet_init()) goto error;

   et = eet_open(tmp_file, EET_FILE_MODE_WRITE);
   if (!et) goto error;

   for (i = 0; i < sizeof (_shaders_source) / sizeof (_shaders_source[0]); ++i)
     if (!_evas_gl_common_shader_program_binary_save(&(shared->shader[_shaders_source[i].id]),
                                                     _shaders_source[i].name,
                                                     et))
       goto error;

   if (eet_close(et) != EET_ERROR_NONE) goto error;
   if (rename(tmp_file,bin_file_path) < 0) goto error;
   eet_shutdown();
   return 1;

error:
   if (et) eet_close(et);
   if (evas_gl_common_file_cache_file_exists(tmp_file)) unlink(tmp_file);
   eet_shutdown();
   return 0;
}

int
evas_gl_common_shader_program_init(Evas_GL_Shared *shared)
{
   // gl support binary shader and get env of binary shader path
   if (shared->info.bin_program &&
       _evas_gl_common_shader_binary_init(shared)) return 1;
   /* compile all shader.*/
   if (!_evas_gl_common_shader_source_init(shared)) return 0;
   /* success compile all shader. if gl support binary shader, we need to save */
   if (shared->info.bin_program) _evas_gl_common_shader_binary_save(shared);
   return 1;
}

void
evas_gl_common_shader_program_init_done(void)
{
#ifdef GL_GLES
   glReleaseShaderCompiler();
#else
   if (glsym_glReleaseShaderCompiler) glsym_glReleaseShaderCompiler();
#endif
}

void
evas_gl_common_shader_program_shutdown(Evas_GL_Program *p)
{
   if (p->vert) glDeleteShader(p->vert);
   if (p->frag) glDeleteShader(p->frag);
   if (p->prog) glDeleteProgram(p->prog);
}
