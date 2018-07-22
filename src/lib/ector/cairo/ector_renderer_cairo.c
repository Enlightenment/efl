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

typedef enum {
  CAIRO_OPERATOR_CLEAR,

  CAIRO_OPERATOR_SOURCE,
  CAIRO_OPERATOR_OVER,
  CAIRO_OPERATOR_IN,
  CAIRO_OPERATOR_OUT,
  CAIRO_OPERATOR_ATOP,

  CAIRO_OPERATOR_DEST,
  CAIRO_OPERATOR_DEST_OVER,
  CAIRO_OPERATOR_DEST_IN,
  CAIRO_OPERATOR_DEST_OUT,
  CAIRO_OPERATOR_DEST_ATOP,

  CAIRO_OPERATOR_XOR,
  CAIRO_OPERATOR_ADD,
  CAIRO_OPERATOR_SATURATE,

  CAIRO_OPERATOR_MULTIPLY,
  CAIRO_OPERATOR_SCREEN,
  CAIRO_OPERATOR_OVERLAY,
  CAIRO_OPERATOR_DARKEN,
  CAIRO_OPERATOR_LIGHTEN,
  CAIRO_OPERATOR_COLOR_DODGE,
  CAIRO_OPERATOR_COLOR_BURN,
  CAIRO_OPERATOR_HARD_LIGHT,
  CAIRO_OPERATOR_SOFT_LIGHT,
  CAIRO_OPERATOR_DIFFERENCE,
  CAIRO_OPERATOR_EXCLUSION,
  CAIRO_OPERATOR_HSL_HUE,
  CAIRO_OPERATOR_HSL_SATURATION,
  CAIRO_OPERATOR_HSL_COLOR,
  CAIRO_OPERATOR_HSL_LUMINOSITY
} cairo_operator_t;

static void (*cairo_translate)(cairo_t *cr, double tx, double ty) = NULL;
static void (*cairo_matrix_init)(cairo_matrix_t *matrix,
                                 double xx, double yx,
                                 double xy, double yy,
                                 double x0, double y0) = NULL;
static void (*cairo_transform)(cairo_t *cr, const cairo_matrix_t *matrix) = NULL;
static void (*cairo_set_source_rgba)(cairo_t *cr,
                                     double red, double green, double blue,
                                     double alpha) = NULL;
static void (*cairo_set_operator)(cairo_t *cr, cairo_operator_t op) = NULL;
static void (*cairo_matrix_init_identity)(cairo_matrix_t *matrix) = NULL;

static void (*cairo_new_path)(cairo_t *cr) = NULL;
static void (*cairo_rectangle)(cairo_t *cr, double x, double y, double width, double height) = NULL;
static void (*cairo_clip)(cairo_t *cr) = NULL;
static void (*cairo_device_to_user)(cairo_t *cr, double *x, double *y) = NULL;

// This routine is shared for all gradient types to use
void (*cairo_pattern_add_color_stop_rgba)(cairo_pattern_t *pattern, double offset,
                                          double red, double green, double blue, double alpha) = NULL;

static cairo_matrix_t identity;

// Cairo needs unpremultiplied color, so force unpremul here
void
_ector_renderer_cairo_ector_renderer_color_set(Eo *obj EINA_UNUSED,
                                                                 Ector_Renderer_Cairo_Data *pd,
                                                                 int r, int g, int b, int a)
{
   pd->generic->color.r = r;
   pd->generic->color.g = g;
   pd->generic->color.b = b;
   pd->generic->color.a = a;
}

void
_ector_renderer_cairo_ector_renderer_color_get(const Eo *obj EINA_UNUSED,
                                                                 Ector_Renderer_Cairo_Data *pd,
                                                                 int *r, int *g, int *b, int *a)
{
   if (r) *r = pd->generic->color.r;
   if (g) *g = pd->generic->color.g;
   if (b) *b = pd->generic->color.b;
   if (a) *a = pd->generic->color.a;
}

static Eina_Bool
_ector_renderer_cairo_ector_renderer_prepare(Eo *obj, Ector_Renderer_Cairo_Data *pd)
{
   if (!pd->parent)
     {
        pd->parent = efl_data_xref(pd->generic->surface, ECTOR_CAIRO_SURFACE_CLASS, obj);
     }
   if (pd->generic->m)
     {
        if (!pd->m) pd->m = malloc(sizeof (cairo_matrix_t));
        cairo_matrix_init(pd->m,
                          pd->generic->m->xx, pd->generic->m->yx,
                          pd->generic->m->xy, pd->generic->m->yy,
                          pd->generic->m->xz, pd->generic->m->yz);
     }
   else
     {
        free(pd->m);
        pd->m = NULL;
     }

   return EINA_TRUE;
}

