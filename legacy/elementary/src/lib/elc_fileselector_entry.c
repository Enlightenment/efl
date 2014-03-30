//FIXME this widget should inherit from file selector button
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"
#include "elm_interface_fileselector.h"
#include "elm_widget_fileselector_entry.h"

EAPI Eo_Op ELM_OBJ_FILESELECTOR_ENTRY_BASE_ID = EO_NOOP;

#define MY_CLASS ELM_OBJ_FILESELECTOR_ENTRY_CLASS

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

static void
_elm_fileselector_entry_smart_sizing_eval(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   Evas_Coord minw = -1, minh = -1;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   edje_object_size_min_calc(wd->resize_obj, &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, -1, -1);
}

static void
_elm_fileselector_entry_smart_focus_next_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_TRUE;
}

static void
_elm_fileselector_entry_smart_focus_direction_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_FALSE;
}

static void
_elm_fileselector_entry_smart_focus_next(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Object *chain[2];
   Evas_Object *to_focus;
   unsigned char i;

   Elm_Fileselector_Entry_Smart_Data *sd = _pd;

   Elm_Focus_Direction dir = va_arg(*list, Elm_Focus_Direction);
   Evas_Object **next =  va_arg(*list, Evas_Object **);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Eina_Bool int_ret = EINA_FALSE;

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
     goto end;

   i = elm_widget_focus_get(chain[1]);

   if (elm_widget_focus_next_get(chain[i], dir, next))
     {
        int_ret = EINA_TRUE;
        goto end;
     }

   i = !i;

   if (elm_widget_focus_next_get(chain[i], dir, &to_focus))
     {
        *next = to_focus;
        int_ret = !!i;
        goto end;
     }

end:
   if (ret) *ret = int_ret;
}

static void
_elm_fileselector_entry_smart_theme(Eo *obj, void *_pd, va_list *list)
{
   const char *style;
   char buf[1024];

   Elm_Fileselector_Entry_Smart_Data *sd = _pd;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   Eina_Bool int_ret;
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   eo_do_super(obj, MY_CLASS, elm_obj_widget_theme_apply(&int_ret));
   if (!int_ret) return;

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

   if (ret) *ret = EINA_TRUE;
}

static void
_elm_fileselector_entry_smart_disable(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool val;

   Elm_Fileselector_Entry_Smart_Data *sd = _pd;

   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret;

   eo_do_super(obj, MY_CLASS, elm_obj_widget_disable(&int_ret));
   if (!int_ret) return;

   val = elm_widget_disabled_get(obj);

   elm_widget_disabled_set(sd->button, val);
   elm_widget_disabled_set(sd->entry, val);

   if (ret) *ret = EINA_TRUE;
}

