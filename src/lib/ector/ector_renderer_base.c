#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include <Ector.h>

#include "ector_private.h"

typedef struct _Ector_Renderer_Generic_Base_Data Ector_Renderer_Generic_Base_Data;
struct _Ector_Renderer_Generic_Base_Data
{
   Eina_Matrix3 *m;

   struct {
      double x;
      double y;
   } origin;

   struct {
      int r, g, b, a;
   } color;

   Ector_Renderer *mask;

   Ector_Quality q;
   Eina_Bool visibility;
};

void
_ector_renderer_base_transformation_set(Eo *obj EINA_UNUSED,
                                        Ector_Renderer_Generic_Base_Data *pd,
                                        const Eina_Matrix3 *m)
{
   Eina_Matrix3 *tmp = pd->m;

   pd->m = NULL;
   if (!m)
     {
        free(tmp);
        tmp = NULL;
     }
   else
     {
        if (!tmp) tmp = malloc(sizeof (Eina_Matrix3));
        if (!tmp) return ;

        memcpy(tmp, m, sizeof (Eina_Matrix3));
     }

   pd->m = tmp;
}

const Eina_Matrix3 *
_ector_renderer_base_transformation_get(Eo *obj EINA_UNUSED,
                                        Ector_Renderer_Generic_Base_Data *pd)
{
   return pd->m;
}

void
_ector_renderer_base_origin_set(Eo *obj EINA_UNUSED,
                                Ector_Renderer_Generic_Base_Data *pd,
                                double x, double y)
{
   pd->origin.x = x;
   pd->origin.y = y;
}

void
_ector_renderer_base_origin_get(Eo *obj EINA_UNUSED,
                                Ector_Renderer_Generic_Base_Data *pd,
                                double *x, double *y)
{
   if (x) *x = pd->origin.x;
   if (y) *y = pd->origin.y;
}

void
_ector_renderer_base_visibility_set(Eo *obj EINA_UNUSED,
                                    Ector_Renderer_Generic_Base_Data *pd,
                                    Eina_Bool v)
{
   pd->visibility = v;
}

Eina_Bool
_ector_renderer_base_visibility_get(Eo *obj EINA_UNUSED,
                                    Ector_Renderer_Generic_Base_Data *pd)
{
   return pd->visibility;
}

void
_ector_renderer_base_color_set(Eo *obj EINA_UNUSED,
                               Ector_Renderer_Generic_Base_Data *pd,
                               int r, int g, int b, int a)
{
   pd->color.r = r;
   pd->color.g = g;
   pd->color.b = b;
   pd->color.a = a;
}

void
_ector_renderer_base_color_get(Eo *obj EINA_UNUSED,
                               Ector_Renderer_Generic_Base_Data *pd,
                               int *r, int *g, int *b, int *a)
{
   if (r) *r = pd->color.r;
   if (g) *g = pd->color.g;
   if (b) *b = pd->color.b;
   if (a) *a = pd->color.a;
}

void
_ector_renderer_base_mask_set(Eo *obj EINA_UNUSED,
                              Ector_Renderer_Generic_Base_Data *pd,
                              Ector_Renderer *r)
{
   _ector_renderer_replace(&pd->mask, r);
}

Ector_Renderer *
_ector_renderer_base_mask_get(Eo *obj EINA_UNUSED,
                              Ector_Renderer_Generic_Base_Data *pd)
{
   return pd->mask;
}

void
_ector_renderer_base_quality_set(Eo *obj EINA_UNUSED,
                                 Ector_Renderer_Generic_Base_Data *pd,
                                 Ector_Quality q)
{
   pd->q = q;
}

Ector_Quality
_ector_renderer_base_quality_get(Eo *obj EINA_UNUSED,
                                 Ector_Renderer_Generic_Base_Data *pd)
{
   return pd->q;
}

#include "ector_renderer_generic_base.eo.c"
