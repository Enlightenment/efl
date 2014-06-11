//FIXME this widget should inherit from file selector button
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"
#include "elm_interface_fileselector.h"
#include "elm_widget_fileselector_entry.h"

 
#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
#include "elm_interface_atspi_accessible.h"
#include "elm_interface_atspi_accessible.eo.h"

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

#define SIG_FWD(name)                                                       \
  static void                                                               \
  _##name##_fwd(void *data, Evas_Object * obj EINA_UNUSED, void *event_info) \
  {                                                                         \
     evas_object_smart_callback_call(data, SIG_##name, event_info);         \
  }
SIG_FWD(CHANGED)
SIG_FWD(PRESS)
SIG_FWD(LONGPRESSED)
SIG_FWD(CLICKED)
SIG_FWD(CLICKED_DOUBLE)
SIG_FWD(FOCUSED)
SIG_FWD(UNFOCUSED)
SIG_FWD(SELECTION_PASTE)
SIG_FWD(SELECTION_COPY)
SIG_FWD(SELECTION_CUT)
SIG_FWD(UNPRESSED)
#undef SIG_FWD

static void
_FILE_CHOSEN_fwd(void *data,
                 Evas_Object *obj EINA_UNUSED,
                 void *event_info)
{
   const char *file = event_info;
   char *s;

   ELM_FILESELECTOR_ENTRY_DATA_GET(data, sd);

   s = elm_entry_utf8_to_markup(file);
   if (!s) return;
   elm_object_text_set(sd->entry, s);
   free(s);
   evas_object_smart_callback_call(data, SIG_FILE_CHOSEN, event_info);
}

static void
_ACTIVATED_fwd(void *data,
               Evas_Object *obj EINA_UNUSED,
               void *event_info)
{
   const char *file;

   ELM_FILESELECTOR_ENTRY_DATA_GET(data, sd);

   file = elm_object_text_get(sd->entry);
   elm_fileselector_path_set(sd->button, file);
   evas_object_smart_callback_call(data, SIG_ACTIVATED, event_info);
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
_elm_fileselector_entry_elm_widget_focus_next(Eo *obj EINA_UNUSED, Elm_Fileselector_Entry_Data *sd, Elm_Focus_Direction dir, Evas_Object **next)
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

   if (elm_widget_focus_next_get(chain[i], dir, next)) return EINA_TRUE;

   i = !i;

   if (elm_widget_focus_next_get(chain[i], dir, &to_focus))
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
   eo_do_super(obj, MY_CLASS, int_ret = elm_obj_widget_theme_apply());
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
   eo_do_super(obj, MY_CLASS, int_ret = elm_obj_widget_disable());
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
        Eina_Bool int_ret = EINA_FALSE;
        eo_do_super(obj, MY_CLASS,
                    int_ret = elm_obj_layout_text_set(part, label));
        return int_ret;
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
        eo_do_super(obj, MY_CLASS, text = elm_obj_layout_text_get(part));
        return text;
     }

   return elm_object_text_get(sd->button);
}

EOLIAN static Eina_Bool
_elm_fileselector_entry_elm_container_content_set(Eo *obj, Elm_Fileselector_Entry_Data *sd, const char *part, Evas_Object *content)
{
   if (part && strcmp(part, "button icon"))
     {
        Eina_Bool int_ret = EINA_FALSE;
        eo_do_super(obj, MY_CLASS, int_ret = elm_obj_container_content_set(part, content));
        return int_ret;
     }

   elm_layout_content_set(sd->button, NULL, content);

   return EINA_TRUE;
}

EOLIAN static Evas_Object *
_elm_fileselector_entry_elm_container_content_get(Eo *obj, Elm_Fileselector_Entry_Data *sd, const char *part)
{
   if (part && strcmp(part, "button icon"))
     {
        Evas_Object *ret = NULL;
        eo_do_super(obj, MY_CLASS, ret = elm_obj_container_content_get(part));
        return ret;
     }

   return elm_layout_content_get(sd->button, NULL);
}

