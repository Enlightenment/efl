#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_UI_FOCUS_COMPOSITION_PROTECTED
#define EFL_UI_FOCUS_OBJECT_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"
#include "efl_ui_focus_composition_adapter.eo.h"

#define MY_CLASS EFL_UI_FOCUS_COMPOSITION_MIXIN

typedef struct {
   Eina_List *order, *targets_ordered;
   Eina_List *register_target, *registered_targets;
   Efl_Ui_Focus_Manager *registered, *old_manager;
   Eina_Bool dirty;
   Eina_Bool logical;
} Efl_Ui_Focus_Composition_Data;

static void
_state_apply(Eo *obj, Efl_Ui_Focus_Composition_Data *pd)
{
   Efl_Ui_Focus_Manager *manager;

   //Legacy code compatibility, only update the custom chain of elements if legacy was NOT messing with it.
   if (elm_widget_is_legacy(obj) && elm_object_focus_custom_chain_get(obj)) return;

   manager = pd->registered;

   if (manager)
     {
        Eina_List *n;
        Eina_List *safed = NULL;
        Efl_Ui_Focus_Object *o;

        pd->old_manager = manager;
        //remove all of them
        EINA_LIST_FREE(pd->registered_targets, o)
          {
             if (!!eina_list_data_find(pd->register_target, o))
               safed = eina_list_append(safed, o);
             else
               efl_ui_focus_manager_calc_unregister(manager, o);

             if (efl_isa(o, EFL_UI_FOCUS_COMPOSITION_ADAPTER_CLASS))
               {
                  efl_ui_focus_composition_adapter_focus_manager_parent_set(o, NULL);
                  efl_ui_focus_composition_adapter_focus_manager_object_set(o, NULL);
               }
          }
        pd->registered_targets = safed;

        EINA_LIST_FOREACH(pd->register_target, n, o)
          {
             if (!!eina_list_data_find(pd->registered_targets, o)) continue;

             if (!pd->logical)
               efl_ui_focus_manager_calc_register(manager, o, obj, NULL);
             else
               efl_ui_focus_manager_calc_register_logical(manager, o, obj, NULL);

             if (efl_isa(o, EFL_UI_FOCUS_COMPOSITION_ADAPTER_CLASS))
               {
                  efl_ui_focus_composition_adapter_focus_manager_parent_set(o, obj);
                  efl_ui_focus_composition_adapter_focus_manager_object_set(o, manager);
               }

             pd->registered_targets = eina_list_append(pd->registered_targets, o);
          }

        efl_ui_focus_manager_calc_update_order(manager, obj, eina_list_clone(pd->targets_ordered));
     }
   else
     {
        Efl_Ui_Focus_Object *o;

        EINA_LIST_FREE(pd->registered_targets, o)
          {
             efl_ui_focus_manager_calc_unregister(pd->old_manager, o);

             if (efl_isa(o, EFL_UI_FOCUS_COMPOSITION_ADAPTER_CLASS))
               {
                  efl_ui_focus_composition_adapter_focus_manager_parent_set(o, NULL);
                  efl_ui_focus_composition_adapter_focus_manager_object_set(o, NULL);
               }
          }
     }
}
static void
_invalidate(void *data, const Efl_Event *ev)
{
   Efl_Ui_Focus_Composition_Data *pd = efl_data_scope_get(data, EFL_UI_FOCUS_COMPOSITION_MIXIN);
   pd->register_target = eina_list_remove(pd->register_target, ev->object);
   pd->registered_targets = eina_list_remove(pd->registered_targets, ev->object);
}

