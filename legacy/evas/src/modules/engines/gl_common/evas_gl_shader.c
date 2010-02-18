#include "evas_gl_private.h"

/////////////////////////////////////////////
#if defined (GLES_VARIETY_S3C6410)
const unsigned int rect_frag_bin[] =
{
# include "shader/rect_frag_bin_s3c6410.h"
};
#endif

const char rect_frag_glsl[] =
#include "shader/rect_frag.h"
  ;
Evas_GL_Program_Source shader_rect_frag_src =
{
   rect_frag_glsl,
#if defined (GLES_VARIETY_S3C6410)
     rect_frag_bin, sizeof(rect_frag_bin)
#else     
     NULL, 0
#endif     
};

#if defined (GLES_VARIETY_S3C6410)
const unsigned int rect_vert_bin[] =
{
# include "shader/rect_vert_bin_s3c6410.h"
};
#endif
const char rect_vert_glsl[] =
#include "shader/rect_vert.h"
  ;
Evas_GL_Program_Source shader_rect_vert_src =
{
   rect_vert_glsl,
#if defined (GLES_VARIETY_S3C6410)
     rect_vert_bin, sizeof(rect_vert_bin)
#else     
     NULL, 0
#endif     
};

/////////////////////////////////////////////
#if defined (GLES_VARIETY_S3C6410)
const unsigned int font_frag_bin[] =
{
# include "shader/font_frag_bin_s3c6410.h"
};
#endif

const char font_frag_glsl[] =
#include "shader/font_frag.h"
  ;
Evas_GL_Program_Source shader_font_frag_src =
{
   font_frag_glsl,
#if defined (GLES_VARIETY_S3C6410)
     font_frag_bin, sizeof(font_frag_bin)
#else     
     NULL, 0
#endif     
};

#if defined (GLES_VARIETY_S3C6410)
const unsigned int font_vert_bin[] =
{
# include "shader/font_vert_bin_s3c6410.h"
};
#endif
const char font_vert_glsl[] =
#include "shader/font_vert.h"
  ;
Evas_GL_Program_Source shader_font_vert_src =
{
   font_vert_glsl,
#if defined (GLES_VARIETY_S3C6410)
     font_vert_bin, sizeof(font_vert_bin)
#else     
     NULL, 0
#endif     
};

/////////////////////////////////////////////
#if defined (GLES_VARIETY_S3C6410)
const unsigned int yuv_frag_bin[] =
{
# include "shader/yuv_frag_bin_s3c6410.h"
};
#endif

const char yuv_frag_glsl[] =
#include "shader/yuv_frag.h"
  ;
Evas_GL_Program_Source shader_yuv_frag_src =
{
   yuv_frag_glsl,
#if defined (GLES_VARIETY_S3C6410)
     yuv_frag_bin, sizeof(yuv_frag_bin)
#else     
     NULL, 0
#endif     
};

#if defined (GLES_VARIETY_S3C6410)
const unsigned int yuv_vert_bin[] =
{
# include "shader/yuv_vert_bin_s3c6410.h"
};
#endif
const char yuv_vert_glsl[] =
#include "shader/yuv_vert.h"
  ;
Evas_GL_Program_Source shader_yuv_vert_src =
{
   yuv_vert_glsl,
#if defined (GLES_VARIETY_S3C6410)
     yuv_vert_bin, sizeof(yuv_vert_bin)
#else     
     NULL, 0
#endif     
};

/////////////////////////////////////////////
#if defined (GLES_VARIETY_S3C6410)
const unsigned int yuv_nomul_frag_bin[] =
{
# include "shader/yuv_nomul_frag_bin_s3c6410.h"
};
#endif

const char yuv_nomul_frag_glsl[] =
#include "shader/yuv_nomul_frag.h"
  ;
Evas_GL_Program_Source shader_yuv_nomul_frag_src =
{
   yuv_nomul_frag_glsl,
#if defined (GLES_VARIETY_S3C6410)
     yuv_nomul_frag_bin, sizeof(yuv_nomul_frag_bin)
#else     
     NULL, 0
#endif     
};

#if defined (GLES_VARIETY_S3C6410)
const unsigned int yuv_nomul_vert_bin[] =
{
# include "shader/yuv_nomul_vert_bin_s3c6410.h"
};
#endif
const char yuv_nomul_vert_glsl[] =
#include "shader/yuv_nomul_vert.h"
  ;
Evas_GL_Program_Source shader_yuv_nomul_vert_src =
{
   yuv_nomul_vert_glsl,
#if defined (GLES_VARIETY_S3C6410)
     yuv_nomul_vert_bin, sizeof(yuv_nomul_vert_bin)
#else     
     NULL, 0
#endif     
};