static void
_elm_fileselector_entry_smart_text_set(Eo *obj, void *_pd, va_list *list)
{
   Elm_Fileselector_Entry_Smart_Data *sd = _pd;
   const char *part = va_arg(*list, const char *);
   const char *label = va_arg(*list, const char *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Eina_Bool int_ret;

   if (part && strcmp(part, "default"))
     {
        eo_do_super(obj, MY_CLASS,
                    elm_obj_layout_text_set(part, label, &int_ret));
        goto end;
     }

   elm_object_text_set(sd->button, label);
   int_ret = EINA_TRUE;

end:
   if (ret) *ret = int_ret;
}

static void
_elm_fileselector_entry_smart_text_get(Eo *obj, void *_pd, va_list *list)
{
   Elm_Fileselector_Entry_Smart_Data *sd = _pd;

   const char *part = va_arg(*list, const char *);
   const char **text = va_arg(*list, const char **);

   if (part && strcmp(part, "default"))
     {
        eo_do_super(obj, MY_CLASS, elm_obj_layout_text_get(part, text));
        return;
     }

   *text = elm_object_text_get(sd->button);
}

static void
_elm_fileselector_entry_smart_content_set(Eo *obj, void *_pd, va_list *list)
{
   Elm_Fileselector_Entry_Smart_Data *sd = _pd;

   const char *part = va_arg(*list, const char *);
   Evas_Object *content = va_arg(*list, Evas_Object *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Eina_Bool int_ret;

   if (part && strcmp(part, "button icon"))
     {
        eo_do_super(obj, MY_CLASS, elm_obj_container_content_set(part, content, &int_ret));
        goto end;
     }

   elm_layout_content_set(sd->button, NULL, content);

   int_ret = EINA_TRUE;
end:
   if (ret) *ret = int_ret;
}

static void
_elm_fileselector_entry_smart_content_get(Eo *obj, void *_pd, va_list *list)
{
   Elm_Fileselector_Entry_Smart_Data *sd = _pd;

   const char *part = va_arg(*list, const char *);
   Evas_Object **ret = va_arg(*list, Evas_Object **);
   *ret = NULL;

   if (part && strcmp(part, "button icon"))
     {
        eo_do_super(obj, MY_CLASS, elm_obj_container_content_get(part, ret));
        return;
     }

   *ret = elm_layout_content_get(sd->button, NULL);
}

static void
_elm_fileselector_entry_smart_content_unset(Eo *obj, void *_pd, va_list *list)
{
   Elm_Fileselector_Entry_Smart_Data *sd = _pd;

   const char *part = va_arg(*list, const char *);
   Evas_Object **ret = va_arg(*list, Evas_Object **);
   Evas_Object *int_ret;

   if (part && strcmp(part, "button icon"))
     {
        eo_do_super(obj, MY_CLASS, elm_obj_container_content_unset(part, &int_ret));
        goto end;
     }

   int_ret = elm_layout_content_unset(sd->button, NULL);
end:
   if (ret) *ret = int_ret;
}

static void
_elm_fileselector_entry_smart_add(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Fileselector_Entry_Smart_Data *priv = _pd;

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

static void
_elm_fileselector_entry_smart_del(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Fileselector_Entry_Smart_Data *sd = _pd;

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

static void
_constructor(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME_LEGACY),
         evas_obj_smart_callbacks_descriptions_set(_smart_callbacks, NULL));
}

EINA_DEPRECATED EAPI void
elm_fileselector_entry_selected_set(Evas_Object *obj,
                                    const char *path)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj);
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, elm_interface_fileselector_selected_set(path, &ret));
}

static void
_selected_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *path = va_arg(*list, const char *);
   Elm_Fileselector_Entry_Smart_Data *sd = _pd;

   elm_fileselector_path_set(sd->button, path);
}

EINA_DEPRECATED EAPI const char *
elm_fileselector_entry_selected_get(const Evas_Object *obj)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj, NULL);
   const char *ret = NULL;
   eo_do((Eo *) obj, elm_interface_fileselector_selected_get(&ret));
   return ret;
}

static void
_selected_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char **ret = va_arg(*list, const char **);
   Elm_Fileselector_Entry_Smart_Data *sd = _pd;

   *ret = elm_fileselector_path_get(sd->button);
}

EAPI void
elm_fileselector_entry_window_title_set(Evas_Object *obj,
                                        const char *title)
{
   ELM_FILESELECTOR_ENTRY_CHECK(obj);
   eo_do(obj, elm_obj_fileselector_entry_window_title_set(title));
}

static void
_window_title_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *title = va_arg(*list, const char *);
   Elm_Fileselector_Entry_Smart_Data *sd = _pd;

   elm_fileselector_button_window_title_set(sd->button, title);
}

EAPI const char *
elm_fileselector_entry_window_title_get(const Evas_Object *obj)
{
   ELM_FILESELECTOR_ENTRY_CHECK(obj) NULL;
   const char *ret = NULL;
   eo_do((Eo *) obj, elm_obj_fileselector_entry_window_title_get(&ret));
   return ret;
}

