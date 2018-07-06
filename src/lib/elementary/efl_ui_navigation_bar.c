#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_PART_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "efl_ui_navigation_bar_private.h"
#include "efl_ui_navigation_bar_part.eo.h"
#include "elm_part_helper.h"

#define MY_CLASS EFL_UI_NAVIGATION_BAR_CLASS
#define MY_CLASS_NAME "Efl.Ui.Navigation_Bar"

static void
_back_button_clicked_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *navigation_bar = data;

   Eo *stack = efl_provider_find(navigation_bar, EFL_UI_STACK_CLASS);
   if (!stack)
     {
        ERR("Cannot find EFL_UI_STACK_CLASS instance!");
        return;
     }

   efl_ui_stack_pop(stack);
}

EOLIAN static Eo *
_efl_ui_navigation_bar_efl_object_constructor(Eo *obj, Efl_Ui_Navigation_Bar_Data *pd)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   if (!elm_widget_theme_klass_get(obj))
     elm_widget_theme_klass_set(obj, "navigation_bar");
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME);

   elm_widget_sub_object_parent_add(obj);

   elm_widget_can_focus_set(obj, EINA_TRUE);
   if (!elm_widget_theme_object_set(obj, wd->resize_obj,
                                    elm_widget_theme_klass_get(obj),
                                    elm_widget_theme_element_get(obj),
                                    elm_widget_theme_style_get(obj)))
     CRI("Failed to set layout!");

   Eo *back_button = efl_add(EFL_UI_BUTTON_CLASS, obj,
                             elm_widget_element_update(obj, efl_added, "back_button"),
                             efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, _back_button_clicked_cb, obj),
                             efl_gfx_entity_visible_set(efl_added, EINA_FALSE));

   pd->back_button = back_button;

   return obj;
}


/* Standard widget overrides */
ELM_PART_CONTENT_DEFAULT_IMPLEMENT(efl_ui_navigation_bar, Efl_Ui_Navigation_Bar_Data)
ELM_PART_TEXT_DEFAULT_GET(efl_ui_navigation_bar, "efl.text")
ELM_PART_TEXT_DEFAULT_IMPLEMENT(efl_ui_navigation_bar, Efl_Ui_Navigation_Bar_Data)

#define EFL_UI_NAVIGATION_BAR_EXTRA_OPS \
ELM_PART_TEXT_DEFAULT_OPS(efl_ui_navigation_bar)

static Eina_Bool
_efl_ui_navigation_bar_content_set(Eo *obj, Efl_Ui_Navigation_Bar_Data *_pd EINA_UNUSED, const char *part, Efl_Gfx_Entity *content)
{
   if (eina_streq(part, "left_content"))
     {
        if (content)
          efl_layout_signal_emit(obj, "efl,state,left_content,set", "efl");
        else
          efl_layout_signal_emit(obj, "efl,state,left_content,unset", "efl");
        efl_layout_signal_process(obj, EINA_FALSE);

        return efl_content_set(efl_part(efl_super(obj, MY_CLASS), "efl.left_content"), content);
     }
   else if (eina_streq(part, "right_content"))
     return efl_content_set(efl_part(efl_super(obj, MY_CLASS), "efl.right_content"), content);

   return efl_content_set(efl_part(efl_super(obj, MY_CLASS), part), content);
}

static Efl_Gfx_Entity *
_efl_ui_navigation_bar_content_get(const Eo *obj, Efl_Ui_Navigation_Bar_Data *_pd EINA_UNUSED, const char *part)
{
   if (eina_streq(part, "left_content"))
     efl_content_get(efl_part(efl_super(obj, MY_CLASS), "efl.left_content"));
   else if (eina_streq(part, "right_content"))
     efl_content_get(efl_part(efl_super(obj, MY_CLASS), "efl.right_content"));

   return efl_content_get(efl_part(efl_super(obj, MY_CLASS), part));
}

static Efl_Gfx_Entity *
_efl_ui_navigation_bar_content_unset(Eo *obj, Efl_Ui_Navigation_Bar_Data *_pd EINA_UNUSED, const char *part)
{
  if (eina_streq(part, "left_content"))
    {
       efl_layout_signal_emit(obj, "efl,state,left_content,unset", "efl");
       efl_layout_signal_process(obj, EINA_FALSE);
       return efl_content_unset(efl_part(efl_super(obj, MY_CLASS), "efl.left_content"));
    }
  else if (eina_streq(part, "right_content"))
    return efl_content_unset(efl_part(efl_super(obj, MY_CLASS), "efl.right_content"));

  return efl_content_unset(efl_part(efl_super(obj, MY_CLASS), part));
}


/* Efl.Part begin */
EOLIAN static Efl_Object *
_efl_ui_navigation_bar_efl_part_part_get(const Eo *obj, Efl_Ui_Navigation_Bar_Data *priv EINA_UNUSED, const char *part)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(part, NULL);

   if (eina_streq(part, "back_button"))
     return ELM_PART_IMPLEMENT(EFL_UI_NAVIGATION_BAR_PART_BACK_BUTTON_CLASS, obj, part);
   else if (eina_streq(part, "left_content") || eina_streq(part, "right_content"))
     return ELM_PART_IMPLEMENT(EFL_UI_NAVIGATION_BAR_PART_CLASS, obj, part);

   return efl_part_get(efl_super(obj, MY_CLASS), part);
}

