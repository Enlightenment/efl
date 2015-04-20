#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <math.h>
#include <float.h>

#include <Eina.h>
#include <Ector.h>
#include <cairo/Ector_Cairo.h>

#include "ector_private.h"
#include "ector_cairo_private.h"

typedef struct _cairo_path_t cairo_path_t;

typedef enum _cairo_line_cap {
  CAIRO_LINE_CAP_BUTT,
  CAIRO_LINE_CAP_ROUND,
  CAIRO_LINE_CAP_SQUARE
} cairo_line_cap_t;

typedef enum _cairo_line_join {
  CAIRO_LINE_JOIN_MITER,
  CAIRO_LINE_JOIN_ROUND,
  CAIRO_LINE_JOIN_BEVEL
} cairo_line_join_t;

static void (*cairo_move_to)(cairo_t *cr, double x, double y) = NULL;
static void (*cairo_line_to)(cairo_t *cr, double x, double y) = NULL;
static void (*cairo_curve_to)(cairo_t *cr,
                              double x1, double y1,
                              double x2, double y2,
                              double x3, double y3) = NULL;
static void (*cairo_close_path)(cairo_t *cr) = NULL;

static void (*cairo_fill)(cairo_t *cr) = NULL;
static void (*cairo_fill_preserve)(cairo_t *cr) = NULL;
static void (*cairo_stroke)(cairo_t *cr) = NULL;

static void (*cairo_set_source_rgba)(cairo_t *cr,
                                     double red, double green,
                                     double blue, double alpha) = NULL;


static cairo_path_t *(*cairo_copy_path)(cairo_t *cr) = NULL;
static void (*cairo_path_destroy)(cairo_path_t *path) = NULL;
static void (*cairo_new_path)(cairo_t *cr) = NULL;
static void (*cairo_append_path)(cairo_t *cr, const cairo_path_t *path) = NULL;

static void (*cairo_set_line_width)(cairo_t *cr, double width) = NULL;
static void (*cairo_set_line_cap)(cairo_t *cr, cairo_line_cap_t line_cap) = NULL;
static void (*cairo_set_line_join)(cairo_t *cr, cairo_line_join_t line_join) = NULL;

static void (*cairo_save)(cairo_t *cr) = NULL;
static void (*cairo_restore)(cairo_t *cr) = NULL;

typedef struct _Ector_Renderer_Cairo_Shape_Data Ector_Renderer_Cairo_Shape_Data;
struct _Ector_Renderer_Cairo_Shape_Data
{
   Ector_Cairo_Surface_Data *parent;
   Ector_Renderer_Generic_Shape_Data *shape;
   Ector_Renderer_Generic_Base_Data *base;
   cairo_path_t *path;
};

static Eina_Bool
_ector_renderer_cairo_shape_path_changed(void *data, Eo *obj, const Eo_Event_Description *desc EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ector_Renderer_Cairo_Shape_Data *pd = data;

   USE(obj, cairo_path_destroy, EINA_TRUE);

   if (pd->path) cairo_path_destroy(pd->path);
   pd->path = NULL;

   return EINA_TRUE;
}

