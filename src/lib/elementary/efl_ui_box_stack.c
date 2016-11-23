#define EFL_GFX_SIZE_HINT_PROTECTED

#include "efl_ui_box_private.h"

#define MY_CLASS EFL_UI_BOX_STACK_CLASS

EOLIAN static Eo *
_efl_ui_box_stack_efl_object_constructor(Eo *obj, void *_pd EINA_UNUSED)
{
   efl_constructor(efl_super(obj, MY_CLASS));
   efl_pack_layout_engine_set(obj, MY_CLASS, NULL);
   return obj;
}

EOLIAN static void
_efl_ui_box_stack_efl_pack_layout_layout_do(Efl_Class *klass EINA_UNUSED,
                                            void *_pd EINA_UNUSED, Eo *obj,
                                            const void *data EINA_UNUSED)
{
   Evas_Object_Box_Option *opt;
   Evas_Object_Box_Data *bd;
   int minw = 0, minh = 0;
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
        int mw = 0, mh = 0;

        efl_gfx_size_hint_combined_min_get(child, &mw, &mh);
        if (mw > minw) minw = mw;
        if (mh > minh) minh = mh;
     }
   efl_gfx_size_hint_restricted_min_set(obj, minw, minh);
}

#include "efl_ui_box_stack.eo.c"