static void
_window_title_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char **ret = va_arg(*list, const char **);
   Elm_Fileselector_Entry_Smart_Data *sd = _pd;

   *ret = elm_fileselector_button_window_title_get(sd->button);
}

EAPI void
elm_fileselector_entry_window_size_set(Evas_Object *obj,
                                       Evas_Coord width,
                                       Evas_Coord height)
{
   ELM_FILESELECTOR_ENTRY_CHECK(obj);
   eo_do(obj, elm_obj_fileselector_entry_window_size_set(width, height));
}

static void
_window_size_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Coord width = va_arg(*list, Evas_Coord);
   Evas_Coord height = va_arg(*list, Evas_Coord);
   Elm_Fileselector_Entry_Smart_Data *sd = _pd;

   elm_fileselector_button_window_size_set(sd->button, width, height);
}

EAPI void
elm_fileselector_entry_window_size_get(const Evas_Object *obj,
                                       Evas_Coord *width,
                                       Evas_Coord *height)
{
   ELM_FILESELECTOR_ENTRY_CHECK(obj);
   eo_do((Eo *) obj, elm_obj_fileselector_entry_window_size_get(width, height));
}

static void
_window_size_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Coord *width = va_arg(*list, Evas_Coord *);
   Evas_Coord *height = va_arg(*list, Evas_Coord *);
   Elm_Fileselector_Entry_Smart_Data *sd = _pd;

   elm_fileselector_button_window_size_get(sd->button, width, height);
}

EINA_DEPRECATED EAPI void
elm_fileselector_entry_path_set(Evas_Object *obj,
                                const char *path)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj);
   eo_do(obj, elm_interface_fileselector_path_set(path));
}

static void
_path_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   char *s;

   Elm_Fileselector_Entry_Smart_Data *sd = _pd;
   const char *path = va_arg(*list, const char *);

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
   eo_do((Eo *) obj, elm_interface_fileselector_path_get(&ret));
   return ret;
}

static void
_path_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char **ret = va_arg(*list, const char **);
   Elm_Fileselector_Entry_Smart_Data *sd = _pd;

   free(sd->path);
   sd->path = elm_entry_markup_to_utf8(elm_object_text_get(sd->entry));
   *ret = sd->path;
}

EINA_DEPRECATED EAPI void
elm_fileselector_entry_expandable_set(Evas_Object *obj,
                                      Eina_Bool value)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj);
   eo_do(obj, elm_interface_fileselector_expandable_set(value));
}

static void
_expandable_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool value = va_arg(*list, int);
   Elm_Fileselector_Entry_Smart_Data *sd = _pd;

   elm_fileselector_expandable_set(sd->button, value);
}

EINA_DEPRECATED EAPI Eina_Bool
elm_fileselector_entry_expandable_get(const Evas_Object *obj)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj, EINA_FALSE);
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_interface_fileselector_expandable_get(&ret));
   return ret;
}

static void
_expandable_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Fileselector_Entry_Smart_Data *sd = _pd;

   *ret = elm_fileselector_expandable_get(sd->button);
}

EINA_DEPRECATED EAPI void
elm_fileselector_entry_folder_only_set(Evas_Object *obj,
                                       Eina_Bool value)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj);
   eo_do(obj, elm_interface_fileselector_folder_only_set(value));
}

static void
_folder_only_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool value = va_arg(*list, int);
   Elm_Fileselector_Entry_Smart_Data *sd = _pd;

   elm_fileselector_folder_only_set(sd->button, value);
}

EINA_DEPRECATED EAPI Eina_Bool
elm_fileselector_entry_folder_only_get(const Evas_Object *obj)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj, EINA_FALSE);
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_interface_fileselector_folder_only_get(&ret));
   return ret;
}

static void
_folder_only_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Fileselector_Entry_Smart_Data *sd = _pd;

   *ret = elm_fileselector_folder_only_get(sd->button);
}

