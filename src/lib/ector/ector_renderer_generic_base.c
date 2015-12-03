#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include <Ector.h>

#include "ector_private.h"

#define MY_CLASS ECTOR_RENDERER_GENERIC_BASE_CLASS

static void
_ector_renderer_generic_base_eo_base_destructor(Eo *obj, Ector_Renderer_Generic_Base_Data *pd)
{
   if (pd->m) free(pd->m);
   eo_unref(pd->surface);

   eo_do_super(obj, MY_CLASS, eo_destructor());
}

static Eo_Base *
_ector_renderer_generic_base_eo_base_finalize(Eo *obj, Ector_Renderer_Generic_Base_Data *pd)
{
   if (!pd->surface)
     {
        CRI("surface is not set yet, go fix your code!");
        return NULL;
     }
   pd->finalized = EINA_TRUE;
   return eo_do_super_ret(obj, MY_CLASS, obj, eo_finalize());
}

static Ector_Generic_Surface *
_ector_renderer_generic_base_surface_get(Eo *obj EINA_UNUSED, Ector_Renderer_Generic_Base_Data *pd)
{
   return pd->surface;
}

static void
_ector_renderer_generic_base_surface_set(Eo *obj EINA_UNUSED, Ector_Renderer_Generic_Base_Data *pd, Ector_Generic_Surface *s)
{
   if (pd->finalized)
     {
        CRI("surface_set can be called during object creation only!");
        return;
     }
   pd->surface = eo_xref(s, obj);
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
   _eo_refplace(&pd->mask, r);
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

static unsigned int
_ector_renderer_generic_base_crc_get(Eo *obj EINA_UNUSED,
                                     Ector_Renderer_Generic_Base_Data *pd)
{
   unsigned int crc;

   crc = eina_crc((void*) &pd->color, sizeof(pd->color), 0xffffffff, EINA_TRUE);
   crc = eina_crc((void*) &pd->q, sizeof(pd->q), crc, EINA_FALSE);
   crc = eina_crc((void*) &pd->origin, sizeof(pd->origin), crc, EINA_FALSE);

   if (pd->m) crc = eina_crc((void*) pd->m, sizeof(Eina_Matrix3), crc, EINA_FALSE);
   if (pd->mask) crc = _renderer_crc_get(pd->mask, crc);

   return crc;
}

#include "ector_renderer_generic_base.eo.c"
