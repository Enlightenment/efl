#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_PART_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"
#include "efl_ui_tab_page_private.h"
#include "elm_part_helper.h"

#define MY_CLASS EFL_UI_TAB_PAGE_CLASS

EOLIAN static Eina_Bool
_efl_ui_tab_page_efl_content_content_set(Eo *obj, Efl_Ui_Tab_Page_Data *sd, Eo *content)
{
   if (sd->content)
     {
        efl_content_unset(efl_part(obj, "efl.content"));
        efl_del(sd->content);
     }

   sd->content = content;
   efl_content_set(efl_part(obj, "efl.content"), sd->content);

   return EINA_TRUE;
}

EOLIAN static Eo *
_efl_ui_tab_page_efl_content_content_get(const Eo *obj EINA_UNUSED, Efl_Ui_Tab_Page_Data *sd)
{
   return sd->content;
}

EOLIAN static Efl_Object *
_efl_ui_tab_page_efl_object_constructor(Eo *obj, Efl_Ui_Tab_Page_Data *sd)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   if (!elm_widget_theme_klass_get(obj))
     elm_widget_theme_klass_set(obj, "tab_page");

   obj = efl_constructor(efl_super(obj, MY_CLASS));

   if (!elm_widget_theme_object_set(obj, wd->resize_obj,
                                    elm_widget_theme_klass_get(obj),
                                    elm_widget_theme_element_get(obj),
                                    elm_widget_theme_style_get(obj)))
     CRI("Failed to set layout!");

   elm_widget_sub_object_parent_add(obj);

   efl_ui_widget_focus_allow_set(obj, EINA_TRUE);

   sd->content = NULL;
   sd->tab_label = NULL;
   sd->tab_icon = NULL;

   return obj;
}

EOLIAN static void
_efl_ui_tab_page_efl_object_destructor(Eo *obj, Efl_Ui_Tab_Page_Data *sd EINA_UNUSED)
{
   efl_destructor(efl_super(obj, MY_CLASS));
}

/* Efl.Part begin */

EOLIAN static Eo *
_efl_ui_tab_page_efl_part_part_get(const Eo *obj, Efl_Ui_Tab_Page_Data *sd EINA_UNUSED, const char *part)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(part, NULL);

   if (eina_streq(part, "tab"))
     return ELM_PART_IMPLEMENT(EFL_UI_TAB_PAGE_PART_TAB_CLASS, obj, part);

   return efl_part_get(efl_super(obj, MY_CLASS), part);
}

EOLIAN static void
_efl_ui_tab_page_part_tab_icon_set(Eo *obj, void *_pd EINA_UNUSED, const char *path)
{
   Elm_Part_Data *pd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_MIXIN);
   Efl_Ui_Tab_Page_Data *sd = efl_data_scope_get(pd->obj, EFL_UI_TAB_PAGE_CLASS);

   eina_stringshare_replace(&sd->tab_icon, path);

   Efl_Ui_Tab_Page_Tab_Changed_Event event;
   event.changed_info = EFL_UI_TAB_PAGE_TAB_CHANGED_ICON;

   efl_event_callback_call(pd->obj, EFL_UI_TAB_PAGE_EVENT_TAB_CHANGED, &event);
}

EOLIAN static const char *
_efl_ui_tab_page_part_tab_icon_get(const Eo *obj, void *_pd EINA_UNUSED)
{
   Elm_Part_Data *pd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_MIXIN);
   Efl_Ui_Tab_Page_Data *sd = efl_data_scope_get(pd->obj, EFL_UI_TAB_PAGE_CLASS);

   return sd->tab_icon;
}

EOLIAN static void
_efl_ui_tab_page_part_tab_efl_text_text_set(Eo *obj, void *_pd EINA_UNUSED, const char *text)
{
   Elm_Part_Data *pd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_MIXIN);
   Efl_Ui_Tab_Page_Data *sd = efl_data_scope_get(pd->obj, EFL_UI_TAB_PAGE_CLASS);

   eina_stringshare_replace(&sd->tab_label, text);

   Efl_Ui_Tab_Page_Tab_Changed_Event event;
   event.changed_info = EFL_UI_TAB_PAGE_TAB_CHANGED_LABEL;

   efl_event_callback_call(pd->obj, EFL_UI_TAB_PAGE_EVENT_TAB_CHANGED, &event);
}

EOLIAN static const char *
_efl_ui_tab_page_part_tab_efl_text_text_get(const Eo *obj, void *_pd EINA_UNUSED)
{
   Elm_Part_Data *pd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_MIXIN);
   Efl_Ui_Tab_Page_Data *sd = efl_data_scope_get(pd->obj, EFL_UI_TAB_PAGE_CLASS);

   return sd->tab_label;
}

#include "efl_ui_tab_page_part_tab.eo.c"

/* Efl.Part end */

#include "efl_ui_tab_page.eo.c"
