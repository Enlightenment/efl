#include "evas_gl_private.h"
#include "evas_gl_3d_private.h"

#define E3D_MAX_TEXTURE_COUNT       8
#define E3D_MAX_VERTEX_ATTRIB_COUNT 8

struct _E3D_Renderer
{
   Eina_List     *programs;

   GLuint         fbo;
   GLuint         program;
   E3D_Texture   *textures[E3D_MAX_TEXTURE_COUNT];

   Eina_Bool      vertex_attrib_enable[E3D_MAX_VERTEX_ATTRIB_COUNT];
   Eina_Bool      depth_test_enable;
   GLuint         texDepth;
   GLint          smap_sampler;
   Eina_Bool      render_to_texture;
   GLuint         texcolorpick;
   GLint          colortex_sampler;
};

static inline GLenum
_gl_assembly_get(Evas_Canvas3D_Vertex_Assembly assembly)
{
   switch (assembly)
     {
      case EVAS_CANVAS3D_VERTEX_ASSEMBLY_POINTS:
         return GL_POINTS;
      case EVAS_CANVAS3D_VERTEX_ASSEMBLY_LINES:
         return GL_LINES;
      case EVAS_CANVAS3D_VERTEX_ASSEMBLY_LINE_STRIP:
         return GL_LINE_STRIP;
      case EVAS_CANVAS3D_VERTEX_ASSEMBLY_LINE_LOOP:
         return GL_LINE_LOOP;
      case EVAS_CANVAS3D_VERTEX_ASSEMBLY_TRIANGLES:
         return GL_TRIANGLES;
      case EVAS_CANVAS3D_VERTEX_ASSEMBLY_TRIANGLE_STRIP:
         return GL_TRIANGLE_STRIP;
      case EVAS_CANVAS3D_VERTEX_ASSEMBLY_TRIANGLE_FAN:
         return GL_TRIANGLE_FAN;
      default:
         return GL_NONE;
     }
}

static inline GLenum
_gl_blend_func_get(Evas_Canvas3D_Blend_Func blend_func)
{
   switch (blend_func)
     {
      case EVAS_CANVAS3D_BLEND_FUNC_ZERO:
         return GL_ZERO;
      case EVAS_CANVAS3D_BLEND_FUNC_ONE:
         return GL_ONE;
      case EVAS_CANVAS3D_BLEND_FUNC_SRC_COLOR:
         return GL_SRC_COLOR;
      case EVAS_CANVAS3D_BLEND_FUNC_ONE_MINUS_SRC_COLOR:
         return GL_ONE_MINUS_SRC_COLOR;
      case EVAS_CANVAS3D_BLEND_FUNC_DST_COLOR:
         return GL_DST_COLOR;
      case EVAS_CANVAS3D_BLEND_FUNC_ONE_MINUS_DST_COLOR:
         return GL_ONE_MINUS_DST_COLOR;
      case EVAS_CANVAS3D_BLEND_FUNC_SRC_ALPHA:
         return GL_SRC_ALPHA;
      case EVAS_CANVAS3D_BLEND_FUNC_ONE_MINUS_SRC_ALPHA:
         return GL_ONE_MINUS_SRC_ALPHA;
      case EVAS_CANVAS3D_BLEND_FUNC_DST_ALPHA:
         return GL_DST_ALPHA;
      case EVAS_CANVAS3D_BLEND_FUNC_ONE_MINUS_DST_ALPHA:
         return GL_ONE_MINUS_DST_ALPHA;
      case EVAS_CANVAS3D_BLEND_FUNC_CONSTANT_COLOR:
         return GL_CONSTANT_COLOR;
      case EVAS_CANVAS3D_BLEND_FUNC_ONE_MINUS_CONSTANT_COLOR:
         return GL_ONE_MINUS_CONSTANT_COLOR;
      case EVAS_CANVAS3D_BLEND_FUNC_CONSTANT_ALPHA:
         return GL_CONSTANT_ALPHA;
      case EVAS_CANVAS3D_BLEND_FUNC_ONE_MINUS_CONSTANT_ALPHA:
         return GL_ONE_MINUS_CONSTANT_ALPHA;
      case EVAS_CANVAS3D_BLEND_FUNC_SRC_ALPHA_SATURATE:
         return GL_SRC_ALPHA_SATURATE;
      default:
         return GL_ZERO;
     }
}

