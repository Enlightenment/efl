#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_PROTECTED
#define ELM_INTERFACE_FILESELECTOR_BETA

#include <Elementary.h>
#include "Eio_Eo.h"
#include "elm_priv.h"
#include "elm_fileselector_button.eo.h"
#include "elm_fileselector_entry.eo.h"
#include "elm_interface_fileselector.h"
#include "elm_widget_fileselector_button.h"
#include "elm_fileselector.eo.h"

#define MY_CLASS ELM_FILESELECTOR_BUTTON_CLASS

#define MY_CLASS_NAME "Elm_Fileselector_Button"
#define MY_CLASS_NAME_LEGACY "elm_fileselector_button"

/* FIXME: need a way to find a gap between the size of item and thumbnail */
#define GENGRID_PADDING 16

#define DEFAULT_WINDOW_TITLE "Select a file"

#define ELM_PRIV_FILESELECTOR_BUTTON_SIGNALS(cmd) \
   cmd(SIG_FILE_CHOSEN, "file,chosen", "s") \

ELM_PRIV_FILESELECTOR_BUTTON_SIGNALS(ELM_PRIV_STATIC_VARIABLE_DECLARE);

static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   ELM_PRIV_FILESELECTOR_BUTTON_SIGNALS(ELM_PRIV_SMART_CALLBACKS_DESC)
   {SIG_WIDGET_LANG_CHANGED, ""}, /**<handled by elm_widget */
   {SIG_WIDGET_ACCESS_CHANGED, ""}, /**<handled by elm_widget */
   {SIG_LAYOUT_FOCUSED, ""}, /**< handled by elm_layout */
   {SIG_LAYOUT_UNFOCUSED, ""}, /**< handled by elm_layout */
   {NULL, NULL}
};
#undef ELM_PRIV_FILESELECTOR_BUTTON_SIGNALS

EOLIAN static Efl_Ui_Theme_Apply
_elm_fileselector_button_efl_ui_widget_theme_apply(Eo *obj, Elm_Fileselector_Button_Data *sd EINA_UNUSED)
{
   Efl_Ui_Theme_Apply int_ret = EFL_UI_THEME_APPLY_FAILED;

   char buf[4096];
   const char *style;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EFL_UI_THEME_APPLY_FAILED);

   style = eina_stringshare_add(elm_widget_style_get(obj));

   snprintf(buf, sizeof(buf), "fileselector_button/%s", style);

   /* file selector button's style has an extra bit */
   eina_stringshare_replace(&(wd->style), buf);

   int_ret = efl_ui_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (!int_ret) return EFL_UI_THEME_APPLY_FAILED;

   eina_stringshare_replace(&(wd->style), style);

   eina_stringshare_del(style);

   return int_ret;
}

static void
_selection_done(void *data, const Efl_Event *event EINA_UNUSED)
{
   Elm_Fileselector_Button_Data *sd = data;
   Efl_Model *model;;
   Evas_Object *del;

   model = elm_interface_fileselector_selected_model_get(sd->fs);
   if (model)
     {
        Eina_Value *path;
        char *file;

        efl_replace(&sd->fsd.model, model);

        path = efl_model_property_get(model, "path");
        file = eina_value_to_string(path);
        eina_stringshare_replace(&sd->fsd.path, file);

        efl_event_callback_call
          (sd->obj, ELM_FILESELECTOR_BUTTON_EVENT_FILE_CHOSEN, model);
        _event_to_legacy_call
          (sd->obj, ELM_FILESELECTOR_BUTTON_EVENT_FILE_CHOSEN, file);

        eina_value_free(path);
        free(file);
     }
   else
     {
        _model_event_call
          (sd->obj, ELM_FILESELECTOR_BUTTON_EVENT_FILE_CHOSEN, NULL, NULL);
     }

   del = sd->fsw;
   sd->fs = NULL;
   sd->fsw = NULL;
   evas_object_del(del);
}

