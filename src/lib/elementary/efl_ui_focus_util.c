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
   Efl_Ui_Widget *top, *o;
   Efl_Ui_Focus_Manager *m;
   m = efl_ui_focus_object_focus_manager_get(user);

   top = elm_widget_top_get(user);

   o = efl_key_data_get(top, "__delayed_focus_set");
   if (o) efl_event_callback_del(o, EFL_UI_FOCUS_OBJECT_EVENT_MANAGER_CHANGED, _manager_changed, o);
   efl_key_data_set(top, "__delayed_focus_set", NULL);

   if (!m)
     {
        //delayed focusung
        efl_key_data_set(top, "__delayed_focus_set", user);
        efl_event_callback_add(user, EFL_UI_FOCUS_OBJECT_EVENT_MANAGER_CHANGED, _manager_changed, user);
        return;
     }

   efl_ui_focus_manager_focus_set(m, user);
}

#include "efl_ui_focus_util.eo.c"
