#define EFL_GFX_HINT_PROTECTED

#include "efl_ui_box_private.h"

#define MY_CLASS EFL_UI_BOX_STACK_CLASS

EOLIAN static void
_efl_ui_box_stack_efl_pack_layout_layout_update(Eo *obj, void *_pd EINA_UNUSED)
{
   Evas_Object_Box_Option *opt;
   Evas_Object_Box_Data *bd;
   Eina_Size2D min = { 0, 0 }, cmin;
   Eina_List *l;

   EINA_SAFETY_ON_FALSE_RETURN(efl_isa(obj, EFL_UI_BOX_CLASS));
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   bd = efl_data_scope_get(wd->resize_obj, EVAS_BOX_CLASS);

   evas_object_box_layout_stack(wd->resize_obj, bd, NULL);

   /* Note: legacy evas_object_box_layout_stack sets the box min size to be
    * the MAX() of the children's sizes, rather than their min sizes. By fixing
    * that, we can implement elm_win_resize_object_add() with a Efl.Ui.Box. */
   EINA_LIST_FOREACH(bd->children, l, opt)
     {
        Evas_Object *child = opt->obj;

        cmin = efl_gfx_hint_size_combined_min_get(child);
        if (cmin.w > min.w) min.w = cmin.w;
        if (cmin.h > min.h) min.h = cmin.h;
     }
   efl_gfx_hint_size_restricted_min_set(obj, min);
}

#include "efl_ui_box_stack.eo.c"