static Evas_Object *
_new_window_add(Elm_Fileselector_Button_Data *sd)
{
   Evas_Object *win, *bg;

   win = elm_win_add(NULL, "fileselector_button", ELM_WIN_DIALOG_BASIC);
   elm_win_title_set(win, sd->window_title);
   elm_win_autodel_set(win, EINA_TRUE);
   efl_event_callback_add
         (win, EFL_UI_WIN_EVENT_DELETE_REQUEST, _selection_done, sd);

   bg = elm_bg_add(win);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   evas_object_resize(win, sd->w, sd->h);
   return win;
}

static Evas_Object *
_parent_win_get(Evas_Object *obj)
{
   while (!efl_isa(obj, EFL_UI_WIN_CLASS))
     obj = elm_object_parent_widget_get(obj);

   return obj;
}

static void
_activate(Elm_Fileselector_Button_Data *sd)
{
   Eina_Bool is_inwin = EINA_FALSE;

   if (sd->fs) return;

   if (sd->inwin_mode)
     {
        sd->fsw = _parent_win_get(sd->obj);

        if (!sd->fsw)
          sd->fsw = _new_window_add(sd);
        else
          {
             sd->fsw = elm_win_inwin_add(sd->fsw);
             is_inwin = EINA_TRUE;
          }
     }
   else
     sd->fsw = _new_window_add(sd);

   sd->fs = elm_fileselector_add(sd->fsw);
   elm_fileselector_hidden_visible_set(sd->fs, sd->fsd.hidden_visible);
   efl_ui_mirrored_set
     (sd->fs, efl_ui_mirrored_get(sd->obj));
   efl_ui_mirrored_automatic_set(sd->fs, EINA_FALSE);
   elm_fileselector_expandable_set(sd->fs, sd->fsd.expandable);
   elm_fileselector_folder_only_set(sd->fs, sd->fsd.folder_only);
   elm_fileselector_is_save_set(sd->fs, sd->fsd.is_save);
   elm_interface_fileselector_selected_model_set(sd->fs, sd->fsd.model);
   evas_object_size_hint_weight_set
     (sd->fs, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(sd->fs, EVAS_HINT_FILL, EVAS_HINT_FILL);
   efl_event_callback_add
     (sd->fs, ELM_FILESELECTOR_EVENT_DONE, _selection_done, sd);
   evas_object_show(sd->fs);

   if (is_inwin)
     {
        elm_win_inwin_content_set(sd->fsw, sd->fs);
        elm_win_inwin_activate(sd->fsw);
     }
   else
     {
        elm_win_resize_object_add(sd->fsw, sd->fs);
        evas_object_show(sd->fsw);
     }
}

static void
_button_clicked(void *data, const Efl_Event *event EINA_UNUSED)
{
   _activate(data);
}

static void _noref_death(void *data EINA_UNUSED, const Efl_Event *event);
static void _invalidated(void *data EINA_UNUSED, const Efl_Event *event);

EFL_CALLBACKS_ARRAY_DEFINE(noref_death,
                           { EFL_EVENT_NOREF, _noref_death },
                           { EFL_EVENT_INVALIDATE, _invalidated });

static void
_noref_death(void *data EINA_UNUSED, const Efl_Event *event)
{
   efl_event_callback_array_del(event->object, noref_death(), NULL);
   efl_del(event->object);
}

static void
_invalidated(void *data EINA_UNUSED, const Efl_Event *event)
{
   // This means our parent is dying, EFL_EVENT_NOREF can be called after invalidated
   efl_event_callback_array_del(event->object, noref_death(), NULL);
}

EOLIAN static void
_elm_fileselector_button_efl_canvas_group_group_add(Eo *obj, Elm_Fileselector_Button_Data *priv)
{
   const char *path;

   efl_canvas_group_add(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);

   priv->window_title = eina_stringshare_add(DEFAULT_WINDOW_TITLE);
   path = eina_environment_home_get();
   if (path) priv->fsd.path = eina_stringshare_add(path);
   else priv->fsd.path = eina_stringshare_add("/");

   priv->fsd.model = efl_add_ref(EIO_MODEL_CLASS, obj,
                                 eio_model_path_set(efl_added, priv->fsd.path),
                                 efl_event_callback_array_add(efl_added, noref_death(), NULL));

   priv->fsd.expandable = _elm_config->fileselector_expand_enable;
   priv->inwin_mode = _elm_config->inwin_dialogs_enable;
   priv->w = 400;
   priv->h = 400;

   efl_ui_mirrored_automatic_set(obj, EINA_FALSE);

   efl_event_callback_add(obj, EFL_UI_EVENT_CLICKED, _button_clicked, priv);

   efl_ui_widget_theme_apply(obj);
   elm_widget_can_focus_set(obj, EINA_TRUE);
}

EOLIAN static void
_elm_fileselector_button_efl_canvas_group_group_del(Eo *obj, Elm_Fileselector_Button_Data *sd)
{
   if (sd->fsd.model)
     efl_unref(sd->fsd.model);
   eina_stringshare_del(sd->window_title);
   eina_stringshare_del(sd->fsd.path);
   if (sd->fsd.selection)
     efl_unref(sd->fsd.selection);
   eina_stringshare_del(sd->fsd.selection_path);
   evas_object_del(sd->fsw);

   efl_canvas_group_del(efl_super(obj, MY_CLASS));
}

EOLIAN static Eina_Bool
_elm_fileselector_button_efl_ui_autorepeat_autorepeat_supported_get(const Eo *obj EINA_UNUSED, Elm_Fileselector_Button_Data *sd EINA_UNUSED)
{
   return EINA_FALSE;
}

EAPI Evas_Object *
elm_fileselector_button_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   return elm_legacy_add(MY_CLASS, parent);
}