static Eina_Bool
_ector_renderer_cairo_shape_ector_renderer_generic_base_prepare(Eo *obj, Ector_Renderer_Cairo_Shape_Data *pd)
{
   const Efl_Gfx_Path_Command *cmds = NULL;
   const double *pts = NULL;

   eo_do_super(obj, ECTOR_RENDERER_CAIRO_SHAPE_CLASS, ector_renderer_prepare());

   if (pd->shape->fill)
     eo_do(pd->shape->fill, ector_renderer_prepare());
   if (pd->shape->stroke.fill)
     eo_do(pd->shape->stroke.fill, ector_renderer_prepare());
   if (pd->shape->stroke.marker)
     eo_do(pd->shape->stroke.marker, ector_renderer_prepare());

   // shouldn't that be moved to the cairo base object
   if (!pd->parent)
     {
        Eo *parent;

        eo_do(obj, parent = eo_parent_get());
        if (!parent) return EINA_FALSE;
        pd->parent = eo_data_xref(parent, ECTOR_CAIRO_SURFACE_CLASS, obj);
        if (!pd->parent) return EINA_FALSE;
     }

   eo_do(obj, efl_gfx_shape_path_get(&cmds, &pts));
   if (!pd->path && cmds)
     {
        USE(obj, cairo_new_path, EINA_FALSE);

        cairo_new_path(pd->parent->cairo);

        for (; *cmds != EFL_GFX_PATH_COMMAND_TYPE_END; cmds++)
          {
             switch (*cmds)
               {
                case EFL_GFX_PATH_COMMAND_TYPE_MOVE_TO:
                   USE(obj, cairo_move_to, EINA_FALSE);

                   cairo_move_to(pd->parent->cairo, pts[0], pts[1]);

                   pts += 2;
                   break;
                case EFL_GFX_PATH_COMMAND_TYPE_LINE_TO:
                   USE(obj, cairo_line_to, EINA_FALSE);

                   cairo_line_to(pd->parent->cairo, pts[0], pts[1]);

                   pts += 2;
                   break;
                case EFL_GFX_PATH_COMMAND_TYPE_CUBIC_TO:
                   USE(obj, cairo_curve_to, EINA_FALSE);

                   // Be careful, we do have a different order than
                   // cairo, first is destination point, followed by
                   // the control point. The opposite of cairo.
                   cairo_curve_to(pd->parent->cairo,
                                  pts[2], pts[3], pts[4], pts[5], // control points
                                  pts[0], pts[1]); // destination point

                   pts += 6;
                   break;
                case EFL_GFX_PATH_COMMAND_TYPE_CLOSE:
                   USE(obj, cairo_close_path, EINA_FALSE);

                   cairo_close_path(pd->parent->cairo);
                   break;
                case EFL_GFX_PATH_COMMAND_TYPE_LAST:
                case EFL_GFX_PATH_COMMAND_TYPE_END:
                   break;
               }
          }

        USE(obj, cairo_copy_path, EINA_FALSE);

        pd->path = cairo_copy_path(pd->parent->cairo);
     }

   return EINA_TRUE;
}

static Eina_Bool
_ector_renderer_cairo_shape_ector_renderer_generic_base_draw(Eo *obj, Ector_Renderer_Cairo_Shape_Data *pd, Ector_Rop op, Eina_Array *clips, unsigned int mul_col)
{
   int r, g, b, a;
   if (pd->path == NULL) return EINA_FALSE;

   USE(obj, cairo_save, EINA_FALSE);
   cairo_save(pd->parent->cairo);

   eo_do_super(obj, ECTOR_RENDERER_CAIRO_SHAPE_CLASS, ector_renderer_draw(op, clips, mul_col));

   USE(obj, cairo_new_path, EINA_FALSE);
   USE(obj, cairo_append_path, EINA_FALSE);

   cairo_new_path(pd->parent->cairo);
   cairo_append_path(pd->parent->cairo, pd->path);

   if (pd->shape->fill)
     eo_do(pd->shape->fill, ector_renderer_cairo_base_fill());

   if (pd->shape->stroke.fill || pd->shape->stroke.color.a > 0)
     {
        USE(obj, cairo_fill_preserve, EINA_FALSE);
        USE(obj, cairo_set_source_rgba, EINA_FALSE);
        USE(obj, cairo_stroke, EINA_FALSE);
        USE(obj, cairo_set_line_width, EINA_FALSE);
        USE(obj, cairo_set_line_cap, EINA_FALSE);
        USE(obj, cairo_set_line_join, EINA_FALSE);

        cairo_fill_preserve(pd->parent->cairo);

        if (pd->shape->stroke.fill)
          eo_do(pd->shape->stroke.fill, ector_renderer_cairo_base_fill());
       else
         {
            r = (((pd->shape->stroke.color.r * R_VAL(&mul_col)) + 0xff) >> 8);
            g = (((pd->shape->stroke.color.g * G_VAL(&mul_col)) + 0xff) >> 8);
            b = (((pd->shape->stroke.color.b * B_VAL(&mul_col)) + 0xff) >> 8);
            a = (((pd->shape->stroke.color.a * A_VAL(&mul_col)) + 0xff) >> 8);
            ector_color_argb_unpremul(a, &r, &g, &b);
            cairo_set_source_rgba(pd->parent->cairo, r/255.0, g/255.0, b/255.0, a/255.0);
         }

       // Set dash, cap and join
       cairo_set_line_width(pd->parent->cairo, (pd->shape->stroke.width * pd->shape->stroke.scale * 2));
       cairo_set_line_cap(pd->parent->cairo, pd->shape->stroke.cap);
       cairo_set_line_join(pd->parent->cairo, pd->shape->stroke.join);
       cairo_stroke(pd->parent->cairo);
     }
   else
     {
        USE(obj, cairo_fill, EINA_FALSE);
        cairo_fill(pd->parent->cairo);
     }

   USE(obj, cairo_restore, EINA_FALSE);
   cairo_restore(pd->parent->cairo);
   return EINA_TRUE;
}

