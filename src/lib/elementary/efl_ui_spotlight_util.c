#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include "elm_priv.h"

typedef struct {

} Efl_Ui_Spotlight_Util_Data;

EOLIAN static Efl_Ui_Spotlight_Container*
_efl_ui_spotlight_util_stack_gen(Efl_Ui_Widget *parent)
{
   Efl_Ui_Spotlight_Manager *manager = efl_new(EFL_UI_SPOTLIGHT_FADE_MANAGER_CLASS);
   return efl_add(EFL_UI_SPOTLIGHT_CONTAINER_CLASS, parent,
                  efl_ui_spotlight_manager_set(efl_added, manager));
}


#include "efl_ui_spotlight_util.eo.c"