EOLIAN static Eo *
_elm_fileselector_button_efl_object_constructor(Eo *obj, Elm_Fileselector_Button_Data *sd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   sd->obj = obj;

   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);
   evas_object_smart_callbacks_descriptions_set(obj, _smart_callbacks);
   efl_access_object_role_set(obj, EFL_ACCESS_ROLE_PUSH_BUTTON);
   legacy_object_focus_handle(obj);

   return obj;
}

EAPI void
elm_fileselector_button_window_title_set(Eo *obj, const char *title)
{
   ELM_FILESELECTOR_BUTTON_CHECK(obj);
   ELM_FILESELECTOR_BUTTON_DATA_GET_OR_RETURN(obj, sd);
   eina_stringshare_replace(&sd->window_title, title);
   if (sd->fsw) elm_win_title_set(sd->fsw, sd->window_title);
}

EAPI const char *
elm_fileselector_button_window_title_get(const Eo *obj)
{
   ELM_FILESELECTOR_BUTTON_CHECK(obj) NULL;
   ELM_FILESELECTOR_BUTTON_DATA_GET_OR_RETURN_VAL(obj, sd, NULL);
   return sd->window_title;
}

EAPI void
elm_fileselector_button_window_size_set(Eo *obj, Evas_Coord width, Evas_Coord height)
{
   ELM_FILESELECTOR_BUTTON_CHECK(obj);
   ELM_FILESELECTOR_BUTTON_DATA_GET_OR_RETURN(obj, sd);
   sd->w = width;
   sd->h = height;
   if (sd->fsw) evas_object_resize(sd->fsw, sd->w, sd->h);
}

EAPI void
elm_fileselector_button_window_size_get(const Eo *obj, Evas_Coord *width, Evas_Coord *height)
{
   if (width) *width = 0;
   if (height) *height = 0;
   ELM_FILESELECTOR_BUTTON_CHECK(obj);
   ELM_FILESELECTOR_BUTTON_DATA_GET_OR_RETURN(obj, sd);
   if (width) *width = sd->w;
   if (height) *height = sd->h;
}

