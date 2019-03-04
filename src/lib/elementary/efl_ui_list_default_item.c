#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_UI_LIST_DEFAULT_ITEM_PROTECTED
#define EFL_PART_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "efl_ui_list_default_item_part_icon.eo.h"
#include "efl_ui_list_default_item_part_end.eo.h"
#include "elm_part_helper.h"

#define MY_CLASS      EFL_UI_LIST_DEFAULT_ITEM_CLASS
#define MY_CLASS_PFX  efl_ui_list_default_item

#define MY_CLASS_NAME "Efl.Ui.List_Default_Item"

EOLIAN static Efl_Object *
_efl_ui_list_default_item_efl_object_finalize(Eo *obj, void *pd EINA_UNUSED)
{
   Eo *eo;
   eo = efl_finalize(efl_super(obj, MY_CLASS));
   ELM_WIDGET_DATA_GET_OR_RETURN(eo, wd, eo);
   Efl_Ui_Theme_Apply_Error theme_apply_ret = efl_ui_layout_theme_set(obj, "list_item", NULL, NULL);

   if (theme_apply_ret == EFL_UI_THEME_APPLY_ERROR_GENERIC)
     CRI("Default Item(%p) failed to set theme [efl/list_item]!", eo);
   return eo;
}

EOLIAN static void
_efl_ui_list_default_item_efl_object_destructor(Eo *obj, void *pd EINA_UNUSED)
{
   efl_destructor(efl_super(obj, MY_CLASS));
}

/* Efl.Part */

ELM_PART_TEXT_DEFAULT_GET(efl_ui_list_default_item, "efl.text")
ELM_PART_TEXT_DEFAULT_IMPLEMENT(efl_ui_list_default_item, void)
ELM_PART_MARKUP_DEFAULT_IMPLEMENT(efl_ui_list_default_item, void)
ELM_PART_CONTENT_DEFAULT_GET(efl_ui_list_default_item, "efl.icon")
ELM_PART_CONTENT_DEFAULT_IMPLEMENT(efl_ui_list_default_item, void
)

Eina_Bool
_efl_ui_list_default_item_part_icon_efl_content_content_set(Eo *obj, void *pd EINA_UNUSED, Efl_Gfx_Entity *content)
{
   Elm_Part_Data *wd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   return efl_content_set(efl_part(efl_super(wd->obj, MY_CLASS), wd->part), content);
}

Efl_Gfx_Entity *
_efl_ui_list_default_item_part_icon_efl_content_content_get(const Eo *obj, void *pd EINA_UNUSED)
{
   Elm_Part_Data *wd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   return efl_content_get(efl_part(efl_super(wd->obj, MY_CLASS), wd->part));
}

Efl_Gfx_Entity *
_efl_ui_list_default_item_part_icon_efl_content_content_unset(Eo *obj, void *pd EINA_UNUSED)
{
   Elm_Part_Data *wd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   return efl_content_unset(efl_part(efl_super(wd->obj, MY_CLASS), wd->part));
}
#include "efl_ui_list_default_item_part_icon.eo.c"

Eina_Bool
_efl_ui_list_default_item_part_end_efl_content_content_set(Eo *obj, void *pd EINA_UNUSED, Efl_Gfx_Entity *content)
{
   Elm_Part_Data *wd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   return efl_content_set(efl_part(efl_super(wd->obj, MY_CLASS), wd->part), content);
}

Efl_Gfx_Entity *
_efl_ui_list_default_item_part_end_efl_content_content_get(const Eo *obj, void *pd EINA_UNUSED)
{
   Elm_Part_Data *wd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   return efl_content_get(efl_part(efl_super(wd->obj, MY_CLASS), wd->part));
}

Efl_Gfx_Entity *
_efl_ui_list_default_item_part_end_efl_content_content_unset(Eo *obj, void *pd EINA_UNUSED)
{
   Elm_Part_Data *wd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   return efl_content_unset(efl_part(efl_super(wd->obj, MY_CLASS), wd->part));
}
#include "efl_ui_list_default_item_part_end.eo.c"

EOLIAN static Efl_Object *
_efl_ui_list_default_item_efl_part_part_get(const Eo *obj, void *wd EINA_UNUSED, const char *part)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(part, NULL);
   if (eina_streq(part, "text"))
     return ELM_PART_IMPLEMENT(EFL_UI_LAYOUT_PART_TEXT_CLASS, obj, "efl.text");
   else if (eina_streq(part, "icon"))
     return ELM_PART_IMPLEMENT(EFL_UI_LIST_DEFAULT_ITEM_PART_ICON_CLASS, obj, "efl.icon");
   else if (eina_streq(part, "end"))
     return ELM_PART_IMPLEMENT(EFL_UI_LIST_DEFAULT_ITEM_PART_END_CLASS, obj, "efl.end");

   return efl_part_get(efl_super(obj, MY_CLASS), part);
}
/* Efl.Part end */

/* Internal EO APIs and hidden overrides */
#define EFL_UI_LIST_DEFAULT_ITEM_EXTRA_OPS             \
  ELM_PART_TEXT_DEFAULT_OPS(efl_ui_list_default_item), \
  ELM_PART_CONTENT_DEFAULT_OPS(efl_ui_list_default_item)

#include "efl_ui_list_default_item.eo.c"
