#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#define EFL_PART_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "efl_ui_navigation_layout_private.h"
#include "elm_part_helper.h"

#define MY_CLASS EFL_UI_NAVIGATION_LAYOUT_CLASS
#define MY_CLASS_NAME "Efl.Ui.Navigation_Layout"

EOLIAN static void
_efl_ui_navigation_layout_bar_set(Eo *obj, Efl_Ui_Navigation_Layout_Data *pd, Efl_Ui_Layout *bar)
{
   EINA_SAFETY_ON_FALSE_RETURN(efl_isa(bar, EFL_UI_LAYOUT_BASE_CLASS));

   efl_content_set(efl_part(obj, "efl.bar"), bar);
   pd->bar = bar;
}

EOLIAN static Efl_Ui_Layout *
_efl_ui_navigation_layout_bar_get(const Eo *obj EINA_UNUSED, Efl_Ui_Navigation_Layout_Data *pd)
{
   return pd->bar;
}

EOLIAN static Eo *
_efl_ui_navigation_layout_efl_object_constructor(Eo *obj, Efl_Ui_Navigation_Layout_Data *pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   if (!elm_widget_theme_klass_get(obj))
     elm_widget_theme_klass_set(obj, "navigation_layout");
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME);

   elm_widget_can_focus_set(obj, EINA_TRUE);
   if (elm_widget_theme_object_set(obj, wd->resize_obj,
                                       elm_widget_theme_klass_get(obj),
                                       elm_widget_theme_element_get(obj),
                                       elm_widget_theme_style_get(obj)) == EFL_UI_THEME_APPLY_ERROR_GENERIC)
     CRI("Failed to set layout!");

   return obj;
}

/* Standard widget overrides */
ELM_PART_CONTENT_DEFAULT_IMPLEMENT(efl_ui_navigation_layout, Efl_Ui_Navigation_Layout_Data)

#include "efl_ui_navigation_layout.eo.c"
