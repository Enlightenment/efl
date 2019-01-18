//FIXME this widget should inherit from file selector button
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_PROTECTED
#define ELM_INTERFACE_FILESELECTOR_BETA
#define EFL_PART_PROTECTED

#include <Elementary.h>
#include "Eio_Eo.h"
#include "elm_priv.h"
#include "elm_fileselector_button.eo.h"
#include "elm_fileselector_entry.eo.h"
#include "elm_interface_fileselector.h"
#include "elm_widget_fileselector_entry.h"
#include "elm_entry.eo.h"
#include "elm_fileselector.eo.h"

#include "elm_fileselector_entry_part.eo.h"
#include "elm_part_helper.h"

#define MY_CLASS ELM_FILESELECTOR_ENTRY_CLASS

#define MY_CLASS_NAME "Elm_Fileselector_Entry"
#define MY_CLASS_NAME_LEGACY "elm_fileselector_entry"

#define ELM_PRIV_FILESELECTOR_ENTRY_SIGNALS(cmd) \
   cmd(SIG_CHANGED, "changed", "") \
   cmd(SIG_ACTIVATED, "activated", "") \
   cmd(SIG_PRESS, "press", "") \
   cmd(SIG_LONGPRESSED, "longpressed", "") \
   cmd(SIG_CLICKED, "clicked", "") \
   cmd(SIG_CLICKED_DOUBLE, "clicked,double", "") \
   cmd(SIG_FOCUSED, "focused", "") \
   cmd(SIG_UNFOCUSED, "unfocused", "") \
   cmd(SIG_SELECTION_PASTE, "selection,paste", "") \
   cmd(SIG_SELECTION_COPY, "selection,copy", "") \
   cmd(SIG_SELECTION_CUT, "selection,cut", "") \
   cmd(SIG_UNPRESSED, "unpressed", "") \
   cmd(SIG_FILE_CHOSEN, "file,chosen", "s") \

ELM_PRIV_FILESELECTOR_ENTRY_SIGNALS(ELM_PRIV_STATIC_VARIABLE_DECLARE);

static const Evas_Smart_Cb_Description _smart_callbacks[] =
{
   ELM_PRIV_FILESELECTOR_ENTRY_SIGNALS(ELM_PRIV_SMART_CALLBACKS_DESC)
   {SIG_WIDGET_LANG_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_WIDGET_ACCESS_CHANGED, ""}, /**< handled by elm_widget */
   {NULL, NULL}
};
#undef ELM_PRIV_FILESELECTOR_ENTRY_SIGNALS

#define SIG_FWD(name, event)                                                      \
  static void                                                               \
  _##name##_fwd(void *data, const Efl_Event *ev EINA_UNUSED)                                          \
  {                                                                         \
     efl_event_callback_legacy_call(data, event, ev->info);          \
  }
SIG_FWD(CHANGED, ELM_FILESELECTOR_ENTRY_EVENT_CHANGED)
SIG_FWD(PRESS, ELM_FILESELECTOR_ENTRY_EVENT_PRESS)
SIG_FWD(LONGPRESSED, EFL_UI_EVENT_LONGPRESSED)
SIG_FWD(CLICKED, EFL_UI_EVENT_CLICKED)
SIG_FWD(CLICKED_DOUBLE, EFL_UI_EVENT_CLICKED_DOUBLE)
SIG_FWD(SELECTION_PASTE, EFL_UI_EVENT_SELECTION_PASTE)
SIG_FWD(SELECTION_COPY, EFL_UI_EVENT_SELECTION_COPY)
SIG_FWD(SELECTION_CUT, EFL_UI_EVENT_SELECTION_CUT)
SIG_FWD(UNPRESSED, EFL_UI_EVENT_UNPRESSED)
#undef SIG_FWD