/////////////////////////////////////////////
#if defined (GLES_VARIETY_S3C6410)
const unsigned int tex_frag_bin[] =
{
# include "shader/tex_frag_bin_s3c6410.h"
};
#endif

const char tex_frag_glsl[] =
#include "shader/tex_frag.h"
  ;
Evas_GL_Program_Source shader_tex_frag_src =
{
   tex_frag_glsl,
#if defined (GLES_VARIETY_S3C6410)
     tex_frag_bin, sizeof(tex_frag_bin)
#else     
     NULL, 0
#endif     
};

#if defined (GLES_VARIETY_S3C6410)
const unsigned int tex_vert_bin[] =
{
# include "shader/tex_vert_bin_s3c6410.h"
};
#endif
const char tex_vert_glsl[] =
#include "shader/tex_vert.h"
  ;
Evas_GL_Program_Source shader_tex_vert_src =
{
   tex_vert_glsl,
#if defined (GLES_VARIETY_S3C6410)
     tex_vert_bin, sizeof(tex_vert_bin)
#else     
     NULL, 0
#endif     
};

/////////////////////////////////////////////
#if defined (GLES_VARIETY_S3C6410)
const unsigned int tex_nomul_frag_bin[] =
{
# include "shader/tex_nomul_frag_bin_s3c6410.h"
};
#endif

const char tex_nomul_frag_glsl[] =
#include "shader/tex_nomul_frag.h"
  ;
Evas_GL_Program_Source shader_tex_nomul_frag_src =
{
   tex_nomul_frag_glsl,
#if defined (GLES_VARIETY_S3C6410)
     tex_nomul_frag_bin, sizeof(tex_nomul_frag_bin)
#else     
     NULL, 0
#endif     
};

#if defined (GLES_VARIETY_S3C6410)
const unsigned int tex_nomul_vert_bin[] =
{
# include "shader/tex_nomul_vert_bin_s3c6410.h"
};
#endif
const char tex_nomul_vert_glsl[] =
#include "shader/tex_nomul_vert.h"
  ;
Evas_GL_Program_Source shader_tex_nomul_vert_src =
{
   tex_nomul_vert_glsl,
#if defined (GLES_VARIETY_S3C6410)
     tex_nomul_vert_bin, sizeof(tex_nomul_vert_bin)
#else     
     NULL, 0
#endif     
};

/////////////////////////////////////////////
#if defined (GLES_VARIETY_S3C6410)
const unsigned int img_frag_bin[] =
{
# include "shader/img_frag_bin_s3c6410.h"
};
#endif

const char img_frag_glsl[] =
#include "shader/img_frag.h"
  ;
Evas_GL_Program_Source shader_img_frag_src =
{
   img_frag_glsl,
#if defined (GLES_VARIETY_S3C6410)
     img_frag_bin, sizeof(img_frag_bin)
#else     
     NULL, 0
#endif     
};

#if defined (GLES_VARIETY_S3C6410)
const unsigned int img_vert_bin[] =
{
# include "shader/img_vert_bin_s3c6410.h"
};
#endif
const char img_vert_glsl[] =
#include "shader/img_vert.h"
  ;
Evas_GL_Program_Source shader_img_vert_src =
{
   img_vert_glsl,
#if defined (GLES_VARIETY_S3C6410)
     img_vert_bin, sizeof(img_vert_bin)
#else     
     NULL, 0
#endif     
};

/////////////////////////////////////////////
#if defined (GLES_VARIETY_S3C6410)
const unsigned int img_nomul_frag_bin[] =
{
# include "shader/img_nomul_frag_bin_s3c6410.h"
};
#endif

const char img_nomul_frag_glsl[] =
#include "shader/img_nomul_frag.h"
  ;
Evas_GL_Program_Source shader_img_nomul_frag_src =
{
   img_nomul_frag_glsl,
#if defined (GLES_VARIETY_S3C6410)
     img_nomul_frag_bin, sizeof(img_nomul_frag_bin)
#else     
     NULL, 0
#endif     
};

#if defined (GLES_VARIETY_S3C6410)
const unsigned int img_nomul_vert_bin[] =
{
# include "shader/img_nomul_vert_bin_s3c6410.h"
};
#endif
const char img_nomul_vert_glsl[] =
#include "shader/img_nomul_vert.h"
  ;
Evas_GL_Program_Source shader_img_nomul_vert_src =
{
   img_nomul_vert_glsl,
#if defined (GLES_VARIETY_S3C6410)
     img_nomul_vert_bin, sizeof(img_nomul_vert_bin)
#else     
     NULL, 0
#endif     
};

/////////////////////////////////////////////
#if defined (GLES_VARIETY_S3C6410)
const unsigned int img_bgra_frag_bin[] =
{
# include "shader/img_bgra_frag_bin_s3c6410.h"
};
#endif