void
_elm_fileselector_button_path_set_internal(Evas_Object *obj, const char *path)
{
   ELM_FILESELECTOR_BUTTON_DATA_GET_OR_RETURN(obj, sd);

   Efl_Model *model = efl_add(EIO_MODEL_CLASS, obj, eio_model_path_set(efl_added, path));
   if (!model)
     {
        ERR("Efl.Model allocation error");
        return;
     }

   // XXX: the efl_ref here smells wrong. fsd.model is only unreffed ONCE so this obj leaks...
   efl_replace(&sd->fsd.model, model);

   eina_stringshare_replace(&sd->fsd.path, path);

   if (sd->fs) elm_interface_fileselector_selected_model_set(sd->fs, model);
}

EINA_DEPRECATED EAPI void
elm_fileselector_button_path_set(Evas_Object *obj, const char *path)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj);
   elm_fileselector_path_set(obj, path);
}

EOLIAN static void
_elm_fileselector_button_efl_ui_view_model_set(Eo *obj EINA_UNUSED, Elm_Fileselector_Button_Data *sd, Efl_Model *model)
{
   char *file = NULL;

   efl_replace(&sd->fsd.model, model);

   if (model)
     {
        Eina_Value *path;

        path = efl_model_property_get(model, "path");
        file = eina_value_to_string(path);
        eina_value_free(path);
     }

   eina_stringshare_replace(&sd->fsd.path, file);

   _event_to_legacy_call
     (sd->obj, ELM_FILESELECTOR_BUTTON_EVENT_FILE_CHOSEN, file);

   free(file);

   if (sd->fs) elm_interface_fileselector_selected_model_set(sd->fs, model);
}

const char *
_elm_fileselector_button_path_get_internal(const Evas_Object *obj)
{
   ELM_FILESELECTOR_BUTTON_DATA_GET_OR_RETURN_VAL(obj, sd, NULL);
   return sd->fsd.path;
}

EINA_DEPRECATED EAPI const char *
elm_fileselector_button_path_get(const Evas_Object *obj)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj, NULL);
   return elm_fileselector_path_get(obj);
}

EOLIAN static Efl_Model *
_elm_fileselector_button_efl_ui_view_model_get(const Eo *obj EINA_UNUSED, Elm_Fileselector_Button_Data *sd)
{
   return sd->fsd.model;
}

EINA_DEPRECATED EAPI void
elm_fileselector_button_expandable_set(Evas_Object *obj,
                                       Eina_Bool value)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj);
   elm_interface_fileselector_expandable_set(obj, value);
}

EOLIAN static void
_elm_fileselector_button_elm_interface_fileselector_expandable_set(Eo *obj EINA_UNUSED, Elm_Fileselector_Button_Data *sd, Eina_Bool value)
{
   sd->fsd.expandable = value;

   if (sd->fs) elm_fileselector_expandable_set(sd->fs, sd->fsd.expandable);
}

EINA_DEPRECATED EAPI Eina_Bool
elm_fileselector_button_expandable_get(const Evas_Object *obj)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj, EINA_FALSE);
   Eina_Bool ret = EINA_FALSE;
   ret = elm_interface_fileselector_expandable_get((Eo *) obj);
   return ret;
}

EOLIAN static Eina_Bool
_elm_fileselector_button_elm_interface_fileselector_expandable_get(const Eo *obj EINA_UNUSED, Elm_Fileselector_Button_Data *sd)
{
   return sd->fsd.expandable;
}

EINA_DEPRECATED EAPI void
elm_fileselector_button_folder_only_set(Evas_Object *obj,
                                        Eina_Bool value)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj);
   elm_interface_fileselector_folder_only_set(obj, value);
}

EOLIAN static void
_elm_fileselector_button_elm_interface_fileselector_folder_only_set(Eo *obj EINA_UNUSED, Elm_Fileselector_Button_Data *sd, Eina_Bool value)
{
   sd->fsd.folder_only = value;

   if (sd->fs) elm_fileselector_folder_only_set(sd->fs, sd->fsd.folder_only);
}

EINA_DEPRECATED EAPI Eina_Bool
elm_fileselector_button_folder_only_get(const Evas_Object *obj)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj, EINA_FALSE);
   Eina_Bool ret = EINA_FALSE;
   ret = elm_interface_fileselector_folder_only_get((Eo *) obj);
   return ret;
}

