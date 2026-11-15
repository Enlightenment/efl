#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_UI_LIST_PLACEHOLDER_ITEM_PROTECTED
#define EFL_PART_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_part_helper.h"

#define MY_CLASS      EFL_UI_LIST_PLACEHOLDER_ITEM_CLASS
#define MY_CLASS_PFX  efl_ui_list_placeholder_item

#define MY_CLASS_NAME "Efl.Ui.List_Placeholder_Item"

/* The theme is chosen here, not at finalize, so that the content of a part
 * given while the item is being constructed is swallowed by the group that
 * has that part. */
EOLIAN static Efl_Object *
_efl_ui_list_placeholder_item_efl_object_constructor(Eo *obj, void *pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   if (!elm_widget_theme_klass_get(obj))
     elm_widget_theme_klass_set(obj, "list_item");
   elm_widget_theme_style_set(obj, "placeholder");
   return obj;
}

EOLIAN static void
_efl_ui_list_placeholder_item_efl_object_destructor(Eo *obj, void *pd EINA_UNUSED)
{
   efl_destructor(efl_super(obj, MY_CLASS));
}

/* Efl.Part */
ELM_PART_CONTENT_DEFAULT_GET(efl_ui_list_placeholder_item, "efl.content")
ELM_PART_CONTENT_DEFAULT_IMPLEMENT(efl_ui_list_placeholder_item, void)
/* Efl.Part end */

EOLIAN static Efl_Object *
_efl_ui_list_placeholder_item_efl_part_part_get(const Eo *obj, void *wd EINA_UNUSED, const char *part)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(part, NULL);
   if (eina_streq(part, "content"))
     {
        _elm_part_layout_theme_forward(obj);
        return ELM_PART_IMPLEMENT(EFL_UI_LAYOUT_PART_CONTENT_CLASS, obj, "efl.content");
     }

   return efl_part_get(efl_super(obj, MY_CLASS), part);
}

/* Internal EO APIs and hidden overrides */

#define EFL_UI_LIST_PLACEHOLDER_ITEM_EXTRA_OPS \
  ELM_PART_CONTENT_DEFAULT_OPS(efl_ui_list_placeholder_item)

#include "efl_ui_list_placeholder_item.eo.c"
