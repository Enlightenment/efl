#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>

#define MY_CLASS      EFL_UI_LIST_VIEW_CLASS
#define MY_CLASS_PFX  efl_ui_list_view

#define MY_CLASS_NAME "Efl.Ui.List_View"

static Eo *
_efl_ui_list_view_efl_object_constructor(Eo *obj, void *pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   efl_ui_collection_view_position_manager_set(obj, efl_new(EFL_UI_POSITION_MANAGER_LIST_CLASS));

   return obj;
}

#include "efl_ui_list_view.eo.c"