static void
_FILE_CHOSEN_fwd(void *data, const Efl_Event *event)
{
   Efl_Model *model = event->info;
   Eo *fs = data;
   Eina_Value *path;
   char *file = NULL;
   ELM_FILESELECTOR_ENTRY_DATA_GET(fs, sd);

   efl_ui_view_model_set(sd->entry, model);
   efl_ui_model_connect(sd->entry, "default", "path");

   path = efl_model_property_get(model, "path");
   file = eina_value_to_string(path);

   _model_event_call
     (fs, ELM_FILESELECTOR_ENTRY_EVENT_FILE_CHOSEN, model, file);

   eina_value_free(path);
   free(file);
}

static void
_ACTIVATED_fwd(void *data, const Efl_Event *event)
{
   const char *file;
   Efl_Model *bmodel, *model;
   Eina_Value path;

   ELM_FILESELECTOR_ENTRY_DATA_GET(data, sd);

   file = elm_object_text_get(sd->entry);

   bmodel = efl_ui_view_model_get(sd->button);
   if (bmodel)
     {
         model = efl_add(efl_class_get(bmodel), NULL);
         eina_value_setup(&path, EINA_VALUE_TYPE_STRING);
         eina_value_set(&path, file);
         efl_model_property_set(model, "path", &path);
         eina_value_flush(&path);
         efl_ui_view_model_set(sd->button, model);
     }

   efl_event_callback_legacy_call
     (data, ELM_FILESELECTOR_ENTRY_EVENT_ACTIVATED, event->info);
}

EOLIAN static void
_elm_fileselector_entry_elm_layout_sizing_eval(Eo *obj, Elm_Fileselector_Entry_Data *sd EINA_UNUSED)
{
   Evas_Coord minw = -1, minh = -1;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   edje_object_size_min_calc(wd->resize_obj, &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, -1, -1);
}

EOLIAN static Efl_Ui_Theme_Apply_Result
_elm_fileselector_entry_efl_ui_widget_theme_apply(Eo *obj, Elm_Fileselector_Entry_Data *sd)
{
   const char *style;
   char buf[1024];

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EFL_UI_THEME_APPLY_RESULT_FAIL);

   Efl_Ui_Theme_Apply_Result int_ret = EFL_UI_THEME_APPLY_RESULT_FAIL;
   int_ret = efl_ui_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (!int_ret) return EFL_UI_THEME_APPLY_RESULT_FAIL;

   style = elm_widget_style_get(obj);

   efl_ui_mirrored_set(sd->button, efl_ui_mirrored_get(obj));

   if (elm_object_disabled_get(obj))
     elm_layout_signal_emit(obj, "elm,state,disabled", "elm");

   if (!style) style = "default";
   snprintf(buf, sizeof(buf), "fileselector_entry/%s", style);
   elm_widget_style_set(sd->button, buf);
   elm_widget_style_set(sd->entry, buf);

   edje_object_message_signal_process(wd->resize_obj);

   elm_layout_sizing_eval(obj);

   return int_ret;
}

EOLIAN static Eina_Bool
_elm_fileselector_entry_efl_ui_widget_on_disabled_update(Eo *obj, Elm_Fileselector_Entry_Data *sd, Eina_Bool disabled)
{
   if (!efl_ui_widget_on_disabled_update(efl_super(obj, MY_CLASS), disabled))
     return EINA_FALSE;

   elm_widget_disabled_set(sd->button, disabled);
   elm_widget_disabled_set(sd->entry, disabled);

   return EINA_TRUE;
}

static Eina_Bool
_elm_fileselector_entry_text_set(Eo *obj, Elm_Fileselector_Entry_Data *sd, const char *part, const char *label)
{
   if (part && strcmp(part, "elm.text"))
     {
        efl_text_set(efl_part(efl_super(obj, MY_CLASS), part), label);
     }

   elm_object_text_set(sd->button, label);
   return EINA_TRUE;
}

static const char *
_elm_fileselector_entry_text_get(Eo *obj, Elm_Fileselector_Entry_Data *sd, const char *part)
{
   if (part && strcmp(part, "elm.text"))
     {
        const char *text = NULL;
        text = efl_text_get(efl_part(efl_super(obj, MY_CLASS), part));
        return text;
     }

   return elm_object_text_get(sd->button);
}

