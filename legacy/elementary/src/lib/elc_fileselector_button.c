#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"
#include "elm_interface_fileselector.h"
#include "elm_widget_fileselector_button.h"

#define MY_CLASS ELM_OBJ_FILESELECTOR_BUTTON_CLASS

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

EOLIAN static Eina_Bool
_elc_fileselector_button_elm_widget_theme_apply(Eo *obj, Elc_Fileselector_Button_Data *sd EINA_UNUSED)
{
   Eina_Bool int_ret = EINA_FALSE;

   char buf[4096];
   const char *style;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   style = eina_stringshare_add(elm_widget_style_get(obj));

   snprintf(buf, sizeof(buf), "fileselector_button/%s", style);

   /* file selector button's style has an extra bit */
   eina_stringshare_replace(&(wd->style), buf);

   eo_do_super(obj, MY_CLASS, int_ret = elm_obj_widget_theme_apply());
   if (!int_ret) return EINA_FALSE;

   eina_stringshare_replace(&(wd->style), style);

   eina_stringshare_del(style);

   return EINA_TRUE;
}

static void
_selection_done(void *data,
                Evas_Object *obj EINA_UNUSED,
                void *event_info)
{
   Elc_Fileselector_Button_Data *sd = data;
   const char *file = event_info;
   Evas_Object *del;

   if (file) eina_stringshare_replace(&sd->fsd.path, file);

   del = sd->fsw;
   sd->fs = NULL;
   sd->fsw = NULL;
   evas_object_del(del);

   evas_object_smart_callback_call
     (sd->obj, SIG_FILE_CHOSEN, (void *)file);
}

static Evas_Object *
_new_window_add(Elc_Fileselector_Button_Data *sd)
{
   Evas_Object *win, *bg;

   win = elm_win_add(NULL, "fileselector_button", ELM_WIN_DIALOG_BASIC);
   elm_win_title_set(win, sd->window_title);
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_smart_callback_add(win, "delete,request", _selection_done, sd);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   evas_object_resize(win, sd->w, sd->h);
   return win;
}

static Evas_Object *
_parent_win_get(Evas_Object *obj)
{
   while ((obj) && (strcmp(evas_object_type_get(obj), "elm_win")))
     obj = elm_object_parent_widget_get(obj);

   return obj;
}

static void
_activate(Elc_Fileselector_Button_Data *sd)
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
   elm_widget_mirrored_set
     (sd->fs, elm_widget_mirrored_get(sd->obj));
   elm_widget_mirrored_automatic_set(sd->fs, EINA_FALSE);
   elm_fileselector_expandable_set(sd->fs, sd->fsd.expandable);
   elm_fileselector_folder_only_set(sd->fs, sd->fsd.folder_only);
   elm_fileselector_is_save_set(sd->fs, sd->fsd.is_save);
   elm_fileselector_selected_set(sd->fs, sd->fsd.path);
   evas_object_size_hint_weight_set
     (sd->fs, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(sd->fs, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(sd->fs, "done", _selection_done, sd);
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
_button_clicked(void *data,
                Evas_Object *obj EINA_UNUSED,
                void *event_info EINA_UNUSED)
{
   _activate(data);
}

EOLIAN static void
_elc_fileselector_button_evas_smart_add(Eo *obj, Elc_Fileselector_Button_Data *priv)
{
   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());
   elm_widget_sub_object_parent_add(obj);

   priv->window_title = eina_stringshare_add(DEFAULT_WINDOW_TITLE);
   if (getenv("HOME")) priv->fsd.path = eina_stringshare_add(getenv("HOME"));
   else priv->fsd.path = eina_stringshare_add("/");

   priv->fsd.expandable = _elm_config->fileselector_expand_enable;
   priv->inwin_mode = _elm_config->inwin_dialogs_enable;
   priv->w = 400;
   priv->h = 400;

   elm_widget_mirrored_automatic_set(obj, EINA_FALSE);

   evas_object_smart_callback_add(obj, "clicked", _button_clicked, priv);

   eo_do(obj, elm_obj_widget_theme_apply());
   elm_widget_can_focus_set(obj, EINA_TRUE);
}

EOLIAN static void
_elc_fileselector_button_evas_smart_del(Eo *obj, Elc_Fileselector_Button_Data *sd)
{
   eina_stringshare_del(sd->window_title);
   eina_stringshare_del(sd->fsd.path);
   eina_stringshare_del(sd->fsd.selection);
   evas_object_del(sd->fsw);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_del());
}

EOLIAN static Eina_Bool
_elc_fileselector_button_elm_button_admits_autorepeat_get(Eo *obj EINA_UNUSED, Elc_Fileselector_Button_Data *sd EINA_UNUSED)
{
   return EINA_FALSE;
}

EAPI Evas_Object *
elm_fileselector_button_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = eo_add(MY_CLASS, parent);
   eo_unref(obj);
   return obj;
}

