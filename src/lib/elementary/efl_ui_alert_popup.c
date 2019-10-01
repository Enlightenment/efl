#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_PART_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "efl_ui_alert_popup_private.h"
#include "efl_ui_alert_popup_part.eo.h"
#include "elm_part_helper.h"

#define MY_CLASS EFL_UI_ALERT_POPUP_CLASS
#define MY_CLASS_NAME "Efl.Ui.Alert_Popup"

static const char PART_NAME_BUTTON[] = "button";
static const char PART_NAME_BUTTON_LAYOUT[EFL_UI_ALERT_POPUP_BUTTON_COUNT][15] =
                                                {"button_layout1",
                                                 "button_layout2",
                                                 "button_layout3"};

static const char BUTTON_SWALLOW_NAME[EFL_UI_ALERT_POPUP_BUTTON_COUNT][20] =
                                                {"efl.button1",
                                                 "efl.button2",
                                                 "efl.button3"};

static const Elm_Layout_Part_Alias_Description _text_aliases[] =
{
   {"title", "efl.text.title"},
   {NULL, NULL}
};

static Eina_Bool
_efl_ui_alert_popup_text_set(Eo *obj, Efl_Ui_Alert_Popup_Data *pd, const char *part, const char *label)
{
   if (!_elm_layout_part_aliasing_eval(obj, &part, EINA_TRUE))
      return EINA_FALSE;
   efl_text_set(efl_part(efl_super(obj, MY_CLASS), part), label);
   if (eina_streq(part, "efl.text.title"))
     {
        Eina_Bool changed = eina_stringshare_replace(&pd->title_text, label);
        if (changed)
          {
             efl_text_set(efl_part(efl_super(obj, MY_CLASS), part), label);
             if (label)
               elm_layout_signal_emit(obj, "efl,title,visible,on", "efl");
             else
               elm_layout_signal_emit(obj, "efl,title,visible,off", "efl");

             ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);
             edje_object_message_signal_process(wd->resize_obj);
             efl_canvas_group_change(obj);
          }
     }

   return EINA_TRUE;
}

const char *
_efl_ui_alert_popup_text_get(Eo *obj EINA_UNUSED, Efl_Ui_Alert_Popup_Data *pd, const char *part)
{
   if (!_elm_layout_part_aliasing_eval(obj, &part, EINA_TRUE))
      return EINA_FALSE;
   if (eina_streq(part, "efl.text.title"))
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

   Efl_Ui_Alert_Popup_Button_Clicked_Event event;
   event.button_type = EFL_UI_ALERT_POPUP_BUTTON_POSITIVE;

   efl_event_callback_call(popup_obj, EFL_UI_ALERT_POPUP_EVENT_BUTTON_CLICKED, &event);
}

static void
_negative_button_clicked_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *popup_obj = data;

   Efl_Ui_Alert_Popup_Button_Clicked_Event event;
   event.button_type = EFL_UI_ALERT_POPUP_BUTTON_NEGATIVE;

   efl_event_callback_call(popup_obj, EFL_UI_ALERT_POPUP_EVENT_BUTTON_CLICKED, &event);
}

static void
_user_button_clicked_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *popup_obj = data;

   Efl_Ui_Alert_Popup_Button_Clicked_Event event;
   event.button_type = EFL_UI_ALERT_POPUP_BUTTON_USER;

   efl_event_callback_call(popup_obj, EFL_UI_ALERT_POPUP_EVENT_BUTTON_CLICKED, &event);
}

static void
_apply_button_style(Eo *obj, Efl_Ui_Alert_Popup_Data *pd, int button_cnt)
{
   if (pd->button[EFL_UI_ALERT_POPUP_BUTTON_USER])
     {
        if (button_cnt > 1)
          elm_widget_element_update(obj,
                                    pd->button[EFL_UI_ALERT_POPUP_BUTTON_USER],
                                    "left_button");
     }

   if (pd->button[EFL_UI_ALERT_POPUP_BUTTON_POSITIVE])
     {
        if (button_cnt == 2)
          {
             if (pd->button[EFL_UI_ALERT_POPUP_BUTTON_USER])
               elm_widget_element_update(obj,
                                         pd->button[EFL_UI_ALERT_POPUP_BUTTON_POSITIVE],
                                         "right_button");
             else
               elm_widget_element_update(obj,
                                         pd->button[EFL_UI_ALERT_POPUP_BUTTON_POSITIVE],
                                         "left_button");
          }
     }

   if (pd->button[EFL_UI_ALERT_POPUP_BUTTON_NEGATIVE])
     {
        if (button_cnt > 1)
          elm_widget_element_update(obj,
                                    pd->button[EFL_UI_ALERT_POPUP_BUTTON_NEGATIVE],
                                    "right_button");
     }
}