static Eina_Bool
_elm_fileselector_entry_content_set(Eo *obj, Elm_Fileselector_Entry_Data *sd, const char *part, Evas_Object *content)
{
   if (part && strcmp(part, "button icon"))
     {
        return efl_content_set(efl_part(efl_super(obj, MY_CLASS), part), content);
     }

   elm_layout_content_set(sd->button, NULL, content);

   return EINA_TRUE;
}

static Evas_Object *
_elm_fileselector_entry_content_get(Eo *obj, Elm_Fileselector_Entry_Data *sd, const char *part)
{
   if (part && strcmp(part, "button icon"))
     {
        return efl_content_get(efl_part(efl_super(obj, MY_CLASS), part));
     }

   return elm_layout_content_get(sd->button, NULL);
}

static Evas_Object *
_elm_fileselector_entry_content_unset(Eo *obj, Elm_Fileselector_Entry_Data *sd, const char *part)
{
   if (part && strcmp(part, "button icon"))
     {
        return efl_content_unset(efl_part(efl_super(obj, MY_CLASS), part));
     }

   return elm_layout_content_unset(sd->button, NULL);
}

EOLIAN static void
_elm_fileselector_entry_efl_canvas_group_group_add(Eo *obj, Elm_Fileselector_Entry_Data *priv)
{
   efl_canvas_group_add(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);

   priv->button = elm_fileselector_button_add(obj);
   efl_ui_mirrored_automatic_set(priv->button, EINA_FALSE);
   efl_ui_mirrored_set(priv->button, efl_ui_mirrored_get(obj));
   elm_widget_style_set(priv->button, "fileselector_entry/default");

   elm_fileselector_expandable_set
     (priv->button, _elm_config->fileselector_expand_enable);

#define SIG_FWD(name, event) \
  efl_event_callback_add(priv->button, event, _##name##_fwd, obj)
   SIG_FWD(CLICKED, EFL_UI_EVENT_CLICKED);
   SIG_FWD(UNPRESSED, EFL_UI_EVENT_UNPRESSED);
   SIG_FWD(FILE_CHOSEN, ELM_FILESELECTOR_BUTTON_EVENT_FILE_CHOSEN);
#undef SIG_FWD

   priv->entry = elm_entry_add(obj);
   elm_entry_scrollable_set(priv->entry, EINA_TRUE);
   efl_ui_mirrored_automatic_set(priv->entry, EINA_FALSE);
   elm_widget_style_set(priv->entry, "fileselector_entry/default");
   elm_entry_single_line_set(priv->entry, EINA_TRUE);
   elm_entry_editable_set(priv->entry, EINA_TRUE);

#define SIG_FWD(name, event) \
  efl_event_callback_add(priv->entry, event, _##name##_fwd, obj)
   SIG_FWD(CHANGED, ELM_ENTRY_EVENT_CHANGED);
   SIG_FWD(ACTIVATED, ELM_ENTRY_EVENT_ACTIVATED);
   SIG_FWD(PRESS, ELM_ENTRY_EVENT_PRESS);
   SIG_FWD(LONGPRESSED, EFL_UI_EVENT_LONGPRESSED);
   SIG_FWD(CLICKED, EFL_UI_EVENT_CLICKED);
   SIG_FWD(CLICKED_DOUBLE, EFL_UI_EVENT_CLICKED_DOUBLE);
   SIG_FWD(SELECTION_PASTE, EFL_UI_EVENT_SELECTION_PASTE);
   SIG_FWD(SELECTION_COPY, EFL_UI_EVENT_SELECTION_COPY);
   SIG_FWD(SELECTION_CUT, EFL_UI_EVENT_SELECTION_CUT);
#undef SIG_FWD

   efl_event_callback_forwarder_add(priv->entry, EFL_UI_FOCUS_OBJECT_EVENT_FOCUS_CHANGED, obj);

   if (!elm_layout_theme_set
       (obj, "fileselector_entry", "base", elm_widget_style_get(obj)))
     CRI("Failed to set layout!");
   else
     {
        elm_layout_content_set(obj, "elm.swallow.button", priv->button);
        elm_layout_content_set(obj, "elm.swallow.entry", priv->entry);
     }

   elm_layout_sizing_eval(obj);
}

EOLIAN static void
_elm_fileselector_entry_efl_canvas_group_group_del(Eo *obj, Elm_Fileselector_Entry_Data *sd)
{
   free(sd->path);

   efl_canvas_group_del(efl_super(obj, MY_CLASS));
}

EAPI Evas_Object *
elm_fileselector_entry_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   return elm_legacy_add(MY_CLASS, parent);
}

EOLIAN static Eo *
_elm_fileselector_entry_efl_object_constructor(Eo *obj, Elm_Fileselector_Entry_Data *sd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);
   evas_object_smart_callbacks_descriptions_set(obj, _smart_callbacks);
   efl_access_object_role_set(obj, EFL_ACCESS_ROLE_GROUPING);
   legacy_child_focus_handle(obj);

   return obj;
}

