#include "efl_gfx_vg_value_provider.h"

#define MY_CLASS EFL_GFX_VG_VALUE_PROVIDER_CLASS

EOLIAN static Eo *
_efl_gfx_vg_value_provider_efl_object_constructor(Eo *obj,
                                           Efl_Gfx_Vg_Value_Provider_Data *pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   pd->flag = EFL_GFX_VG_VALUE_PROVIDER_FLAGS_NONE;

   return obj;
}

EOLIAN static void
_efl_gfx_vg_value_provider_efl_object_destructor(Eo *obj,
                                          Efl_Gfx_Vg_Value_Provider_Data *pd EINA_UNUSED)
{
   if (pd->keypath) eina_stringshare_del(pd->keypath);
   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN void
_efl_gfx_vg_value_provider_keypath_set(Eo *obj EINA_UNUSED, Efl_Gfx_Vg_Value_Provider_Data *pd, Eina_Stringshare *keypath)
{
   if(!keypath) return;
   eina_stringshare_replace(&pd->keypath, keypath);
}

EOLIAN Eina_Stringshare*
_efl_gfx_vg_value_provider_keypath_get(const Eo *obj EINA_UNUSED, Efl_Gfx_Vg_Value_Provider_Data *pd)
{
   return pd->keypath;
}

EOLIAN void
_efl_gfx_vg_value_provider_transform_set(Eo *obj EINA_UNUSED, Efl_Gfx_Vg_Value_Provider_Data *pd, Eina_Matrix4 *m)
{
   if (m)
     {
        if (!pd->m)
          {
             pd->m = malloc(sizeof (Eina_Matrix4));
             if (!pd->m) return;
          }
        pd->flag = pd->flag | EFL_GFX_VG_VALUE_PROVIDER_FLAGS_TRANSFORM_MATRIX;
        memcpy(pd->m, m, sizeof (Eina_Matrix4));
     }
   else
     {
        free(pd->m);
        pd->m = NULL;
     }
}

EOLIAN Eina_Matrix4*
_efl_gfx_vg_value_provider_transform_get(const Eo *obj EINA_UNUSED, Efl_Gfx_Vg_Value_Provider_Data *pd)
{
   return pd->m;
}

EOAPI void
_efl_gfx_vg_value_provider_fill_color_set(Eo *obj EINA_UNUSED, Efl_Gfx_Vg_Value_Provider_Data *pd, int r, int g, int b, int a)
{
   pd->flag = pd->flag | EFL_GFX_VG_VALUE_PROVIDER_FLAGS_FILL_COLOR;

   pd->fill.r = r;
   pd->fill.g = g;
   pd->fill.b = b;
   pd->fill.a = a;
}

EOAPI void
_efl_gfx_vg_value_provider_fill_color_get(const Eo *obj EINA_UNUSED, Efl_Gfx_Vg_Value_Provider_Data *pd, int *r, int *g, int *b, int *a)
{
   if (r) *r = pd->fill.r;
   if (g) *g = pd->fill.g;
   if (b) *b = pd->fill.b;
   if (a) *a = pd->fill.a;
}

EOAPI void
_efl_gfx_vg_value_provider_stroke_color_set(Eo *obj EINA_UNUSED, Efl_Gfx_Vg_Value_Provider_Data *pd, int r, int g, int b, int a)
{
   pd->flag = pd->flag | EFL_GFX_VG_VALUE_PROVIDER_FLAGS_STROKE_COLOR;

   pd->stroke.r = r;
   pd->stroke.g = g;
   pd->stroke.b = b;
   pd->stroke.a = a;
}

EOAPI void
_efl_gfx_vg_value_provider_stroke_color_get(const Eo *obj EINA_UNUSED, Efl_Gfx_Vg_Value_Provider_Data *pd, int *r, int *g, int *b, int *a)
{
   if (r) *r = pd->stroke.r;
   if (g) *g = pd->stroke.g;
   if (b) *b = pd->stroke.b;
   if (a) *a = pd->stroke.a;
}

EOAPI void
_efl_gfx_vg_value_provider_stroke_width_set(Eo *obj EINA_UNUSED, Efl_Gfx_Vg_Value_Provider_Data *pd, double w)
{
  if (w < 0) return ;

  pd->flag = pd->flag | EFL_GFX_VG_VALUE_PROVIDER_FLAGS_STROKE_WIDTH;
  pd->stroke.width = w;
}

EOAPI double
_efl_gfx_vg_value_provider_stroke_width_get(const Eo *obj EINA_UNUSED, Efl_Gfx_Vg_Value_Provider_Data *pd)
{
   return pd->stroke.width;
}

EOAPI Efl_Gfx_Vg_Value_Provider_Flags
_efl_gfx_vg_value_provider_updated_get(const Eo *obj EINA_UNUSED, Efl_Gfx_Vg_Value_Provider_Data *pd)
{
   return pd->flag;
}


#include "efl_gfx_vg_value_provider.eo.c"
