#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"
#include "efl_ui_popup_alert_private.h"
#include "efl_ui_popup_alert_part.eo.h"
#include "elm_part_helper.h"

#define MY_CLASS EFL_UI_POPUP_ALERT_CLASS
#define MY_CLASS_NAME "Efl.Ui.Popup.Alert"

static const char PART_NAME_BUTTON[] = "button";
static const char PART_NAME_BUTTON_LAYOUT[EFL_UI_POPUP_ALERT_BUTTON_COUNT][15] =
                                                {"button_layout1",
                                                 "button_layout2",
                                                 "button_layout3"};

EOLIAN static void
_efl_ui_popup_alert_elm_layout_sizing_eval(Eo *obj, Efl_Ui_Popup_Alert_Data *pd EINA_UNUSED)
{
   elm_layout_sizing_eval(efl_super(obj, MY_CLASS));

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   Evas_Coord minw = -1, minh = -1;

   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   edje_object_size_min_restricted_calc
      (wd->resize_obj, &minw, &minh, minw, minh);
   efl_gfx_size_hint_min_set(obj, EINA_SIZE2D(minw, minh));
}

static Eina_Bool
_efl_ui_popup_alert_text_set(Eo *obj, Efl_Ui_Popup_Alert_Data *pd, const char *part, const char *label)
{
   if (eina_streq(part, "title") || eina_streq(part, "elm.text.title"))
     {
        if (pd->title_text)
          {
             eina_stringshare_del(pd->title_text);
             pd->title_text = NULL;
          }

        pd->title_text = eina_stringshare_add(label);
        efl_text_set(efl_part(efl_super(obj, MY_CLASS), "elm.text.title"), label);
        elm_layout_signal_emit(obj, "elm,title,show", "elm");

        ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);
        edje_object_message_signal_process(wd->resize_obj);
        elm_layout_sizing_eval(obj);
     }
   else
     efl_text_set(efl_part(efl_super(obj, MY_CLASS), part), label);

   return EINA_TRUE;
}

const char *
_efl_ui_popup_alert_text_get(Eo *obj EINA_UNUSED, Efl_Ui_Popup_Alert_Data *pd, const char *part)
{
   if (eina_streq(part, "title") || eina_streq(part, "elm.text.title"))
     {
        if (pd->title_text)
          return pd->title_text;

        return NULL;
     }

   return efl_text_get(efl_part(efl_super(obj, MY_CLASS), part));
}

static void
_positive_button_clicked_cb(void *data, Eo *obj EINA_UNUSED,
                            void *event_info EINA_UNUSED)
{
   Eo *popup_obj = data;

   Efl_Ui_Popup_Alert_Clicked_Event event;
   event.button_type = EFL_UI_POPUP_ALERT_BUTTON_POSITIVE;

   efl_event_callback_call(popup_obj, EFL_UI_POPUP_ALERT_EVENT_CLICKED, &event);
}

static void
_negative_button_clicked_cb(void *data, Eo *obj EINA_UNUSED,
                            void *event_info EINA_UNUSED)
{
   Eo *popup_obj = data;

   Efl_Ui_Popup_Alert_Clicked_Event event;
   event.button_type = EFL_UI_POPUP_ALERT_BUTTON_NEGATIVE;

   efl_event_callback_call(popup_obj, EFL_UI_POPUP_ALERT_EVENT_CLICKED, &event);
}

static void
_user_button_clicked_cb(void *data, Eo *obj EINA_UNUSED,
                        void *event_info EINA_UNUSED)
{
   Eo *popup_obj = data;

   Efl_Ui_Popup_Alert_Clicked_Event event;
   event.button_type = EFL_UI_POPUP_ALERT_BUTTON_USER;

   efl_event_callback_call(popup_obj, EFL_UI_POPUP_ALERT_EVENT_CLICKED, &event);
}

