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

typedef enum _cairo_fill_rule {
  CAIRO_FILL_RULE_WINDING,
  CAIRO_FILL_RULE_EVEN_ODD
} cairo_fill_rule_t;

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

static void (*cairo_set_dash) (cairo_t *cr, const double *dashes, int num_dashes, double offset) = NULL;
static void (*cairo_set_fill_rule) (cairo_t *cr, cairo_fill_rule_t fill_rule);

typedef struct _Ector_Renderer_Cairo_Shape_Data Ector_Renderer_Cairo_Shape_Data;
struct _Ector_Renderer_Cairo_Shape_Data
{
   Efl_Gfx_Shape_Public *public_shape;

   Ector_Cairo_Surface_Data *parent;
   Ector_Renderer_Shape_Data *shape;
   Ector_Renderer_Data *base;

   cairo_path_t *path;
};

EOLIAN static void
_ector_renderer_cairo_shape_efl_gfx_path_commit(Eo *obj EINA_UNUSED,
                                                Ector_Renderer_Cairo_Shape_Data *pd)
{
   if (pd->path)
     {
        cairo_path_destroy(pd->path);
        pd->path = NULL;
     }
}

static Eina_Bool
_ector_renderer_cairo_shape_ector_renderer_prepare(Eo *obj, Ector_Renderer_Cairo_Shape_Data *pd)
{
   const Efl_Gfx_Path_Command *cmds = NULL;
   const double *pts = NULL;

   ector_renderer_prepare(efl_super(obj, ECTOR_RENDERER_CAIRO_SHAPE_CLASS));

   // shouldn't this be moved to the cairo base object?
   if (!pd->parent)
     {
        Ector_Renderer_Data *base;

        base = efl_data_scope_get(obj, ECTOR_RENDERER_CLASS);
        pd->parent = efl_data_xref(base->surface, ECTOR_CAIRO_SURFACE_CLASS, obj);
     }

   efl_gfx_path_get(obj, &cmds, &pts);
   if (!pd->path && cmds)
     {
        cairo_new_path(pd->parent->cairo);

        for (; *cmds != EFL_GFX_PATH_COMMAND_TYPE_END; cmds++)
          {
             switch (*cmds)
               {
                case EFL_GFX_PATH_COMMAND_TYPE_MOVE_TO:
                   cairo_move_to(pd->parent->cairo, pts[0], pts[1]);

                   pts += 2;
                   break;
                case EFL_GFX_PATH_COMMAND_TYPE_LINE_TO:
                   cairo_line_to(pd->parent->cairo, pts[0], pts[1]);

                   pts += 2;
                   break;
                case EFL_GFX_PATH_COMMAND_TYPE_CUBIC_TO:

                   cairo_curve_to(pd->parent->cairo,
                                  pts[0], pts[1], pts[2], pts[3], // control points
                                  pts[4], pts[5]); // destination point

                   pts += 6;
                   break;
                case EFL_GFX_PATH_COMMAND_TYPE_CLOSE:
                   cairo_close_path(pd->parent->cairo);
                   break;
                case EFL_GFX_PATH_COMMAND_TYPE_LAST:
                case EFL_GFX_PATH_COMMAND_TYPE_END:
                   break;
               }
          }

        pd->path = cairo_copy_path(pd->parent->cairo);
     }

   return EINA_TRUE;
}

static Eina_Bool
_ector_renderer_cairo_shape_ector_renderer_draw(Eo *obj, Ector_Renderer_Cairo_Shape_Data *pd, Efl_Gfx_Render_Op op, Eina_Array *clips, unsigned int mul_col)
{
   int r, g, b, a;
   unsigned i;
   Efl_Gfx_Fill_Rule fill_rule;

   if (pd->path == NULL) return EINA_FALSE;

   cairo_save(pd->parent->cairo);

   ector_renderer_draw(efl_super(obj, ECTOR_RENDERER_CAIRO_SHAPE_CLASS), op, clips, mul_col);

   cairo_new_path(pd->parent->cairo);
   cairo_append_path(pd->parent->cairo, pd->path);

   fill_rule = efl_gfx_shape_fill_rule_get(obj);
   if (fill_rule == EFL_GFX_FILL_RULE_ODD_EVEN)
     cairo_set_fill_rule(pd->parent->cairo, CAIRO_FILL_RULE_EVEN_ODD);
  else
    cairo_set_fill_rule(pd->parent->cairo, CAIRO_FILL_RULE_WINDING);

   if (pd->shape->fill)
     ector_renderer_cairo_op_fill(pd->shape->fill, mul_col);

   if (pd->shape->stroke.fill || pd->public_shape->stroke.color.a > 0)
     {
        cairo_fill_preserve(pd->parent->cairo);

        if (pd->shape->stroke.fill)
          ector_renderer_cairo_op_fill(pd->shape->stroke.fill, mul_col);
       else
         {
            r = (((pd->public_shape->stroke.color.r * R_VAL(&mul_col)) + 0xff) >> 8);
            g = (((pd->public_shape->stroke.color.g * G_VAL(&mul_col)) + 0xff) >> 8);
            b = (((pd->public_shape->stroke.color.b * B_VAL(&mul_col)) + 0xff) >> 8);
            a = (((pd->public_shape->stroke.color.a * A_VAL(&mul_col)) + 0xff) >> 8);
            ector_color_argb_unpremul(a, &r, &g, &b);
            cairo_set_source_rgba(pd->parent->cairo, r/255.0, g/255.0, b/255.0, a/255.0);
            if (pd->public_shape->stroke.dash)
              {
                 double *dashinfo;

                 dashinfo = (double *) malloc(2 * pd->public_shape->stroke.dash_length * sizeof(double));
                 for (i = 0; i < pd->public_shape->stroke.dash_length; i++)
                   {
                      dashinfo[i*2] = pd->public_shape->stroke.dash[i].length;
                      dashinfo[i*2 + 1] = pd->public_shape->stroke.dash[i].gap;
                   }
                 cairo_set_dash(pd->parent->cairo, dashinfo, pd->public_shape->stroke.dash_length * 2, 0);
                 free(dashinfo);
              }
         }

       // Set dash, cap and join
       cairo_set_line_width(pd->parent->cairo, (pd->public_shape->stroke.width * pd->public_shape->stroke.scale));
       cairo_set_line_cap(pd->parent->cairo, (cairo_line_cap_t) pd->public_shape->stroke.cap);
       cairo_set_line_join(pd->parent->cairo, (cairo_line_join_t) pd->public_shape->stroke.join);
       cairo_stroke(pd->parent->cairo);
     }
   else
     {
        cairo_fill(pd->parent->cairo);
     }

   cairo_restore(pd->parent->cairo);
   return EINA_TRUE;
}

