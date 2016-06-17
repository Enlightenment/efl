//FIXME this widget should inherit from file selector button
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"
#include "elm_interface_fileselector.h"
#include "elm_widget_fileselector_entry.h"

#include "elm_fileselector_entry_internal_part.eo.h"
#include "elm_part_helper.h"

#define MY_CLASS ELM_FILESELECTOR_ENTRY_CLASS

#define MY_CLASS_NAME "Elm_Fileselector_Entry"
#define MY_CLASS_NAME_LEGACY "elm_fileselector_entry"

EAPI const char ELM_FILESELECTOR_ENTRY_SMART_NAME[] = "elm_fileselector_entry";

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
  static Eina_Bool                                                               \
  _##name##_fwd(void *data, const Eo_Event *ev EINA_UNUSED)                                          \
  {                                                                         \
     eo_event_callback_call(data, event, ev->info);          \
                                                                      \
     return EINA_TRUE;                                                \
  }
SIG_FWD(CHANGED, ELM_FILESELECTOR_ENTRY_EVENT_CHANGED)
SIG_FWD(PRESS, ELM_FILESELECTOR_ENTRY_EVENT_PRESS)
SIG_FWD(LONGPRESSED, EFL_UI_EVENT_LONGPRESSED)
SIG_FWD(CLICKED, EFL_UI_EVENT_CLICKED)
SIG_FWD(CLICKED_DOUBLE, EFL_UI_EVENT_CLICKED_DOUBLE)
SIG_FWD(FOCUSED, ELM_WIDGET_EVENT_FOCUSED)
SIG_FWD(UNFOCUSED, ELM_WIDGET_EVENT_UNFOCUSED)
SIG_FWD(SELECTION_PASTE, EFL_UI_EVENT_SELECTION_PASTE)
SIG_FWD(SELECTION_COPY, EFL_UI_EVENT_SELECTION_COPY)
SIG_FWD(SELECTION_CUT, EFL_UI_EVENT_SELECTION_CUT)
SIG_FWD(UNPRESSED, EFL_UI_EVENT_UNPRESSED)
#undef SIG_FWD

static void
_file_chosen_path_then(void *data, void *v)
{
   const char *file = NULL;
   char *s;

   eina_value_get(v, &file);

   if (!file) return;
   ELM_FILESELECTOR_ENTRY_DATA_GET(data, sd);

   evas_object_smart_callback_call(data, "file,chosen", (void *) file);

   s = elm_entry_utf8_to_markup(file);
   elm_object_text_set(sd->entry, s);
   free(s);
}

static Eina_Bool
_FILE_CHOSEN_fwd(void *data, const Eo_Event *event)
{
   Efl_Model *model = event->info;
   Eina_Promise *promise = NULL;

   if (!model) return EINA_TRUE;

   promise = efl_model_property_get(model, "path");
   eina_promise_then(promise, _file_chosen_path_then, NULL, data);

   // EVENTS: should not call legacy
   //eo_event_callback_call
   //  (data, ELM_FILESELECTOR_ENTRY_EVENT_FILE_CHOSEN, event->info);

   return EINA_TRUE;
}

// EVENTS: should not need this function
static void
_FILE_CHOSEN_fwd_path(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   const char *path = event_info;

   Eo_Event e = { NULL, NULL, NULL };
   if (path)
     e.info = eo_add(EIO_MODEL_CLASS, NULL, eio_model_path_set(eo_self, path));
   _FILE_CHOSEN_fwd(data, &e);
}

static Eina_Bool
_ACTIVATED_fwd(void *data, const Eo_Event *event)
{
   const char *file;
   Efl_Model *bmodel, *model;
   Eina_Value path;

   ELM_FILESELECTOR_ENTRY_DATA_GET(data, sd);

   file = elm_object_text_get(sd->entry);

   bmodel = elm_interface_fileselector_model_get(sd->button);
   if (bmodel)
     {
         model = eo_add(eo_class_get(bmodel), NULL);
         eina_value_setup(&path, EINA_VALUE_TYPE_STRING);
         eina_value_set(&path, file);
         efl_model_property_set(model, "path", &path, NULL);
         eina_value_flush(&path);
         elm_interface_fileselector_model_set(sd->button, model);
     }

   eo_event_callback_call
     (data, ELM_FILESELECTOR_ENTRY_EVENT_ACTIVATED, event->info);

   return EINA_TRUE;
}

