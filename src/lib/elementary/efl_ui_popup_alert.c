#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_PART_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "efl_ui_popup_alert_private.h"
#include "efl_ui_popup_alert_part.eo.h"
#include "elm_part_helper.h"

#define MY_CLASS EFL_UI_POPUP_ALERT_CLASS
#define MY_CLASS_NAME "Efl.Ui.Popup_Alert"

static const char PART_NAME_BUTTON[] = "button";
static const char PART_NAME_BUTTON_LAYOUT[EFL_UI_POPUP_ALERT_BUTTON_COUNT][15] =
                                                {"button_layout1",
                                                 "button_layout2",
                                                 "button_layout3"};

static const char BUTTON_SWALLOW_NAME[EFL_UI_POPUP_ALERT_BUTTON_COUNT][20] =
                                                {"efl.button1",
                                                 "efl.button2",
                                                 "efl.button3"};

static Eina_Bool
_efl_ui_popup_alert_text_set(Eo *obj, Efl_Ui_Popup_Alert_Data *pd, const char *part, const char *label)
{
   if (eina_streq(part, "title") || eina_streq(part, "efl.text.title"))
     {
        Eina_Bool changed = eina_stringshare_replace(&pd->title_text, label);
        if (changed)
          {
             efl_text_set(efl_part(efl_super(obj, MY_CLASS), "title"), label);
             if (label)
               elm_layout_signal_emit(obj, "efl,title,show", "efl");
             else
               elm_layout_signal_emit(obj, "efl,title,hide", "efl");

             ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);
             edje_object_message_signal_process(wd->resize_obj);
             elm_layout_sizing_eval(obj);
          }
     }
   else
     efl_text_set(efl_part(efl_super(obj, MY_CLASS), part), label);

   return EINA_TRUE;
}

const char *
_efl_ui_popup_alert_text_get(Eo *obj EINA_UNUSED, Efl_Ui_Popup_Alert_Data *pd, const char *part)
{
   if (eina_streq(part, "title") || eina_streq(part, "efl.text.title"))
     {
        if (pd->title_text)
          return pd->title_text;

        return NULL;
     }

   return efl_text_get(efl_part(efl_super(obj, MY_CLASS), part));
}

static void
_positive_button_clicked_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *popup_obj = data;

   Efl_Ui_Popup_Alert_Button_Clicked_Event event;
   event.button_type = EFL_UI_POPUP_ALERT_BUTTON_POSITIVE;

   efl_event_callback_call(popup_obj, EFL_UI_POPUP_ALERT_EVENT_BUTTON_CLICKED, &event);
}

static void
_negative_button_clicked_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *popup_obj = data;

   Efl_Ui_Popup_Alert_Button_Clicked_Event event;
   event.button_type = EFL_UI_POPUP_ALERT_BUTTON_NEGATIVE;

   efl_event_callback_call(popup_obj, EFL_UI_POPUP_ALERT_EVENT_BUTTON_CLICKED, &event);
}

static void
_user_button_clicked_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *popup_obj = data;

   Efl_Ui_Popup_Alert_Button_Clicked_Event event;
   event.button_type = EFL_UI_POPUP_ALERT_BUTTON_USER;

   efl_event_callback_call(popup_obj, EFL_UI_POPUP_ALERT_EVENT_BUTTON_CLICKED, &event);
}

