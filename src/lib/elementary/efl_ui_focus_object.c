#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"

typedef struct {
  Eina_Bool old_focus;
} Efl_Ui_Focus_Object_Data;

EOLIAN static void
_efl_ui_focus_object_focus_set(Eo *obj, Efl_Ui_Focus_Object_Data *pd, Eina_Bool focus)
{
   if (pd->old_focus == focus) return;

   efl_event_callback_call(obj, EFL_UI_FOCUS_OBJECT_EVENT_FOCUS_CHANGED , (void*) (uintptr_t) focus);
   pd->old_focus = focus;
}


#include "efl_ui_focus_object.eo.c"
typedef struct {

} Efl_Ui_Focus_User_Data;
#include "efl_ui_focus_user.eo.c"