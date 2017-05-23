#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"
#include "efl_ui_widget_scroll_manager.h"

#define MY_CLASS EFL_UI_SCROLL_MANAGER_CLASS
#define MY_CLASS_NAME "Efl_Ui_Scroll_Manager"


EOLIAN static void
_efl_ui_scroll_manager_efl_canvas_group_group_add(Eo *obj EINA_UNUSED,
                                                  Efl_Ui_Scroll_Manager_Data *sd EINA_UNUSED)
{
}

EOLIAN static Eo *
_efl_ui_scroll_manager_efl_object_constructor(Eo *obj,
                                              Efl_Ui_Scroll_Manager_Data *sd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   return obj;
}


#include "efl_ui_scroll_manager.eo.c"
