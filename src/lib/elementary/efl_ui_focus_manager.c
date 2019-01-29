#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"

EAPI void
efl_ui_focus_relation_free(Efl_Ui_Focus_Relations *rel)
{
   eina_list_free(rel->right);
   eina_list_free(rel->left);
   eina_list_free(rel->top);
   eina_list_free(rel->down);
   free(rel);
}

#include "efl_ui_focus_manager.eo.c"
#include "efl_ui_focus_manager_window_root.eo.c"