static Eina_Bool
_ector_renderer_cairo_shape_ector_renderer_cairo_base_fill(Eo *obj EINA_UNUSED,
                                                           Ector_Renderer_Cairo_Shape_Data *pd EINA_UNUSED)
{
   // FIXME: let's find out how to fill a shape with a shape later.
   // I need to read SVG specification and see how to map that with cairo.
   ERR("fill with shape not implemented\n");
   return EINA_FALSE;
}

static void
_ector_renderer_cairo_shape_ector_renderer_generic_base_bounds_get(Eo *obj,
                                                                   Ector_Renderer_Cairo_Shape_Data *pd EINA_UNUSED,
                                                                   Eina_Rectangle *r)
{
   Ector_Renderer_Cairo_Base_Data *bd;

   // FIXME: It should be possible to actually ask cairo about that
   eo_do(obj, efl_gfx_shape_bounds_get(r));

   bd = eo_data_scope_get(obj, ECTOR_RENDERER_CAIRO_BASE_CLASS);
   r->x += bd->generic->origin.x;
   r->y += bd->generic->origin.y;
}

void
_ector_renderer_cairo_shape_eo_base_constructor(Eo *obj, Ector_Renderer_Cairo_Shape_Data *pd)
{
   eo_do_super(obj, ECTOR_RENDERER_CAIRO_SHAPE_CLASS, eo_constructor());
   pd->shape = eo_data_xref(obj, ECTOR_RENDERER_GENERIC_SHAPE_MIXIN, obj);
   pd->base = eo_data_xref(obj, ECTOR_RENDERER_GENERIC_BASE_CLASS, obj);

   eo_do(obj,
         eo_event_callback_add(EFL_GFX_PATH_CHANGED, _ector_renderer_cairo_shape_path_changed, pd));
}

void
_ector_renderer_cairo_shape_eo_base_destructor(Eo *obj, Ector_Renderer_Cairo_Shape_Data *pd)
{
   Eo *parent;

   eo_do(obj, parent = eo_parent_get());
   eo_data_xunref(parent, pd->parent, obj);

   eo_data_xunref(obj, pd->shape, obj);
   eo_data_xunref(obj, pd->base, obj);

   eo_do_super(obj, ECTOR_RENDERER_CAIRO_SHAPE_CLASS, eo_destructor());

   USE(obj, cairo_path_destroy, );
   if (pd->path) cairo_path_destroy(pd->path);
}


#include "ector_renderer_cairo_shape.eo.c"
