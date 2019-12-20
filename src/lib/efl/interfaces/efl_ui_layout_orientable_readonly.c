#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Efl.h>

typedef struct {

} Efl_Ui_Layout_Orientable_Readonly_Data;

EOLIAN static void
_efl_ui_layout_orientable_readonly_efl_ui_layout_orientable_orientation_set(Eo *obj EINA_UNUSED, Efl_Ui_Layout_Orientable_Readonly_Data *pd EINA_UNUSED, Efl_Ui_Layout_Orientation dir EINA_UNUSED)
{
   EINA_LOG_ERR("This object does not allow setting an orientation");
}

#include "interfaces/efl_ui_layout_orientable_readonly.eo.c"
