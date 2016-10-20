#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"

typedef struct {

} Efl_Ui_Focus_Object_Data;

EOLIAN static void
_efl_ui_focus_object_focus_set(Eo *obj, Efl_Ui_Focus_Object_Data *pd EINA_UNUSED, Eina_Bool focus)
{
   const Efl_Event_Description *desc;

   if (focus)
     desc = EFL_UI_FOCUS_OBJECT_EVENT_FOCUS;
   else
     desc = EFL_UI_FOCUS_OBJECT_EVENT_UNFOCUS;

   efl_event_callback_call(obj, desc, NULL);
}


#include "efl_ui_focus_object.eo.c"
typedef struct {

} Efl_Ui_Focus_User_Data;
#include "efl_ui_focus_user.eo.c"