EOLIAN static void
_efl_ui_popup_alert_button_set(Eo *obj, Efl_Ui_Popup_Alert_Data *pd, Efl_Ui_Popup_Alert_Button type, const char *text, Eo *icon)
{
   int i;
   Eo *cur_content;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if ((type < EFL_UI_POPUP_ALERT_BUTTON_POSITIVE) || (type > EFL_UI_POPUP_ALERT_BUTTON_USER))
     {
        ERR("Wrong type (%d) is passed!", type);
        return;
     }
   if (!pd->button[type])
     {
        pd->button[type] = efl_add(EFL_UI_BUTTON_CLASS, obj,
                                   elm_widget_element_update(obj, efl_added,
                                                             PART_NAME_BUTTON));
        switch (type)
          {
            case EFL_UI_POPUP_ALERT_BUTTON_POSITIVE:
              efl_event_callback_add(pd->button[type], EFL_UI_EVENT_CLICKED,
                                     _positive_button_clicked_cb, obj);
              break;
            case EFL_UI_POPUP_ALERT_BUTTON_NEGATIVE:
              efl_event_callback_add(pd->button[type], EFL_UI_EVENT_CLICKED,
                                     _negative_button_clicked_cb, obj);
              break;
            case EFL_UI_POPUP_ALERT_BUTTON_USER:
              efl_event_callback_add(pd->button[type], EFL_UI_EVENT_CLICKED,
                                     _user_button_clicked_cb, obj);
              break;
            default:
              break;
          }
     }
   efl_text_set(pd->button[type], text);
   efl_content_set(pd->button[type], icon);

   cur_content = efl_content_get(efl_part(obj, "efl.buttons"));
   if (cur_content)
     {
        for (i = 0; i < EFL_UI_POPUP_ALERT_BUTTON_COUNT; i++)
          efl_content_unset(efl_part(cur_content, BUTTON_SWALLOW_NAME[i]));
     }
   else
     {
        cur_content = efl_add(EFL_UI_LAYOUT_CLASS, obj,
                              efl_content_set(efl_part(obj, "efl.buttons"), efl_added));
     }

   int btn_count = !!pd->button[EFL_UI_POPUP_ALERT_BUTTON_POSITIVE] +
                   !!pd->button[EFL_UI_POPUP_ALERT_BUTTON_NEGATIVE] +
                   !!pd->button[EFL_UI_POPUP_ALERT_BUTTON_USER];
   elm_widget_element_update(obj, cur_content, PART_NAME_BUTTON_LAYOUT[btn_count - 1]);

   i = 0;
   if (pd->button[EFL_UI_POPUP_ALERT_BUTTON_USER])
     {
        efl_content_set(efl_part(cur_content, BUTTON_SWALLOW_NAME[i]),
                                 pd->button[EFL_UI_POPUP_ALERT_BUTTON_USER]);
        i++;
     }
   if (pd->button[EFL_UI_POPUP_ALERT_BUTTON_POSITIVE])
     {
        efl_content_set(efl_part(cur_content, BUTTON_SWALLOW_NAME[i]),
                                 pd->button[EFL_UI_POPUP_ALERT_BUTTON_POSITIVE]);
        i++;
     }
   if (pd->button[EFL_UI_POPUP_ALERT_BUTTON_NEGATIVE])
     {
        efl_content_set(efl_part(cur_content, BUTTON_SWALLOW_NAME[i]),
                                 pd->button[EFL_UI_POPUP_ALERT_BUTTON_NEGATIVE]);
     }

   elm_layout_signal_emit(obj, "efl,buttons,show", "efl");
   edje_object_message_signal_process(wd->resize_obj);
   elm_layout_sizing_eval(obj);
}

EOLIAN static Eo *
_efl_ui_popup_alert_efl_object_constructor(Eo *obj,
                                           Efl_Ui_Popup_Alert_Data *pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   if (!elm_widget_theme_klass_get(obj))
     elm_widget_theme_klass_set(obj, "popup_alert");
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME);

   elm_widget_sub_object_parent_add(obj);

   return obj;
}

EOLIAN static void
_efl_ui_popup_alert_efl_object_destructor(Eo *obj, Efl_Ui_Popup_Alert_Data *pd)
{
   ELM_SAFE_FREE(pd->title_text, eina_stringshare_del);
   efl_destructor(efl_super(obj, MY_CLASS));
}

static Eina_Bool
_part_is_efl_ui_popup_alert_part(const Eo *obj EINA_UNUSED, const char *part)
{
   return (eina_streq(part, "title") || eina_streq(part, "efl.text.title"));
}

/* Efl.Part begin */
ELM_PART_OVERRIDE_PARTIAL(efl_ui_popup_alert, EFL_UI_POPUP_ALERT,
                          Efl_Ui_Popup_Alert_Data, _part_is_efl_ui_popup_alert_part)
ELM_PART_OVERRIDE_TEXT_SET(efl_ui_popup_alert, EFL_UI_POPUP_ALERT, Efl_Ui_Popup_Alert_Data)
ELM_PART_OVERRIDE_TEXT_GET(efl_ui_popup_alert, EFL_UI_POPUP_ALERT, Efl_Ui_Popup_Alert_Data)
#include "efl_ui_popup_alert_part.eo.c"

/* Efl.Part end */

#include "efl_ui_popup_alert.eo.c"