EOLIAN static void
_elc_fileselector_button_eo_base_constructor(Eo *obj, Elc_Fileselector_Button_Data *sd)
{
   sd->obj = obj;

   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME_LEGACY),
         evas_obj_smart_callbacks_descriptions_set(_smart_callbacks));
}

EOLIAN static void
_elc_fileselector_button_window_title_set(Eo *obj EINA_UNUSED, Elc_Fileselector_Button_Data *sd, const char *title)
{
   eina_stringshare_replace(&sd->window_title, title);

   if (sd->fsw) elm_win_title_set(sd->fsw, sd->window_title);
}

EOLIAN static const char *
_elc_fileselector_button_window_title_get(Eo *obj EINA_UNUSED, Elc_Fileselector_Button_Data *sd)
{
   return sd->window_title;
}

EOLIAN static void
_elc_fileselector_button_window_size_set(Eo *obj EINA_UNUSED, Elc_Fileselector_Button_Data *sd, Evas_Coord width, Evas_Coord height)
{
   sd->w = width;
   sd->h = height;

   if (sd->fsw) evas_object_resize(sd->fsw, sd->w, sd->h);
}

EOLIAN static void
_elc_fileselector_button_window_size_get(Eo *obj EINA_UNUSED, Elc_Fileselector_Button_Data *sd, Evas_Coord *width, Evas_Coord *height)
{
   if (width) *width = sd->w;
   if (height) *height = sd->h;
}

EINA_DEPRECATED EAPI void
elm_fileselector_button_path_set(Evas_Object *obj,
                                 const char *path)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj);
   eo_do(obj, elm_interface_fileselector_path_set(path));
}

EOLIAN static void
_elc_fileselector_button_elm_interface_fileselector_path_set(Eo *obj EINA_UNUSED, Elc_Fileselector_Button_Data *sd, const char *path)
{
   eina_stringshare_replace(&sd->fsd.path, path);

   if (sd->fs) elm_fileselector_selected_set(sd->fs, sd->fsd.path);
}

EINA_DEPRECATED EAPI const char *
elm_fileselector_button_path_get(const Evas_Object *obj)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj, NULL);
   const char *ret = NULL;
   eo_do((Eo *) obj, ret = elm_interface_fileselector_path_get());
   return ret;
}

EOLIAN static const char *
_elc_fileselector_button_elm_interface_fileselector_path_get(Eo *obj EINA_UNUSED, Elc_Fileselector_Button_Data *sd)
{
   return sd->fsd.path;
}

EINA_DEPRECATED EAPI void
elm_fileselector_button_expandable_set(Evas_Object *obj,
                                       Eina_Bool value)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj);
   eo_do(obj, elm_interface_fileselector_expandable_set(value));
}

EOLIAN static void
_elc_fileselector_button_elm_interface_fileselector_expandable_set(Eo *obj EINA_UNUSED, Elc_Fileselector_Button_Data *sd, Eina_Bool value)
{
   sd->fsd.expandable = value;

   if (sd->fs) elm_fileselector_expandable_set(sd->fs, sd->fsd.expandable);
}

EINA_DEPRECATED EAPI Eina_Bool
elm_fileselector_button_expandable_get(const Evas_Object *obj)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj, EINA_FALSE);
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, ret = elm_interface_fileselector_expandable_get());
   return ret;
}

EOLIAN static Eina_Bool
_elc_fileselector_button_elm_interface_fileselector_expandable_get(Eo *obj EINA_UNUSED, Elc_Fileselector_Button_Data *sd)
{
   return sd->fsd.expandable;
}

EINA_DEPRECATED EAPI void
elm_fileselector_button_folder_only_set(Evas_Object *obj,
                                        Eina_Bool value)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj);
   eo_do(obj, elm_interface_fileselector_folder_only_set(value));
}

EOLIAN static void
_elc_fileselector_button_elm_interface_fileselector_folder_only_set(Eo *obj EINA_UNUSED, Elc_Fileselector_Button_Data *sd, Eina_Bool value)
{
   sd->fsd.folder_only = value;

   if (sd->fs) elm_fileselector_folder_only_set(sd->fs, sd->fsd.folder_only);
}

EINA_DEPRECATED EAPI Eina_Bool
elm_fileselector_button_folder_only_get(const Evas_Object *obj)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj, EINA_FALSE);
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, ret = elm_interface_fileselector_folder_only_get());
   return ret;
}

EOLIAN static Eina_Bool
_elc_fileselector_button_elm_interface_fileselector_folder_only_get(Eo *obj EINA_UNUSED, Elc_Fileselector_Button_Data *sd)
{
   return sd->fsd.folder_only;
}

EINA_DEPRECATED EAPI void
elm_fileselector_button_is_save_set(Evas_Object *obj,
                                    Eina_Bool value)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj);
   eo_do(obj, elm_interface_fileselector_is_save_set(value));
}

EOLIAN static void
_elc_fileselector_button_elm_interface_fileselector_is_save_set(Eo *obj EINA_UNUSED, Elc_Fileselector_Button_Data *sd, Eina_Bool value)
{
   sd->fsd.is_save = value;

   if (sd->fs) elm_fileselector_is_save_set(sd->fs, sd->fsd.is_save);
}

