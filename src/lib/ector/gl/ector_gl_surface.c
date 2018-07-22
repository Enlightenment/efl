#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include <Eet.h>
#include <Ector.h>

#include "gl/Ector_GL.h"
#include "ector_private.h"
#include "ector_gl_private.h"

typedef struct _Ector_GL_Surface_Data Ector_GL_Surface_Data;
struct _Ector_GL_Surface_Data
{
   struct {
      int x, y;
   } reference_point;

   Efl_Gfx_Render_Op op;
};

typedef struct _Ector_Shader Ector_Shader;
struct _Ector_Shader
{
   uint64_t flags;
   GLuint prg;
};

static Eina_Hash *shader_cache = NULL;
static Eet_File *shader_file = NULL;

static void
_shader_free(void *s)
{
   Ector_Shader *shd = s;

   GL.glDeleteProgram(shd->prg);
   free(shd);
}

static Ector_Renderer *
_ector_gl_surface_ector_surface_renderer_factory_new(Eo *obj,
                                                             Ector_GL_Surface_Data *pd EINA_UNUSED,
                                                             const Efl_Class *type)
{
   if (type == ECTOR_RENDERER_SHAPE_MIXIN)
     return efl_add_ref(ECTOR_RENDERER_GL_SHAPE_CLASS, NULL, ector_renderer_surface_set(efl_added, obj));
   else if (type == ECTOR_RENDERER_GRADIENT_LINEAR_MIXIN)
     return efl_add_ref(ECTOR_RENDERER_GL_GRADIENT_LINEAR_CLASS, NULL, ector_renderer_surface_set(efl_added, obj));
   else if (type == ECTOR_RENDERER_GRADIENT_RADIAL_MIXIN)
     return efl_add_ref(ECTOR_RENDERER_GL_GRADIENT_RADIAL_CLASS, NULL, ector_renderer_surface_set(efl_added, obj));

   ERR("Couldn't find class for type: %s\n", efl_class_name_get(type));
   return NULL;
}

static void
_ector_gl_surface_ector_surface_reference_point_set(Eo *obj EINA_UNUSED,
                                                            Ector_GL_Surface_Data *pd,
                                                            int x, int y)
{
   pd->reference_point.x = x;
   pd->reference_point.y = y;
}

#define VERTEX_CNT 3
#define COLOR_CNT 4

static Eina_Bool
_ector_gl_surface_push(Eo *obj,
                       Ector_GL_Surface_Data *pd EINA_UNUSED,
                       uint64_t flags, GLshort *vertex, unsigned int vertex_count, unsigned int mul_col)
{
   unsigned int prog;

   prog = ector_gl_surface_shader_get(obj, flags);

   // FIXME: Not using map/unmap buffer yet, nor any pipe
   // FIXME: Move some of the state change to start of surface drawing?
   GL.glUseProgram(prog);
   GL.glDisable(GL_TEXTURE_2D);
   GL.glDisable(GL_SCISSOR_TEST);
   GL.glVertexAttribPointer(SHAD_VERTEX, VERTEX_CNT, GL_SHORT, GL_FALSE, 0, vertex);
   GL.glEnableVertexAttribArray(SHAD_COLOR);
   GL.glVertexAttribPointer(SHAD_COLOR, COLOR_CNT, GL_UNSIGNED_BYTE, GL_TRUE, 0, &mul_col);
   GL.glDrawArrays(GL_TRIANGLES, 0, vertex_count);

   return EINA_TRUE;
}

static Eina_Bool
_ector_gl_surface_state_define(Eo *obj EINA_UNUSED, Ector_GL_Surface_Data *pd, Efl_Gfx_Render_Op op, Eina_Array *clips)
{
   if (pd->op == op) return EINA_TRUE;

   // FIXME: no pipe yet, so we can just change the mode right away
   // Get & apply matrix transformation too
   switch (op)
     {
      case EFL_GFX_RENDER_OP_BLEND: /**< default op: d = d*(1-sa) + s */
         GL.glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
         GL.glEnable(GL_BLEND);
         break;
      case EFL_GFX_RENDER_OP_COPY: /**< d = s */
         // Just disable blend mode. no need to set blend func
         GL.glDisable(GL_BLEND);
         break;
      case EFL_GFX_RENDER_OP_LAST:
      default:
         return EINA_FALSE;
     }

   pd->op = op;

   // FIXME: we should not ignore clipping, but that can last for later
   (void) clips;

   return EINA_TRUE;
}