EOLIAN static void
_efl_ui_alert_popup_button_set(Eo *obj, Efl_Ui_Alert_Popup_Data *pd, Efl_Ui_Alert_Popup_Button type, const char *text, Eo *icon)
{
   int i;
   Eina_Bool is_btn_created = EINA_FALSE;
   Eo *cur_content;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if ((type < EFL_UI_ALERT_POPUP_BUTTON_POSITIVE) || (type > EFL_UI_ALERT_POPUP_BUTTON_USER))
     {
        ERR("Wrong type (%d) is passed!", type);
        return;
     }
   if (!pd->button[type])
     {
        is_btn_created = EINA_TRUE;
        pd->button[type] = efl_add(EFL_UI_BUTTON_CLASS, obj,
                                   elm_widget_element_update(obj, efl_added,
                                                             PART_NAME_BUTTON));
        switch (type)
          {
            case EFL_UI_ALERT_POPUP_BUTTON_POSITIVE:
              efl_event_callback_add(pd->button[type], EFL_INPUT_EVENT_CLICKED,
                                     _positive_button_clicked_cb, obj);
              break;
            case EFL_UI_ALERT_POPUP_BUTTON_NEGATIVE:
              efl_event_callback_add(pd->button[type], EFL_INPUT_EVENT_CLICKED,
                                     _negative_button_clicked_cb, obj);
              break;
            case EFL_UI_ALERT_POPUP_BUTTON_USER:
              efl_event_callback_add(pd->button[type], EFL_INPUT_EVENT_CLICKED,
                                     _user_button_clicked_cb, obj);
              break;
            default:
              break;
          }
     }
   else
     {
        const char *pre_text = efl_text_get(pd->button[type]);
        if ((pre_text != NULL) && (text != NULL) &&
            (!strcmp(pre_text, text)) &&
            (efl_content_get(pd->button[type]) == icon))
          return;
     }

   efl_text_set(pd->button[type], text);
   efl_content_set(pd->button[type], icon);

   if (is_btn_created)
     {
        int btn_count = !!pd->button[EFL_UI_ALERT_POPUP_BUTTON_POSITIVE] +
                        !!pd->button[EFL_UI_ALERT_POPUP_BUTTON_NEGATIVE] +
                        !!pd->button[EFL_UI_ALERT_POPUP_BUTTON_USER];

        cur_content = efl_content_get(efl_part(obj, "efl.buttons"));
        for (i = 0; i < EFL_UI_ALERT_POPUP_BUTTON_COUNT; i++)
          efl_content_unset(efl_part(cur_content, BUTTON_SWALLOW_NAME[i]));

        elm_widget_element_update(obj, cur_content, PART_NAME_BUTTON_LAYOUT[btn_count - 1]);

        _apply_button_style(obj, pd, btn_count);

        i = 0;
        if (pd->button[EFL_UI_ALERT_POPUP_BUTTON_USER])
          {
             efl_content_set(efl_part(cur_content, BUTTON_SWALLOW_NAME[i]),
                             pd->button[EFL_UI_ALERT_POPUP_BUTTON_USER]);
             i++;
          }

        if (pd->button[EFL_UI_ALERT_POPUP_BUTTON_POSITIVE])
          {
             efl_content_set(efl_part(cur_content, BUTTON_SWALLOW_NAME[i]),
                             pd->button[EFL_UI_ALERT_POPUP_BUTTON_POSITIVE]);
             i++;
          }

        if (pd->button[EFL_UI_ALERT_POPUP_BUTTON_NEGATIVE])
          {
             efl_content_set(efl_part(cur_content, BUTTON_SWALLOW_NAME[i]),
                             pd->button[EFL_UI_ALERT_POPUP_BUTTON_NEGATIVE]);
          }

        elm_layout_signal_emit(obj, "efl,buttons,visible,on", "efl");
        edje_object_message_signal_process(wd->resize_obj);
     }

   efl_canvas_group_change(obj);
}

EOLIAN static Eo *
_efl_ui_alert_popup_efl_object_constructor(Eo *obj,
                                           Efl_Ui_Alert_Popup_Data *pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   if (!elm_widget_theme_klass_get(obj))
     elm_widget_theme_klass_set(obj, "alert_popup");
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME);

   efl_add(EFL_UI_LAYOUT_CLASS, obj,
           efl_content_set(efl_part(obj, "efl.buttons"), efl_added));

   return obj;
}

EOLIAN static void
_efl_ui_alert_popup_efl_object_destructor(Eo *obj, Efl_Ui_Alert_Popup_Data *pd)
{
   ELM_SAFE_FREE(pd->title_text, eina_stringshare_del);
   efl_destructor(efl_super(obj, MY_CLASS));
}

static Eina_Bool
_part_is_efl_ui_alert_popup_part(const Eo *obj, const char *part)
{
   if (!_elm_layout_part_aliasing_eval(obj, &part, EINA_TRUE)) return EINA_FALSE;
   return eina_streq(part, "efl.text.title");
}

/* Efl.Part begin */
ELM_PART_OVERRIDE_PARTIAL(efl_ui_alert_popup, EFL_UI_ALERT_POPUP,
                          Efl_Ui_Alert_Popup_Data, _part_is_efl_ui_alert_popup_part)
ELM_PART_OVERRIDE_TEXT_SET(efl_ui_alert_popup, EFL_UI_ALERT_POPUP, Efl_Ui_Alert_Popup_Data)
ELM_PART_OVERRIDE_TEXT_GET(efl_ui_alert_popup, EFL_UI_ALERT_POPUP, Efl_Ui_Alert_Popup_Data)
#include "efl_ui_alert_popup_part.eo.c"

/* Efl.Part end */
EFL_UI_LAYOUT_TEXT_ALIASES_IMPLEMENT(efl_ui_alert_popup)

#define EFL_UI_ALERT_POPUP_EXTRA_OPS \
   EFL_UI_LAYOUT_TEXT_ALIASES_OPS(efl_ui_alert_popup)
#include "efl_ui_alert_popup.eo.c"