EINA_DEPRECATED EAPI void
elm_fileselector_entry_selected_set(Evas_Object *obj, const char *path)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj);
   elm_fileselector_selected_set(obj, path);
}

Eina_Bool
_elm_fileselector_entry_selected_set_internal(Evas_Object *obj, const char *path)
{
   ELM_FILESELECTOR_ENTRY_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);
   elm_fileselector_path_set(sd->button, path);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_fileselector_entry_elm_interface_fileselector_selected_model_set(Eo *obj EINA_UNUSED,
                                                                      Elm_Fileselector_Entry_Data *sd,
                                                                      Efl_Model *model)
{
   efl_ui_view_model_set(sd->button, model);

   return EINA_TRUE;
}

EINA_DEPRECATED EAPI const char *
elm_fileselector_entry_selected_get(const Evas_Object *obj)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj, NULL);
   return elm_fileselector_selected_get((Eo *) obj);
}

const char *
_elm_fileselector_entry_selected_get_internal(const Evas_Object *obj)
{
   ELM_FILESELECTOR_ENTRY_DATA_GET_OR_RETURN_VAL(obj, sd, NULL);
   return elm_fileselector_path_get(sd->button);
}

EOLIAN static Efl_Model *
_elm_fileselector_entry_elm_interface_fileselector_selected_model_get(const Eo *obj EINA_UNUSED, Elm_Fileselector_Entry_Data *sd)
{
   return efl_ui_view_model_get(sd->button);
}

EAPI void
elm_fileselector_entry_window_title_set(Eo *obj, const char *title)
{
   ELM_FILESELECTOR_ENTRY_CHECK(obj);
   ELM_FILESELECTOR_ENTRY_DATA_GET_OR_RETURN(obj, sd);
   elm_fileselector_button_window_title_set(sd->button, title);
}

EAPI const char *
elm_fileselector_entry_window_title_get(const Eo *obj)
{
   ELM_FILESELECTOR_ENTRY_CHECK(obj) NULL;
   ELM_FILESELECTOR_ENTRY_DATA_GET_OR_RETURN_VAL(obj, sd, NULL);
   return elm_fileselector_button_window_title_get(sd->button);
}

EAPI void
elm_fileselector_entry_window_size_set(Eo *obj, Evas_Coord width, Evas_Coord height)
{
   ELM_FILESELECTOR_ENTRY_CHECK(obj);
   ELM_FILESELECTOR_ENTRY_DATA_GET_OR_RETURN(obj, sd);
   elm_fileselector_button_window_size_set(sd->button, width, height);
}

EAPI void
elm_fileselector_entry_window_size_get(const Eo *obj, Evas_Coord *width, Evas_Coord *height)
{
   if (width) *width = 0;
   if (height) *height = 0;
   ELM_FILESELECTOR_ENTRY_CHECK(obj);
   ELM_FILESELECTOR_ENTRY_DATA_GET_OR_RETURN(obj, sd);
   elm_fileselector_button_window_size_get(sd->button, width, height);
}

