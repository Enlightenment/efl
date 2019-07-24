#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_LAYOUT_PROTECTED
#define EFL_UI_SCROLL_MANAGER_PROTECTED
#define EFL_UI_SCROLLBAR_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"
#include "efl_ui_list_item_private.h"
#include "efl_ui_item_private.h"

#define MY_CLASS      EFL_UI_LIST_CLASS
#define MY_CLASS_PFX  efl_ui_list

#define MY_CLASS_NAME "Efl.Ui.List"

typedef struct {

} Efl_Ui_List_Data;

EOLIAN static Eo *
_efl_ui_list_efl_object_constructor(Eo *obj, Efl_Ui_List_Data *pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   efl_ui_item_container_position_manager_set(obj, efl_new(EFL_UI_LIST_POSITION_MANAGER_CLASS));

   return obj;
}

#include "efl_ui_list.eo.c"
