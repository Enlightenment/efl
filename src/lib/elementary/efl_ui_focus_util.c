#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"

typedef struct {

} Efl_Ui_Focus_Util_Data;

static void
_manager_changed(void *data, const Efl_Event *event EINA_UNUSED)
{
   efl_ui_focus_util_focus(EFL_UI_FOCUS_UTIL_CLASS, data);
}

EOLIAN static void
_efl_ui_focus_util_focus(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, Efl_Ui_Focus_Object *user)
{
   Efl_Ui_Focus_Object *entry;
   Efl_Ui_Widget *top, *o;
   Efl_Ui_Focus_Manager *m, *registered_manager;

   top = elm_widget_top_get(user);

   o = efl_key_data_get(top, "__delayed_focus_set");
   if (o) efl_event_callback_del(o, EFL_UI_FOCUS_OBJECT_EVENT_MANAGER_CHANGED, _manager_changed, o);
   efl_key_data_set(top, "__delayed_focus_set", NULL);

   registered_manager = m = efl_ui_focus_object_focus_manager_get(user);
   entry = user;

   do {
     //check if the root of a manager is the window root, set focus to this object in the manager than
     entry = efl_ui_focus_manager_root_get(m);
     if (efl_isa(m, EFL_UI_WIN_CLASS))
       {
          //we are at the root of the window, we can set the focus to the object
          efl_ui_focus_manager_focus_set(registered_manager, user);
          return;
       }

     //if there is no manager yet, delay the focus setting until this entity gets registered for one chain
     m = efl_ui_focus_object_focus_manager_get(entry);
     if (!m)
       {
          //delayed focusung
          efl_key_data_set(top, "__delayed_focus_set", entry);
          efl_event_callback_add(entry, EFL_UI_FOCUS_OBJECT_EVENT_MANAGER_CHANGED, _manager_changed, user);
          return;
       }
   } while (m);
}

#include "efl_ui_focus_util.eo.c"
