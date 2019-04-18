#define EFL_GFX_HINT_PROTECTED

#include "efl_ui_box_private.h"
#include "efl_ui_container_layout.h"

#define MY_CLASS EFL_UI_BOX_STACK_CLASS

typedef struct _Item_Calc Item_Calc;

struct _Item_Calc
{
   Evas_Object *obj;
   Efl_Ui_Container_Item_Hints hints[2]; /* 0 is x-axis, 1 is y-axis */
};

EOLIAN static void
_efl_ui_box_stack_efl_pack_layout_layout_update(Eo *obj, void *_pd EINA_UNUSED)
{
   Efl_Ui_Box_Data *bd = efl_data_scope_get(obj, EFL_UI_BOX_CLASS);
   Eo *child;
   Efl_Ui_Container_Layout_Calc box_calc[2];
   Efl_Ui_Container_Item_Hints *hints;
   Item_Calc *items, *item;
   Eina_List *l;
   Eina_Size2D want = { 0, 0 };
   Evas_Object *old_child = NULL;
   int i = 0, count;

   count = eina_list_count(bd->children);
   if (!count)
     {
        efl_gfx_hint_size_restricted_min_set(obj, EINA_SIZE2D(0, 0));
        return;
     }

   _efl_ui_container_layout_init(obj, box_calc);

   items = alloca(count * sizeof(*items));
#ifdef DEBUG
   memset(items, 0, count * sizeof(*items));
#endif

   EINA_LIST_FOREACH(bd->children, l, child)
     {
        item = &items[i++];
        item->obj = child;
        hints = item->hints;

        _efl_ui_container_layout_item_init(child, hints);

        if (want.w < hints[0].space)
          want.w = hints[0].space;
        if (want.h < hints[1].space)
          want.h = hints[1].space;
     }

   if (box_calc[0].size < want.w)
     box_calc[0].size = want.w;
   if (box_calc[1].size < want.h)
     box_calc[1].size = want.h;

   for (i = 0; i < count; i++)
     {
        hints = items[i].hints;
        Eina_Rect item_geom;

        hints[0].space = box_calc[0].size -
                         (hints[0].margin[0] + hints[0].margin[1]);
        hints[1].space = box_calc[1].size -
                         (hints[1].margin[0] + hints[1].margin[1]);

        item_geom.w = ((hints[0].weight > 0) && hints[0].fill) ? hints[0].space : 0;
        item_geom.h = ((hints[1].weight > 0) && hints[1].fill) ? hints[1].space : 0;

        _efl_ui_container_layout_min_max_calc(hints, &item_geom.w, &item_geom.h,
                                (hints[0].aspect > 0) && (hints[1].aspect > 0));

        item_geom.x = box_calc[0].pos + hints[0].margin[0] +
                      (hints[0].space - item_geom.w) * hints[0].align;
        item_geom.y = box_calc[1].pos + hints[1].margin[0] +
                      (hints[1].space - item_geom.h) * hints[1].align;

        efl_gfx_entity_geometry_set(items[i].obj, item_geom);

        if (old_child)
          efl_gfx_stack_above(items[i].obj, old_child);
        old_child = items[i].obj;
     }

   want.w += (box_calc[0].margin[0] + box_calc[0].margin[1]);
   want.h += (box_calc[1].margin[0] + box_calc[1].margin[1]);

   efl_gfx_hint_size_restricted_min_set(obj, want);

   efl_event_callback_call(obj, EFL_PACK_EVENT_LAYOUT_UPDATED, NULL);
}

#include "efl_ui_box_stack.eo.c"