static void
_model_free_eo_cb(void *eo)
{
   eo_unref(eo);
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

EOLIAN static Eina_Bool
_elm_fileselector_entry_elm_widget_focus_next_manager_is(Eo *obj EINA_UNUSED, Elm_Fileselector_Entry_Data *sd EINA_UNUSED)
{
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_fileselector_entry_elm_widget_focus_direction_manager_is(Eo *obj EINA_UNUSED, Elm_Fileselector_Entry_Data *sd EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_elm_fileselector_entry_elm_widget_focus_next(Eo *obj EINA_UNUSED, Elm_Fileselector_Entry_Data *sd, Elm_Focus_Direction dir, Evas_Object **next, Elm_Object_Item **next_item)
{
   Evas_Object *chain[2];
   Evas_Object *to_focus;
   unsigned char i;

   /* Direction */
   if (dir == ELM_FOCUS_PREVIOUS)
     {
        chain[0] = sd->button;
        chain[1] = sd->entry;
     }
   else if (dir == ELM_FOCUS_NEXT)
     {
        chain[0] = sd->entry;
        chain[1] = sd->button;
     }
   else
     return EINA_FALSE;

   i = elm_widget_focus_get(chain[1]);

   if (elm_widget_focus_next_get(chain[i], dir, next, next_item)) return EINA_TRUE;

   i = !i;

   if (elm_widget_focus_next_get(chain[i], dir, &to_focus, next_item))
     {
        *next = to_focus;
        return !!i;
     }

   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_elm_fileselector_entry_elm_widget_theme_apply(Eo *obj, Elm_Fileselector_Entry_Data *sd)
{
   const char *style;
   char buf[1024];

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   Eina_Bool int_ret = EINA_FALSE;
   int_ret = elm_obj_widget_theme_apply(eo_super(obj, MY_CLASS));
   if (!int_ret) return EINA_FALSE;

   style = elm_widget_style_get(obj);

   elm_widget_mirrored_set(sd->button, elm_widget_mirrored_get(obj));

   if (elm_object_disabled_get(obj))
     elm_layout_signal_emit(obj, "elm,state,disabled", "elm");

   if (!style) style = "default";
   snprintf(buf, sizeof(buf), "fileselector_entry/%s", style);
   elm_widget_style_set(sd->button, buf);
   elm_widget_style_set(sd->entry, buf);

   edje_object_message_signal_process(wd->resize_obj);

   elm_layout_sizing_eval(obj);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_fileselector_entry_elm_widget_disable(Eo *obj, Elm_Fileselector_Entry_Data *sd)
{
   Eina_Bool val;

   Eina_Bool int_ret = EINA_FALSE;
   int_ret = elm_obj_widget_disable(eo_super(obj, MY_CLASS));
   if (!int_ret) return EINA_FALSE;

   val = elm_widget_disabled_get(obj);

   elm_widget_disabled_set(sd->button, val);
   elm_widget_disabled_set(sd->entry, val);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_fileselector_entry_elm_layout_text_set(Eo *obj, Elm_Fileselector_Entry_Data *sd, const char *part, const char *label)
{
   if (part && strcmp(part, "default"))
     {
        return elm_obj_layout_text_set(eo_super(obj, MY_CLASS), part, label);
     }

   elm_object_text_set(sd->button, label);
   return EINA_TRUE;
}

EOLIAN static const char *
_elm_fileselector_entry_elm_layout_text_get(Eo *obj, Elm_Fileselector_Entry_Data *sd, const char *part)
{
   if (part && strcmp(part, "default"))
     {
        const char *text = NULL;
        text = elm_obj_layout_text_get(eo_super(obj, MY_CLASS), part);
        return text;
     }

   return elm_object_text_get(sd->button);
}

static Eina_Bool
_elm_fileselector_entry_content_set(Eo *obj, Elm_Fileselector_Entry_Data *sd, const char *part, Evas_Object *content)
{
   if (part && strcmp(part, "button icon"))
     {
        return efl_content_set(efl_part(eo_super(obj, MY_CLASS), part), content);
     }

   elm_layout_content_set(sd->button, NULL, content);

   return EINA_TRUE;
}

static Evas_Object *
_elm_fileselector_entry_content_get(Eo *obj, Elm_Fileselector_Entry_Data *sd, const char *part)
{
   if (part && strcmp(part, "button icon"))
     {
        return efl_content_get(efl_part(eo_super(obj, MY_CLASS), part));
     }

   return elm_layout_content_get(sd->button, NULL);
}

static Evas_Object *
_elm_fileselector_entry_content_unset(Eo *obj, Elm_Fileselector_Entry_Data *sd, const char *part)
{
   if (part && strcmp(part, "button icon"))
     {
        return efl_content_unset(efl_part(eo_super(obj, MY_CLASS), part));
     }

   return elm_layout_content_unset(sd->button, NULL);
}

EOLIAN static void
_elm_fileselector_entry_evas_object_smart_add(Eo *obj, Elm_Fileselector_Entry_Data *priv)
{
   evas_obj_smart_add(eo_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);

   priv->button = elm_fileselector_button_add(obj);
   elm_widget_mirrored_automatic_set(priv->button, EINA_FALSE);
   elm_widget_mirrored_set(priv->button, elm_widget_mirrored_get(obj));
   elm_widget_style_set(priv->button, "fileselector_entry/default");

   elm_fileselector_expandable_set
     (priv->button, _elm_config->fileselector_expand_enable);

#define SIG_FWD(name, event) \
  eo_event_callback_add(priv->button, event, _##name##_fwd, obj)
   SIG_FWD(CLICKED, EFL_UI_EVENT_CLICKED);
   SIG_FWD(UNPRESSED, EFL_UI_EVENT_UNPRESSED);
   // EVENTS: should not call legacy
   //SIG_FWD(FILE_CHOSEN, ELM_FILESELECTOR_BUTTON_EVENT_FILE_CHOSEN);
#undef SIG_FWD
   // EVENTS: should not need this "callback_add"
   evas_object_smart_callback_add(priv->button, "file,chosen", _FILE_CHOSEN_fwd_path, obj);

   priv->entry = elm_entry_add(obj);
   elm_entry_scrollable_set(priv->entry, EINA_TRUE);
   elm_widget_mirrored_automatic_set(priv->entry, EINA_FALSE);
   elm_widget_style_set(priv->entry, "fileselector_entry/default");
   elm_entry_single_line_set(priv->entry, EINA_TRUE);
   elm_entry_editable_set(priv->entry, EINA_TRUE);

#define SIG_FWD(name, event) \
  eo_event_callback_add(priv->entry, event, _##name##_fwd, obj)
   SIG_FWD(CHANGED, ELM_ENTRY_EVENT_CHANGED);
   SIG_FWD(ACTIVATED, ELM_ENTRY_EVENT_ACTIVATED);
   SIG_FWD(PRESS, ELM_ENTRY_EVENT_PRESS);
   SIG_FWD(LONGPRESSED, EFL_UI_EVENT_LONGPRESSED);
   SIG_FWD(CLICKED, EFL_UI_EVENT_CLICKED);
   SIG_FWD(CLICKED_DOUBLE, EFL_UI_EVENT_CLICKED_DOUBLE);
   SIG_FWD(FOCUSED, ELM_WIDGET_EVENT_FOCUSED);
   SIG_FWD(UNFOCUSED, ELM_WIDGET_EVENT_UNFOCUSED);
   SIG_FWD(SELECTION_PASTE, EFL_UI_EVENT_SELECTION_PASTE);
   SIG_FWD(SELECTION_COPY, EFL_UI_EVENT_SELECTION_COPY);
   SIG_FWD(SELECTION_CUT, EFL_UI_EVENT_SELECTION_CUT);
#undef SIG_FWD

   if (!elm_layout_theme_set
       (obj, "fileselector_entry", "base", elm_widget_style_get(obj)))
     CRI("Failed to set layout!");
   else
     {
        elm_layout_content_set(obj, "elm.swallow.button", priv->button);
        elm_layout_content_set(obj, "elm.swallow.entry", priv->entry);
     }

   elm_widget_can_focus_set(obj, EINA_TRUE);

   elm_layout_sizing_eval(obj);
}

EOLIAN static void
_elm_fileselector_entry_evas_object_smart_del(Eo *obj, Elm_Fileselector_Entry_Data *sd)
{
   free(sd->path);

   evas_obj_smart_del(eo_super(obj, MY_CLASS));
}

EAPI Evas_Object *
elm_fileselector_entry_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   return eo_add(MY_CLASS, parent);
}

EOLIAN static Eo *
_elm_fileselector_entry_eo_base_constructor(Eo *obj, Elm_Fileselector_Entry_Data *sd EINA_UNUSED)
{
   obj = eo_constructor(eo_super(obj, MY_CLASS));
   evas_obj_type_set(obj, MY_CLASS_NAME_LEGACY);
   evas_obj_smart_callbacks_descriptions_set(obj, _smart_callbacks);
   elm_interface_atspi_accessible_role_set(obj, ELM_ATSPI_ROLE_GROUPING);

   return obj;
}

EINA_DEPRECATED EAPI void
elm_fileselector_entry_selected_set(Evas_Object *obj, const char *path)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj);
   ELM_FILESELECTOR_ENTRY_DATA_GET_OR_RETURN(obj, sd);
   elm_fileselector_button_path_set(sd->button, path);
}

EOLIAN static void
_elm_fileselector_entry_elm_interface_fileselector_selected_model_set(Eo *obj EINA_UNUSED,
                                                                      Elm_Fileselector_Entry_Data *sd,
                                                                      Efl_Model *model,
                                                                      Eina_Promise_Owner *promise_owner)
{
   elm_interface_fileselector_model_set(sd->button, model);
   eina_promise_owner_value_set(promise_owner, eo_ref(model), _model_free_eo_cb);
}

EINA_DEPRECATED EAPI const char *
elm_fileselector_entry_selected_get(const Evas_Object *obj)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj, NULL);
   ELM_FILESELECTOR_ENTRY_DATA_GET_OR_RETURN_VAL(obj, sd, NULL);
   return elm_fileselector_button_path_get(sd->button);
}

EOLIAN static Efl_Model *
_elm_fileselector_entry_elm_interface_fileselector_selected_model_get(Eo *obj EINA_UNUSED, Elm_Fileselector_Entry_Data *sd)
{
   return elm_interface_fileselector_model_get(sd->button);
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
   ELM_FILESELECTOR_ENTRY_DATA_GET_OR_RETURN(obj, sd);
   char *s = elm_entry_utf8_to_markup(path);
   if (s)
     {
        elm_object_text_set(sd->entry, s);
        free(s);
     }

   elm_fileselector_button_path_set(sd->button, path);
}

static void
_fs_entry_model_path_get_then(void *data, void *v)
{
   Elm_Fileselector_Entry_Data *sd = data;
   char *path = NULL;
   char *s;

   if (!v)
     return;

   eina_value_get(v, &path);
   s = elm_entry_utf8_to_markup(path);
   if (s)
     {
        elm_object_text_set(sd->entry, s);
        free(s);
     }
}

EOLIAN static void
_elm_fileselector_entry_elm_interface_fileselector_model_set(Eo *obj EINA_UNUSED, Elm_Fileselector_Entry_Data *sd, Efl_Model *model)
{
   Eina_Promise *p = NULL;
   elm_interface_fileselector_model_set(sd->button, model);

   p = efl_model_property_get(model, "path");
   eina_promise_then(p, _fs_entry_model_path_get_then, NULL, sd);
}

EINA_DEPRECATED EAPI const char *
elm_fileselector_entry_path_get(const Evas_Object *obj)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj, NULL);
   ELM_FILESELECTOR_ENTRY_DATA_GET_OR_RETURN_VAL(obj, sd, NULL);
   free(sd->path);
   sd->path = elm_entry_markup_to_utf8(elm_object_text_get(sd->entry));
   return sd->path;
}

EOLIAN static Efl_Model *
_elm_fileselector_entry_elm_interface_fileselector_model_get(Eo *obj EINA_UNUSED, Elm_Fileselector_Entry_Data *sd)
{
   Efl_Model *bmodel, *ret;
   Eina_Value path;
   bmodel = elm_interface_fileselector_model_get(sd->button);
   if (!bmodel)
     {
        WRN("no base Efl.Model");
        return NULL;
     }

   ret = eo_add(eo_class_get(bmodel), NULL);
   free(sd->path);
   sd->path = elm_entry_markup_to_utf8(elm_object_text_get(sd->entry));
   eina_value_setup(&path, EINA_VALUE_TYPE_STRING);
   eina_value_set(&path, sd->path);
   efl_model_property_set(ret, "path", &path, NULL);
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
_elm_fileselector_entry_elm_interface_fileselector_expandable_get(Eo *obj EINA_UNUSED, Elm_Fileselector_Entry_Data *sd)
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
_elm_fileselector_entry_elm_interface_fileselector_folder_only_get(Eo *obj EINA_UNUSED, Elm_Fileselector_Entry_Data *sd)
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
_elm_fileselector_entry_elm_interface_fileselector_is_save_get(Eo *obj EINA_UNUSED, Elm_Fileselector_Entry_Data *sd)
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
_elm_fileselector_entry_class_constructor(Eo_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

/* Efl.Part begin */

ELM_PART_OVERRIDE(elm_fileselector_entry, ELM_FILESELECTOR_ENTRY, ELM_LAYOUT, Elm_Fileselector_Entry_Data, Elm_Part_Data)
ELM_PART_OVERRIDE_CONTENT_SET(elm_fileselector_entry, ELM_FILESELECTOR_ENTRY, ELM_LAYOUT, Elm_Fileselector_Entry_Data, Elm_Part_Data)
ELM_PART_OVERRIDE_CONTENT_GET(elm_fileselector_entry, ELM_FILESELECTOR_ENTRY, ELM_LAYOUT, Elm_Fileselector_Entry_Data, Elm_Part_Data)
ELM_PART_OVERRIDE_CONTENT_UNSET(elm_fileselector_entry, ELM_FILESELECTOR_ENTRY, ELM_LAYOUT, Elm_Fileselector_Entry_Data, Elm_Part_Data)
#include "elm_fileselector_entry_internal_part.eo.c"

/* Efl.Part end */

#include "elm_fileselector_entry.eo.c"
