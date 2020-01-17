#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include "elm_priv.h"

typedef struct {
   Efl_Ui_Layout_Orientation dir;
} Efl_Ui_Separator_Data;

#define MY_CLASS      EFL_UI_SEPARATOR_CLASS


EOLIAN static Efl_Object*
_efl_ui_separator_efl_object_constructor(Eo *obj, Efl_Ui_Separator_Data *pd EINA_UNUSED)
{
   if (!elm_widget_theme_klass_get(obj))
     elm_widget_theme_klass_set(obj, "separator");
   return efl_constructor(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_ui_separator_efl_ui_layout_orientable_orientation_set(Eo *obj EINA_UNUSED, Efl_Ui_Separator_Data *pd, Efl_Ui_Layout_Orientation dir)
{
   pd->dir = dir;
}

EOLIAN static Efl_Ui_Layout_Orientation
_efl_ui_separator_efl_ui_layout_orientable_orientation_get(const Eo *ob EINA_UNUSED, Efl_Ui_Separator_Data *pd)
{
   return pd->dir;
}

EOLIAN static Eina_Error
_efl_ui_separator_efl_ui_widget_theme_apply(Eo *obj, Efl_Ui_Separator_Data *pd)
{
   if (efl_ui_layout_orientation_is_horizontal(pd->dir, EINA_TRUE))
     elm_widget_theme_element_set(obj, "horizontal");
   else
     elm_widget_theme_element_set(obj, "vertical");
   return efl_ui_widget_theme_apply(efl_super(obj, MY_CLASS));
}


#include "efl_ui_separator.eo.c"
