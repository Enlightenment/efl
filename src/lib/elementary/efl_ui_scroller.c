#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_LAYOUT_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"
#include "efl_ui_widget_scroller.h"
#include "elm_widget_layout.h"

#define MY_CLASS EFL_UI_SCROLLER_CLASS
#define MY_CLASS_NAME "Efl_Ui_Scroller"


EOLIAN static void
_efl_ui_scroller_efl_canvas_group_group_add(Eo *obj EINA_UNUSED,
                                            Efl_Ui_Scroller_Data *sd EINA_UNUSED)
{
   Eo *smanager = efl_add(EFL_UI_SCROLL_MANAGER_CLASS, obj);
}

EOLIAN static Eo *
_efl_ui_scroller_efl_object_constructor(Eo *obj,
                                        Efl_Ui_Scroller_Data *sd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   return obj;
}


#include "efl_ui_scroller.eo.c"
