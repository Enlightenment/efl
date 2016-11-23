#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
#define ELM_INTERFACE_ATSPI_WIDGET_ACTION_PROTECTED
#define ELM_WIN_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"

#define MY_CLASS EFL_UI_WIN_STANDARD_CLASS

EOLIAN static Eo *
_efl_ui_win_standard_efl_object_finalize(Eo *obj, void *pd EINA_UNUSED)
{
   obj = efl_finalize(efl_super(obj, MY_CLASS));
   if (!obj) return NULL;

   _elm_win_standard_init(obj);
   return obj;
}

#include "efl_ui_win_standard.eo.c"