EOLIAN static void
_efl_ui_focus_composition_composition_elements_set(Eo *obj, Efl_Ui_Focus_Composition_Data *pd, Eina_List *logical_order)
{
   Evas_Object *elem;
   Eina_List *n;

   pd->targets_ordered = eina_list_free(pd->targets_ordered);
   EINA_LIST_FREE(pd->register_target, elem)
     {
        efl_event_callback_del(elem, EFL_EVENT_INVALIDATE, _invalidate, obj);
     }

   pd->order = eina_list_free(pd->order);
   pd->order = logical_order;

   //now build a composition_elements list
   EINA_LIST_FOREACH(logical_order, n, elem)
     {
        Efl_Ui_Focus_Object *o = elem;

        EINA_SAFETY_ON_NULL_GOTO(elem, cont);

        if (!efl_isa(elem, EFL_UI_WIDGET_CLASS))
          {
             if (efl_isa(elem, EFL_UI_FOCUS_OBJECT_MIXIN))
               {
                  pd->register_target = eina_list_append(pd->register_target , o);
                  efl_event_callback_add(o, EFL_EVENT_INVALIDATE, _invalidate, obj);
               }
             else
               {
                  EINA_SAFETY_ERROR("List contains element that is not EFL_UI_FOCUS_OBJECT_MIXIN or EFL_GFX_ENTITY_INTERFACE or EFL_UI_WIDGET_CLASS");
                  continue;
               }
          }

        pd->targets_ordered = eina_list_append(pd->targets_ordered, o);
cont:
        continue;
     }
   _state_apply(obj, pd);
}

EOLIAN static Eina_List*
_efl_ui_focus_composition_composition_elements_get(const Eo *obj EINA_UNUSED, Efl_Ui_Focus_Composition_Data *pd)
{
   return eina_list_clone(pd->order);
}

EOLIAN static Eina_Bool
_efl_ui_focus_composition_efl_ui_widget_focus_state_apply(Eo *obj, Efl_Ui_Focus_Composition_Data *pd, Efl_Ui_Widget_Focus_State current_state, Efl_Ui_Widget_Focus_State *configured_state, Efl_Ui_Widget *redirect)
{
   Eina_Bool registered;

   configured_state->logical = EINA_TRUE;

   //shortcut for having the same configurations
   if (current_state.manager == configured_state->manager && !current_state.manager)
     return !!current_state.manager;

   if (configured_state->logical == current_state.logical &&
       configured_state->manager == current_state.manager &&
       configured_state->parent == current_state.parent)
     return !!current_state.manager;

   registered = efl_ui_widget_focus_state_apply(efl_super(obj, MY_CLASS), current_state, configured_state, redirect);

   if (registered)
     {
        if (efl_isa(obj, EFL_UI_FOCUS_MANAGER_INTERFACE))
          pd->registered = obj;
        else
          pd->registered = configured_state->manager;
     }
   else
     pd->registered = NULL;

   _state_apply(obj, pd);

   return registered;
}

EOLIAN static void
_efl_ui_focus_composition_dirty(Eo *obj EINA_UNUSED, Efl_Ui_Focus_Composition_Data *pd)
{
   pd->dirty = EINA_TRUE;
}

