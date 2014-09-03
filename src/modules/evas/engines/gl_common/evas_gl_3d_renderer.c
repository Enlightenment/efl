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
};

static inline GLenum
_gl_assembly_get(Evas_3D_Vertex_Assembly assembly)
{
   switch (assembly)
     {
      case EVAS_3D_VERTEX_ASSEMBLY_POINTS:
         return GL_POINTS;
      case EVAS_3D_VERTEX_ASSEMBLY_LINES:
         return GL_LINES;
      case EVAS_3D_VERTEX_ASSEMBLY_LINE_STRIP:
         return GL_LINE_STRIP;
      case EVAS_3D_VERTEX_ASSEMBLY_LINE_LOOP:
         return GL_LINE_LOOP;
      case EVAS_3D_VERTEX_ASSEMBLY_TRIANGLES:
         return GL_TRIANGLES;
      case EVAS_3D_VERTEX_ASSEMBLY_TRIANGLE_STRIP:
         return GL_TRIANGLE_STRIP;
      case EVAS_3D_VERTEX_ASSEMBLY_TRIANGLE_FAN:
         return GL_TRIANGLE_FAN;
      default:
         return GL_NONE;
     }
}

static inline void
_renderer_vertex_attrib_array_enable(E3D_Renderer *renderer, int index)
{
   if (renderer->vertex_attrib_enable[index])
     return;

   glEnableVertexAttribArray(index);
   renderer->vertex_attrib_enable[index] = EINA_TRUE;
}

static inline void
_renderer_vertex_attrib_array_disable(E3D_Renderer *renderer, int index)
{
   if (!renderer->vertex_attrib_enable[index])
     return;

   glDisableVertexAttribArray(index);
   renderer->vertex_attrib_enable[index] = EINA_FALSE;
}

static inline void
_renderer_vertex_attrib_pointer_set(E3D_Renderer *renderer EINA_UNUSED, int index,
                                    const Evas_3D_Vertex_Buffer *buffer)
{
   glVertexAttribPointer(index, buffer->element_count, GL_FLOAT,
                         GL_FALSE, buffer->stride, buffer->data);
}

static inline void
_renderer_elements_draw(E3D_Renderer *renderer EINA_UNUSED, Evas_3D_Vertex_Assembly assembly,
                        int count, Evas_3D_Index_Format format, const void *indices)
{
   GLenum mode = _gl_assembly_get(assembly);

   if (format == EVAS_3D_INDEX_FORMAT_UNSIGNED_BYTE)
     glDrawElements(mode, count, GL_UNSIGNED_BYTE, indices);
   else if (format == EVAS_3D_INDEX_FORMAT_UNSIGNED_SHORT)
     glDrawElements(mode, count, GL_UNSIGNED_SHORT, indices);
}

static inline void
_renderer_array_draw(E3D_Renderer *renderer EINA_UNUSED,
                     Evas_3D_Vertex_Assembly assembly, int count)
{
   GLenum mode = _gl_assembly_get(assembly);

   glDrawArrays(mode, 0, count);
}

static inline void
_renderer_program_use(E3D_Renderer *renderer ,E3D_Program *program)
{
   GLuint prog = e3d_program_id_get(program);

   if (renderer->program != prog)
     {
        glUseProgram(prog);
        renderer->program = prog;
     }
}

static inline void
_renderer_texture_bind(E3D_Renderer *renderer, E3D_Draw_Data *data)
{
   int i;

   for (i = 0; i < EVAS_3D_MATERIAL_ATTRIB_COUNT; i++)
     {
        if (data->materials[i].tex0)
          {
             if (renderer->textures[data->materials[i].sampler0] != data->materials[i].tex0)
               {
                  glActiveTexture(GL_TEXTURE0 + data->materials[i].sampler0);
                  glBindTexture(GL_TEXTURE_2D, data->materials[i].tex0->tex);
                  e3d_texture_param_update(data->materials[i].tex0);

                  renderer->textures[data->materials[i].sampler0] = data->materials[i].tex0;
               }
          }

        if (data->materials[i].tex1)
          {
             if (renderer->textures[data->materials[i].sampler1] != data->materials[i].tex1)
               {
                  glActiveTexture(GL_TEXTURE0 + data->materials[i].sampler1);
                  glBindTexture(GL_TEXTURE_2D, data->materials[i].tex1->tex);
                  e3d_texture_param_update(data->materials[i].tex1);

                  renderer->textures[data->materials[i].sampler1] = data->materials[i].tex1;
               }
          }
     }
     glActiveTexture(GL_TEXTURE0 + data->smap_sampler);
     glBindTexture(GL_TEXTURE_2D, renderer->texDepth);
}

static inline void
_renderer_depth_test_enable(E3D_Renderer *renderer, Eina_Bool enable)
{
   if (renderer->depth_test_enable != enable)
     {
        if (enable)
          {
             glEnable(GL_DEPTH_TEST);
             /* Use default depth func. */
          }
        else
          {
             glDisable(GL_DEPTH_TEST);
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

   glBindFramebuffer(GL_FRAMEBUFFER, target->fbo);
   glViewport(0, 0, target->w, target->h);
   renderer->fbo = target->fbo;
   renderer->texDepth = target->texDepth;
}

void
e3d_renderer_clear(E3D_Renderer *renderer EINA_UNUSED, const Evas_Color *color)
{
   glClearColor(color->r, color->g, color->b, color->a);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
        if (e3d_program_shade_mode_get(p) == data->mode &&
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
   if (data->mode != EVAS_3D_SHADE_MODE_SHADOW_MAP_RENDER)
   _renderer_texture_bind(renderer, data);

   /* Set up vertex attrib pointers. */
   index = 0;

   for (i = 0; i < EVAS_3D_VERTEX_ATTRIB_COUNT; i++)
     {
        const Evas_3D_Vertex_Buffer *buffer;

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
   glFlush();
}
