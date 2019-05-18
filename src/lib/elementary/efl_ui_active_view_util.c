#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include "elm_priv.h"

typedef struct {

} Efl_Ui_Active_View_Util_Data;

EOLIAN static Efl_Ui_Active_View_Container*
_efl_ui_active_view_util_stack_gen(Efl_Ui_Widget *parent)
{
   Efl_Ui_Active_View_View_Manager *manager = efl_add(EFL_UI_ACTIVE_VIEW_VIEW_MANAGER_STACK_CLASS, parent);
   return efl_add(EFL_UI_ACTIVE_VIEW_CONTAINER_CLASS, parent,
                  efl_ui_active_view_manager_set(efl_added, manager));
}


#include "efl_ui_active_view_util.eo.c"
