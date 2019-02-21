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
   efl_ui_focus_util_focus(data);
}

static Eina_Bool
_can_take_focus(Efl_Ui_Focus_Manager *m, Efl_Ui_Focus_Object *user)
{
   if (efl_isa(user, EFL_UI_FOCUS_MANAGER_INTERFACE))
     return !!efl_ui_focus_manager_request_subchild(user, efl_ui_focus_manager_root_get(user));
   else
     return !!efl_ui_focus_manager_request_subchild(m, user);
}

EOLIAN static void
_efl_ui_focus_util_focus(Efl_Ui_Focus_Object *user)
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

   if (m && !efl_ui_widget_focus_allow_get(user) && !_can_take_focus(m, user))
     return;

   //move up the manager chain and see if we can end at a winow or a NULL m
   while (m && !efl_isa(m, EFL_UI_WIN_CLASS))
     {
        entry = efl_ui_focus_manager_root_get(m);
        m = efl_ui_focus_object_focus_manager_get(entry);
     }

   if (!m)
     {
        efl_key_data_set(top, "__delayed_focus_set", entry);
        efl_event_callback_add(entry,
                               EFL_UI_FOCUS_OBJECT_EVENT_MANAGER_CHANGED,
                               _manager_changed, user);
     }
   else if (efl_isa(m, EFL_UI_WIN_CLASS))
     {
        efl_ui_focus_manager_focus_set(registered_manager, user);
     }
}

EOLIAN static Efl_Ui_Focus_Manager*
_efl_ui_focus_util_active_manager(Efl_Ui_Focus_Manager *manager)
{
   while (efl_ui_focus_manager_redirect_get(manager))
     manager = efl_ui_focus_manager_redirect_get(manager);

   return manager;
}

EOLIAN static Efl_Ui_Focus_Direction
_efl_ui_focus_util_direction_complement(Efl_Ui_Focus_Direction dir)
{
  #define COMP(a,b) \
        if (dir == a) return b; \
        if (dir == b) return a;

    COMP(EFL_UI_FOCUS_DIRECTION_RIGHT, EFL_UI_FOCUS_DIRECTION_LEFT)
    COMP(EFL_UI_FOCUS_DIRECTION_UP, EFL_UI_FOCUS_DIRECTION_DOWN)
    COMP(EFL_UI_FOCUS_DIRECTION_PREVIOUS, EFL_UI_FOCUS_DIRECTION_NEXT)

    #undef COMP

    return EFL_UI_FOCUS_DIRECTION_LAST;
}


#include "efl_ui_focus_util.eo.c"