#ifndef GL_GLES
static inline GLenum
_gl_comparison_func_get(Evas_Canvas3D_Comparison comparison_func)
{
   switch (comparison_func)
     {
      case EVAS_CANVAS3D_COMPARISON_NEVER:
         return GL_NEVER;
      case EVAS_CANVAS3D_COMPARISON_LESS:
         return GL_LESS;
      case EVAS_CANVAS3D_COMPARISON_EQUAL:
         return GL_EQUAL;
      case EVAS_CANVAS3D_COMPARISON_LEQUAL:
         return GL_LEQUAL;
      case EVAS_CANVAS3D_COMPARISON_GREATER:
         return GL_GREATER;
      case EVAS_CANVAS3D_COMPARISON_NOTEQUAL:
         return GL_NOTEQUAL;
      case EVAS_CANVAS3D_COMPARISON_GEQUAL:
         return GL_GEQUAL;
      case EVAS_CANVAS3D_COMPARISON_ALWAYS:
         return GL_ALWAYS;
      default:
         return GL_ALWAYS;
     }
}
#endif

static inline void
_renderer_vertex_attrib_array_enable(E3D_Renderer *renderer, int index)
{
   if (renderer->vertex_attrib_enable[index])
     return;

   GL_TH(glEnableVertexAttribArray, index);
   renderer->vertex_attrib_enable[index] = EINA_TRUE;
}

static inline void
_renderer_vertex_attrib_array_disable(E3D_Renderer *renderer, int index)
{
   if (!renderer->vertex_attrib_enable[index])
     return;

   GL_TH(glDisableVertexAttribArray, index);
   renderer->vertex_attrib_enable[index] = EINA_FALSE;
}

static inline void
_renderer_vertex_attrib_pointer_set(E3D_Renderer *renderer EINA_UNUSED, int index,
                                    const Evas_Canvas3D_Vertex_Buffer *buffer)
{
   GL_TH(glVertexAttribPointer, index, buffer->element_count, GL_FLOAT,
                                GL_FALSE, buffer->stride, buffer->data);
}

static inline void
_renderer_elements_draw(E3D_Renderer *renderer EINA_UNUSED, Evas_Canvas3D_Vertex_Assembly assembly,
                        int count, Evas_Canvas3D_Index_Format format, const void *indices)
{
   GLenum mode = _gl_assembly_get(assembly);

   if (format == EVAS_CANVAS3D_INDEX_FORMAT_UNSIGNED_BYTE)
     GL_TH(glDrawElements, mode, count, GL_UNSIGNED_BYTE, indices);
   else if (format == EVAS_CANVAS3D_INDEX_FORMAT_UNSIGNED_SHORT)
     GL_TH(glDrawElements, mode, count, GL_UNSIGNED_SHORT, indices);
}

static inline void
_renderer_array_draw(E3D_Renderer *renderer EINA_UNUSED,
                     Evas_Canvas3D_Vertex_Assembly assembly, int count)
{
   GLenum mode = _gl_assembly_get(assembly);

   GL_TH(glDrawArrays, mode, 0, count);
}

static inline void
_renderer_program_use(E3D_Renderer *renderer ,E3D_Program *program)
{
   GLuint prog = e3d_program_id_get(program);

   if (renderer->program != prog)
     {
        GL_TH(glUseProgram, prog);
        renderer->program = prog;
     }
}

