#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include <Ector.h>

#include "ector_private.h"

static void
_ector_renderer_generic_base_eo_base_destructor(Eo *obj, Ector_Renderer_Generic_Base_Data *pd)
{
   if (pd->m) free(pd->m);
   eo_do_super(obj, ECTOR_RENDERER_GENERIC_BASE_CLASS, eo_destructor());
}

static void
_ector_renderer_generic_base_transformation_set(Eo *obj EINA_UNUSED,
                                                Ector_Renderer_Generic_Base_Data *pd,
                                                const Eina_Matrix3 *m)
{
   if (!m)
     {
        free(pd->m);
        pd->m = NULL;
     }
   else
     {
        if (!pd->m) pd->m = malloc(sizeof (Eina_Matrix3));
        if (!pd->m) return ;

        memcpy(pd->m, m, sizeof (Eina_Matrix3));
     }
}

static const Eina_Matrix3 *
_ector_renderer_generic_base_transformation_get(Eo *obj EINA_UNUSED,
                                                Ector_Renderer_Generic_Base_Data *pd)
{
   return pd->m;
}

static void
_ector_renderer_generic_base_origin_set(Eo *obj EINA_UNUSED,
                                        Ector_Renderer_Generic_Base_Data *pd,
                                        double x, double y)
{
   pd->origin.x = x;
   pd->origin.y = y;
}

static void
_ector_renderer_generic_base_origin_get(Eo *obj EINA_UNUSED,
                                        Ector_Renderer_Generic_Base_Data *pd,
                                        double *x, double *y)
{
   if (x) *x = pd->origin.x;
   if (y) *y = pd->origin.y;
}

static void
_ector_renderer_generic_base_visibility_set(Eo *obj EINA_UNUSED,
                                            Ector_Renderer_Generic_Base_Data *pd,
                                            Eina_Bool v)
{
   pd->visibility = v;
}

static Eina_Bool
_ector_renderer_generic_base_visibility_get(Eo *obj EINA_UNUSED,
                                            Ector_Renderer_Generic_Base_Data *pd)
{
   return pd->visibility;
}

static void
_ector_renderer_generic_base_color_set(Eo *obj EINA_UNUSED,
                                       Ector_Renderer_Generic_Base_Data *pd,
                                       int r, int g, int b, int a)
{
   pd->color.r = r;
   pd->color.g = g;
   pd->color.b = b;
   pd->color.a = a;
}

static void
_ector_renderer_generic_base_color_get(Eo *obj EINA_UNUSED,
                                       Ector_Renderer_Generic_Base_Data *pd,
                                       int *r, int *g, int *b, int *a)
{
   if (r) *r = pd->color.r;
   if (g) *g = pd->color.g;
   if (b) *b = pd->color.b;
   if (a) *a = pd->color.a;
}

static void
_ector_renderer_generic_base_mask_set(Eo *obj EINA_UNUSED,
                                      Ector_Renderer_Generic_Base_Data *pd,
                                      Ector_Renderer *r)
{
   _ector_renderer_replace(&pd->mask, r);
}

static Ector_Renderer *
_ector_renderer_generic_base_mask_get(Eo *obj EINA_UNUSED,
                                      Ector_Renderer_Generic_Base_Data *pd)
{
   return pd->mask;
}

static void
_ector_renderer_generic_base_quality_set(Eo *obj EINA_UNUSED,
                                         Ector_Renderer_Generic_Base_Data *pd,
                                         Ector_Quality q)
{
   pd->q = q;
}

static Ector_Quality
_ector_renderer_generic_base_quality_get(Eo *obj EINA_UNUSED,
                                         Ector_Renderer_Generic_Base_Data *pd)
{
   return pd->q;
}

static Eina_Bool
_ector_renderer_generic_base_prepare(Eo *obj EINA_UNUSED,
                                     Ector_Renderer_Generic_Base_Data *pd)
{
   if (pd->mask)
     eo_do(pd->mask, ector_renderer_prepare());

   return EINA_TRUE;
}

#include "ector_renderer_generic_base.eo.c"
