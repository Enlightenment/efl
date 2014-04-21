#ifdef HAVE_CONFIG_H
  #include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_widget.h"
#include "elm_priv.h"

#include "assert.h"

#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
#define ELM_INTERFACE_ATSPI_COMPONENT_PROTECTED

#include "elm_interface_atspi_accessible.h"
#include "elm_interface_atspi_accessible.eo.h"
#include "elm_interface_atspi_component.eo.h"
#include "elm_interface_atspi_widget.eo.h"

typedef struct _Elm_Interface_Atspi_Widget_Data Elm_Interface_Atspi_Widget_Data;

struct _Elm_Interface_Atspi_Widget_Data {
     Elm_Atspi_Role role;
     const char *description;
};

static void
_on_focus_change(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Eina_Bool val = data ? EINA_TRUE : EINA_FALSE;
   elm_interface_atspi_accessible_state_changed_signal_emit(obj, ELM_ATSPI_STATE_FOCUSED, val);
}

EOLIAN void
_elm_interface_atspi_widget_eo_base_constructor(Eo *obj EINA_UNUSED, Elm_Interface_Atspi_Widget_Data *pd)
{
   eo_do_super(obj, ELM_INTERFACE_ATSPI_WIDGET_CLASS, eo_constructor());

   pd->role = ELM_ATSPI_ROLE_UNKNOWN;

   // Elm_Widget_Access_Object can only be constructed on top of Elm_Widget
   assert(eo_isa(obj, ELM_OBJ_WIDGET_CLASS));

   evas_object_smart_callback_add(obj, "focused", _on_focus_change, (void*)1);
   evas_object_smart_callback_add(obj, "unfocused", _on_focus_change, NULL);
}

EOLIAN void
_elm_interface_atspi_widget_eo_base_destructor(Eo *obj EINA_UNUSED, Elm_Interface_Atspi_Widget_Data *pd)
{
   Eo *parent;
   if (pd->description) eina_stringshare_del(pd->description);

   eo_do(obj, parent = elm_interface_atspi_accessible_parent_get());

   if (parent && !eo_destructed_is(parent))
     elm_interface_atspi_accessible_children_changed_del_signal_emit(parent, obj);

   eo_do_super(obj, ELM_INTERFACE_ATSPI_WIDGET_CLASS, eo_destructor());
}

EOLIAN static Eina_Bool
_elm_interface_atspi_widget_elm_interface_atspi_component_focus_grab(Eo *obj, Elm_Interface_Atspi_Widget_Data *pd EINA_UNUSED)
{
   if (elm_object_focus_allow_get(obj))
     {
       Ecore_Evas *ee = ecore_evas_ecore_evas_get(evas_object_evas_get(obj));
       if (!ee) return EINA_FALSE;
       ecore_evas_activate(ee);
       elm_object_focus_set(obj, EINA_TRUE);
       return EINA_TRUE;
     }
   return EINA_FALSE;
}

EOLIAN static const char*
_elm_interface_atspi_widget_elm_interface_atspi_accessible_name_get(Eo *obj EINA_UNUSED, Elm_Interface_Atspi_Widget_Data *_pd EINA_UNUSED)
{
   return elm_object_text_get(obj);
}

EOLIAN static const char*
_elm_interface_atspi_widget_elm_interface_atspi_accessible_description_get(Eo *obj EINA_UNUSED, Elm_Interface_Atspi_Widget_Data *_pd)
{
   return _pd->description;
}

EOLIAN static void
_elm_interface_atspi_widget_elm_interface_atspi_accessible_description_set(Eo *obj EINA_UNUSED, Elm_Interface_Atspi_Widget_Data *_pd, const char *descr)
{
   eina_stringshare_replace(&_pd->description, descr);
}

EOLIAN static Elm_Atspi_Role
_elm_interface_atspi_widget_elm_interface_atspi_accessible_role_get(Eo *obj EINA_UNUSED, Elm_Interface_Atspi_Widget_Data *pd EINA_UNUSED)
{
   return pd->role;
}

EOLIAN static void
_elm_interface_atspi_widget_elm_interface_atspi_accessible_role_set(Eo *obj EINA_UNUSED, Elm_Interface_Atspi_Widget_Data *pd, Elm_Atspi_Role role)
{
   pd->role = role;
}

EOLIAN static Eina_List*
_elm_interface_atspi_widget_elm_interface_atspi_accessible_children_get(Eo *obj EINA_UNUSED, Elm_Interface_Atspi_Widget_Data *pd EINA_UNUSED)
{
   Eina_List *l, *accs = NULL;
   Elm_Widget_Smart_Data *wd;
   Evas_Object *widget;

   wd = eo_data_scope_get(obj, ELM_OBJ_WIDGET_CLASS);
   if (!wd) return NULL;

   EINA_LIST_FOREACH(wd->subobjs, l, widget)
     {
        if (!elm_object_widget_check(widget)) continue;
        if (eo_isa(widget, ELM_INTERFACE_ATSPI_ACCESSIBLE_CLASS))
          accs = eina_list_append(accs, widget);
     }
   return accs;
}

EOLIAN static Elm_Atspi_State_Set
_elm_interface_atspi_widget_elm_interface_atspi_accessible_state_set_get(Eo *obj, Elm_Interface_Atspi_Widget_Data *pd EINA_UNUSED)
{
   Elm_Atspi_State_Set states = 0;

   eo_do_super(obj, ELM_INTERFACE_ATSPI_WIDGET_CLASS, states = elm_interface_atspi_accessible_state_set_get());

   if (evas_object_visible_get(obj))
     {
        STATE_TYPE_SET(states, ELM_ATSPI_STATE_VISIBLE);
        STATE_TYPE_SET(states, ELM_ATSPI_STATE_SHOWING);
     }
   if (elm_object_focus_get(obj))
     STATE_TYPE_SET(states, ELM_ATSPI_STATE_FOCUSED);
   if (elm_object_focus_allow_get(obj))
     STATE_TYPE_SET(states, ELM_ATSPI_STATE_FOCUSABLE);
   if (!elm_object_disabled_get(obj))
     {
        STATE_TYPE_SET(states, ELM_ATSPI_STATE_ENABLED);
        STATE_TYPE_SET(states, ELM_ATSPI_STATE_ACTIVE);
     }

   return states;
}

#include "elm_interface_atspi_widget.eo.c"
