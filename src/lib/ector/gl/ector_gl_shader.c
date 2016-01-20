#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include <Ector.h>

#include "gl/Ector_GL.h"
#include "ector_private.h"
#include "ector_gl_private.h"

#include "shader/ector_gl_shaders.x"

static const char *_shader_flags[SHADER_FLAG_COUNT] = {
  "TEX",
  "BGRA",
  "MASK",
  "SAM12",
  "SAM21",
  "SAM22",
  "MASKSAM12",
  "MASKSAM21",
  "MASKSAM22",
  "IMG",
  "BIGENDIAN",
  "YUV",
  "YUY2",
  "NV12",
  "YUV_709",
  "EXTERNAL",
  "AFILL",
  "NOMUL",
  "ALPHA",
  "RGB_A_PAIR"
};

static Eina_Strbuf *
_ector_gl_shader_glsl_get(uint64_t flags, const char *base)
{
   Eina_Strbuf *r;
   unsigned int k;

   r = eina_strbuf_new();
   for (k =0; k < SHADER_FLAG_COUNT; k++)
     {
        if (flags & (1 << k))
          eina_strbuf_append_printf(r, "#define SHD_%s\n", _shader_flags[k]);
     }

   eina_strbuf_append(r, base);

   return r;
}

static GLint
_ector_gl_shader_glsl_compile(GLuint s, const Eina_Strbuf *shader, const char *type)
{
   const char *str;
   GLint ok = 0;

   str = eina_strbuf_string_get(shader);

   GL.glShaderSource(s, 1, &str, NULL);
   GL.glCompileShader(s);
   GL.glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
   if (!ok)
     {
        Eina_Strbuf *err;

        err = eina_strbuf_new();
        if (!err) goto on_error;
        eina_strbuf_append_printf(err, "compile of %s shader", type);

        gl_compile_link_error(s, eina_strbuf_string_get(err), EINA_TRUE);
        ERR("Abort %s:\n%s", eina_strbuf_string_get(err), str);

        eina_strbuf_free(err);
     }

 on_error:
   return ok;
}

static GLint
_ector_gl_shader_glsl_link(uint64_t flags,
                           const Eina_Strbuf *vertex,
                           const Eina_Strbuf *fragment)
{
   GLuint vtx = 0, frg = 0, prg = 0;
   GLint ok = 0;

   vtx = GL.glCreateShader(GL_VERTEX_SHADER);
   frg = GL.glCreateShader(GL_FRAGMENT_SHADER);

   // Compiling vertex shader
   ok = _ector_gl_shader_glsl_compile(vtx, vertex, "vertex");
   if (!ok) goto on_error;

   // Compile fragment shader
   ok = _ector_gl_shader_glsl_compile(frg, fragment, "fragment");
   if (!ok) goto on_error;

   // Link both shader together
   ok = 0;

   prg = GL.glCreateProgram();
#ifndef GL_GLES
   if ((GL.glGetProgramBinary) && (GL.glProgramParameteri))
     GL.glProgramParameteri(prg, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, GL_TRUE);
#endif
   GL.glAttachShader(prg, vtx);
   GL.glAttachShader(prg, frg);

   GL.glBindAttribLocation(prg, SHAD_VERTEX,  "vertex");
   GL.glBindAttribLocation(prg, SHAD_COLOR,   "color");
   GL.glBindAttribLocation(prg, SHAD_TEXUV,   "tex_coord");
   GL.glBindAttribLocation(prg, SHAD_TEXUV2,  "tex_coord2");
   GL.glBindAttribLocation(prg, SHAD_TEXUV3,  "tex_coord3");
   GL.glBindAttribLocation(prg, SHAD_TEXA,    "tex_coorda");
   GL.glBindAttribLocation(prg, SHAD_TEXSAM,  "tex_sample");
   GL.glBindAttribLocation(prg, SHAD_MASK,    "mask_coord");
   GL.glBindAttribLocation(prg, SHAD_MASKSAM, "tex_masksample");

   GL.glLinkProgram(prg);
   GL.glGetProgramiv(prg, GL_LINK_STATUS, &ok);
   if (!ok)
     {
        gl_compile_link_error(prg, "link fragment and vertex shaders", EINA_FALSE);
        ERR("Abort compile of shader (flags: %16" PRIx64 ")", flags);
        GL.glDeleteProgram(prg);
        prg = 0;
        goto on_error;
     }

 on_error:
   if (vtx) GL.glDeleteShader(vtx);
   if (frg) GL.glDeleteShader(frg);

   return prg;
}

GLuint
ector_gl_shader_compile(uint64_t flags)
{
   Eina_Strbuf *vertex, *fragment;
   GLuint shd = 0;

   vertex = _ector_gl_shader_glsl_get(flags, vertex_glsl);
   fragment = _ector_gl_shader_glsl_get(flags, fragment_glsl);
   if (!vertex || !fragment) goto on_error;

   shd = _ector_gl_shader_glsl_link(flags, vertex, fragment);

 on_error:
   eina_strbuf_free(vertex);
   eina_strbuf_free(fragment);

#ifdef GL_GLES
   GL.glReleaseShaderCompiler();
#else
   if (GL.glReleaseShaderCompiler)
     GL.glReleaseShaderCompiler();
#endif

   return shd;
}