EINA_DEPRECATED EAPI void
elm_fileselector_entry_is_save_set(Evas_Object *obj,
                                   Eina_Bool value)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj);
   eo_do(obj, elm_interface_fileselector_is_save_set(value));
}

static void
_is_save_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool value = va_arg(*list, int);
   Elm_Fileselector_Entry_Smart_Data *sd = _pd;

   elm_fileselector_is_save_set(sd->button, value);
}

EINA_DEPRECATED EAPI Eina_Bool
elm_fileselector_entry_is_save_get(const Evas_Object *obj)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj, EINA_FALSE);
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_interface_fileselector_is_save_get(&ret));
   return ret;
}

static void
_is_save_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Fileselector_Entry_Smart_Data *sd = _pd;

   *ret = elm_fileselector_is_save_get(sd->button);
}

EAPI void
elm_fileselector_entry_inwin_mode_set(Evas_Object *obj,
                                      Eina_Bool value)
{
   ELM_FILESELECTOR_ENTRY_CHECK(obj);
   eo_do(obj, elm_obj_fileselector_entry_inwin_mode_set(value));
}

static void
_inwin_mode_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool value = va_arg(*list, int);
   Elm_Fileselector_Entry_Smart_Data *sd = _pd;

   elm_fileselector_button_inwin_mode_set(sd->button, value);
}

EAPI Eina_Bool
elm_fileselector_entry_inwin_mode_get(const Evas_Object *obj)
{
   ELM_FILESELECTOR_ENTRY_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_fileselector_entry_inwin_mode_get(&ret));
   return ret;
}

