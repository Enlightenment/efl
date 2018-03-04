#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

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
_realized_cb(void *data, const Efl_Event *ev)
{
   Elm_Widget_Item_Static_Focus_Data *pd = efl_data_scope_get(data, MY_CLASS);

   if (ev->info != data) return;

   pd->realized = EINA_TRUE;
   efl_ui_focus_object_prepare_logical(ev->object);
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
_elm_widget_item_static_focus_efl_ui_focus_object_prepare_logical(Eo *obj, Elm_Widget_Item_Static_Focus_Data *pd EINA_UNUSED)
{
   Eo *logical_child;
   Elm_Widget_Item_Data *wpd = efl_data_scope_get(obj, ELM_WIDGET_ITEM_CLASS);

   efl_ui_focus_object_prepare_logical(efl_super(obj, MY_CLASS));

   if (!pd->realized)
     {
        WRN("This item is not realized, thus things will fall over, better return NOW");
        return;
     }

   logical_child = efl_ui_focus_manager_request_subchild(wpd->widget, obj);

   if (!logical_child)
     {
        if (!pd->adapter)
          {
             // parent has to stay the object, since this is used to get the item of a adapter
             pd->adapter = efl_add(EFL_UI_FOCUS_COMPOSITION_ADAPTER_CLASS, obj);
             efl_ui_focus_composition_adapter_canvas_object_set(pd->adapter,  wpd->view);
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
}

EOLIAN static Efl_Object*
_elm_widget_item_static_focus_efl_object_constructor(Eo *obj, Elm_Widget_Item_Static_Focus_Data *pd EINA_UNUSED)
{
   Elm_Widget_Item_Data *wpd = efl_data_scope_get(obj, ELM_WIDGET_ITEM_CLASS);
   Eo *ret = efl_constructor(efl_super(obj, MY_CLASS));

   if (efl_isa(wpd->widget, ELM_GENLIST_CLASS))
     {
        efl_event_callback_add(wpd->widget, ELM_GENLIST_EVENT_REALIZED, _realized_cb, obj);
        efl_event_callback_add(wpd->widget, ELM_GENLIST_EVENT_UNREALIZED, _unrealized_cb, obj);
     }
   else
     {
        efl_event_callback_add(wpd->widget, ELM_GENGRID_EVENT_REALIZED, _realized_cb, obj);
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
        efl_event_callback_del(wpd->widget, ELM_GENLIST_EVENT_REALIZED, _realized_cb, obj);
        efl_event_callback_del(wpd->widget, ELM_GENLIST_EVENT_UNREALIZED, _unrealized_cb, obj);
     }
   else
     {
        efl_event_callback_del(wpd->widget, ELM_GENGRID_EVENT_REALIZED, _realized_cb, obj);
        efl_event_callback_del(wpd->widget, ELM_GENGRID_EVENT_UNREALIZED, _unrealized_cb, obj);
     }

   if (pd->adapter)
     efl_del(pd->adapter);

   return efl_destructor(efl_super(obj, MY_CLASS));
}


#include "elm_widget_item_static_focus.eo.c"