EINA_DEPRECATED EAPI Eina_Bool
elm_fileselector_button_is_save_get(const Evas_Object *obj)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj, EINA_FALSE);
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, ret = elm_interface_fileselector_is_save_get());
   return ret;
}

EOLIAN static Eina_Bool
_elc_fileselector_button_elm_interface_fileselector_is_save_get(Eo *obj EINA_UNUSED, Elc_Fileselector_Button_Data *sd)
{
   return sd->fsd.is_save;
}

EOLIAN static void
_elc_fileselector_button_elm_interface_fileselector_mode_set(Eo *obj EINA_UNUSED, Elc_Fileselector_Button_Data *sd, Elm_Fileselector_Mode mode)
{
   sd->fsd.mode = mode;

   if (sd->fs) elm_fileselector_mode_set(sd->fs, mode);
}

EOLIAN static Elm_Fileselector_Mode
_elc_fileselector_button_elm_interface_fileselector_mode_get(Eo *obj EINA_UNUSED, Elc_Fileselector_Button_Data *sd)
{
   return sd->fsd.mode;
}

EOLIAN static void
_elc_fileselector_button_elm_interface_fileselector_sort_method_set(Eo *obj EINA_UNUSED, Elc_Fileselector_Button_Data *sd, Elm_Fileselector_Sort sort)
{
   sd->fsd.sort_type = sort;

   if (sd->fs) elm_fileselector_sort_method_set(sd->fs, sort);
}

EOLIAN static Elm_Fileselector_Sort
_elc_fileselector_button_elm_interface_fileselector_sort_method_get(Eo *obj EINA_UNUSED, Elc_Fileselector_Button_Data *sd)
{
   return sd->fsd.sort_type;
}

EOLIAN static void
_elc_fileselector_button_elm_interface_fileselector_multi_select_set(Eo *obj EINA_UNUSED, Elc_Fileselector_Button_Data *sd, Eina_Bool value)
{
   sd->fsd.multi = value;

   if (sd->fs) elm_fileselector_multi_select_set(sd->fs, sd->fsd.multi);
}

EOLIAN static Eina_Bool
_elc_fileselector_button_elm_interface_fileselector_multi_select_get(Eo *obj EINA_UNUSED, Elc_Fileselector_Button_Data *sd)
{
   return sd->fsd.multi;
}

EOLIAN static const Eina_List*
_elc_fileselector_button_elm_interface_fileselector_selected_paths_get(Eo *obj EINA_UNUSED, Elc_Fileselector_Button_Data *sd)
{
   if (sd->fs) return elm_fileselector_selected_paths_get(sd->fs);

   return NULL;
}

EOLIAN static const char*
_elc_fileselector_button_elm_interface_fileselector_selected_get(Eo *obj EINA_UNUSED, Elc_Fileselector_Button_Data *sd)
{
   if (sd->fs) return elm_fileselector_selected_get(sd->fs);

   return sd->fsd.selection;
}

EOLIAN static Eina_Bool
_elc_fileselector_button_elm_interface_fileselector_selected_set(Eo *obj EINA_UNUSED, Elc_Fileselector_Button_Data *sd, const char *_path)
{
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
     }

   eina_stringshare_replace(&sd->fsd.selection, _path);

   return ret;
}

EOLIAN static void
_elc_fileselector_button_elm_interface_fileselector_thumbnail_size_set(Eo *obj EINA_UNUSED, Elc_Fileselector_Button_Data *sd, Evas_Coord w, Evas_Coord h)
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
_elc_fileselector_button_elm_interface_fileselector_thumbnail_size_get(Eo *obj EINA_UNUSED, Elc_Fileselector_Button_Data *sd, Evas_Coord *w, Evas_Coord *h)
{
   if (w) *w = sd->fsd.thumbnail_size.w;
   if (h) *h = sd->fsd.thumbnail_size.h;
}

EOLIAN static void
_elc_fileselector_button_elm_interface_fileselector_hidden_visible_set(Eo *obj EINA_UNUSED, Elc_Fileselector_Button_Data *sd, Eina_Bool visible)
{
   sd->fsd.hidden_visible = visible;

   if (sd->fs) elm_fileselector_hidden_visible_set(sd->fs, visible);
}

EOLIAN static Eina_Bool
_elc_fileselector_button_elm_interface_fileselector_hidden_visible_get(Eo *obj EINA_UNUSED, Elc_Fileselector_Button_Data *sd)
{
   return sd->fsd.hidden_visible;
}

EOLIAN static void
_elc_fileselector_button_inwin_mode_set(Eo *obj EINA_UNUSED, Elc_Fileselector_Button_Data *sd, Eina_Bool value)
{
   sd->inwin_mode = value;
}

EOLIAN static Eina_Bool
_elc_fileselector_button_inwin_mode_get(Eo *obj EINA_UNUSED, Elc_Fileselector_Button_Data *sd)
{
   return sd->inwin_mode;
}

#include "elc_fileselector_button.eo.c"