static void
_ector_gl_shader_textures_bind(Ector_Shader *p)
{
   struct {
      const char *name;
      int enabled;
   } textures[] = {
      { "tex", 0 },
      { "texm", 0 },
      { "texa", 0 },
      { "texu", 0 },
      { "texv", 0 },
      { "texuv", 0 },
      { NULL, 0 }
   };
   Eina_Bool hastex = 0;
   int tex_count = 0;
   GLint loc;
   int i;

   if (!p) return;

   if ((p->flags & SHADER_FLAG_TEX) != 0)
     {
        textures[0].enabled = 1;
        hastex = 1;
     }
   if ((p->flags & SHADER_FLAG_MASK) != 0)
     {
        textures[1].enabled = 1;
        hastex = 1;
     }
   if ((p->flags & SHADER_FLAG_RGB_A_PAIR) != 0)
     {
        textures[2].enabled = 1;
        hastex = 1;
     }
   if (p->flags & SHADER_FLAG_YUV)
     {
        textures[3].enabled = 1;
        textures[4].enabled = 1;
        hastex = 1;
     }
   else if ((p->flags & SHADER_FLAG_NV12) || (p->flags & SHADER_FLAG_YUY2))
     {
        textures[5].enabled = 1;
        hastex = 1;
     }

   if (hastex)
     {
        GL.glUseProgram(p->prg); // FIXME: is this necessary??
        for (i = 0; textures[i].name; i++)
          {
             if (!textures[i].enabled) continue;
             loc = GL.glGetUniformLocation(p->prg, textures[i].name);
             if (loc < 0)
               {
                  ERR("Couldn't find uniform '%s' (shader: %16" PRIx64 ")",
                      textures[i].name, p->flags);
               }
             GL.glUniform1i(loc, tex_count++);
          }
     }
}

static Ector_Shader *
_ector_gl_shader_load(uint64_t flags)
{
   Eina_Strbuf *buf;
   Ector_Shader *r = NULL;
   void *data;
   int *formats = NULL;
   int length = 0, num = 0;
   GLuint prg;
   GLint ok = 0, vtx = GL_NONE, frg = GL_NONE;
   Eina_Bool direct = 1;

   buf = eina_strbuf_new();
   if (!buf) return NULL;

   eina_strbuf_append_printf(buf, "ector/shader/%16" PRIx64, flags);

   data = (void*) eet_read_direct(shader_file, eina_strbuf_string_get(buf), &length);
   if (!data)
     {
        data = eet_read(shader_file, eina_strbuf_string_get(buf), &length);
        direct = 0;
     }
   if ((!data) || (length <= 0)) goto on_error;

   GL.glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &num);
   if (num <= 0) goto on_error;

   formats = calloc(num, sizeof(int));
   if (!formats) goto on_error;

   GL.glGetIntegerv(GL_PROGRAM_BINARY_FORMATS, formats);
   if (!formats[0]) goto on_error;

   prg = GL.glCreateProgram();
   // TODO: invalid rendering error occurs when attempting to use a
   // glProgramBinary.  In order to render correctly, we should create a
   // dummy vertex shader.
   vtx = GL.glCreateShader(GL_VERTEX_SHADER);
   GL.glAttachShader(prg, vtx);
   frg = GL.glCreateShader(GL_FRAGMENT_SHADER);
   GL.glAttachShader(prg, frg);

   GL.glProgramBinary(prg, formats[0], data, length);

   GL.glBindAttribLocation(prg, SHAD_VERTEX,  "vertex");
   GL.glBindAttribLocation(prg, SHAD_COLOR,   "color");
   GL.glBindAttribLocation(prg, SHAD_TEXUV,   "tex_coord");
   GL.glBindAttribLocation(prg, SHAD_TEXUV2,  "tex_coord2");
   GL.glBindAttribLocation(prg, SHAD_TEXUV3,  "tex_coord3");
   GL.glBindAttribLocation(prg, SHAD_TEXA,    "tex_coorda");
   GL.glBindAttribLocation(prg, SHAD_TEXSAM,  "tex_sample");
   GL.glBindAttribLocation(prg, SHAD_MASK,    "mask_coord");
   GL.glBindAttribLocation(prg, SHAD_MASKSAM, "tex_masksample");

   GL.glGetProgramiv(prg, GL_LINK_STATUS, &ok);
   if (!ok)
     {
        gl_compile_link_error(prg, "load a program object", EINA_FALSE);
        ERR("Abort load of program (%s)", eina_strbuf_string_get(buf));
        GL.glDeleteProgram(prg);
        goto on_error;
     }

   r = malloc(sizeof (Ector_Shader));
   r->prg = prg;
   r->flags = flags;

   _ector_gl_shader_textures_bind(r);

 on_error:
   if (vtx) GL.glDeleteShader(vtx);
   if (frg) GL.glDeleteShader(frg);
   free(formats);

   if (!direct) free(data);
   eina_strbuf_free(buf);
   return r;
}

