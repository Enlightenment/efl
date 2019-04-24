#include "efl_ui_container_layout.h"

void
_efl_ui_container_layout_min_max_calc(Efl_Ui_Container_Item_Hints *item, int *cw, int *ch, Eina_Bool aspect_check)
{
   int w = *cw, h = *ch;
   Eina_Size2D aspect = { item[0].aspect, item[1].aspect };

   if (aspect_check)
     {
        w = h * aspect.w / aspect.h;
        if (w > *cw)
          {
             w = *cw;
             h = w * aspect.h / aspect.w;
          }
     }

   if (w > item[0].max)
     {
        w = item[0].max;
        if (aspect_check) h = w * aspect.h / aspect.w;
     }
   if (h > item[1].max)
     {
        h = item[1].max;
        if (aspect_check) w = h * aspect.w / aspect.h;
     }
   if (w < item[0].min)
     {
        w = item[0].min;
        if (aspect_check) h = w * aspect.h / aspect.w;
     }
   if (h < item[1].min)
     {
        h = item[1].min;
        if (aspect_check) w = h * aspect.w / aspect.h;
     }
   *cw = w;
   *ch = h;
}

void
_efl_ui_container_layout_item_init(Eo* o, Efl_Ui_Container_Item_Hints *item)
{
   Eina_Size2D max;
   Eina_Size2D min;
   Eina_Size2D aspect;
   Efl_Gfx_Hint_Aspect aspect_type;

   efl_gfx_hint_weight_get(o, &item[0].weight, &item[1].weight);
   efl_gfx_hint_align_get(o, &item[0].align, &item[1].align);
   efl_gfx_hint_margin_get(o, &item[0].margin[0], &item[0].margin[1],
                                   &item[1].margin[0], &item[1].margin[1]);
   efl_gfx_hint_fill_get(o, &item[0].fill, &item[1].fill);
   max = efl_gfx_hint_size_max_get(o);
   min = efl_gfx_hint_size_combined_min_get(o);
   efl_gfx_hint_aspect_get(o, &aspect_type, &aspect);
   item[0].aspect = aspect.w;
   item[1].aspect = aspect.h;
   item[0].aspect_type = aspect_type;
   item[1].aspect_type = aspect_type;

   if (EINA_DBL_EQ(item[0].align, -1))
     {
        item[0].align = 0.5;
        item[0].fill = EINA_TRUE;
     }
   else if (item[0].align < 0) item[0].align = 0;
   else if (item[0].align > 1) item[0].align = 1;
   if (EINA_DBL_EQ(item[1].align, -1))
     {
        item[1].align = 0.5;
        item[1].fill = EINA_TRUE;
     }
   else if (item[1].align < 0) item[1].align = 0;
   else if (item[1].align > 1) item[1].align = 1;

   if (min.w < 0) min.w = 0;
   if (min.h < 0) min.h = 0;

   if (max.w < 0) max.w = INT_MAX;
   if (max.h < 0) max.h = INT_MAX;

   item[0].max = max.w;
   item[1].max = max.h;
   item[0].min = min.w;
   item[1].min = min.h;

   if ((item[0].aspect <= 0) || (item[1].aspect <= 0))
     {
        if ((item[0].aspect <= 0) ^ (item[1].aspect <= 0))
          {
             ERR("Invalid aspect parameter for obj(%p)", o);
             item[0].aspect = item[1].aspect = 0;
             item[0].aspect_type = item[1].aspect_type = EFL_GFX_HINT_ASPECT_NONE;
          }
     }
   else
     {
        _efl_ui_container_layout_min_max_calc(item, &item[0].min, &item[1].min,
                                              EINA_TRUE);
     }


   item[0].space = item[0].min + item[0].margin[0] + item[0].margin[1];
   item[1].space = item[1].min + item[1].margin[0] + item[1].margin[1];
}

void
_efl_ui_container_layout_init(Eo* obj, Efl_Ui_Container_Layout_Calc *calc)
{
   Eina_Rect geom;
   Eina_Bool pad_scalable;

   geom = efl_gfx_entity_geometry_get(obj);
   efl_gfx_hint_margin_get(obj, &calc[0].margin[0], &calc[0].margin[1],
                                     &calc[1].margin[0], &calc[1].margin[1]);
   calc[0].scale = calc[1].scale = efl_gfx_entity_scale_get(obj);

   efl_gfx_arrangement_content_padding_get(obj, &calc[0].pad, &calc[1].pad, &pad_scalable);
   calc[0].pad = pad_scalable ? (calc[0].pad * calc[0].scale) : calc[0].pad;
   calc[1].pad = pad_scalable ? (calc[1].pad * calc[1].scale) : calc[1].pad;

   // pack align is used if "no item has a weight"
   efl_gfx_arrangement_content_align_get(obj, &calc[0].align, &calc[1].align);
   if (calc[0].align < 0)
     {
        calc[0].fill = EINA_TRUE;
        calc[0].align = 0.5;
     }
   else if (calc[0].align > 1)
     {
        calc[0].align = 1;
     }
   else
     {
        calc[0].fill = EINA_FALSE;
     }

   if (calc[1].align < 0)
     {
        calc[1].fill = EINA_TRUE;
        calc[1].align = 0.5;
     }
   else if (calc[1].align > 1)
     {
        calc[1].align = 1;
     }
   else
     {
        calc[1].fill = EINA_FALSE;
     }
   calc[0].pos = geom.x + calc[0].margin[0];
   calc[1].pos = geom.y + calc[1].margin[0];
   calc[0].size = geom.w - calc[0].margin[0] - calc[0].margin[1];
   calc[1].size = geom.h - calc[1].margin[0] - calc[1].margin[1];
}