EOLIAN static void
_efl_ui_popup_alert_button_set(Eo *obj, Efl_Ui_Popup_Alert_Data *pd, Efl_Ui_Popup_Alert_Button type, const char *text)
{
   if (pd->button[type])
     {
        efl_del(pd->button[type]);
        pd->button[type] = NULL;
     }
   pd->button[type] = efl_add(EFL_UI_BUTTON_CLASS, obj,
                              elm_widget_element_update(obj, efl_added, PART_NAME_BUTTON),
                              efl_text_set(efl_added, text));

   switch (type)
     {
      case EFL_UI_POPUP_ALERT_BUTTON_POSITIVE:
         evas_object_smart_callback_add(pd->button[type], "clicked",
                                        _positive_button_clicked_cb,
                                        obj);
         break;
      case EFL_UI_POPUP_ALERT_BUTTON_NEGATIVE:
         evas_object_smart_callback_add(pd->button[type], "clicked",
                                        _negative_button_clicked_cb,
                                        obj);
         break;
      case EFL_UI_POPUP_ALERT_BUTTON_USER:
         evas_object_smart_callback_add(pd->button[type], "clicked",
                                        _user_button_clicked_cb,
                                        obj);
         break;
      default:
         break;
     }

   Eo *cur_content = efl_content_unset(efl_part(obj, "buttons"));
   if (cur_content)
     {
        efl_content_unset(efl_part(cur_content, "elm.swallow.button1"));
        efl_content_unset(efl_part(cur_content, "elm.swallow.button2"));
        efl_content_unset(efl_part(cur_content, "elm.swallow.button3"));
        efl_del(cur_content);
     }

   if (pd->button[EFL_UI_POPUP_ALERT_BUTTON_POSITIVE]
       && pd->button[EFL_UI_POPUP_ALERT_BUTTON_NEGATIVE]
       && pd->button[EFL_UI_POPUP_ALERT_BUTTON_USER])
     {
        pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_3]
           = efl_add(EFL_UI_LAYOUT_CLASS, obj,
                     elm_widget_element_update(obj, efl_added,
                                               PART_NAME_BUTTON_LAYOUT[EFL_UI_POPUP_ALERT_BUTTON_3]));
        efl_content_set(efl_part(pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_3], "elm.swallow.button1"), pd->button[EFL_UI_POPUP_ALERT_BUTTON_USER]);
        efl_content_set(efl_part(pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_3], "elm.swallow.button2"), pd->button[EFL_UI_POPUP_ALERT_BUTTON_POSITIVE]);
        efl_content_set(efl_part(pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_3], "elm.swallow.button3"), pd->button[EFL_UI_POPUP_ALERT_BUTTON_NEGATIVE]);
        efl_content_set(efl_part(obj, "buttons"), pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_3]);
     }
   else if (pd->button[EFL_UI_POPUP_ALERT_BUTTON_POSITIVE]
            && pd->button[EFL_UI_POPUP_ALERT_BUTTON_NEGATIVE])
     {
        pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_2]
           = efl_add(EFL_UI_LAYOUT_CLASS, obj,
                     elm_widget_element_update(obj, efl_added,
                                               PART_NAME_BUTTON_LAYOUT[EFL_UI_POPUP_ALERT_BUTTON_2]));
        efl_content_set(efl_part(pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_2], "elm.swallow.button1"), pd->button[EFL_UI_POPUP_ALERT_BUTTON_POSITIVE]);
        efl_content_set(efl_part(pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_2], "elm.swallow.button2"), pd->button[EFL_UI_POPUP_ALERT_BUTTON_NEGATIVE]);
        efl_content_set(efl_part(obj, "buttons"), pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_2]);
     }
   else if (pd->button[EFL_UI_POPUP_ALERT_BUTTON_NEGATIVE]
            && pd->button[EFL_UI_POPUP_ALERT_BUTTON_USER])
     {
        pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_2]
           = efl_add(EFL_UI_LAYOUT_CLASS, obj,
                     elm_widget_element_update(obj, efl_added,
                                               PART_NAME_BUTTON_LAYOUT[EFL_UI_POPUP_ALERT_BUTTON_2]));
        efl_content_set(efl_part(pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_2], "elm.swallow.button1"), pd->button[EFL_UI_POPUP_ALERT_BUTTON_USER]);
        efl_content_set(efl_part(pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_2], "elm.swallow.button2"), pd->button[EFL_UI_POPUP_ALERT_BUTTON_NEGATIVE]);
        efl_content_set(efl_part(obj, "buttons"), pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_2]);
     }
   else if (pd->button[EFL_UI_POPUP_ALERT_BUTTON_POSITIVE]
            && pd->button[EFL_UI_POPUP_ALERT_BUTTON_USER])
     {
        pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_2]
           = efl_add(EFL_UI_LAYOUT_CLASS, obj,
                     elm_widget_element_update(obj, efl_added,
                                               PART_NAME_BUTTON_LAYOUT[EFL_UI_POPUP_ALERT_BUTTON_2]));
        efl_content_set(efl_part(pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_2], "elm.swallow.button1"), pd->button[EFL_UI_POPUP_ALERT_BUTTON_USER]);
        efl_content_set(efl_part(pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_2], "elm.swallow.button2"), pd->button[EFL_UI_POPUP_ALERT_BUTTON_POSITIVE]);
        efl_content_set(efl_part(obj, "buttons"), pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_2]);
     }
   else if (pd->button[EFL_UI_POPUP_ALERT_BUTTON_POSITIVE])
     {
        pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_1]
           = efl_add(EFL_UI_LAYOUT_CLASS, obj,
                     elm_widget_element_update(obj, efl_added,
                                               PART_NAME_BUTTON_LAYOUT[EFL_UI_POPUP_ALERT_BUTTON_1]));
        efl_content_set(efl_part(pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_1], "elm.swallow.button1"), pd->button[EFL_UI_POPUP_ALERT_BUTTON_POSITIVE]);
        efl_content_set(efl_part(obj, "buttons"), pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_1]);
     }
   else if (pd->button[EFL_UI_POPUP_ALERT_BUTTON_NEGATIVE])
     {
        pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_1]
           = efl_add(EFL_UI_LAYOUT_CLASS, obj,
                     elm_widget_element_update(obj, efl_added,
                                               PART_NAME_BUTTON_LAYOUT[EFL_UI_POPUP_ALERT_BUTTON_1]));
        efl_content_set(efl_part(pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_1], "elm.swallow.button1"), pd->button[EFL_UI_POPUP_ALERT_BUTTON_NEGATIVE]);
        efl_content_set(efl_part(obj, "buttons"), pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_1]);
     }
   else if (pd->button[EFL_UI_POPUP_ALERT_BUTTON_USER])
     {
        pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_1]
           = efl_add(EFL_UI_LAYOUT_CLASS, obj,
                     elm_widget_element_update(obj, efl_added,
                                               PART_NAME_BUTTON_LAYOUT[EFL_UI_POPUP_ALERT_BUTTON_1]));
        efl_content_set(efl_part(pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_1], "elm.swallow.button1"), pd->button[EFL_UI_POPUP_ALERT_BUTTON_USER]);
        efl_content_set(efl_part(obj, "buttons"), pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_1]);
     }

   elm_layout_signal_emit(obj, "elm,buttons,show", "elm");
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
   if (pd->title_text)
     {
        eina_stringshare_del(pd->title_text);
        pd->title_text = NULL;
     }

   efl_destructor(efl_super(obj, MY_CLASS));
}

static Eina_Bool
_part_is_efl_ui_popup_alert_part(const Eo *obj EINA_UNUSED, const char *part)
{
   return (eina_streq(part, "title") || eina_streq(part, "elm.text.title"));
}

/* Efl.Part begin */
ELM_PART_OVERRIDE_PARTIAL(efl_ui_popup_alert, EFL_UI_POPUP_ALERT,
                          Efl_Ui_Popup_Alert_Data, _part_is_efl_ui_popup_alert_part)
ELM_PART_OVERRIDE_TEXT_SET(efl_ui_popup_alert, EFL_UI_POPUP_ALERT, Efl_Ui_Popup_Alert_Data)
ELM_PART_OVERRIDE_TEXT_GET(efl_ui_popup_alert, EFL_UI_POPUP_ALERT, Efl_Ui_Popup_Alert_Data)
#include "efl_ui_popup_alert_part.eo.c"

/* Efl.Part end */

#define EFL_UI_POPUP_ALERT_EXTRA_OPS \
   ELM_LAYOUT_SIZING_EVAL_OPS(efl_ui_popup_alert)

#include "efl_ui_popup_alert.eo.c"
