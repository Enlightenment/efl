#ifdef HAVE_CONFIG_H
  #include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_widget.h"
#include "elm_priv.h"

#define ELM_INTERFACE_ATSPI_ACTION_PROTECTED

#include "elm_interface_atspi_action.eo.h"

EOLIAN const char *
_elm_interface_atspi_action_localized_name_get(Eo *obj, void *pd EINA_UNUSED, int id)
{
   const char *ret = NULL;

   eo_do(obj, ret = elm_interface_atspi_action_name_get(id));
#ifdef ENABLE_NLS
   ret = gettext(ret);
#endif
   return ret;
}

#include "elm_interface_atspi_action.eo.c"
