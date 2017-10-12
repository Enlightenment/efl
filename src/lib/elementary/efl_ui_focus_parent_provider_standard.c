#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"

typedef struct {

} Efl_Ui_Focus_Parent_Provider_Standard_Data;

EOLIAN static Efl_Ui_Focus_Object*
_efl_ui_focus_parent_provider_standard_efl_ui_focus_parent_provider_find_logical_parent(Eo *obj EINA_UNUSED, Efl_Ui_Focus_Parent_Provider_Standard_Data *pd EINA_UNUSED, Efl_Ui_Focus_Object *widget)
{
   return elm_object_parent_widget_get(widget);
}


#include "efl_ui_focus_parent_provider_standard.eo.c"
