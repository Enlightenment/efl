#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_PROTECTED
#define EFL_UI_WIDGET_PART_BG_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "efl_ui_item_private.h"
//part
#include "efl_ui_item_part_text.eo.h"
#include "efl_ui_item_part_icon.eo.h"
#include "efl_ui_item_part_extra.eo.h"
#include "efl_ui_item_part_content.eo.h"
#include "elm_part_helper.h"

#define MY_CLASS      EFL_UI_ITEM_CLASS
#define MY_CLASS_PFX  efl_ui_item

#define MY_CLASS_NAME "Efl.Ui.Item"

/* Efl.Part */
EOLIAN static void
_efl_ui_item_part_text_efl_text_text_set(Eo *obj, void *pd EINA_UNUSED, const char *text)
{
   Elm_Part_Data *wd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   efl_text_set(efl_part(efl_super(wd->obj, MY_CLASS), wd->part), text);

   efl_layout_signal_emit(wd->obj, "efl,state,text,set", "efl");
   elm_layout_sizing_eval(wd->obj);
}

EOLIAN static const char*
_efl_ui_item_part_text_efl_text_text_get(const Eo *obj, void *pd EINA_UNUSED)
{
   Elm_Part_Data *wd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   return efl_text_get(efl_part(efl_super(wd->obj, MY_CLASS), wd->part));
}

EOLIAN static void
_efl_ui_item_part_text_efl_text_markup_markup_set(Eo *obj, void *pd EINA_UNUSED, const char *markup)
{
   Elm_Part_Data *wd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   efl_text_markup_set(efl_part(efl_super(wd->obj, MY_CLASS), wd->part), markup);

   efl_layout_signal_emit(obj, "efl,state,text,set", "efl");
   elm_layout_sizing_eval(obj);
}

EOLIAN static const char*
_efl_ui_item_part_text_efl_text_markup_markup_get(const Eo *obj, void *pd EINA_UNUSED)
{
   Elm_Part_Data *wd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   return efl_text_markup_get(efl_part(efl_super(wd->obj, MY_CLASS), wd->part));
}

EOLIAN static const char *
_efl_ui_item_part_text_efl_ui_l10n_l10n_text_get(const Eo *obj, void *_pd EINA_UNUSED, const char **domain)
{
   Elm_Part_Data *pd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   return elm_widget_part_translatable_text_get(pd->obj, pd->part, domain);
}

EOLIAN static void
_efl_ui_item_part_text_efl_ui_l10n_l10n_text_set(Eo *obj, void *_pd EINA_UNUSED, const char *text, const char *domain)
{
   Elm_Part_Data *pd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   elm_widget_part_translatable_text_set(pd->obj, pd->part, text, domain);
}
#include "efl_ui_item_part_text.eo.c"

/* Efl.Ui.List_Default_Item_Part_Icon */

Eina_Bool
_efl_ui_item_part_icon_efl_content_content_set(Eo *obj, void *pd EINA_UNUSED, Efl_Gfx_Entity *content)
{
   Elm_Part_Data *wd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   return efl_content_set(efl_part(efl_super(wd->obj, MY_CLASS), wd->part), content);
}

Efl_Gfx_Entity *
_efl_ui_item_part_icon_efl_content_content_get(const Eo *obj, void *pd EINA_UNUSED)
{
   Elm_Part_Data *wd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   return efl_content_get(efl_part(efl_super(wd->obj, MY_CLASS), wd->part));
}

Efl_Gfx_Entity *
_efl_ui_item_part_icon_efl_content_content_unset(Eo *obj, void *pd EINA_UNUSED)
{
   Elm_Part_Data *wd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   return efl_content_unset(efl_part(efl_super(wd->obj, MY_CLASS), wd->part));
}
#include "efl_ui_item_part_icon.eo.c"

Eina_Bool
_efl_ui_item_part_extra_efl_content_content_set(Eo *obj, void *pd EINA_UNUSED, Efl_Gfx_Entity *content)
{
   Elm_Part_Data *wd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   return efl_content_set(efl_part(efl_super(wd->obj, MY_CLASS), wd->part), content);
}

Efl_Gfx_Entity *
_efl_ui_item_part_extra_efl_content_content_get(const Eo *obj, void *pd EINA_UNUSED)
{
   Elm_Part_Data *wd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   return efl_content_get(efl_part(efl_super(wd->obj, MY_CLASS), wd->part));
}

Efl_Gfx_Entity *
_efl_ui_item_part_extra_efl_content_content_unset(Eo *obj, void *pd EINA_UNUSED)
{
   Elm_Part_Data *wd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   return efl_content_unset(efl_part(efl_super(wd->obj, MY_CLASS), wd->part));
}
#include "efl_ui_item_part_extra.eo.c"

