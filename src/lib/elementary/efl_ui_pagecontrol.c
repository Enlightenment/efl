#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_PACK_LAYOUT_PROTECTED


#include <Elementary.h>
#include "elm_priv.h"
#include "efl_ui_widget_pagecontrol.h"

#define MY_CLASS EFL_UI_PAGECONTROL_CLASS

#define MY_CLASS_NAME "Efl_Ui_Pagecontrol"


EOLIAN static Eo *
_efl_ui_pagecontrol_efl_object_constructor(Eo *obj,
                                           Efl_Ui_Pagecontrol_Data *sd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   return obj;
}

EOLIAN static void
_efl_ui_pagecontrol_efl_canvas_group_group_add(Eo *obj,
                                               Efl_Ui_Pagecontrol_Data *sd)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   efl_canvas_group_add(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);

   sd->cnt = 0;
   sd->page = 0;
   sd->ratio = 0.0;
   sd->orientation = EFL_ORIENT_HORIZONTAL;
}

EOLIAN static void
_efl_ui_pagecontrol_efl_gfx_size_set(Eo *obj,
                                     Efl_Ui_Pagecontrol_Data *sd,
                                     Evas_Coord w,
                                     Evas_Coord h)
{
   if ((sd->w == w) && (sd->h == h)) return;

   efl_gfx_size_set(efl_super(obj, MY_CLASS), w, h);

   sd->w = w;
   sd->h = h;

   efl_gfx_size_set(sd->event, w, h);
   efl_gfx_size_set(sd->viewport.foreclip, w, h);
   efl_gfx_size_set(sd->viewport.backclip, w, h);

   efl_ui_pagecontrol_update(obj);
}

EOLIAN static void
_efl_ui_pagecontrol_efl_gfx_position_set(Eo *obj,
                                         Efl_Ui_Pagecontrol_Data *sd,
                                         Evas_Coord x,
                                         Evas_Coord y)
{
   if ((sd->x == x) && (sd->y == y)) return;

   efl_gfx_position_set(efl_super(obj, MY_CLASS), x, y);

   sd->x = x;
   sd->y = y;

   efl_gfx_position_set(sd->event, x, y);
   efl_gfx_position_set(sd->viewport.foreclip, x, y);
   efl_gfx_position_set(sd->viewport.backclip, x, y);
}

EOLIAN static int
_efl_ui_pagecontrol_efl_container_content_count(Eo *obj EINA_UNUSED,
                                                Efl_Ui_Pagecontrol_Data *pd)
{
   return eina_list_count(pd->content_list);
}

EOLIAN static Eina_Bool
_efl_ui_pagecontrol_efl_pack_linear_pack_end(Eo *obj,
                                             Efl_Ui_Pagecontrol_Data *pd,
                                             Efl_Gfx *subobj)
{
   efl_parent_set(subobj, obj);
   pd->cnt += 1;

   pd->content_list = eina_list_append(pd->content_list, subobj);
   efl_gfx_stack_raise(pd->event);

   efl_ui_pagecontrol_update(obj);

   return EINA_TRUE;
}

EOLIAN static Efl_Gfx *
_efl_ui_pagecontrol_efl_pack_linear_pack_content_get(Eo *obj EINA_UNUSED,
                                                     Efl_Ui_Pagecontrol_Data *pd,
                                                     int index)
{
   return eina_list_nth(pd->content_list, index);
}

EOLIAN static int
_efl_ui_pagecontrol_efl_pack_linear_pack_index_get(Eo *obj EINA_UNUSED,
                                                   Efl_Ui_Pagecontrol_Data *pd,
                                                   const Efl_Gfx *subobj)
{
   return eina_list_data_idx(pd->content_list, (void *)subobj);
}

EOLIAN static void
_efl_ui_pagecontrol_update(Eo *obj,
                           Efl_Ui_Pagecontrol_Data *pd)
{
   ERR("%d %.2lf", pd->page, pd->ratio);
   efl_event_callback_call(obj, EFL_UI_PAGECONTROL_EVENT_UPDATED, NULL);
}


EOAPI EFL_VOID_FUNC_BODY(efl_ui_pagecontrol_update)

#define EFL_UI_PAGECONTROL_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_OPS(efl_ui_pagecontrol), \
   EFL_OBJECT_OP_FUNC(efl_ui_pagecontrol_update, _efl_ui_pagecontrol_update)

#include "efl_ui_pagecontrol.eo.c"