static void
_inwin_mode_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Fileselector_Entry_Smart_Data *sd = _pd;

   *ret = elm_fileselector_button_inwin_mode_get(sd->button);
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),

        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_ADD), _elm_fileselector_entry_smart_add),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_DEL), _elm_fileselector_entry_smart_del),

        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_DISABLE), _elm_fileselector_entry_smart_disable),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_THEME_APPLY), _elm_fileselector_entry_smart_theme),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_NEXT_MANAGER_IS), _elm_fileselector_entry_smart_focus_next_manager_is),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_NEXT),  _elm_fileselector_entry_smart_focus_next),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_DIRECTION_MANAGER_IS), _elm_fileselector_entry_smart_focus_direction_manager_is),

        EO_OP_FUNC(ELM_OBJ_CONTAINER_ID(ELM_OBJ_CONTAINER_SUB_ID_CONTENT_SET), _elm_fileselector_entry_smart_content_set),
        EO_OP_FUNC(ELM_OBJ_CONTAINER_ID(ELM_OBJ_CONTAINER_SUB_ID_CONTENT_GET), _elm_fileselector_entry_smart_content_get),
        EO_OP_FUNC(ELM_OBJ_CONTAINER_ID(ELM_OBJ_CONTAINER_SUB_ID_CONTENT_UNSET), _elm_fileselector_entry_smart_content_unset),

        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_TEXT_SET), _elm_fileselector_entry_smart_text_set),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_TEXT_GET), _elm_fileselector_entry_smart_text_get),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_SIZING_EVAL), _elm_fileselector_entry_smart_sizing_eval),

        EO_OP_FUNC(ELM_INTERFACE_FILESELECTOR_ID(ELM_INTERFACE_FILESELECTOR_SUB_ID_SELECTED_SET), _selected_set),
        EO_OP_FUNC(ELM_INTERFACE_FILESELECTOR_ID(ELM_INTERFACE_FILESELECTOR_SUB_ID_SELECTED_GET), _selected_get),
        EO_OP_FUNC(ELM_INTERFACE_FILESELECTOR_ID(ELM_INTERFACE_FILESELECTOR_SUB_ID_PATH_SET), _path_set),
        EO_OP_FUNC(ELM_INTERFACE_FILESELECTOR_ID(ELM_INTERFACE_FILESELECTOR_SUB_ID_PATH_GET), _path_get),
        EO_OP_FUNC(ELM_INTERFACE_FILESELECTOR_ID(ELM_INTERFACE_FILESELECTOR_SUB_ID_EXPANDABLE_SET), _expandable_set),
        EO_OP_FUNC(ELM_INTERFACE_FILESELECTOR_ID(ELM_INTERFACE_FILESELECTOR_SUB_ID_EXPANDABLE_GET), _expandable_get),
        EO_OP_FUNC(ELM_INTERFACE_FILESELECTOR_ID(ELM_INTERFACE_FILESELECTOR_SUB_ID_FOLDER_ONLY_SET), _folder_only_set),
        EO_OP_FUNC(ELM_INTERFACE_FILESELECTOR_ID(ELM_INTERFACE_FILESELECTOR_SUB_ID_FOLDER_ONLY_GET), _folder_only_get),
        EO_OP_FUNC(ELM_INTERFACE_FILESELECTOR_ID(ELM_INTERFACE_FILESELECTOR_SUB_ID_IS_SAVE_SET), _is_save_set),
        EO_OP_FUNC(ELM_INTERFACE_FILESELECTOR_ID(ELM_INTERFACE_FILESELECTOR_SUB_ID_IS_SAVE_GET), _is_save_get),

        EO_OP_FUNC(ELM_OBJ_FILESELECTOR_ENTRY_ID(ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_WINDOW_TITLE_SET), _window_title_set),
        EO_OP_FUNC(ELM_OBJ_FILESELECTOR_ENTRY_ID(ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_WINDOW_TITLE_GET), _window_title_get),
        EO_OP_FUNC(ELM_OBJ_FILESELECTOR_ENTRY_ID(ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_WINDOW_SIZE_SET), _window_size_set),
        EO_OP_FUNC(ELM_OBJ_FILESELECTOR_ENTRY_ID(ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_WINDOW_SIZE_GET), _window_size_get),
        EO_OP_FUNC(ELM_OBJ_FILESELECTOR_ENTRY_ID(ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_INWIN_MODE_SET), _inwin_mode_set),
        EO_OP_FUNC(ELM_OBJ_FILESELECTOR_ENTRY_ID(ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_INWIN_MODE_GET), _inwin_mode_get),
        EO_OP_FUNC_SENTINEL
   };
   eo_class_funcs_set(klass, func_desc);

   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}
static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_WINDOW_TITLE_SET, "Set the title for a given file selector entry widget's window."),
     EO_OP_DESCRIPTION(ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_WINDOW_TITLE_GET, "Get the title set for a given file selector entry widget's window."),
     EO_OP_DESCRIPTION(ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_WINDOW_SIZE_SET, "Set the size of a given file selector entry widget's window, holding the file selector itself."),
     EO_OP_DESCRIPTION(ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_WINDOW_SIZE_GET, "Get the size of a given file selector entry widget's window, holding the file selector itself."),
     EO_OP_DESCRIPTION(ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_INWIN_MODE_SET, "Set whether a given file selector entry widget's internal file selector will raise an Elementary 'inner window', instead of a dedicated Elementary window."),
     EO_OP_DESCRIPTION(ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_INWIN_MODE_GET, "Get whether a given file selector entry widget's internal file selector will raise an Elementary 'inner window', instead of a dedicated Elementary window."),
     EO_OP_DESCRIPTION_SENTINEL
};
static const Eo_Class_Description class_desc = {
     EO_VERSION,
     MY_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&ELM_OBJ_FILESELECTOR_ENTRY_BASE_ID, op_desc, ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_LAST),
     NULL,
     sizeof(Elm_Fileselector_Entry_Smart_Data),
     _class_constructor,
     NULL
};
EO_DEFINE_CLASS(elm_obj_fileselector_entry_class_get, &class_desc, ELM_OBJ_LAYOUT_CLASS, ELM_INTERFACE_FILESELECTOR_CLASS, NULL);