EOLIAN static Evas_Object *
_elm_fileselector_entry_elm_container_content_unset(Eo *obj, Elm_Fileselector_Entry_Data *sd, const char *part)
{
   if (part && strcmp(part, "button icon"))
     {
        Evas_Object *ret = NULL;
        eo_do_super(obj, MY_CLASS, ret = elm_obj_container_content_unset(part));
        return ret;
     }

   return elm_layout_content_unset(sd->button, NULL);
}

EOLIAN static void
_elm_fileselector_entry_evas_object_smart_add(Eo *obj, Elm_Fileselector_Entry_Data *priv)
{
   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());
   elm_widget_sub_object_parent_add(obj);

   priv->button = elm_fileselector_button_add(obj);
   elm_widget_mirrored_automatic_set(priv->button, EINA_FALSE);
   elm_widget_mirrored_set(priv->button, elm_widget_mirrored_get(obj));
   elm_widget_style_set(priv->button, "fileselector_entry/default");

   elm_fileselector_expandable_set
     (priv->button, _elm_config->fileselector_expand_enable);

#define SIG_FWD(name) \
  evas_object_smart_callback_add(priv->button, SIG_##name, _##name##_fwd, obj)
   SIG_FWD(CLICKED);
   SIG_FWD(UNPRESSED);
   SIG_FWD(FILE_CHOSEN);
#undef SIG_FWD

   priv->entry = elm_entry_add(obj);
   elm_entry_scrollable_set(priv->entry, EINA_TRUE);
   elm_widget_mirrored_automatic_set(priv->entry, EINA_FALSE);
   elm_widget_style_set(priv->entry, "fileselector_entry/default");
   elm_entry_single_line_set(priv->entry, EINA_TRUE);
   elm_entry_editable_set(priv->entry, EINA_TRUE);

#define SIG_FWD(name) \
  evas_object_smart_callback_add(priv->entry, SIG_##name, _##name##_fwd, obj)
   SIG_FWD(CHANGED);
   SIG_FWD(ACTIVATED);
   SIG_FWD(PRESS);
   SIG_FWD(LONGPRESSED);
   SIG_FWD(CLICKED);
   SIG_FWD(CLICKED_DOUBLE);
   SIG_FWD(FOCUSED);
   SIG_FWD(UNFOCUSED);
   SIG_FWD(SELECTION_PASTE);
   SIG_FWD(SELECTION_COPY);
   SIG_FWD(SELECTION_CUT);
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

   eo_do_super(obj, MY_CLASS, evas_obj_smart_del());
}

EAPI Evas_Object *
elm_fileselector_entry_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = eo_add(MY_CLASS, parent);
   eo_unref(obj);
   return obj;
}

EOLIAN static void
_elm_fileselector_entry_eo_base_constructor(Eo *obj, Elm_Fileselector_Entry_Data *sd EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME_LEGACY),
         evas_obj_smart_callbacks_descriptions_set(_smart_callbacks),
         elm_interface_atspi_accessible_role_set(ELM_ATSPI_ROLE_GROUPING));
}

EINA_DEPRECATED EAPI void
elm_fileselector_entry_selected_set(Evas_Object *obj,
                                    const char *path)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj);
   eo_do(obj, elm_interface_fileselector_selected_set(path));
}

EOLIAN static Eina_Bool
_elm_fileselector_entry_elm_interface_fileselector_selected_set(Eo *obj EINA_UNUSED, Elm_Fileselector_Entry_Data *sd, const char *path)
{
   elm_fileselector_path_set(sd->button, path);
   return EINA_TRUE;
}

EINA_DEPRECATED EAPI const char *
elm_fileselector_entry_selected_get(const Evas_Object *obj)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj, NULL);
   const char *ret = NULL;
   eo_do((Eo *) obj, ret = elm_interface_fileselector_selected_get());
   return ret;
}