static Eina_Bool
_ector_renderer_cairo_shape_ector_renderer_cairo_op_fill(Eo *obj EINA_UNUSED,
                                                           Ector_Renderer_Cairo_Shape_Data *pd EINA_UNUSED,
                                                           unsigned int mul_col EINA_UNUSED)
{
   // FIXME: let's find out how to fill a shape with a shape later.
   // I need to read SVG specification and see how to map that with cairo.
   ERR("fill with shape not implemented\n");
   return EINA_FALSE;
}

static void
_ector_renderer_cairo_shape_efl_gfx_path_bounds_get(const Eo *obj,
                                                      Ector_Renderer_Cairo_Shape_Data *pd EINA_UNUSED,
                                                      Eina_Rect *r)
{
   Ector_Renderer_Cairo_Data *bd;

   // FIXME: It should be possible to ask cairo about this
   efl_gfx_path_bounds_get(obj, r);

   bd = efl_data_scope_get(obj, ECTOR_RENDERER_CAIRO_CLASS);
   r->x += bd->generic->origin.x;
   r->y += bd->generic->origin.y;
}

Eo *
_ector_renderer_cairo_shape_efl_object_constructor(Eo *obj, Ector_Renderer_Cairo_Shape_Data *pd)
{
   obj = efl_constructor(efl_super(obj, ECTOR_RENDERER_CAIRO_SHAPE_CLASS));

   if (!obj) return NULL;

   pd->public_shape = efl_data_xref(obj, EFL_GFX_SHAPE_MIXIN, obj);
   pd->shape = efl_data_xref(obj, ECTOR_RENDERER_SHAPE_MIXIN, obj);
   pd->base = efl_data_xref(obj, ECTOR_RENDERER_CLASS, obj);

   return obj;
}

static Efl_Object *
_ector_renderer_cairo_shape_efl_object_finalize(Eo *obj, Ector_Renderer_Cairo_Shape_Data *pd)
{
   obj = efl_finalize(efl_super(obj, ECTOR_RENDERER_CAIRO_SHAPE_CLASS));
   if (!obj) return NULL;

   USE(pd->base, cairo_path_destroy, NULL);
   USE(pd->base, cairo_restore, NULL);
   USE(pd->base, cairo_fill, NULL);
   USE(pd->base, cairo_set_dash, NULL);
   USE(pd->base, cairo_fill_preserve, NULL);
   USE(pd->base, cairo_set_source_rgba, NULL);
   USE(pd->base, cairo_stroke, NULL);
   USE(pd->base, cairo_set_line_width, NULL);
   USE(pd->base, cairo_set_line_cap, NULL);
   USE(pd->base, cairo_set_line_join, NULL);
   USE(pd->base, cairo_new_path, NULL);
   USE(pd->base, cairo_append_path, NULL);
   USE(pd->base, cairo_save, NULL);
   USE(pd->base, cairo_copy_path, NULL);
   USE(pd->base, cairo_close_path, NULL);
   USE(pd->base, cairo_curve_to, NULL);
   USE(pd->base, cairo_line_to, NULL);
   USE(pd->base, cairo_move_to, NULL);
   USE(pd->base, cairo_set_fill_rule, NULL);

   return obj;
}

void
_ector_renderer_cairo_shape_efl_object_destructor(Eo *obj, Ector_Renderer_Cairo_Shape_Data *pd)
{
   Ector_Renderer_Data *base;

   // FIXME: As base class destructor can't call destructor of mixin class.
   // Call explicit API to free shape data.
   efl_gfx_path_reset(obj);

   base = efl_data_scope_get(obj, ECTOR_RENDERER_CLASS);
   efl_data_xunref(base->surface, pd->parent, obj);

   efl_data_xunref(obj, pd->shape, obj);
   efl_data_xunref(obj, pd->base, obj);

   efl_destructor(efl_super(obj, ECTOR_RENDERER_CAIRO_SHAPE_CLASS));

   if (pd->path) cairo_path_destroy(pd->path);
}

unsigned int
_ector_renderer_cairo_shape_ector_renderer_crc_get(const Eo *obj,
                                                                Ector_Renderer_Cairo_Shape_Data *pd)
{
   unsigned int crc;

   crc = ector_renderer_crc_get(efl_super(obj, ECTOR_RENDERER_CAIRO_SHAPE_CLASS));

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

#include "ector_renderer_cairo_shape.eo.c"