static unsigned int
_ector_gl_surface_shader_get(Eo *obj EINA_UNUSED, Ector_GL_Surface_Data *pd EINA_UNUSED, uint64_t flags)
{
   Ector_Shader *shd;
   Eina_Strbuf *buf = NULL;
   void *data = NULL;
   int length = 0, size = 0;
   GLenum format;
   GLuint prg;

   shd = eina_hash_find(shader_cache, &flags);
   if (shd) return shd->prg;

   shd = _ector_gl_shader_load(flags);
   if (shd)
     {
        eina_hash_direct_add(shader_cache, &shd->flags, shd);
        return shd->prg;
     }

   prg = ector_gl_shader_compile(flags);
   if (prg <= 0) return -1;

   GL.glGetProgramiv(prg, GL_PROGRAM_BINARY_LENGTH, &length);
   if (length <= 0) return prg;

   if (GL.glGetProgramBinary)
     {
        data = malloc(length);
        if (!data) return prg;

        GL.glGetProgramBinary(prg, length, &size, &format, data);
        if (length != size) goto on_error;
     }

   shd = malloc(sizeof (Ector_Shader));
   if (!shd) goto on_error;

   shd->prg = prg;
   shd->flags = flags;

   // Save the shader in the cache file
   eina_hash_direct_add(shader_cache, &shd->flags, shd);

   // Save binary shader in the cache file
   if (GL.glGetProgramBinary)
     {
        buf = eina_strbuf_new();
        eina_strbuf_append_printf(buf, "ector/shader/%16" PRIx64, flags);

        eet_write(shader_file, eina_strbuf_string_get(buf), data, length, 1);

        eina_strbuf_free(buf);
     }

 on_error:
   free(data);

   return prg;
}

static void
_ector_gl_surface_efl_object_destructor(Eo *obj, Ector_GL_Surface_Data *pd EINA_UNUSED)
{
   efl_destructor(efl_super(obj, ECTOR_GL_SURFACE_CLASS));

   eina_hash_free(shader_cache);
   shader_cache = NULL;
   eet_close(shader_file);
   shader_file = NULL;
}

static Efl_Object *
_ector_gl_surface_efl_object_constructor(Eo *obj, Ector_GL_Surface_Data *pd EINA_UNUSED)
{
   Eina_Strbuf *file_path = NULL;

   obj = efl_constructor(efl_super(obj, ECTOR_GL_SURFACE_CLASS));
   if (!obj) return NULL;

   if (shader_cache) return obj;

   // Only initialize things once
   shader_cache = eina_hash_int64_new(_shader_free);

   /* glsym_glProgramBinary = _ector_gl_symbol_get(obj, "glProgramBinary"); */
   /* glsym_glGetProgramBinary = _ector_gl_symbol_get(obj, "glGetProgramBinary"); */
   /* glsym_glProgramParameteri = _ector_gl_symbol_get(obj, "glProgramParameteri"); */
   /* glsym_glReleaseShaderCompiler = _ector_gl_symbol_get(obj, "glReleaseShaderCompiler"); */

   if (GL.glProgramBinary && GL.glGetProgramBinary)
     {
        file_path = eina_strbuf_new();
        if (eina_environment_home_get())
          eina_strbuf_append(file_path, eina_environment_home_get());
        else
          eina_strbuf_append(file_path, eina_environment_tmp_get());
        eina_strbuf_append_printf(file_path, "%c.cache", EINA_PATH_SEP_C);
        // FIXME: test and create path if necessary
        eina_strbuf_append_printf(file_path, "%cector", EINA_PATH_SEP_C);
        eina_strbuf_append_printf(file_path, "%cector-shader-%i.%i.eet",
                                  EINA_PATH_SEP_C, EFL_VERSION_MAJOR, EFL_VERSION_MINOR);
        shader_file = eet_open(eina_strbuf_string_get(file_path), EET_FILE_MODE_READ_WRITE);
     }
   if (!shader_file)
     {
        ERR("Unable to create '%s' ector binary shader file.", eina_strbuf_string_get(file_path));
        GL.glProgramBinary = NULL;
     }
   eina_strbuf_free(file_path);

   return obj;
}

#include "ector_gl_surface.eo.c"