EINA_DEPRECATED EAPI void
elm_fileselector_entry_path_set(Evas_Object *obj,
                                const char *path)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj);
   elm_fileselector_path_set(obj, path);
}

void
_elm_fileselector_entry_path_set_internal(Evas_Object *obj, const char *path)
{
   ELM_FILESELECTOR_ENTRY_DATA_GET_OR_RETURN(obj, sd);
   char *s;

   elm_fileselector_path_set(sd->button, path);

   s = elm_entry_utf8_to_markup(path);
   if (s)
     {
        elm_object_text_set(sd->entry, s);
        free(s);
     }
}

EOLIAN static void
_elm_fileselector_entry_efl_ui_view_model_set(Eo *obj EINA_UNUSED, Elm_Fileselector_Entry_Data *sd, Efl_Model *model)
{
   efl_ui_view_model_set(sd->button, model);
   efl_ui_view_model_set(sd->entry, model);
   efl_ui_model_connect(sd->entry, "default", "path");
}

EINA_DEPRECATED EAPI const char *
elm_fileselector_entry_path_get(const Evas_Object *obj)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj, NULL);
   return elm_fileselector_path_get(obj);
}

const char *
_elm_fileselector_entry_path_get_internal(const Evas_Object *obj)
{
   ELM_FILESELECTOR_ENTRY_DATA_GET_OR_RETURN_VAL(obj, sd, NULL);
   free(sd->path);
   sd->path = elm_entry_markup_to_utf8(elm_object_text_get(sd->entry));
   return sd->path;
}

EOLIAN static Efl_Model *
_elm_fileselector_entry_efl_ui_view_model_get(const Eo *obj EINA_UNUSED, Elm_Fileselector_Entry_Data *sd)
{
   Efl_Model *bmodel, *ret;
   Eina_Value path;
   bmodel = efl_ui_view_model_get(sd->button);
   if (!bmodel)
     {
        WRN("no base Efl.Model");
        return NULL;
     }

   ret = efl_add(efl_class_get(bmodel), NULL);
   free(sd->path);
   sd->path = elm_entry_markup_to_utf8(elm_object_text_get(sd->entry));
   eina_value_setup(&path, EINA_VALUE_TYPE_STRING);
   eina_value_set(&path, sd->path);
   efl_model_property_set(ret, "path", &path);
   eina_value_flush(&path);

   return ret;
}

EINA_DEPRECATED EAPI void
elm_fileselector_entry_expandable_set(Evas_Object *obj,
                                      Eina_Bool value)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj);
   elm_interface_fileselector_expandable_set(obj, value);
}

EOLIAN static void
_elm_fileselector_entry_elm_interface_fileselector_expandable_set(Eo *obj EINA_UNUSED, Elm_Fileselector_Entry_Data *sd, Eina_Bool value)
{
   elm_fileselector_expandable_set(sd->button, value);
}

EINA_DEPRECATED EAPI Eina_Bool
elm_fileselector_entry_expandable_get(const Evas_Object *obj)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj, EINA_FALSE);
   return elm_interface_fileselector_expandable_get((Eo *) obj);
}

EOLIAN static Eina_Bool
_elm_fileselector_entry_elm_interface_fileselector_expandable_get(const Eo *obj EINA_UNUSED, Elm_Fileselector_Entry_Data *sd)
{
   return elm_fileselector_expandable_get(sd->button);
}

EINA_DEPRECATED EAPI void
elm_fileselector_entry_folder_only_set(Evas_Object *obj,
                                       Eina_Bool value)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj);
   elm_interface_fileselector_folder_only_set(obj, value);
}

EOLIAN static void
_elm_fileselector_entry_elm_interface_fileselector_folder_only_set(Eo *obj EINA_UNUSED, Elm_Fileselector_Entry_Data *sd, Eina_Bool value)
{
   elm_fileselector_folder_only_set(sd->button, value);
}

