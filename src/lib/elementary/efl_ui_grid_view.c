#ifdef HAVE_CONFIG_H
#include "elementary_config.h"
#endif

#define ELM_LAYOUT_PROTECTED
#define EFL_UI_SCROLL_MANAGER_PROTECTED
#define EFL_UI_SCROLLBAR_PROTECTED

#include <Efl_Ui.h>

#define MY_CLASS      EFL_UI_GRID_VIEW_CLASS
#define MY_CLASS_PFX  efl_ui_grid_view

#define MY_CLASS_NAME "Efl.Ui.Grid_View"

EOLIAN static Eo *
_efl_ui_grid_view_efl_object_constructor(Eo *obj, void *pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   efl_ui_collection_view_position_manager_set(obj, efl_new(EFL_UI_POSITION_MANAGER_GRID_CLASS));

   return obj;
}

#include "efl_ui_grid_view.eo.c"