static inline void
_renderer_texture_bind(E3D_Renderer *renderer, E3D_Draw_Data *data)
{
   int i;

   for (i = 0; i < EVAS_CANVAS3D_MATERIAL_ATTRIB_COUNT; i++)
     {
        if (data->materials[i].tex0)
          {
             if (renderer->textures[data->materials[i].sampler0] != data->materials[i].tex0)
               {
                  GL_TH(glActiveTexture, GL_TEXTURE0 + data->materials[i].sampler0);
                  GL_TH(glBindTexture, GL_TEXTURE_2D, data->materials[i].tex0->tex);
                  e3d_texture_param_update(data->materials[i].tex0);

                  renderer->textures[data->materials[i].sampler0] = data->materials[i].tex0;
               }
          }

        if (data->materials[i].tex1)
          {
             if (renderer->textures[data->materials[i].sampler1] != data->materials[i].tex1)
               {
                  GL_TH(glActiveTexture, GL_TEXTURE0 + data->materials[i].sampler1);
                  GL_TH(glBindTexture, GL_TEXTURE_2D, data->materials[i].tex1->tex);
                  e3d_texture_param_update(data->materials[i].tex1);

                  renderer->textures[data->materials[i].sampler1] = data->materials[i].tex1;
               }
          }
     }
   if ((data->flags & E3D_SHADER_FLAG_SHADOWED) && (renderer->smap_sampler != data->smap_sampler))
     {
        GL_TH(glActiveTexture, GL_TEXTURE0 + data->smap_sampler);
        GL_TH(glBindTexture, GL_TEXTURE_2D, renderer->texDepth);
        renderer->smap_sampler = data->smap_sampler;
     }
   if (renderer->render_to_texture)
     {
        GL_TH(glActiveTexture, GL_TEXTURE0 + data->colortex_sampler);
        GL_TH(glBindTexture, GL_TEXTURE_2D, renderer->texcolorpick);
        renderer->colortex_sampler = data->colortex_sampler;
     }
}

static inline void
_renderer_depth_test_enable(E3D_Renderer *renderer, Eina_Bool enable)
{
   if (renderer->depth_test_enable != enable)
     {
        if (enable)
          {
             GL_TH(glEnable, GL_DEPTH_TEST);
             /* Use default depth func. */
          }
        else
          {
             GL_TH(glDisable, GL_DEPTH_TEST);
          }

        renderer->depth_test_enable = enable;
     }
}

E3D_Renderer *
e3d_renderer_new(void)
{
   E3D_Renderer *renderer = NULL;

   renderer = (E3D_Renderer *)calloc(1, sizeof(E3D_Renderer));

   if (renderer == NULL)
     {
        ERR("Failed to allocate memory.");
        return NULL;
     }

   return renderer;
}

void
e3d_renderer_free(E3D_Renderer *renderer)
{
   Eina_List *l;
   E3D_Program *p;

   EINA_LIST_FOREACH(renderer->programs, l, p)
     {
        e3d_program_free(p);
     }

   eina_list_free(renderer->programs);
}

void
e3d_renderer_target_set(E3D_Renderer *renderer, E3D_Drawable *target)
{
   if (renderer->fbo == target->fbo)
     return;

   GL_TH(glBindFramebuffer, GL_FRAMEBUFFER, target->fbo);
   GL_TH(glViewport, 0, 0, target->w, target->h);
   renderer->fbo = target->fbo;
   renderer->texDepth = target->texDepth;
   renderer->texcolorpick = target->texcolorpick;
   renderer->render_to_texture = EINA_FALSE;
}

void
e3d_renderer_color_pick_target_set(E3D_Renderer *renderer, E3D_Drawable *drawable)
{
   GL_TH(glBindFramebuffer, GL_FRAMEBUFFER, drawable->color_pick_fb_id);
   GL_TH(glFramebufferTexture2D, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                 GL_TEXTURE_2D, drawable->texcolorpick, 0);
#ifdef GL_GLES
   GL_TH(glBindRenderbuffer, GL_RENDERBUFFER, drawable->depth_stencil_buf);
   GL_TH(glFramebufferTexture2D, GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                 GL_TEXTURE_2D, drawable->depth_stencil_buf, 0);
#else
   GL_TH(glBindRenderbuffer, GL_RENDERBUFFER, drawable->depth_stencil_buf);
   GL_TH(glFramebufferRenderbuffer, GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                                    GL_RENDERBUFFER, drawable->depth_stencil_buf);
#endif
   GL_TH(glViewport, 0, 0, drawable->w, drawable->h);
   renderer->texDepth = drawable->texDepth;
   renderer->texcolorpick = drawable->texcolorpick;
   renderer->render_to_texture = EINA_TRUE;
}