EINA_DEPRECATED EAPI Eina_Bool
elm_fileselector_entry_folder_only_get(const Evas_Object *obj)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj, EINA_FALSE);
   return elm_interface_fileselector_folder_only_get((Eo *) obj);
}

EOLIAN static Eina_Bool
_elm_fileselector_entry_elm_interface_fileselector_folder_only_get(const Eo *obj EINA_UNUSED, Elm_Fileselector_Entry_Data *sd)
{
   return elm_fileselector_folder_only_get(sd->button);
}

EINA_DEPRECATED EAPI void
elm_fileselector_entry_is_save_set(Evas_Object *obj,
                                   Eina_Bool value)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj);
   elm_interface_fileselector_is_save_set(obj, value);
}

EOLIAN static void
_elm_fileselector_entry_elm_interface_fileselector_is_save_set(Eo *obj EINA_UNUSED, Elm_Fileselector_Entry_Data *sd, Eina_Bool value)
{
   elm_fileselector_is_save_set(sd->button, value);
}

EINA_DEPRECATED EAPI Eina_Bool
elm_fileselector_entry_is_save_get(const Evas_Object *obj)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj, EINA_FALSE);
   return elm_interface_fileselector_is_save_get((Eo *) obj);
}

EOLIAN static Eina_Bool
_elm_fileselector_entry_elm_interface_fileselector_is_save_get(const Eo *obj EINA_UNUSED, Elm_Fileselector_Entry_Data *sd)
{
   return elm_fileselector_is_save_get(sd->button);
}

EAPI void
elm_fileselector_entry_inwin_mode_set(Eo *obj, Eina_Bool value)
{
   ELM_FILESELECTOR_ENTRY_CHECK(obj);
   ELM_FILESELECTOR_ENTRY_DATA_GET_OR_RETURN(obj, sd);
   elm_fileselector_button_inwin_mode_set(sd->button, value);
}

EAPI Eina_Bool
elm_fileselector_entry_inwin_mode_get(const Eo *obj)
{
   ELM_FILESELECTOR_ENTRY_CHECK(obj) EINA_FALSE;
   ELM_FILESELECTOR_ENTRY_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);
   return elm_fileselector_button_inwin_mode_get(sd->button);
}

EOLIAN static void
_elm_fileselector_entry_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

/* Efl.Part begin */

ELM_PART_OVERRIDE(elm_fileselector_entry, ELM_FILESELECTOR_ENTRY, Elm_Fileselector_Entry_Data)
ELM_PART_OVERRIDE_CONTENT_SET(elm_fileselector_entry, ELM_FILESELECTOR_ENTRY, Elm_Fileselector_Entry_Data)
ELM_PART_OVERRIDE_CONTENT_GET(elm_fileselector_entry, ELM_FILESELECTOR_ENTRY, Elm_Fileselector_Entry_Data)
ELM_PART_OVERRIDE_CONTENT_UNSET(elm_fileselector_entry, ELM_FILESELECTOR_ENTRY, Elm_Fileselector_Entry_Data)
ELM_PART_OVERRIDE_TEXT_SET(elm_fileselector_entry, ELM_FILESELECTOR_ENTRY, Elm_Fileselector_Entry_Data)
ELM_PART_OVERRIDE_TEXT_GET(elm_fileselector_entry, ELM_FILESELECTOR_ENTRY, Elm_Fileselector_Entry_Data)
ELM_PART_CONTENT_DEFAULT_GET(elm_fileselector_entry, "button icon")
#include "elm_fileselector_entry_part.eo.c"

/* Efl.Part end */

/* Internal EO APIs and hidden overrides */

#define ELM_FILESELECTOR_ENTRY_EXTRA_OPS \
   ELM_PART_CONTENT_DEFAULT_OPS(elm_fileselector_entry), \
   ELM_LAYOUT_SIZING_EVAL_OPS(elm_fileselector_entry), \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(elm_fileselector_entry)

#include "elm_fileselector_entry.eo.c"
