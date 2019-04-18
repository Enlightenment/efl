#ifdef HAVE_CONFIG_H
  #include "elementary_config.h"
#endif

#define EFL_ACCESS_ACTION_PROTECTED

#include <Elementary.h>
#include "elm_widget.h"
#include "elm_priv.h"

EOLIAN const char *
_efl_access_action_action_localized_name_get(const Eo *obj, void *pd EINA_UNUSED, int id)
{
   const char *ret = NULL;

   ret = efl_access_action_name_get(obj, id);
#ifdef ENABLE_NLS
   ret = gettext(ret);
#endif
   return ret;
}

#include "efl_access_action.eo.c"
