#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Efl.h>

typedef struct {

} Efl_Ui_Direction_Readonly_Data;

EOLIAN static void
_efl_ui_direction_readonly_efl_ui_direction_direction_set(Eo *obj EINA_UNUSED, Efl_Ui_Direction_Readonly_Data *pd EINA_UNUSED, Efl_Ui_Dir dir EINA_UNUSED)
{
   EINA_LOG_ERR("This object does not allow setting of a direction");
}

#include "interfaces/efl_ui_direction_readonly.eo.c"
