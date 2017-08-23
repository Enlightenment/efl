#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"
#include "efl_ui_popup_alert_private.h"

#define MY_CLASS EFL_UI_POPUP_ALERT_CLASS
#define MY_CLASS_NAME "Efl.Ui.Popup.Alert"

EOLIAN static void
_efl_ui_popup_alert_elm_layout_sizing_eval(Eo *obj, Efl_Ui_Popup_Alert_Data *pd EINA_UNUSED)
{
   elm_layout_sizing_eval(efl_super(obj, MY_CLASS));

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   Evas_Coord minw = -1, minh = -1;

   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   edje_object_size_min_restricted_calc
     (wd->resize_obj, &minw, &minh, minw, minh);
   evas_object_size_hint_min_set(obj, minw, minh);

   Evas_Coord w, h;
   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
}

EOLIAN static void
_efl_ui_popup_alert_title_set(Eo *obj, Efl_Ui_Popup_Alert_Data *pd, const char *text)
{
   if (pd->title_text)
     {
        eina_stringshare_del(pd->title_text);
        pd->title_text = NULL;
     }

   pd->title_text = eina_stringshare_add(text);
   elm_object_part_text_set(obj, "elm.text.title", text);
   elm_layout_signal_emit(obj, "elm,title,show", "elm");

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   edje_object_message_signal_process(wd->resize_obj);
   elm_layout_sizing_eval(obj);
}

EOLIAN static const char *
_efl_ui_popup_alert_title_get(Eo *obj EINA_UNUSED, Efl_Ui_Popup_Alert_Data *pd)
{
   if (pd->title_text)
    return pd->title_text;
   return NULL;
}

static void
_positive_button_clicked_cb(void *data, Evas_Object *obj EINA_UNUSED,
                            void *event_info EINA_UNUSED)
{
    Eo *popup_obj = data;
    efl_event_callback_call(popup_obj, EFL_UI_POPUP_ALERT_EVENT_CLICKED, (void *)(uintptr_t)EFL_UI_POPUP_ALERT_BUTTON_POSITIVE);
}

static void
_negative_button_clicked_cb(void *data, Evas_Object *obj EINA_UNUSED,
                            void *event_info EINA_UNUSED)
{
   Eo *popup_obj = data;
   efl_event_callback_call(popup_obj, EFL_UI_POPUP_ALERT_EVENT_CLICKED, (void *)(uintptr_t)EFL_UI_POPUP_ALERT_BUTTON_NEGATIVE);
}

static void
_user_button_clicked_cb(void *data, Evas_Object *obj EINA_UNUSED,
                        void *event_info EINA_UNUSED)
{
   Eo *popup_obj = data;
   efl_event_callback_call(popup_obj, EFL_UI_POPUP_ALERT_EVENT_CLICKED, (void *)(uintptr_t)EFL_UI_POPUP_ALERT_BUTTON_USER);
}

