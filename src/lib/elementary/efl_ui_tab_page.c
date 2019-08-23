#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_PART_PROTECTED

#include <Efl_Ui.h>
#include "elm_priv.h"
#include "efl_ui_tab_page_private.h"
#include "elm_part_helper.h"

#define MY_CLASS EFL_UI_TAB_PAGE_CLASS

static void
_invalidate_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   efl_content_unset(data);
}

EOLIAN static Eina_Bool
_efl_ui_tab_page_efl_content_content_set(Eo *obj, Efl_Ui_Tab_Page_Data *sd, Eo *content)
{
   if (sd->content)
     {
        efl_content_unset(efl_part(obj, "efl.content"));
        efl_event_callback_del(sd->content, EFL_EVENT_INVALIDATE, _invalidate_cb, obj);
        efl_del(sd->content);
        sd->content = NULL;
     }

   if (content && !efl_ui_widget_sub_object_add(obj, content))
     {
        efl_event_callback_call(obj, EFL_CONTENT_EVENT_CONTENT_CHANGED, NULL);
        return EINA_FALSE;
     }

   sd->content = content;
   efl_event_callback_add(sd->content, EFL_EVENT_INVALIDATE, _invalidate_cb, obj);
   efl_event_callback_call(obj, EFL_CONTENT_EVENT_CONTENT_CHANGED, content);

   efl_content_set(efl_part(obj, "efl.content"), sd->content);

   return EINA_TRUE;
}

EOLIAN static Efl_Gfx_Entity*
_efl_ui_tab_page_efl_content_content_unset(Eo *obj, Efl_Ui_Tab_Page_Data *pd)
{
   efl_event_callback_del(pd->content, EFL_EVENT_INVALIDATE, _invalidate_cb, obj);
   pd->content = NULL;
   efl_event_callback_call(obj, EFL_CONTENT_EVENT_CONTENT_CHANGED, NULL);
   return efl_content_unset(efl_part(obj, "efl.content"));
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

   if (elm_widget_theme_object_set(obj, wd->resize_obj,
                                       elm_widget_theme_klass_get(obj),
                                       elm_widget_theme_element_get(obj),
                                       elm_widget_theme_style_get(obj)) == EFL_UI_THEME_APPLY_ERROR_GENERIC)
     CRI("Failed to set layout!");

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


EOLIAN static Efl_Ui_Tab_Bar_Default_Item*
_efl_ui_tab_page_tab_bar_item_get(const Eo *obj, Efl_Ui_Tab_Page_Data *pd)
{
  if (!pd->tab_bar_icon)
    {
       pd->tab_bar_icon = efl_add(EFL_UI_TAB_BAR_DEFAULT_ITEM_CLASS, (Eo*)obj);
       efl_text_set(pd->tab_bar_icon, pd->tab_label);
       efl_ui_tab_bar_default_item_icon_set(pd->tab_bar_icon, pd->tab_icon);
    }

  return pd->tab_bar_icon;
}

/* Efl.Part end */

#include "efl_ui_tab_page.eo.c"