const char img_bgra_frag_glsl[] =
#include "shader/img_bgra_frag.h"
  ;
Evas_GL_Program_Source shader_img_bgra_frag_src =
{
   img_bgra_frag_glsl,
#if defined (GLES_VARIETY_S3C6410)
     img_bgra_frag_bin, sizeof(img_bgra_frag_bin)
#else     
     NULL, 0
#endif     
};

#if defined (GLES_VARIETY_S3C6410)
const unsigned int img_bgra_vert_bin[] =
{
# include "shader/img_bgra_vert_bin_s3c6410.h"
};
#endif
const char img_bgra_vert_glsl[] =
#include "shader/img_bgra_vert.h"
  ;
Evas_GL_Program_Source shader_img_bgra_vert_src =
{
   img_bgra_vert_glsl,
#if defined (GLES_VARIETY_S3C6410)
     img_bgra_vert_bin, sizeof(img_bgra_vert_bin)
#else     
     NULL, 0
#endif     
};

/////////////////////////////////////////////
#if defined (GLES_VARIETY_S3C6410)
const unsigned int img_bgra_nomul_frag_bin[] =
{
# include "shader/img_bgra_nomul_frag_bin_s3c6410.h"
};
#endif

const char img_bgra_nomul_frag_glsl[] =
#include "shader/img_bgra_nomul_frag.h"
  ;
Evas_GL_Program_Source shader_img_bgra_nomul_frag_src =
{
   img_bgra_nomul_frag_glsl,
#if defined (GLES_VARIETY_S3C6410)
     img_bgra_nomul_frag_bin, sizeof(img_bgra_nomul_frag_bin)
#else     
     NULL, 0
#endif     
};

#if defined (GLES_VARIETY_S3C6410)
const unsigned int img_bgra_nomul_vert_bin[] =
{
# include "shader/img_bgra_nomul_vert_bin_s3c6410.h"
};
#endif
const char img_bgra_nomul_vert_glsl[] =
#include "shader/img_bgra_nomul_vert.h"
  ;
Evas_GL_Program_Source shader_img_bgra_nomul_vert_src =
{
   img_bgra_nomul_vert_glsl,
#if defined (GLES_VARIETY_S3C6410)
     img_bgra_nomul_vert_bin, sizeof(img_bgra_nomul_vert_bin)
#else     
     NULL, 0
#endif     
};

/////////////////////////////////////////////
static void
gl_compile_link_error(GLuint target, const char *action)
{
   int loglen = 0, chars = 0;
   char *logtxt;
   
   glGetProgramiv(target, GL_INFO_LOG_LENGTH, &loglen);
   logtxt = calloc(loglen, sizeof(char));
   if (logtxt)
     {
        glGetProgramInfoLog(target, loglen, &chars, logtxt);
        printf("Failed to %s: %s\n", action, logtxt);
        free(logtxt);
     }
}

void
evas_gl_common_shader_program_init(Evas_GL_Program *p, 
                                   Evas_GL_Program_Source *vert,
                                   Evas_GL_Program_Source *frag,
                                   const char *name)
{
   GLint ok;

   p->vert = glCreateShader(GL_VERTEX_SHADER);
   p->frag = glCreateShader(GL_FRAGMENT_SHADER);
#if defined (GLES_VARIETY_S3C6410)
   glShaderBinary(1, &(p->vert), 0, vert->bin, vert->bin_size);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glShaderBinary(1, &(p->frag), 0, frag->bin, frag->bin_size);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
#else 
   glShaderSource(p->vert, 1,
                  (const char **)&(vert->src), NULL);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glCompileShader(p->vert);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glGetShaderiv(p->vert, GL_COMPILE_STATUS, &ok);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   if (!ok)
     {
        gl_compile_link_error(p->vert, "compile vertex shader");
        printf("Abort compile of shader vert (%s):\n%s\n", name, vert->src);
        return;
     }
   glShaderSource(p->frag, 1,
                  (const char **)&(frag->src), NULL);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glCompileShader(p->frag);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glGetShaderiv(p->frag, GL_COMPILE_STATUS, &ok);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   if (!ok)
     {
        gl_compile_link_error(p->frag, "compile fragment shader");
        printf("Abort compile of shader frag (%s):\n%s\n", name, frag->src);
        return;
     }
#endif
   p->prog = glCreateProgram();
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
   
   glLinkProgram(p->prog);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glGetProgramiv(p->prog, GL_LINK_STATUS, &ok);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   if (!ok)
     {
        gl_compile_link_error(p->prog, "link fragment and vertex shaders");
        printf("Abort compile of shader frag (%s):\n%s\n", name, frag->src);
        printf("Abort compile of shader vert (%s):\n%s\n", name, vert->src);
        return;
     }
}
