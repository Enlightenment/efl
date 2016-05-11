#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include <Ector.h>

#include "gl/Ector_GL.h"
#include "ector_private.h"
#include "ector_gl_private.h"

typedef struct _Ector_Renderer_GL_Shape_Data Ector_Renderer_GL_Shape_Data;
struct _Ector_Renderer_GL_Shape_Data
{
   Efl_Gfx_Shape_Public *public_shape;

   Ector_Renderer_Generic_Shape_Data *shape;
   Ector_Renderer_Generic_Base_Data *base;

   GLshort *vertex;
};

static Eina_Bool
_ector_renderer_gl_shape_path_changed(void *data, const Eo_Event *event EINA_UNUSED)
{
   Ector_Renderer_GL_Shape_Data *pd = data;

   free(pd->vertex);
   pd->vertex = NULL;

   return EINA_TRUE;
}

static Eina_Bool
_ector_renderer_gl_shape_ector_renderer_generic_base_prepare(Eo *obj, Ector_Renderer_GL_Shape_Data *pd)
{
   Eina_Rectangle bounding_box;
   Eina_Bool r;

   if (pd->vertex) return EINA_TRUE;

   r = ector_renderer_prepare(eo_super(obj, ECTOR_RENDERER_GL_SHAPE_CLASS));

   ector_renderer_bounds_get(obj, &bounding_box);

   pd->vertex = malloc(sizeof (GLshort) * 6 * 3);

   // Pushing 2 triangles
   pd->vertex[0] = bounding_box.x;
   pd->vertex[1] = bounding_box.y;
   pd->vertex[2] = 0;
   pd->vertex[3] = bounding_box.x + bounding_box.w;
   pd->vertex[4] = bounding_box.y;
   pd->vertex[5] = 0;
   pd->vertex[6] = bounding_box.x;
   pd->vertex[7] = bounding_box.y + bounding_box.h;
   pd->vertex[8] = 0;

   pd->vertex[9] = bounding_box.x;
   pd->vertex[10] = bounding_box.y + bounding_box.h;
   pd->vertex[11] = 0;
   pd->vertex[12] = bounding_box.x + bounding_box.w;
   pd->vertex[13] = bounding_box.y + bounding_box.h;
   pd->vertex[14] = 0;
   pd->vertex[15] = bounding_box.x + bounding_box.w;
   pd->vertex[16] = bounding_box.y;
   pd->vertex[17] = 0;

   return r;
}

static Eina_Bool
_ector_renderer_gl_shape_ector_renderer_generic_base_draw(Eo *obj, Ector_Renderer_GL_Shape_Data *pd, Efl_Gfx_Render_Op op, Eina_Array *clips, unsigned int mul_col)
{
   uint64_t flags = 0;

   ector_renderer_draw(eo_super(obj, ECTOR_RENDERER_GL_SHAPE_CLASS), op, clips, mul_col);

   // FIXME: adjust flags content correctly
   // FIXME: should not ignore clips (idea is that the geometry will be cliped here and the
   // context will just look up clips for match with current pipe to render)...

   if (pd->shape->fill)
     {
        ector_renderer_gl_fill(pd->shape->fill, flags, pd->vertex, 6, mul_col);
     }
   else
     {
        ector_gl_surface_push(pd->base->surface, flags, pd->vertex, 6, mul_col);
     }

   return EINA_TRUE;
}

static Eina_Bool
_ector_renderer_gl_shape_ector_renderer_gl_fill(Eo *obj EINA_UNUSED,
                                                     Ector_Renderer_GL_Shape_Data *pd EINA_UNUSED,
                                                     uint64_t flags EINA_UNUSED,
                                                     GLshort *vertex EINA_UNUSED,
                                                     unsigned int vertex_count EINA_UNUSED,
                                                     unsigned int mul_col EINA_UNUSED)
{
   // FIXME: let's find out how to fill a shape with a shape later.
   // I need to read SVG specification and see what to do here.
   ERR("fill with shape not implemented\n");
   return EINA_FALSE;
}

static void
_ector_renderer_gl_shape_ector_renderer_generic_base_bounds_get(Eo *obj, Ector_Renderer_GL_Shape_Data *pd, Eina_Rectangle *r)
{
   efl_gfx_shape_bounds_get(obj, r);

   r->x += pd->base->origin.x;
   r->y += pd->base->origin.y;
}

static unsigned int
_ector_renderer_gl_shape_ector_renderer_generic_base_crc_get(Eo *obj, Ector_Renderer_GL_Shape_Data *pd)
{
   unsigned int crc;

   crc = ector_renderer_crc_get(eo_super(obj, ECTOR_RENDERER_GL_SHAPE_CLASS));

   // This code should be shared with other implementation
   crc = eina_crc((void*) &pd->shape->stroke.marker, sizeof (pd->shape->stroke.marker), crc, EINA_FALSE);
   crc = eina_crc((void*) &pd->public_shape->stroke.scale, sizeof (pd->public_shape->stroke.scale) * 3, crc, EINA_FALSE); // scale, width, centered
   crc = eina_crc((void*) &pd->public_shape->stroke.color, sizeof (pd->public_shape->stroke.color), crc, EINA_FALSE);
   crc = eina_crc((void*) &pd->public_shape->stroke.cap, sizeof (pd->public_shape->stroke.cap), crc, EINA_FALSE);
   crc = eina_crc((void*) &pd->public_shape->stroke.join, sizeof (pd->public_shape->stroke.join), crc, EINA_FALSE);

   if (pd->shape->fill) crc = _renderer_crc_get(pd->shape->fill, crc);
   if (pd->shape->stroke.fill) crc = _renderer_crc_get(pd->shape->stroke.fill, crc);
   if (pd->shape->stroke.marker) crc = _renderer_crc_get(pd->shape->stroke.marker, crc);
   if (pd->public_shape->stroke.dash_length)
     {
        crc = eina_crc((void*) pd->public_shape->stroke.dash, sizeof (Efl_Gfx_Dash) * pd->public_shape->stroke.dash_length, crc, EINA_FALSE);
     }

   return crc;
}

static Eo_Base *
_ector_renderer_gl_shape_eo_base_constructor(Eo *obj, Ector_Renderer_GL_Shape_Data *pd)
{
   obj = eo_constructor(eo_super(obj, ECTOR_RENDERER_GL_SHAPE_CLASS));

   if (!obj) return NULL;

   pd->public_shape = eo_data_xref(obj, EFL_GFX_SHAPE_MIXIN, obj);
   pd->shape = eo_data_xref(obj, ECTOR_RENDERER_GENERIC_SHAPE_MIXIN, obj);
   pd->base = eo_data_xref(obj, ECTOR_RENDERER_GENERIC_BASE_CLASS, obj);

   eo_event_callback_add(obj, EFL_GFX_PATH_CHANGED, _ector_renderer_gl_shape_path_changed, pd);

   return obj;
}

static void
_ector_renderer_gl_shape_eo_base_destructor(Eo *obj, Ector_Renderer_GL_Shape_Data *pd)
{
   eo_data_xunref(obj, pd->shape, obj);
   eo_data_xunref(obj, pd->base, obj);
   eo_data_xunref(obj, pd->public_shape, obj);
}

#include "ector_renderer_gl_shape.eo.c"
