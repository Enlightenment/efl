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

typedef struct {
   double xx; double yx;
   double xy; double yy;
   double x0; double y0;
} cairo_matrix_t;

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

typedef struct _Ector_Renderer_Cairo_Base_Data Ector_Renderer_Cairo_Base_Data;
struct _Ector_Renderer_Cairo_Base_Data
{
   Ector_Cairo_Surface_Data *parent;
   Ector_Renderer_Generic_Base_Data *generic;

   cairo_matrix_t *m;
};

static cairo_matrix_t identity;

// Cairo need unpremul color, so force unpremul here
void
_ector_renderer_cairo_base_ector_renderer_generic_base_color_set(Eo *obj EINA_UNUSED,
                                                                 Ector_Renderer_Cairo_Base_Data *pd,
                                                                 int r, int g, int b, int a)
{
   ector_color_argb_unpremul(a, &r ,&g, &b);
   pd->generic->color.r = r;
   pd->generic->color.g = g;
   pd->generic->color.b = b;
   pd->generic->color.a = a;
}

void
_ector_renderer_cairo_base_ector_renderer_generic_base_color_get(Eo *obj EINA_UNUSED,
                                                                 Ector_Renderer_Cairo_Base_Data *pd,
                                                                 int *r, int *g, int *b, int *a)
{
   if (r) *r = pd->generic->color.r;
   if (g) *g = pd->generic->color.g;
   if (b) *b = pd->generic->color.b;
   if (a) *a = pd->generic->color.a;

   ector_color_argb_premul(pd->generic->color.a, r, g, b);
}

static Eina_Bool
_ector_renderer_cairo_base_ector_renderer_generic_base_prepare(Eo *obj, Ector_Renderer_Cairo_Base_Data *pd)
{
   if (!pd->parent)
     {
        Eo *parent;

        eo_do(obj, parent = eo_parent_get());
        if (!parent) return EINA_FALSE;
        pd->parent = eo_data_xref(parent, ECTOR_CAIRO_SURFACE_CLASS, obj);
     }
   if (pd->generic->m)
     {
        USE(obj, cairo_matrix_init, EINA_FALSE);

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
_ector_renderer_cairo_base_ector_renderer_generic_base_draw(Eo *obj,
                                                            Ector_Renderer_Cairo_Base_Data *pd,
                                                            Ector_Rop op,
                                                            Eina_Array *clips EINA_UNUSED,
                                                            int x, int y,
                                                            unsigned int mul_col)
{
   double r, g, b, a;
   cairo_operator_t cop;
   double cx, cy;

   USE(obj, cairo_translate, EINA_FALSE);
   USE(obj, cairo_set_source_rgba, EINA_FALSE);
   USE(obj, cairo_transform, EINA_FALSE);
   USE(obj, cairo_set_operator, EINA_FALSE);

   switch (op)
     {
      case ECTOR_ROP_BLEND:
         cop = CAIRO_OPERATOR_OVER;
         break;
      case ECTOR_ROP_COPY:
      default:
         cop = CAIRO_OPERATOR_SOURCE;
         break;
     }

   r = ((double)((pd->generic->color.r * R_VAL(&mul_col)) >> 8)) / 255;
   g = ((double)((pd->generic->color.g * G_VAL(&mul_col)) >> 8)) / 255;
   b = ((double)((pd->generic->color.b * B_VAL(&mul_col)) >> 8)) / 255;
   a = ((double)((pd->generic->color.a * A_VAL(&mul_col)) >> 8)) / 255;

   cairo_set_operator(pd->parent->cairo, cop);
   cairo_transform(pd->parent->cairo, &identity);
   if (pd->m) cairo_transform(pd->parent->cairo, pd->m);
   cx = pd->generic->origin.x - pd->parent->current.x;
   cy = pd->generic->origin.y - pd->parent->current.y;
   cairo_translate(pd->parent->cairo, cx, cy);
   pd->parent->current.x = pd->generic->origin.x;
   pd->parent->current.y = pd->generic->origin.y;

   cairo_set_source_rgba(pd->parent->cairo, r, g, b, a);

   return EINA_TRUE;
}

static void
_ector_renderer_cairo_base_eo_base_constructor(Eo *obj, Ector_Renderer_Cairo_Base_Data *pd EINA_UNUSED)
{
   eo_do_super(obj, ECTOR_RENDERER_CAIRO_BASE_CLASS, eo_constructor());

   pd->generic = eo_data_xref(obj, ECTOR_RENDERER_GENERIC_BASE_CLASS, obj);

   USE(obj, cairo_matrix_init_identity, );

   cairo_matrix_init_identity(&identity);
}

static void
_ector_renderer_cairo_base_eo_base_destructor(Eo *obj, Ector_Renderer_Cairo_Base_Data *pd)
{
   Eo *parent;

   free(pd->m);

   eo_do(obj, parent = eo_parent_get());
   eo_data_xunref(parent, pd->parent, obj);
   eo_data_xunref(obj, pd->generic, obj);

   eo_do_super(obj, ECTOR_RENDERER_CAIRO_BASE_CLASS, eo_destructor());
}

#include "ector_renderer_cairo_base.eo.c"