EOLIAN static Eina_Bool
_elm_fileselector_button_elm_interface_fileselector_folder_only_get(const Eo *obj EINA_UNUSED, Elm_Fileselector_Button_Data *sd)
{
   return sd->fsd.folder_only;
}

EINA_DEPRECATED EAPI void
elm_fileselector_button_is_save_set(Evas_Object *obj,
                                    Eina_Bool value)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj);
   elm_interface_fileselector_is_save_set(obj, value);
}

EOLIAN static void
_elm_fileselector_button_elm_interface_fileselector_is_save_set(Eo *obj EINA_UNUSED, Elm_Fileselector_Button_Data *sd, Eina_Bool value)
{
   sd->fsd.is_save = value;

   if (sd->fs) elm_fileselector_is_save_set(sd->fs, sd->fsd.is_save);
}

EINA_DEPRECATED EAPI Eina_Bool
elm_fileselector_button_is_save_get(const Evas_Object *obj)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj, EINA_FALSE);
   Eina_Bool ret = EINA_FALSE;
   ret = elm_interface_fileselector_is_save_get((Eo *) obj);
   return ret;
}

EOLIAN static Eina_Bool
_elm_fileselector_button_elm_interface_fileselector_is_save_get(const Eo *obj EINA_UNUSED, Elm_Fileselector_Button_Data *sd)
{
   return sd->fsd.is_save;
}

EOLIAN static void
_elm_fileselector_button_elm_interface_fileselector_mode_set(Eo *obj EINA_UNUSED, Elm_Fileselector_Button_Data *sd, Elm_Fileselector_Mode mode)
{
   sd->fsd.mode = mode;

   if (sd->fs) elm_fileselector_mode_set(sd->fs, mode);
}

EOLIAN static Elm_Fileselector_Mode
_elm_fileselector_button_elm_interface_fileselector_mode_get(const Eo *obj EINA_UNUSED, Elm_Fileselector_Button_Data *sd)
{
   return sd->fsd.mode;
}

EOLIAN static void
_elm_fileselector_button_elm_interface_fileselector_sort_method_set(Eo *obj EINA_UNUSED, Elm_Fileselector_Button_Data *sd, Elm_Fileselector_Sort sort)
{
   sd->fsd.sort_type = sort;

   if (sd->fs) elm_fileselector_sort_method_set(sd->fs, sort);
}

EOLIAN static Elm_Fileselector_Sort
_elm_fileselector_button_elm_interface_fileselector_sort_method_get(const Eo *obj EINA_UNUSED, Elm_Fileselector_Button_Data *sd)
{
   return sd->fsd.sort_type;
}

EOLIAN static void
_elm_fileselector_button_elm_interface_fileselector_multi_select_set(Eo *obj EINA_UNUSED, Elm_Fileselector_Button_Data *sd, Eina_Bool value)
{
   sd->fsd.multi = value;

   if (sd->fs) elm_fileselector_multi_select_set(sd->fs, sd->fsd.multi);
}

EOLIAN static Eina_Bool
_elm_fileselector_button_elm_interface_fileselector_multi_select_get(const Eo *obj EINA_UNUSED, Elm_Fileselector_Button_Data *sd)
{
   return sd->fsd.multi;
}

const Eina_List *
_elm_fileselector_button_selected_paths_get_internal(const Evas_Object *obj)
{
   ELM_FILESELECTOR_BUTTON_DATA_GET_OR_RETURN_VAL(obj, sd, NULL);

   if (sd->fs) return elm_fileselector_selected_paths_get(sd->fs);

   return NULL;
}

EOLIAN static const Eina_List*
_elm_fileselector_button_elm_interface_fileselector_selected_models_get(const Eo *obj EINA_UNUSED, Elm_Fileselector_Button_Data *sd)
{
   if (sd->fs) return elm_interface_fileselector_selected_models_get(sd->fs);

   return NULL;
}

