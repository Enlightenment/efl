#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_UI_FOCUS_COMPOSITION_ADAPTER_PROTECTED
#define ELM_WIDGET_ITEM_PROTECTED
#define MY_CLASS ELM_WIDGET_ITEM_STATIC_FOCUS_CLASS

#include <Elementary.h>
#include "elm_genlist.eo.h"
#include "elm_gengrid.eo.h"
#include "elm_priv.h"
#include "efl_ui_focus_composition_adapter.eo.h"

typedef struct {
   Eo *adapter;
   Eina_Bool realized;
} Elm_Widget_Item_Static_Focus_Data;

static void
_realized_set(Elm_Widget_Item_Static_Focus *f)
{
   Elm_Widget_Item_Static_Focus_Data *pd = efl_data_scope_get(f, MY_CLASS);

   pd->realized = EINA_TRUE;
}

static void
_list_realized_cb(void *data, const Efl_Event *ev)
{
   if (ev->info != data) return;

   _realized_set(data);

   if (!elm_object_item_disabled_get(data) &&
       elm_genlist_item_type_get(data) != ELM_GENLIST_ITEM_GROUP)
     efl_ui_focus_object_prepare_logical(data);
}

static void
_grid_realized_cb(void *data, const Efl_Event *ev)
{
   const Elm_Gen_Item_Class *itc;
   Eina_Bool is_group = EINA_FALSE;

   if (ev->info != data) return;

   _realized_set(data);

   itc = elm_gengrid_item_item_class_get(data);

   is_group = (itc && itc->item_style && !strcmp(itc->item_style, "group_index"));

   if (!elm_object_item_disabled_get(data) && !is_group)
     efl_ui_focus_object_prepare_logical(data);
}

static void
_unrealized_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Elm_Widget_Item_Static_Focus_Data *pd = efl_data_scope_get(data, MY_CLASS);

   if (ev->info != data) return;

   if (pd) /* if the obect is dead pd is NULL */
     {
        //only delete the adapter when not focused, this will lead to awfull artifacts
        if (!efl_ui_focus_object_focus_get(pd->adapter))
          {
             efl_del(pd->adapter);
          }
        pd->realized = EINA_FALSE;
     }
}

EOLIAN static void
_elm_widget_item_static_focus_efl_ui_focus_object_prepare_logical_none_recursive(Eo *obj, Elm_Widget_Item_Static_Focus_Data *pd EINA_UNUSED)
{
   Eo *logical_child;
   Elm_Widget_Item_Data *wpd = efl_data_scope_get(obj, ELM_WIDGET_ITEM_CLASS);

   efl_ui_focus_object_prepare_logical_none_recursive(efl_super(obj, MY_CLASS));

   if (!pd->realized)
     {
        WRN("This item is not realized, thus things will fall over, better return NOW");
        return;
     }

   logical_child = efl_ui_focus_manager_request_subchild(wpd->widget, obj);

   if (logical_child == pd->adapter)
     {
        Eo *next_widget;
        next_widget = efl_ui_focus_manager_request_move(wpd->widget, EFL_UI_FOCUS_DIRECTION_NEXT, logical_child, EINA_TRUE);

        if (efl_isa(next_widget, ELM_WIDGET_ITEM_STATIC_FOCUS_CLASS))
          {
             next_widget = NULL;
          }
        //check if this is the item block representation of genlist
        else if (efl_isa(next_widget, EFL_UI_FOCUS_COMPOSITION_ADAPTER_CLASS) && efl_ui_focus_object_focus_parent_get(next_widget) == wpd->widget)
          {
             next_widget = NULL;
          }
        logical_child = next_widget;
     }

   if (!logical_child)
     {
        if (!pd->adapter)
          {
             // parent has to stay the object, since this is used to get the item of a adapter
             pd->adapter = efl_add(EFL_UI_FOCUS_COMPOSITION_ADAPTER_CLASS, obj);
             efl_ui_focus_composition_adapter_focus_manager_parent_set(pd->adapter, wpd->widget);
             efl_ui_focus_composition_adapter_focus_manager_object_set(pd->adapter, wpd->widget);
             efl_wref_add(pd->adapter, &pd->adapter);
             efl_ui_focus_manager_calc_register(wpd->widget, pd->adapter, obj, NULL);
          }
     }
   else if (logical_child && logical_child != pd->adapter)
     {
        efl_ui_focus_manager_calc_unregister(wpd->widget, pd->adapter);
        efl_del(pd->adapter);
        pd->adapter = NULL;
     }

   //genlist sometimes changes views when doing quick scrolls so reset the view in every possible call
   if (pd->adapter)
     efl_ui_focus_composition_adapter_canvas_object_set(pd->adapter,  wpd->view);
}

EOLIAN static Efl_Object*
_elm_widget_item_static_focus_efl_object_constructor(Eo *obj, Elm_Widget_Item_Static_Focus_Data *pd EINA_UNUSED)
{
   Elm_Widget_Item_Data *wpd = efl_data_scope_get(obj, ELM_WIDGET_ITEM_CLASS);
   Eo *ret = efl_constructor(efl_super(obj, MY_CLASS));

   if (efl_isa(wpd->widget, ELM_GENLIST_CLASS))
     {
        efl_event_callback_add(wpd->widget, ELM_GENLIST_EVENT_REALIZED, _list_realized_cb, obj);
        efl_event_callback_add(wpd->widget, ELM_GENLIST_EVENT_UNREALIZED, _unrealized_cb, obj);
     }
   else
     {
        efl_event_callback_add(wpd->widget, ELM_GENGRID_EVENT_REALIZED, _grid_realized_cb, obj);
        efl_event_callback_add(wpd->widget, ELM_GENGRID_EVENT_UNREALIZED, _unrealized_cb, obj);
     }
   return ret;
}

EOLIAN static void
_elm_widget_item_static_focus_efl_object_destructor(Eo *obj, Elm_Widget_Item_Static_Focus_Data *pd EINA_UNUSED)
{
   Elm_Widget_Item_Data *wpd = efl_data_scope_get(obj, ELM_WIDGET_ITEM_CLASS);
   if (efl_isa(wpd->widget, ELM_GENLIST_CLASS))
     {
        efl_event_callback_del(wpd->widget, ELM_GENLIST_EVENT_REALIZED, _list_realized_cb, obj);
        efl_event_callback_del(wpd->widget, ELM_GENLIST_EVENT_UNREALIZED, _unrealized_cb, obj);
     }
   else
     {
        efl_event_callback_del(wpd->widget, ELM_GENGRID_EVENT_REALIZED, _grid_realized_cb, obj);
        efl_event_callback_del(wpd->widget, ELM_GENGRID_EVENT_UNREALIZED, _unrealized_cb, obj);
     }

   if (pd->adapter)
     efl_del(pd->adapter);

   return efl_destructor(efl_super(obj, MY_CLASS));
}


#include "elm_widget_item_static_focus.eo.c"
