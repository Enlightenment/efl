#include "evas_gl_private.h"

/////////////////////////////////////////////
#if defined (GLES_VARIETY_S3C6410)
const unsigned int rect_frag_bin[] =
{
# include "shader/rect_frag_bin.h"
};
#endif

const char rect_frag_glsl[] =
#include "shader/rect_frag.h"
  ;
Evas_GL_Program_Source shader_rect_frag_src =
{
   rect_frag_glsl,
#if defined (GLES_VARIETY_S3C6410)
     rect_frag_bin, sizeof(rect_frag_bin_end)
#else     
     NULL, 0
#endif     
};

#if defined (GLES_VARIETY_S3C6410)
const unsigned int rect_frag_bin[] =
{
# include "shader/rect_vert_bin.h"
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
const unsigned int img_frag_bin[] =
{
# include "shader/img_frag_bin.h"
};
#endif

const char img_frag_glsl[] =
#include "shader/img_frag.h"
  ;
Evas_GL_Program_Source shader_img_frag_src =
{
   img_frag_glsl,
#if defined (GLES_VARIETY_S3C6410)
     img_frag_bin, sizeof(img_frag_bin_end)
#else     
     NULL, 0
#endif     
};

#if defined (GLES_VARIETY_S3C6410)
const unsigned int img_frag_bin[] =
{
# include "shader/img_vert_bin.h"
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
const unsigned int font_frag_bin[] =
{
# include "shader/font_frag_bin.h"
};
#endif

const char font_frag_glsl[] =
#include "shader/font_frag.h"
  ;
Evas_GL_Program_Source shader_font_frag_src =
{
   font_frag_glsl,
#if defined (GLES_VARIETY_S3C6410)
     font_frag_bin, sizeof(font_frag_bin_end)
#else     
     NULL, 0
#endif     
};

#if defined (GLES_VARIETY_S3C6410)
const unsigned int font_frag_bin[] =
{
# include "shader/font_vert_bin.h"
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
# include "shader/yuv_frag_bin.h"
};
#endif

const char yuv_frag_glsl[] =
#include "shader/yuv_frag.h"
  ;
Evas_GL_Program_Source shader_yuv_frag_src =
{
   yuv_frag_glsl,
#if defined (GLES_VARIETY_S3C6410)
     yuv_frag_bin, sizeof(yuv_frag_bin_end)
#else     
     NULL, 0
#endif     
};

#if defined (GLES_VARIETY_S3C6410)
const unsigned int yuv_frag_bin[] =
{
# include "shader/yuv_vert_bin.h"
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
                                   Evas_GL_Program_Source *frag)
{
   GLint ok;
   
   p->vert = glCreateShader(GL_VERTEX_SHADER);
   p->frag = glCreateShader(GL_FRAGMENT_SHADER);
#if defined (GLES_VARIETY_S3C6410)
   glShaderBinary(1, &(p->vert), 0, vert->bin, vert->bin_size);
   glShaderBinary(1, &(p->frag), 0, frag->bin, vert->bin_size);
#else 
   glShaderSource(p->vert, 1,
                  (const char **)&(vert->src), NULL);
   glCompileShader(p->vert);
   glGetShaderiv(p->vert, GL_COMPILE_STATUS, &ok);
   if (!ok) gl_compile_link_error(p->vert, "compile vertex shader");
   glShaderSource(p->frag, 1,
                  (const char **)&(frag->src), NULL);
   glCompileShader(p->frag);
   glGetShaderiv(p->frag, GL_COMPILE_STATUS, &ok);
   if (!ok) gl_compile_link_error(p->frag, "compile fragment shader");
#endif
   p->prog = glCreateProgram();
   glAttachShader(p->prog, p->vert);
   glAttachShader(p->prog, p->frag);
   
   glBindAttribLocation(p->prog, SHAD_VERTEX, "vertex");
   glBindAttribLocation(p->prog, SHAD_COLOR, "color");
   glBindAttribLocation(p->prog, SHAD_TEXUV, "tex_coord");
   glBindAttribLocation(p->prog, SHAD_TEXUV2, "tex_coord2");
   glBindAttribLocation(p->prog, SHAD_TEXUV3, "tex_coord3");
   
   glLinkProgram(p->prog);
   glGetProgramiv(p->prog, GL_LINK_STATUS, &ok);
   if (!ok) gl_compile_link_error(p->prog, "link fragment and vertex shaders");
}