Eina_Bool
_efl_ui_item_part_content_efl_content_content_set(Eo *obj, void *pd EINA_UNUSED, Efl_Gfx_Entity *content)
{
   Elm_Part_Data *wd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   return efl_content_set(efl_part(efl_super(wd->obj, MY_CLASS), wd->part), content);
}

Efl_Gfx_Entity *
_efl_ui_item_part_content_efl_content_content_get(const Eo *obj, void *pd EINA_UNUSED)
{
   Elm_Part_Data *wd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   return efl_content_get(efl_part(efl_super(wd->obj, MY_CLASS), wd->part));
}

Efl_Gfx_Entity *
_efl_ui_item_part_content_efl_content_content_unset(Eo *obj, void *pd EINA_UNUSED)
{
   Elm_Part_Data *wd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   return efl_content_unset(efl_part(efl_super(wd->obj, MY_CLASS), wd->part));
}
#include "efl_ui_item_part_content.eo.c"
/* Efl.Part */

static void
_item_select(Eo *obj, Efl_Ui_Item_Data *pd)
{
   if (pd->selected &&
       (*(pd->select_mode) != EFL_UI_SELECT_MODE_SINGLE_ALWAYS))
     return;
   if (*(pd->select_mode) == EFL_UI_SELECT_MODE_NONE) return;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   pd->selected = EINA_TRUE;
   edje_object_signal_emit(wd->resize_obj, "efl,state,selected", "efl");
   efl_event_callback_call(obj, EFL_UI_EVENT_ITEM_SELECTED, NULL);
}

static void
_item_unselect(Eo *obj, Efl_Ui_Item_Data *pd)
{
   if (!pd->selected) return;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   pd->selected = EINA_FALSE;
   edje_object_signal_emit(wd->resize_obj, "efl,state,unselected", "efl");
   efl_event_callback_call(obj, EFL_UI_EVENT_ITEM_UNSELECTED, NULL);
}

/* Mouse Controls */
static Eina_Bool
_item_longpressed(void *data)
{
   Efl_Ui_Item *item = data;
   EFL_UI_ITEM_DATA_GET_OR_RETURN(item, pd, ECORE_CALLBACK_CANCEL);

   pd->longpress_timer = NULL;

   efl_event_callback_call(item, EFL_UI_EVENT_LONGPRESSED, NULL);
   return ECORE_CALLBACK_CANCEL;
}

static void
_item_mouse_down(void *data,
                 Evas *evas EINA_UNUSED,
                 Evas_Object *obj EINA_UNUSED,
                 void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;
   Eo *item = data;
   EFL_UI_ITEM_DATA_GET_OR_RETURN(item, pd);
   ELM_WIDGET_DATA_GET_OR_RETURN(item, wd);
   if (wd->disabled) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;

   edje_object_signal_emit(wd->resize_obj, "efl,state,pressed", "efl");

   pd->longpress_timer = ecore_timer_add(_elm_config->longpress_timeout, _item_longpressed, item);
   efl_event_callback_call(item, EFL_UI_EVENT_PRESSED, NULL);
}

static void
_item_mouse_up(void *data,
               Evas *evas EINA_UNUSED,
               Evas_Object *obj EINA_UNUSED,
               void *event_info)
{
   Evas_Event_Mouse_Up *ev = event_info;
   Eo *item = data;
   EFL_UI_ITEM_DATA_GET_OR_RETURN(item, pd);
   ELM_WIDGET_DATA_GET_OR_RETURN(item, wd);
   if (wd->disabled) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD)
     {
        //FIXME: should we send this message to fallback?
        edje_object_signal_emit(wd->resize_obj, "efl,state,unpressed", "efl");
        //efl_event_callback_call(item, EFL_UI_EVENT_UNPRESSED, NULL);
        return;
     }

   if (pd->longpress_timer)
     {
        ecore_timer_del(pd->longpress_timer);
        pd->longpress_timer = NULL;
     }

   edje_object_signal_emit(wd->resize_obj, "efl,state,unpressed", "efl");
   efl_event_callback_call(item, EFL_UI_EVENT_UNPRESSED, NULL);

   if ((*(pd->select_mode) != EFL_UI_SELECT_MODE_SINGLE_ALWAYS) && (pd->selected))
     _item_unselect(item, pd);
   else if (*(pd->select_mode) != EFL_UI_SELECT_MODE_NONE)
     _item_select(item, pd);
}

