#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include "elm_priv.h"

#define MY_CLASS EFL_UI_PAGER_CLASS

typedef struct {

} Efl_Ui_Pager_Data;

EOLIAN static Efl_Object *
_efl_ui_pager_efl_object_constructor(Eo *obj, Efl_Ui_Pager_Data *sd EINA_UNUSED)
{
   Eo *scroller;

   obj = efl_constructor(efl_super(obj, MY_CLASS));

   scroller = efl_new(EFL_UI_SPOTLIGHT_SCROLL_MANAGER_CLASS);
   efl_ui_spotlight_manager_set(obj, scroller);

   return obj;
}

#include "efl_ui_pager.eo.c"