EOLIAN static void
_efl_ui_navigation_bar_part_back_button_efl_gfx_entity_visible_set(Eo *obj, void *_pd EINA_UNUSED, Eina_Bool visible)
{
   Elm_Part_Data *pd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   EFL_UI_NAVIGATION_BAR_DATA_GET_OR_RETURN(pd->obj, ppd);

   if (visible)
     {
        if (!efl_content_set(efl_part(efl_super(pd->obj, MY_CLASS), "efl.back_button"), ppd->back_button))
          ERR("Part for back button(i.e. \"back_button\") does not exist!");
        else
          efl_layout_signal_emit(pd->obj, "efl,state,back_button,visible", "efl");
     }
   else
     {
        efl_content_unset(efl_part(efl_super(pd->obj, MY_CLASS), "efl.back_button"));
        efl_gfx_entity_visible_set(ppd->back_button, visible);
        efl_layout_signal_emit(pd->obj, "efl,state,back_button,hidden", "efl");
     }

     efl_layout_signal_process(pd->obj, EINA_FALSE);
}

EOLIAN static Eina_Bool
_efl_ui_navigation_bar_part_back_button_efl_gfx_entity_visible_get(const Eo *obj, void *_pd EINA_UNUSED)
{
   Elm_Part_Data *pd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   EFL_UI_NAVIGATION_BAR_DATA_GET_OR_RETURN(pd->obj, ppd, EINA_FALSE);

   return efl_gfx_entity_visible_get(ppd->back_button);
}

EOLIAN static void
_efl_ui_navigation_bar_part_back_button_efl_text_text_set(Eo *obj, void *_pd EINA_UNUSED, const char *label)
{
   Elm_Part_Data *pd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   EFL_UI_NAVIGATION_BAR_DATA_GET_OR_RETURN(pd->obj, ppd);

   efl_text_set(ppd->back_button, label);
}

EOLIAN static const char *
_efl_ui_navigation_bar_part_back_button_efl_text_text_get(const Eo *obj, void *_pd EINA_UNUSED)
{
   Elm_Part_Data *pd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   EFL_UI_NAVIGATION_BAR_DATA_GET_OR_RETURN(pd->obj, ppd, NULL);

   return efl_text_get(ppd->back_button);
}

static Eina_Bool
_efl_ui_navigation_bar_part_back_button_efl_content_content_set(Eo *obj, void *_pd EINA_UNUSED, Efl_Gfx_Entity *content)
{
   Elm_Part_Data *pd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   EFL_UI_NAVIGATION_BAR_DATA_GET_OR_RETURN(pd->obj, ppd, EINA_FALSE);

   if (content == ppd->back_button) return EINA_FALSE;

   efl_event_callback_add(content, EFL_UI_EVENT_CLICKED, _back_button_clicked_cb, pd->obj);
   ppd->back_button = content;

   return _efl_ui_navigation_bar_content_set(pd->obj, ppd, pd->part, content);
}

static Efl_Gfx_Entity*
_efl_ui_navigation_bar_part_back_button_efl_content_content_get(const Eo *obj, void *_pd EINA_UNUSED)
{
   Elm_Part_Data *pd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   EFL_UI_NAVIGATION_BAR_DATA_GET_OR_RETURN(pd->obj, ppd, NULL);

   return _efl_ui_navigation_bar_content_get(pd->obj, ppd, pd->part);
}

static Efl_Gfx_Entity*
_efl_ui_navigation_bar_part_back_button_efl_content_content_unset(Eo *obj, void *_pd EINA_UNUSED)
{
   Elm_Part_Data *pd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   EFL_UI_NAVIGATION_BAR_DATA_GET_OR_RETURN(pd->obj, ppd, NULL);

   efl_event_callback_del(ppd->back_button, EFL_UI_EVENT_CLICKED, _back_button_clicked_cb, pd->obj);
   ppd->back_button = NULL;

   return _efl_ui_navigation_bar_content_unset(pd->obj, ppd, pd->part);
}

ELM_PART_OVERRIDE_CONTENT_SET(efl_ui_navigation_bar, EFL_UI_NAVIGATION_BAR, Efl_Ui_Navigation_Bar_Data)
ELM_PART_OVERRIDE_CONTENT_GET(efl_ui_navigation_bar, EFL_UI_NAVIGATION_BAR, Efl_Ui_Navigation_Bar_Data)
ELM_PART_OVERRIDE_CONTENT_UNSET(efl_ui_navigation_bar, EFL_UI_NAVIGATION_BAR, Efl_Ui_Navigation_Bar_Data)
#include "efl_ui_navigation_bar_part.eo.c"
/* Efl.Part end */


/* Efl.Part Back_Button begin */
#include "efl_ui_navigation_bar_part_back_button.eo.c"
/* Efl.Part Back_Button end */

#include "efl_ui_navigation_bar.eo.c"
