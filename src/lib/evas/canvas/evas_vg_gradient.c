#include "evas_common_private.h"
#include "evas_private.h"

#include "evas_vg_private.h"

#include <strings.h>

#define MY_CLASS EFL_CANVAS_VG_GRADIENT_CLASS

static void
_efl_canvas_vg_gradient_efl_gfx_gradient_stop_set(Eo *obj EINA_UNUSED,
                                                Efl_Canvas_Vg_Gradient_Data *pd,
                                                const Efl_Gfx_Gradient_Stop *colors,
                                                unsigned int length)
{
   pd->colors = realloc(pd->colors, length * sizeof(Efl_Gfx_Gradient_Stop));
   if (!pd->colors)
     {
        pd->colors_count = 0;
        return ;
     }

   memcpy(pd->colors, colors, length * sizeof(Efl_Gfx_Gradient_Stop));
   pd->colors_count = length;

   _efl_canvas_vg_node_changed(obj);
}

static void
_efl_canvas_vg_gradient_efl_gfx_gradient_stop_get(const Eo *obj EINA_UNUSED,
                                                Efl_Canvas_Vg_Gradient_Data *pd,
                                                const Efl_Gfx_Gradient_Stop **colors,
                                                unsigned int *length)
{
   if (colors) *colors = pd->colors;
   if (length) *length = pd->colors_count;
}

static void
_efl_canvas_vg_gradient_efl_gfx_gradient_spread_set(Eo *obj EINA_UNUSED,
                                                  Efl_Canvas_Vg_Gradient_Data *pd,
                                                  Efl_Gfx_Gradient_Spread s)
{
   pd->s = s;

   _efl_canvas_vg_node_changed(obj);
}

static Efl_Gfx_Gradient_Spread
_efl_canvas_vg_gradient_efl_gfx_gradient_spread_get(const Eo *obj EINA_UNUSED,
                                                  Efl_Canvas_Vg_Gradient_Data *pd)
{
   return pd->s;
}

static Eina_Bool
_efl_canvas_vg_gradient_efl_gfx_path_interpolate(Eo *obj,
                                         Efl_Canvas_Vg_Gradient_Data *pd,
                                         const Efl_VG *from, const Efl_VG *to,
                                         double pos_map)
{
   Efl_Canvas_Vg_Gradient_Data *fromd, *tod;
   Efl_Gfx_Gradient_Stop *colors;
   unsigned int i;
   double from_map;
   Eina_Bool r;

   r = efl_gfx_path_interpolate(efl_super(obj, EFL_CANVAS_VG_GRADIENT_CLASS), from, to, pos_map);

   fromd = efl_data_scope_get(from, EFL_CANVAS_VG_GRADIENT_CLASS);
   tod = efl_data_scope_get(to, EFL_CANVAS_VG_GRADIENT_CLASS);
   from_map = 1.0 - pos_map;

   if (!r) return EINA_FALSE;
   if (fromd->colors_count != tod->colors_count) return EINA_FALSE;

   colors = realloc(pd->colors, sizeof (Efl_Gfx_Gradient_Stop) * tod->colors_count);
   if (!colors) return EINA_FALSE;

   pd->colors = colors;

#define INTP(Pd, From, To, I, Member, From_Map, Pos_Map)                \
   Pd->colors[I].Member = From->colors[I].Member * From_Map + To->colors[I].Member * Pos_Map

   for (i = 0; i < fromd->colors_count; i++)
     {
        INTP(pd, fromd, tod, i, offset, from_map, pos_map);
        INTP(pd, fromd, tod, i, r, from_map, pos_map);
        INTP(pd, fromd, tod, i, g, from_map, pos_map);
        INTP(pd, fromd, tod, i, b, from_map, pos_map);
        INTP(pd, fromd, tod, i, a, from_map, pos_map);
     }

#undef INTP

   return EINA_TRUE;
}

static void
_efl_canvas_vg_gradient_efl_object_destructor(Eo *obj, Efl_Canvas_Vg_Gradient_Data *pd)
{
   if (pd->colors) free(pd->colors);

   efl_destructor(efl_super(obj, MY_CLASS));
}


EOLIAN static Efl_VG *
_efl_canvas_vg_gradient_efl_duplicate_duplicate(const Eo *obj, Efl_Canvas_Vg_Gradient_Data *pd)

{
   Efl_VG *cn = NULL;

   cn = efl_duplicate(efl_super(obj, MY_CLASS));
   efl_gfx_gradient_stop_set(cn, pd->colors, pd->colors_count);
   efl_gfx_gradient_spread_set(cn, pd->s);
   return cn;
}

EAPI void
evas_vg_gradient_stop_set(Eo *obj, const Efl_Gfx_Gradient_Stop *colors, unsigned int length)
{
   efl_gfx_gradient_stop_set(obj, colors, length);
}

EAPI void
evas_vg_gradient_stop_get(Eo *obj, const Efl_Gfx_Gradient_Stop **colors, unsigned int *length)
{
   efl_gfx_gradient_stop_get(obj, colors, length);
}

EAPI void
evas_vg_gradient_spread_set(Eo *obj, Efl_Gfx_Gradient_Spread s)
{
   efl_gfx_gradient_spread_set(obj, s);
}

EAPI Efl_Gfx_Gradient_Spread
evas_vg_gradient_spread_get(Eo *obj)
{
   return efl_gfx_gradient_spread_get(obj);
}

#include "efl_canvas_vg_gradient.eo.c"