static void
_item_mouse_move(void *data EINA_UNUSED,
                 Evas *evas EINA_UNUSED,
                 Evas_Object *obj EINA_UNUSED,
                 void *event_info)
{
   Evas_Event_Mouse_Move *ev = event_info;
   Eo *item = data;
   EFL_UI_ITEM_DATA_GET_OR_RETURN(item, pd);
   ELM_WIDGET_DATA_GET_OR_RETURN(item, wd);
   if (wd->disabled) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD)
     {
        edje_object_signal_emit(wd->resize_obj, "efl,state,unpressed", "efl");
        return;
     }
}

static void
_item_mouse_in(void *data EINA_UNUSED,
               Evas *evas EINA_UNUSED,
               Evas_Object *obj EINA_UNUSED,
               void *event_info)
{
   Evas_Event_Mouse_In *ev = event_info;
   Eo *item = data;
   EFL_UI_ITEM_DATA_GET_OR_RETURN(item, pd);
   ELM_WIDGET_DATA_GET_OR_RETURN(item, wd);
   if (wd->disabled) return;

   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;
}

static void
_item_mouse_callback_add(Eo *obj, void *data)
{
   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_MOUSE_DOWN, _item_mouse_down, data);
   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_MOUSE_UP, _item_mouse_up, data);
   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_MOUSE_MOVE, _item_mouse_move, data);
   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_MOUSE_IN, _item_mouse_in, data);
}

static void
_item_mouse_callback_del(Eo *obj, void *data)
{
   evas_object_event_callback_del_full
     (obj, EVAS_CALLBACK_MOUSE_DOWN, _item_mouse_down, data);
   evas_object_event_callback_del_full
     (obj, EVAS_CALLBACK_MOUSE_UP, _item_mouse_up, data);
   evas_object_event_callback_del_full
     (obj, EVAS_CALLBACK_MOUSE_MOVE, _item_mouse_move, data);
   evas_object_event_callback_del_full
     (obj, EVAS_CALLBACK_MOUSE_IN, _item_mouse_in, data);
}
/* Mouse Controls ends */

static void
_sizing_eval(Evas_Object *obj, Efl_Ui_Item_Data *pd)
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
_efl_ui_item_elm_layout_sizing_eval(Eo *obj, Efl_Ui_Item_Data *pd)
{
   if (pd->needs_size_calc) return;
   pd->needs_size_calc = EINA_TRUE;

   efl_canvas_group_change(obj);
}

EOLIAN static void
_efl_ui_item_efl_canvas_group_group_calculate(Eo *obj, Efl_Ui_Item_Data *pd)
{
   if (pd->needs_size_calc)
     {
        _sizing_eval(obj, pd);
        pd->needs_size_calc = EINA_FALSE;
     }
}

EOLIAN static Eo *
_efl_ui_item_efl_object_constructor(Eo *obj, Efl_Ui_Item_Data *pd)
{
   pd->obj = obj;
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   return obj;
}

EOLIAN static Efl_Object *
_efl_ui_item_efl_object_finalize(Eo *obj, Efl_Ui_Item_Data *pd EINA_UNUSED)
{
   Eo *eo;
   eo = efl_finalize(efl_super(obj, MY_CLASS));
   ELM_WIDGET_DATA_GET_OR_RETURN(eo, wd, eo);

   /* Support Item Focus Feature */
   elm_widget_can_focus_set(obj, EINA_TRUE);

   _item_mouse_callback_add(wd->resize_obj, eo);
   return eo;
}

EOLIAN static void
_efl_ui_item_efl_object_destructor(Eo *obj, Efl_Ui_Item_Data *pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   _item_mouse_callback_del(wd->resize_obj, obj);
   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static int
_efl_ui_item_index_get(const Eo *obj, Efl_Ui_Item_Data *pd)
{
   return efl_pack_index_get(pd->parent, obj);
}

EOLIAN static void
_efl_ui_item_selected_set(Eo *obj, Efl_Ui_Item_Data *pd, Eina_Bool select)
{
   Eina_Bool selected = !!select;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   if (wd->disabled) return;

   if (selected) _item_select(obj, pd);
   else _item_unselect(obj, pd);
}

EOLIAN static Eina_Bool
_efl_ui_item_selected_get(const Eo *obj EINA_UNUSED, Efl_Ui_Item_Data *pd)
{
   return pd->selected;
}

/* Internal EO APIs and hidden overrides */

#define EFL_UI_ITEM_EXTRA_OPS \
  ELM_LAYOUT_SIZING_EVAL_OPS(efl_ui_item)

#include "efl_ui_item.eo.c"