const char *
_elm_fileselector_button_selected_get_internal(const Evas_Object *obj)
{
   ELM_FILESELECTOR_BUTTON_DATA_GET_OR_RETURN_VAL(obj, sd, NULL);

   if (sd->fs) return elm_fileselector_selected_get(sd->fs);

   return sd->fsd.selection_path;
}

EOLIAN static Efl_Model *
_elm_fileselector_button_elm_interface_fileselector_selected_model_get(const Eo *obj EINA_UNUSED, Elm_Fileselector_Button_Data *sd)
{
   if (sd->fs) return elm_interface_fileselector_selected_model_get(sd->fs);

   return sd->fsd.selection;
}

Eina_Bool
_elm_fileselector_button_selected_set_internal(Evas_Object *obj, const char *_path)
{
   ELM_FILESELECTOR_BUTTON_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);
   Eina_Bool ret = EINA_TRUE;

   if (sd->fs) ret = elm_fileselector_selected_set(sd->fs, _path);
   else
     {
        char *path = ecore_file_realpath(_path);
        if (!ecore_file_is_dir(path) && !ecore_file_exists(path))
          {
             free(path);
             return EINA_FALSE;
          }
        free(path);
     }

   eina_stringshare_replace(&sd->fsd.selection_path, _path);

   return ret;
}

EOLIAN static Eina_Bool
_elm_fileselector_button_elm_interface_fileselector_selected_model_set(Eo *obj EINA_UNUSED, Elm_Fileselector_Button_Data *sd, Efl_Model *model)
{
   if (sd->fs)
     elm_interface_fileselector_selected_model_set(sd->fs, model);

   efl_replace(&sd->fsd.selection, model);

   return EINA_TRUE;
}

EOLIAN static void
_elm_fileselector_button_elm_interface_fileselector_thumbnail_size_set(Eo *obj EINA_UNUSED, Elm_Fileselector_Button_Data *sd, Evas_Coord w, Evas_Coord h)
{
   if (sd->fs)
     {
        elm_fileselector_thumbnail_size_set(sd->fs, w, h);
        elm_fileselector_thumbnail_size_get(sd->fs, &w, &h);
     }
   else if (!w || !h)
     w = h = elm_config_finger_size_get() * 2 - GENGRID_PADDING;

   sd->fsd.thumbnail_size.w = w;
   sd->fsd.thumbnail_size.h = h;
}

EOLIAN static void
_elm_fileselector_button_elm_interface_fileselector_thumbnail_size_get(const Eo *obj EINA_UNUSED, Elm_Fileselector_Button_Data *sd, Evas_Coord *w, Evas_Coord *h)
{
   if (w) *w = sd->fsd.thumbnail_size.w;
   if (h) *h = sd->fsd.thumbnail_size.h;
}

EOLIAN static void
_elm_fileselector_button_elm_interface_fileselector_hidden_visible_set(Eo *obj EINA_UNUSED, Elm_Fileselector_Button_Data *sd, Eina_Bool visible)
{
   sd->fsd.hidden_visible = visible;

   if (sd->fs) elm_fileselector_hidden_visible_set(sd->fs, visible);
}

EOLIAN static Eina_Bool
_elm_fileselector_button_elm_interface_fileselector_hidden_visible_get(const Eo *obj EINA_UNUSED, Elm_Fileselector_Button_Data *sd)
{
   return sd->fsd.hidden_visible;
}

EAPI void
elm_fileselector_button_inwin_mode_set(Eo *obj, Eina_Bool value)
{
   ELM_FILESELECTOR_BUTTON_CHECK(obj);
   ELM_FILESELECTOR_BUTTON_DATA_GET_OR_RETURN(obj, sd);
   sd->inwin_mode = value;
}

EAPI Eina_Bool
elm_fileselector_button_inwin_mode_get(const Eo *obj)
{
   ELM_FILESELECTOR_BUTTON_CHECK(obj) EINA_FALSE;
   ELM_FILESELECTOR_BUTTON_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);
   return sd->inwin_mode;
}

/* Internal EO APIs and hidden overrides */

#define ELM_FILESELECTOR_BUTTON_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(elm_fileselector_button)

#include "elm_fileselector_button.eo.c"