static Eina_Bool
_ector_renderer_cairo_ector_renderer_draw(Eo *obj EINA_UNUSED,
                                                            Ector_Renderer_Cairo_Data *pd,
                                                            Efl_Gfx_Render_Op op,
                                                            Eina_Array *clips EINA_UNUSED,
                                                            unsigned int mul_col)
{
   int r, g, b, a;
   cairo_operator_t cop;
   double cx, cy;

   switch (op)
     {
      case EFL_GFX_RENDER_OP_BLEND:
         cop = CAIRO_OPERATOR_OVER;
         break;
      case EFL_GFX_RENDER_OP_COPY:
      default:
         cop = CAIRO_OPERATOR_SOURCE;
         break;
     }

   r = ((pd->generic->color.r * R_VAL(&mul_col)) >> 8);
   g = ((pd->generic->color.g * G_VAL(&mul_col)) >> 8);
   b = ((pd->generic->color.b * B_VAL(&mul_col)) >> 8);
   a = ((pd->generic->color.a * A_VAL(&mul_col)) >> 8);
   ector_color_argb_unpremul(a, &r, &g, &b);

   cairo_set_operator(pd->parent->cairo, cop);

   if (clips)
     {
        int clip_count =  eina_array_count(clips);
        int i=0;
        for (; i < clip_count ; i++)
          {
             Eina_Rectangle *clip = (Eina_Rectangle *)eina_array_data_get(clips, i);
             double x = (double)clip->x;
             double y = (double)clip->y;

             cairo_new_path(pd->parent->cairo);
             cairo_device_to_user(pd->parent->cairo, &x, &y);
             cairo_rectangle(pd->parent->cairo, x, y, clip->w, clip->h);
          }
        cairo_clip(pd->parent->cairo);
     }

   cairo_transform(pd->parent->cairo, &identity);
   cx = pd->generic->origin.x + pd->parent->current.x;
   cy = pd->generic->origin.y + pd->parent->current.y;

   cairo_translate(pd->parent->cairo, cx, cy);

   if (pd->m) cairo_transform(pd->parent->cairo, pd->m);
   else cairo_transform(pd->parent->cairo, &identity);

   cairo_set_source_rgba(pd->parent->cairo, r/255.0, g/255.0, b/255.0, a/255.0);

   return EINA_TRUE;
}

static Eo *
_ector_renderer_cairo_efl_object_constructor(Eo *obj, Ector_Renderer_Cairo_Data *pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, ECTOR_RENDERER_CAIRO_CLASS));
   if (!obj) return NULL;

   pd->generic = efl_data_xref(obj, ECTOR_RENDERER_CLASS, obj);

   return obj;
}

static Efl_Object *
_ector_renderer_cairo_efl_object_finalize(Eo *obj, Ector_Renderer_Cairo_Data *pd)
{
   obj = efl_finalize(efl_super(obj, ECTOR_RENDERER_CAIRO_CLASS));
   if (!obj) return NULL;

   USE(pd->generic, cairo_matrix_init, NULL);
   USE(pd->generic, cairo_translate, NULL);
   USE(pd->generic, cairo_set_source_rgba, NULL);
   USE(pd->generic, cairo_transform, NULL);
   USE(pd->generic, cairo_set_operator, NULL);
   USE(pd->generic, cairo_new_path, NULL);
   USE(pd->generic, cairo_rectangle, NULL);
   USE(pd->generic, cairo_clip, NULL);
   USE(pd->generic, cairo_device_to_user, NULL);
   USE(pd->generic, cairo_matrix_init_identity, NULL);

   cairo_matrix_init_identity(&identity);

   return obj;
}

static void
_ector_renderer_cairo_efl_object_destructor(Eo *obj, Ector_Renderer_Cairo_Data *pd)
{
   Ector_Renderer_Data *base;

   base = efl_data_scope_get(obj, ECTOR_RENDERER_CLASS);
   efl_data_xunref(base->surface, pd->parent, obj);
   efl_data_xunref(obj, pd->generic, obj);

   free(pd->m);

   efl_destructor(efl_super(obj, ECTOR_RENDERER_CAIRO_CLASS));
}

#include "ector_renderer_cairo.eo.c"
