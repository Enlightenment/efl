#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_PROTECTED
#define EFL_UI_WIDGET_PART_BG_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "efl_ui_grid_item_private.h"

#define MY_CLASS      EFL_UI_GRID_ITEM_CLASS
#define MY_CLASS_PFX  efl_ui_grid_item

#define MY_CLASS_NAME "Efl.Ui.Grid_Item"

static void
_sizing_eval(Evas_Object *obj, Efl_Ui_Grid_Item_Data *pd)
{
   Evas_Coord minh = -1, minw = -1;
   Evas_Coord rest_w = 0, rest_h = 0;
   ELM_WIDGET_DATA_GET_OR_RETURN(pd->obj, wd);

   edje_object_size_min_restricted_calc(wd->resize_obj, &minw, &minh,
                                        rest_w, rest_h);
   evas_object_size_hint_min_set(obj, minw, minh);

   pd->needs_size_calc = EINA_FALSE;
}

static void
_efl_ui_grid_item_elm_layout_sizing_eval(Eo *obj, Efl_Ui_Grid_Item_Data *pd)
{
   if (pd->needs_size_calc) return;
   pd->needs_size_calc = EINA_TRUE;

   efl_canvas_group_change(obj);
}

EOLIAN static void
_efl_ui_grid_item_efl_canvas_group_group_calculate(Eo *obj, Efl_Ui_Grid_Item_Data *pd)
{
   if (pd->needs_size_calc)
     {
        _sizing_eval(obj, pd);
        pd->needs_size_calc = EINA_FALSE;
     }
}

EOLIAN static Eo *
_efl_ui_grid_item_efl_object_constructor(Eo *obj, Efl_Ui_Grid_Item_Data *pd)
{
   pd->obj = obj;
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   return obj;
}

EOLIAN static Efl_Object *
_efl_ui_grid_item_efl_object_finalize(Eo *obj, Efl_Ui_Grid_Item_Data *pd EINA_UNUSED)
{
   Eo *eo;
   eo = efl_finalize(efl_super(obj, MY_CLASS));
   ELM_WIDGET_DATA_GET_OR_RETURN(eo, wd, eo);

   return eo;
}

EOLIAN static void
_efl_ui_grid_item_efl_object_destructor(Eo *obj, Efl_Ui_Grid_Item_Data *pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   efl_destructor(efl_super(obj, MY_CLASS));
}

/* Internal EO APIs and hidden overrides */

#define EFL_UI_GRID_ITEM_EXTRA_OPS \
  ELM_LAYOUT_SIZING_EVAL_OPS(efl_ui_grid_item)

#include "efl_ui_grid_item.eo.c"