EOLIAN static void
_efl_ui_focus_composition_efl_ui_focus_object_setup_order_non_recursive(Eo *obj, Efl_Ui_Focus_Composition_Data *pd EINA_UNUSED)
{
   if (pd->dirty)
     {
        efl_ui_focus_composition_prepare(obj);
        pd->dirty = EINA_FALSE;
     }

   efl_ui_focus_object_setup_order_non_recursive(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_ui_focus_composition_logical_mode_set(Eo *obj EINA_UNUSED, Efl_Ui_Focus_Composition_Data *pd, Eina_Bool logical_mode)
{
   pd->logical = logical_mode;
}

EOLIAN static Eina_Bool
_efl_ui_focus_composition_logical_mode_get(const Eo *obj EINA_UNUSED, Efl_Ui_Focus_Composition_Data *pd)
{
   return pd->logical;
}

EOLIAN static void
_efl_ui_focus_composition_efl_object_invalidate(Eo *obj, Efl_Ui_Focus_Composition_Data *pd EINA_UNUSED)
{
   efl_invalidate(efl_super(obj, MY_CLASS));

   efl_ui_focus_composition_elements_set(obj, NULL);
}


#include "efl_ui_focus_composition.eo.c"

typedef struct {
   Evas_Object *object;
   Efl_Ui_Focus_Manager *manager;
   Efl_Ui_Focus_Object *parent;
}  Efl_Ui_Focus_Composition_Adapter_Data;

static void
_canvas_object_deleted(void *data, const Efl_Event *ev EINA_UNUSED)
{
  efl_ui_focus_composition_adapter_canvas_object_set(data, NULL);
}

static void
_new_geom(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eina_Rect rect = efl_gfx_entity_geometry_get(data);
   efl_event_callback_call(data, EFL_UI_FOCUS_OBJECT_EVENT_FOCUS_GEOMETRY_CHANGED, &rect);
}

EFL_CALLBACKS_ARRAY_DEFINE(canvas_obj,
    {EFL_GFX_ENTITY_EVENT_SIZE_CHANGED, _new_geom},
    {EFL_GFX_ENTITY_EVENT_POSITION_CHANGED, _new_geom},
    {EFL_EVENT_DEL, _canvas_object_deleted},
);

EOLIAN static void
_efl_ui_focus_composition_adapter_canvas_object_set(Eo *obj, Efl_Ui_Focus_Composition_Adapter_Data *pd, Efl_Canvas_Object *v)
{
   if (pd->object)
     {
        efl_event_callback_array_del(pd->object, canvas_obj(), obj);
     }
   pd->object = v;
   if (v)
     {
        efl_event_callback_array_add(pd->object, canvas_obj(), obj);
     }

}

EOLIAN static Efl_Canvas_Object*
_efl_ui_focus_composition_adapter_canvas_object_get(const Eo *obj EINA_UNUSED, Efl_Ui_Focus_Composition_Adapter_Data *pd)
{
   return pd->object;
}

EOLIAN static Eina_Rect
_efl_ui_focus_composition_adapter_efl_ui_focus_object_focus_geometry_get(const Eo *obj EINA_UNUSED, Efl_Ui_Focus_Composition_Adapter_Data *pd EINA_UNUSED)
{
   return efl_gfx_entity_geometry_get(pd->object);
}

EOLIAN static void
_efl_ui_focus_composition_adapter_efl_ui_focus_object_focus_set(Eo *obj EINA_UNUSED, Efl_Ui_Focus_Composition_Adapter_Data *pd, Eina_Bool focus)
{
   efl_ui_focus_object_focus_set(efl_super(obj, EFL_UI_FOCUS_COMPOSITION_ADAPTER_CLASS), focus);

   evas_object_focus_set(pd->object, efl_ui_focus_object_focus_get(obj));
}

EOLIAN static void
_efl_ui_focus_composition_adapter_efl_object_destructor(Eo *obj, Efl_Ui_Focus_Composition_Adapter_Data *pd EINA_UNUSED)
{
   efl_ui_focus_composition_adapter_canvas_object_set(obj, NULL);

   efl_destructor(efl_super(obj, EFL_UI_FOCUS_COMPOSITION_ADAPTER_CLASS));
}

EOLIAN static void
_efl_ui_focus_composition_adapter_focus_manager_object_set(Eo *obj EINA_UNUSED, Efl_Ui_Focus_Composition_Adapter_Data *pd, Efl_Ui_Focus_Manager *v)
{
   pd->manager = v;
}

EOLIAN static void
_efl_ui_focus_composition_adapter_focus_manager_parent_set(Eo *obj EINA_UNUSED, Efl_Ui_Focus_Composition_Adapter_Data *pd, Efl_Ui_Focus_Object *parent)
{
   pd->parent = parent;
}

EOLIAN static Efl_Ui_Focus_Object*
_efl_ui_focus_composition_adapter_efl_ui_focus_object_focus_parent_get(const Eo *obj EINA_UNUSED, Efl_Ui_Focus_Composition_Adapter_Data *pd)
{
   return pd->parent;
}

EOLIAN static Efl_Ui_Focus_Manager*
_efl_ui_focus_composition_adapter_efl_ui_focus_object_focus_manager_get(const Eo *obj EINA_UNUSED, Efl_Ui_Focus_Composition_Adapter_Data *pd)
{
   return pd->manager;
}


#include "efl_ui_focus_composition_adapter.eo.c"