EOLIAN static void
_efl_ui_popup_alert_button_set(Eo *obj, Efl_Ui_Popup_Alert_Data *pd, Efl_Ui_Popup_Alert_Button type, const char *text)
{
   if (pd->button[type])
     {
        evas_object_del(pd->button[type]);
        pd->button[type] = NULL;
     }
   pd->button[type] = elm_button_add(obj);
   elm_object_text_set(pd->button[type], text);

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

   Evas_Object * cur_content = efl_content_unset(efl_part(obj, "buttons"));
   if (cur_content)
     {
        efl_content_unset(efl_part(cur_content, "elm.swallow.button1"));
        efl_content_unset(efl_part(cur_content, "elm.swallow.button2"));
        efl_content_unset(efl_part(cur_content, "elm.swallow.button3"));
        evas_object_del(cur_content);
     }

   if (pd->button[EFL_UI_POPUP_ALERT_BUTTON_POSITIVE]
       && pd->button[EFL_UI_POPUP_ALERT_BUTTON_NEGATIVE]
       && pd->button[EFL_UI_POPUP_ALERT_BUTTON_USER])
     {
        pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_3] = elm_layout_add(obj);
        elm_layout_theme_set(pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_3], "popup", "base", "button3");
        efl_content_set(efl_part(pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_3], "elm.swallow.button1"), pd->button[EFL_UI_POPUP_ALERT_BUTTON_USER]);
        efl_content_set(efl_part(pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_3], "elm.swallow.button2"), pd->button[EFL_UI_POPUP_ALERT_BUTTON_POSITIVE]);
        efl_content_set(efl_part(pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_3], "elm.swallow.button3"), pd->button[EFL_UI_POPUP_ALERT_BUTTON_NEGATIVE]);
        efl_content_set(efl_part(obj, "buttons"), pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_3]);
     }
   else if (pd->button[EFL_UI_POPUP_ALERT_BUTTON_POSITIVE]
            && pd->button[EFL_UI_POPUP_ALERT_BUTTON_NEGATIVE])
     {
        pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_2] = elm_layout_add(obj);
        elm_layout_theme_set(pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_2], "popup", "base", "button2");
        efl_content_set(efl_part(pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_2], "elm.swallow.button1"), pd->button[EFL_UI_POPUP_ALERT_BUTTON_POSITIVE]);
        efl_content_set(efl_part(pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_2], "elm.swallow.button2"), pd->button[EFL_UI_POPUP_ALERT_BUTTON_NEGATIVE]);
        efl_content_set(efl_part(obj, "buttons"), pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_2]);
     }
   else if (pd->button[EFL_UI_POPUP_ALERT_BUTTON_NEGATIVE]
            && pd->button[EFL_UI_POPUP_ALERT_BUTTON_USER])
     {
        pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_2] = elm_layout_add(obj);
        elm_layout_theme_set(pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_2], "popup", "base", "button2");
        efl_content_set(efl_part(pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_2], "elm.swallow.button1"), pd->button[EFL_UI_POPUP_ALERT_BUTTON_USER]);
        efl_content_set(efl_part(pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_2], "elm.swallow.button2"), pd->button[EFL_UI_POPUP_ALERT_BUTTON_NEGATIVE]);
        efl_content_set(efl_part(obj, "buttons"), pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_2]);
     }
   else if (pd->button[EFL_UI_POPUP_ALERT_BUTTON_POSITIVE]
            && pd->button[EFL_UI_POPUP_ALERT_BUTTON_USER])
     {
        pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_2] = elm_layout_add(obj);
        elm_layout_theme_set(pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_2], "popup", "base", "button2");
        efl_content_set(efl_part(pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_2], "elm.swallow.button1"), pd->button[EFL_UI_POPUP_ALERT_BUTTON_USER]);
        efl_content_set(efl_part(pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_2], "elm.swallow.button2"), pd->button[EFL_UI_POPUP_ALERT_BUTTON_POSITIVE]);
        efl_content_set(efl_part(obj, "buttons"), pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_2]);
     }
   else if (pd->button[EFL_UI_POPUP_ALERT_BUTTON_POSITIVE])
     {
        pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_1] = elm_layout_add(obj);
        elm_layout_theme_set(pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_1], "popup", "base", "button1");
        efl_content_set(efl_part(pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_1], "elm.swallow.button1"), pd->button[EFL_UI_POPUP_ALERT_BUTTON_POSITIVE]);
        efl_content_set(efl_part(obj, "buttons"), pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_1]);
     }
   else if (pd->button[EFL_UI_POPUP_ALERT_BUTTON_NEGATIVE])
     {
        pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_1] = elm_layout_add(obj);
        elm_layout_theme_set(pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_1], "popup", "base", "button1");
        efl_content_set(efl_part(pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_1], "elm.swallow.button1"), pd->button[EFL_UI_POPUP_ALERT_BUTTON_NEGATIVE]);
        efl_content_set(efl_part(obj, "buttons"), pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_1]);
     }
   else if (pd->button[EFL_UI_POPUP_ALERT_BUTTON_USER])
     {
        pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_1] = elm_layout_add(obj);
        elm_layout_theme_set(pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_1], "popup", "base", "button1");
        efl_content_set(efl_part(pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_1], "elm.swallow.button1"), pd->button[EFL_UI_POPUP_ALERT_BUTTON_USER]);
        efl_content_set(efl_part(obj, "buttons"), pd->button_layout[EFL_UI_POPUP_ALERT_BUTTON_1]);
     }

     elm_layout_signal_emit(obj, "elm,buttons,show", "elm");
     elm_layout_sizing_eval(obj);
}

EOLIAN static void
_efl_ui_popup_alert_efl_canvas_group_group_add(Eo *obj, Efl_Ui_Popup_Alert_Data *pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   efl_canvas_group_add(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);

   elm_layout_theme_set(efl_super(obj, MY_CLASS), "popup", "base", "alert");
}

EOLIAN static void
_efl_ui_popup_alert_efl_canvas_group_group_del(Eo *obj, Efl_Ui_Popup_Alert_Data *pd)
{
   if (pd->title_text)
     {
        eina_stringshare_del(pd->title_text);
        pd->title_text = NULL;
     }

   efl_canvas_group_del(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_ui_popup_alert_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME, klass);
}

#define EFL_UI_POPUP_ALERT_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(efl_ui_popup_alert), \
   ELM_LAYOUT_SIZING_EVAL_OPS(efl_ui_popup_alert)

#include "efl_ui_popup_alert.eo.c"