Eina_Bool
e3d_renderer_rendering_to_texture_get(E3D_Renderer *renderer)
{
   return renderer->render_to_texture;
}

void
e3d_renderer_clear(E3D_Renderer *renderer EINA_UNUSED, const Evas_Color *color)
{
   GL_TH(glClearColor, color->r, color->g, color->b, color->a);
   GL_TH(glClear, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void
e3d_renderer_draw(E3D_Renderer *renderer, E3D_Draw_Data *data)
{
   Eina_List *l;
   E3D_Program *program = NULL, *p;
   int i, index = 0;

   _renderer_depth_test_enable(renderer, EINA_TRUE);

   EINA_LIST_FOREACH(renderer->programs, l, p)
     {
        if (e3d_program_shader_mode_get(p) == data->mode &&
            e3d_program_shader_flags_get(p) == data->flags)
          {
             program = p;
             break;
          }
     }

   if (program == NULL)
     {
        program = e3d_program_new(data->mode, data->flags);

        if (program == NULL)
          {
             ERR("Failed to create shader program.");
             return;
          }

        renderer->programs = eina_list_append(renderer->programs, program);
     }

   _renderer_program_use(renderer, program);
   e3d_program_uniform_upload(program, data);
   _renderer_texture_bind(renderer, data);

   /* Set up vertex attrib pointers. */
   index = 0;

   for (i = 0; i < EVAS_CANVAS3D_VERTEX_ATTRIB_COUNT; i++)
     {
        const Evas_Canvas3D_Vertex_Buffer *buffer;

        buffer = &data->vertices[i].vertex0;

        if (buffer->data != NULL)
          {
             _renderer_vertex_attrib_array_enable(renderer, index);
             _renderer_vertex_attrib_pointer_set(renderer, index, buffer);
             index++;
          }

        buffer = &data->vertices[i].vertex1;

        if (buffer->data != NULL)
          {
             _renderer_vertex_attrib_array_enable(renderer, index);
             _renderer_vertex_attrib_pointer_set(renderer, index, buffer);
             index++;
          }
     }

   while (index < E3D_MAX_VERTEX_ATTRIB_COUNT)
     _renderer_vertex_attrib_array_disable(renderer, index++);

   if (data->blending)
     {
        GL_TH(glEnable, GL_BLEND);
        GL_TH(glBlendFunc, _gl_blend_func_get(data->blend_sfactor), _gl_blend_func_get(data->blend_dfactor));
     }
   else GL_TH(glDisable, GL_BLEND);

#ifndef GL_GLES
   if (data->alpha_test_enabled)
     {
        GL_TH(glEnable, GL_ALPHA_TEST);
        GL_TH(glAlphaFunc, _gl_comparison_func_get(data->alpha_comparison),
                           (GLclampf)data->alpha_ref_value);
     }
   else GL_TH(glDisable, GL_ALPHA_TEST);
#endif

   if (data->indices)
     {
        _renderer_elements_draw(renderer, data->assembly, data->index_count,
                                data->index_format, data->indices);
     }
   else
     {
        _renderer_array_draw(renderer, data->assembly, data->vertex_count);
     }
}

void
e3d_renderer_flush(E3D_Renderer *renderer EINA_UNUSED)
{
   GL_TH(glFlush);
}

GLint
e3d_renderer_sampler_colortexture_get(E3D_Renderer *renderer)
{
   return renderer->colortex_sampler;
}

GLint
e3d_renderer_sampler_shadowmap_get(E3D_Renderer *renderer)
{
   return renderer->smap_sampler;
}
