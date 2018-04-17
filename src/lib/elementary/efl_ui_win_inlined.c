#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_UI_WIN_PROTECTED
#define EFL_UI_WIN_INLINED_PROTECTED

#include <Elementary.h>

#define MY_CLASS EFL_UI_WIN_INLINED_CLASS
#define MY_CLASS_NAME "Efl.Ui.Win_Inlined"

typedef struct
{
   Eo *parent; /**< This is the parent specific to the inlined window, aka parent2 */
} Efl_Ui_Win_Inlined_Data;

void
efl_ui_win_inlined_parent_set(Eo *obj, Efl_Canvas_Object *parent)
{
   Efl_Ui_Win_Inlined_Data *pd = efl_data_scope_safe_get(obj, MY_CLASS);
   if (!pd) return;
   pd->parent = parent;
}

EOLIAN static Efl_Canvas_Object *
_efl_ui_win_inlined_inlined_parent_get(const Eo *obj EINA_UNUSED, Efl_Ui_Win_Inlined_Data *pd)
{
   return pd->parent;
}

EOLIAN static Efl_Object *
_efl_ui_win_inlined_efl_object_finalize(Eo *obj, Efl_Ui_Win_Inlined_Data *pd EINA_UNUSED)
{
   efl_ui_win_type_set(obj, ELM_WIN_INLINED_IMAGE);
   obj = efl_finalize(efl_super(obj, MY_CLASS));

   return obj;
}

#include "efl_ui_win_inlined.eo.c"
