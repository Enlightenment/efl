#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include <Ector.h>

#include "ector_private.h"

#define MY_CLASS ECTOR_RENDERER_CLASS

static void
_ector_renderer_efl_object_destructor(Eo *obj, Ector_Renderer_Data *pd)
{
   efl_destructor(efl_super(obj, MY_CLASS));

   if (pd->m) free(pd->m);
   efl_unref(pd->surface);
}

static Efl_Object *
_ector_renderer_efl_object_finalize(Eo *obj, Ector_Renderer_Data *pd)
{
   if (!pd->surface)
     {
        CRI("surface is not set yet, go fix your code!");
        return NULL;
     }
   pd->finalized = EINA_TRUE;
   return efl_finalize(efl_super(obj, MY_CLASS));
}

static Ector_Surface *
_ector_renderer_surface_get(const Eo *obj EINA_UNUSED, Ector_Renderer_Data *pd)
{
   return pd->surface;
}

static void
_ector_renderer_surface_set(Eo *obj EINA_UNUSED, Ector_Renderer_Data *pd, Ector_Surface *s)
{
   if (pd->finalized)
     {
        CRI("surface_set can be called during object creation only!");
        return;
     }
   pd->surface = efl_xref(s, obj);
}

static void
_ector_renderer_transformation_set(Eo *obj EINA_UNUSED,
                                                Ector_Renderer_Data *pd,
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
_ector_renderer_transformation_get(const Eo *obj EINA_UNUSED,
                                                Ector_Renderer_Data *pd)
{
   return pd->m;
}

static void
_ector_renderer_origin_set(Eo *obj EINA_UNUSED,
                                        Ector_Renderer_Data *pd,
                                        double x, double y)
{
   pd->origin.x = x;
   pd->origin.y = y;
}

static void
_ector_renderer_origin_get(const Eo *obj EINA_UNUSED,
                                        Ector_Renderer_Data *pd,
                                        double *x, double *y)
{
   if (x) *x = pd->origin.x;
   if (y) *y = pd->origin.y;
}

static void
_ector_renderer_visibility_set(Eo *obj EINA_UNUSED,
                                            Ector_Renderer_Data *pd,
                                            Eina_Bool v)
{
   pd->visibility = v;
}

static Eina_Bool
_ector_renderer_visibility_get(const Eo *obj EINA_UNUSED,
                                            Ector_Renderer_Data *pd)
{
   return pd->visibility;
}

static void
_ector_renderer_color_set(Eo *obj EINA_UNUSED,
                                       Ector_Renderer_Data *pd,
                                       int r, int g, int b, int a)
{
   pd->color.r = r;
   pd->color.g = g;
   pd->color.b = b;
   pd->color.a = a;
}

static void
_ector_renderer_color_get(const Eo *obj EINA_UNUSED,
                                       Ector_Renderer_Data *pd,
                                       int *r, int *g, int *b, int *a)
{
   if (r) *r = pd->color.r;
   if (g) *g = pd->color.g;
   if (b) *b = pd->color.b;
   if (a) *a = pd->color.a;
}

static void
_ector_renderer_mask_set(Eo *obj EINA_UNUSED,
                                      Ector_Renderer_Data *pd,
                                      Ector_Renderer *r)
{
   efl_replace(&pd->mask, r);
}

static Ector_Renderer *
_ector_renderer_mask_get(const Eo *obj EINA_UNUSED,
                                      Ector_Renderer_Data *pd)
{
   return pd->mask;
}

static Eina_Bool
_ector_renderer_prepare(Eo *obj EINA_UNUSED,
                                     Ector_Renderer_Data *pd)
{
   if (pd->mask)
     ector_renderer_prepare(pd->mask);

   return EINA_TRUE;
}

static unsigned int
_ector_renderer_crc_get(const Eo *obj EINA_UNUSED,
                                     Ector_Renderer_Data *pd)
{
   unsigned int crc;

   crc = eina_crc((void*) &pd->color, sizeof(pd->color), 0xffffffff, EINA_TRUE);
   crc = eina_crc((void*) &pd->origin, sizeof(pd->origin), crc, EINA_FALSE);

   if (pd->m) crc = eina_crc((void*) pd->m, sizeof(Eina_Matrix3), crc, EINA_FALSE);
   if (pd->mask) crc = _renderer_crc_get(pd->mask, crc);

   return crc;
}

#include "ector_renderer.eo.c"