EOLIAN static const char *
_elm_fileselector_entry_elm_interface_fileselector_selected_get(Eo *obj EINA_UNUSED, Elm_Fileselector_Entry_Data *sd)
{
   return elm_fileselector_path_get(sd->button);
}

EOLIAN static void
_elm_fileselector_entry_window_title_set(Eo *obj EINA_UNUSED, Elm_Fileselector_Entry_Data *sd, const char *title)
{
   elm_fileselector_button_window_title_set(sd->button, title);
}

EOLIAN static const char *
_elm_fileselector_entry_window_title_get(Eo *obj EINA_UNUSED, Elm_Fileselector_Entry_Data *sd)
{
   return elm_fileselector_button_window_title_get(sd->button);
}

EOLIAN static void
_elm_fileselector_entry_window_size_set(Eo *obj EINA_UNUSED, Elm_Fileselector_Entry_Data *sd, Evas_Coord width, Evas_Coord height)
{
   elm_fileselector_button_window_size_set(sd->button, width, height);
}

EOLIAN static void
_elm_fileselector_entry_window_size_get(Eo *obj EINA_UNUSED, Elm_Fileselector_Entry_Data *sd, Evas_Coord *width, Evas_Coord *height)
{
   elm_fileselector_button_window_size_get(sd->button, width, height);
}

EINA_DEPRECATED EAPI void
elm_fileselector_entry_path_set(Evas_Object *obj,
                                const char *path)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj);
   eo_do(obj, elm_interface_fileselector_path_set(path));
}

EOLIAN static void
_elm_fileselector_entry_elm_interface_fileselector_path_set(Eo *obj EINA_UNUSED, Elm_Fileselector_Entry_Data *sd, const char *path)
{
   char *s;

   elm_fileselector_path_set(sd->button, path);
   s = elm_entry_utf8_to_markup(path);
   if (s)
     {
        elm_object_text_set(sd->entry, s);
        free(s);
     }
}

EINA_DEPRECATED EAPI const char *
elm_fileselector_entry_path_get(const Evas_Object *obj)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj, NULL);
   const char *ret = NULL;
   eo_do((Eo *) obj, ret = elm_interface_fileselector_path_get());
   return ret;
}

EOLIAN static const char *
_elm_fileselector_entry_elm_interface_fileselector_path_get(Eo *obj EINA_UNUSED, Elm_Fileselector_Entry_Data *sd)
{
   free(sd->path);
   sd->path = elm_entry_markup_to_utf8(elm_object_text_get(sd->entry));
   return sd->path;
}

EINA_DEPRECATED EAPI void
elm_fileselector_entry_expandable_set(Evas_Object *obj,
                                      Eina_Bool value)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj);
   eo_do(obj, elm_interface_fileselector_expandable_set(value));
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
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, ret = elm_interface_fileselector_expandable_get());
   return ret;
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
   eo_do(obj, elm_interface_fileselector_folder_only_set(value));
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
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, ret = elm_interface_fileselector_folder_only_get());
   return ret;
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
   eo_do(obj, elm_interface_fileselector_is_save_set(value));
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
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, ret = elm_interface_fileselector_is_save_get());
   return ret;
}

EOLIAN static Eina_Bool
_elm_fileselector_entry_elm_interface_fileselector_is_save_get(Eo *obj EINA_UNUSED, Elm_Fileselector_Entry_Data *sd)
{
   return elm_fileselector_is_save_get(sd->button);
}

EOLIAN static void
_elm_fileselector_entry_inwin_mode_set(Eo *obj EINA_UNUSED, Elm_Fileselector_Entry_Data *sd, Eina_Bool value)
{
   elm_fileselector_button_inwin_mode_set(sd->button, value);
}

EOLIAN static Eina_Bool
_elm_fileselector_entry_inwin_mode_get(Eo *obj EINA_UNUSED, Elm_Fileselector_Entry_Data *sd)
{
   return elm_fileselector_button_inwin_mode_get(sd->button);
}

EOLIAN static void
_elm_fileselector_entry_class_constructor(Eo_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

#include "elc_fileselector_entry.eo.c"
