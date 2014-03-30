#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"
#include "elm_interface_fileselector.h"
#include "elm_widget_fileselector_button.h"

EAPI Eo_Op ELM_OBJ_FILESELECTOR_BUTTON_BASE_ID = EO_NOOP;

#define MY_CLASS ELM_OBJ_FILESELECTOR_BUTTON_CLASS

#define MY_CLASS_NAME "Elm_Fileselector_Button"
#define MY_CLASS_NAME_LEGACY "elm_fileselector_button"

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

static void
_elm_fileselector_button_smart_theme(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret;

   char buf[4096];
   const char *style;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   style = eina_stringshare_add(elm_widget_style_get(obj));

   snprintf(buf, sizeof(buf), "fileselector_button/%s", style);

   /* file selector button's style has an extra bit */
   eina_stringshare_replace(&(wd->style), buf);

   eo_do_super(obj, MY_CLASS, elm_obj_widget_theme_apply(&int_ret));
   if (!int_ret) return;

   eina_stringshare_replace(&(wd->style), style);

   eina_stringshare_del(style);

   if (ret) *ret = EINA_TRUE;
}

static void
_selection_done(void *data,
                Evas_Object *obj EINA_UNUSED,
                void *event_info)
{
   Elm_Fileselector_Button_Smart_Data *sd = data;
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
_new_window_add(Elm_Fileselector_Button_Smart_Data *sd)
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
_activate(Elm_Fileselector_Button_Smart_Data *sd)
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

static void
_elm_fileselector_button_smart_add(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Fileselector_Button_Smart_Data *priv = _pd;

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

   eo_do(obj, elm_obj_widget_theme_apply(NULL));
   elm_widget_can_focus_set(obj, EINA_TRUE);
}

static void
_elm_fileselector_button_smart_del(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Fileselector_Button_Smart_Data *sd = _pd;

   eina_stringshare_del(sd->window_title);
   eina_stringshare_del(sd->fsd.path);
   evas_object_del(sd->fsw);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_del());
}

static void
_elm_fileselector_button_smart_admits_autorepeat_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_FALSE;
}

EAPI Evas_Object *
elm_fileselector_button_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = eo_add(MY_CLASS, parent);
   eo_unref(obj);
   return obj;
}

static void
_constructor(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Fileselector_Button_Smart_Data *sd = _pd;
   sd->obj = obj;

   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME_LEGACY),
         evas_obj_smart_callbacks_descriptions_set(_smart_callbacks, NULL));
}

EAPI void
elm_fileselector_button_window_title_set(Evas_Object *obj,
                                         const char *title)
{
   ELM_FILESELECTOR_BUTTON_CHECK(obj);
   eo_do(obj, elm_obj_fileselector_button_window_title_set(title));
}

static void
_window_title_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *title = va_arg(*list, const char *);
   Elm_Fileselector_Button_Smart_Data *sd = _pd;

   eina_stringshare_replace(&sd->window_title, title);

   if (sd->fsw) elm_win_title_set(sd->fsw, sd->window_title);
}

EAPI const char *
elm_fileselector_button_window_title_get(const Evas_Object *obj)
{
   ELM_FILESELECTOR_BUTTON_CHECK(obj) NULL;
   const char *ret = NULL;
   eo_do((Eo *) obj, elm_obj_fileselector_button_window_title_get(&ret));
   return ret;
}

static void
_window_title_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char **ret = va_arg(*list, const char **);
   Elm_Fileselector_Button_Smart_Data *sd = _pd;

   *ret = sd->window_title;
}

EAPI void
elm_fileselector_button_window_size_set(Evas_Object *obj,
                                        Evas_Coord width,
                                        Evas_Coord height)
{
   ELM_FILESELECTOR_BUTTON_CHECK(obj);
   eo_do(obj, elm_obj_fileselector_button_window_size_set(width, height));
}

static void
_window_size_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Coord width = va_arg(*list, Evas_Coord);
   Evas_Coord height = va_arg(*list, Evas_Coord);
   Elm_Fileselector_Button_Smart_Data *sd = _pd;

   sd->w = width;
   sd->h = height;

   if (sd->fsw) evas_object_resize(sd->fsw, sd->w, sd->h);
}

EAPI void
elm_fileselector_button_window_size_get(const Evas_Object *obj,
                                        Evas_Coord *width,
                                        Evas_Coord *height)
{
   ELM_FILESELECTOR_BUTTON_CHECK(obj);
   eo_do((Eo *) obj, elm_obj_fileselector_button_window_size_get(width, height));
}

static void
_window_size_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Coord *width = va_arg(*list, Evas_Coord *);
   Evas_Coord *height = va_arg(*list, Evas_Coord *);
   Elm_Fileselector_Button_Smart_Data *sd = _pd;

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

static void
_path_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *path = va_arg(*list, const char *);
   Elm_Fileselector_Button_Smart_Data *sd = _pd;

   eina_stringshare_replace(&sd->fsd.path, path);

   if (sd->fs) elm_fileselector_selected_set(sd->fs, sd->fsd.path);
}

EINA_DEPRECATED EAPI const char *
elm_fileselector_button_path_get(const Evas_Object *obj)
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
   Elm_Fileselector_Button_Smart_Data *sd = _pd;

   *ret = sd->fsd.path;
}

EINA_DEPRECATED EAPI void
elm_fileselector_button_expandable_set(Evas_Object *obj,
                                       Eina_Bool value)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj);
   eo_do(obj, elm_interface_fileselector_expandable_set(value));
}

static void
_expandable_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool value = va_arg(*list, int);
   Elm_Fileselector_Button_Smart_Data *sd = _pd;

   sd->fsd.expandable = value;

   if (sd->fs) elm_fileselector_expandable_set(sd->fs, sd->fsd.expandable);
}

EINA_DEPRECATED EAPI Eina_Bool
elm_fileselector_button_expandable_get(const Evas_Object *obj)
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
   Elm_Fileselector_Button_Smart_Data *sd = _pd;

   *ret = sd->fsd.expandable;
}

EINA_DEPRECATED EAPI void
elm_fileselector_button_folder_only_set(Evas_Object *obj,
                                        Eina_Bool value)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj);
   eo_do(obj, elm_interface_fileselector_folder_only_set(value));
}

static void
_folder_only_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool value = va_arg(*list, int);
   Elm_Fileselector_Button_Smart_Data *sd = _pd;

   sd->fsd.folder_only = value;

   if (sd->fs) elm_fileselector_folder_only_set(sd->fs, sd->fsd.folder_only);
}

EINA_DEPRECATED EAPI Eina_Bool
elm_fileselector_button_folder_only_get(const Evas_Object *obj)
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
   Elm_Fileselector_Button_Smart_Data *sd = _pd;

   *ret = sd->fsd.folder_only;
}

EINA_DEPRECATED EAPI void
elm_fileselector_button_is_save_set(Evas_Object *obj,
                                    Eina_Bool value)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj);
   eo_do(obj, elm_interface_fileselector_is_save_set(value));
}

static void
_is_save_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool value = va_arg(*list, int);
   Elm_Fileselector_Button_Smart_Data *sd = _pd;

   sd->fsd.is_save = value;

   if (sd->fs) elm_fileselector_is_save_set(sd->fs, sd->fsd.is_save);
}

EINA_DEPRECATED EAPI Eina_Bool
elm_fileselector_button_is_save_get(const Evas_Object *obj)
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
   Elm_Fileselector_Button_Smart_Data *sd = _pd;

   *ret = sd->fsd.is_save;
}

EAPI void
elm_fileselector_button_inwin_mode_set(Evas_Object *obj,
                                       Eina_Bool value)
{
   ELM_FILESELECTOR_BUTTON_CHECK(obj);
   eo_do(obj, elm_obj_fileselector_button_inwin_mode_set(value));
}

static void
_inwin_mode_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool value = va_arg(*list, int);
   Elm_Fileselector_Button_Smart_Data *sd = _pd;

   sd->inwin_mode = value;
}

EAPI Eina_Bool
elm_fileselector_button_inwin_mode_get(const Evas_Object *obj)
{
   ELM_FILESELECTOR_BUTTON_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_fileselector_button_inwin_mode_get(&ret));
   return ret;
}

static void
_inwin_mode_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Fileselector_Button_Smart_Data *sd = _pd;

   *ret = sd->inwin_mode;
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_ADD), _elm_fileselector_button_smart_add),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_DEL), _elm_fileselector_button_smart_del),

        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_THEME_APPLY), _elm_fileselector_button_smart_theme),

        EO_OP_FUNC(ELM_OBJ_BUTTON_ID(ELM_OBJ_BUTTON_SUB_ID_ADMITS_AUTOREPEAT_GET), _elm_fileselector_button_smart_admits_autorepeat_get),

        EO_OP_FUNC(ELM_INTERFACE_FILESELECTOR_ID(ELM_INTERFACE_FILESELECTOR_SUB_ID_PATH_SET), _path_set),
        EO_OP_FUNC(ELM_INTERFACE_FILESELECTOR_ID(ELM_INTERFACE_FILESELECTOR_SUB_ID_PATH_GET), _path_get),
        EO_OP_FUNC(ELM_INTERFACE_FILESELECTOR_ID(ELM_INTERFACE_FILESELECTOR_SUB_ID_EXPANDABLE_SET), _expandable_set),
        EO_OP_FUNC(ELM_INTERFACE_FILESELECTOR_ID(ELM_INTERFACE_FILESELECTOR_SUB_ID_EXPANDABLE_GET), _expandable_get),
        EO_OP_FUNC(ELM_INTERFACE_FILESELECTOR_ID(ELM_INTERFACE_FILESELECTOR_SUB_ID_FOLDER_ONLY_SET), _folder_only_set),
        EO_OP_FUNC(ELM_INTERFACE_FILESELECTOR_ID(ELM_INTERFACE_FILESELECTOR_SUB_ID_FOLDER_ONLY_GET), _folder_only_get),
        EO_OP_FUNC(ELM_INTERFACE_FILESELECTOR_ID(ELM_INTERFACE_FILESELECTOR_SUB_ID_IS_SAVE_SET), _is_save_set),
        EO_OP_FUNC(ELM_INTERFACE_FILESELECTOR_ID(ELM_INTERFACE_FILESELECTOR_SUB_ID_IS_SAVE_GET), _is_save_get),

        EO_OP_FUNC(ELM_OBJ_FILESELECTOR_BUTTON_ID(ELM_OBJ_FILESELECTOR_BUTTON_SUB_ID_WINDOW_TITLE_SET), _window_title_set),
        EO_OP_FUNC(ELM_OBJ_FILESELECTOR_BUTTON_ID(ELM_OBJ_FILESELECTOR_BUTTON_SUB_ID_WINDOW_TITLE_GET), _window_title_get),
        EO_OP_FUNC(ELM_OBJ_FILESELECTOR_BUTTON_ID(ELM_OBJ_FILESELECTOR_BUTTON_SUB_ID_WINDOW_SIZE_SET), _window_size_set),
        EO_OP_FUNC(ELM_OBJ_FILESELECTOR_BUTTON_ID(ELM_OBJ_FILESELECTOR_BUTTON_SUB_ID_WINDOW_SIZE_GET), _window_size_get),
        EO_OP_FUNC(ELM_OBJ_FILESELECTOR_BUTTON_ID(ELM_OBJ_FILESELECTOR_BUTTON_SUB_ID_INWIN_MODE_SET), _inwin_mode_set),
        EO_OP_FUNC(ELM_OBJ_FILESELECTOR_BUTTON_ID(ELM_OBJ_FILESELECTOR_BUTTON_SUB_ID_INWIN_MODE_GET), _inwin_mode_get),
        EO_OP_FUNC_SENTINEL
   };
   eo_class_funcs_set(klass, func_desc);

   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(ELM_OBJ_FILESELECTOR_BUTTON_SUB_ID_WINDOW_TITLE_SET, "Set the title for a given file selector button widget's window."),
     EO_OP_DESCRIPTION(ELM_OBJ_FILESELECTOR_BUTTON_SUB_ID_WINDOW_TITLE_GET, "Get the title for a given file selector button widget's window."),
     EO_OP_DESCRIPTION(ELM_OBJ_FILESELECTOR_BUTTON_SUB_ID_WINDOW_SIZE_SET, "Set the size of a given file selector button widget's window, holding the file selector itself."),
     EO_OP_DESCRIPTION(ELM_OBJ_FILESELECTOR_BUTTON_SUB_ID_WINDOW_SIZE_GET, "Get the size of a given file selector button widget's window, holding the file selector itself."),
     EO_OP_DESCRIPTION(ELM_OBJ_FILESELECTOR_BUTTON_SUB_ID_INWIN_MODE_SET, "Set whether a given file selector button widget's internal file selector will raise an Elementary 'inner window', instead of a dedicated Elementary window."),
     EO_OP_DESCRIPTION(ELM_OBJ_FILESELECTOR_BUTTON_SUB_ID_INWIN_MODE_GET, "Get whether a given file selector button widget's internal file selector will raise an Elementary 'inner window', instead of a dedicated Elementary window."),
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     MY_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&ELM_OBJ_FILESELECTOR_BUTTON_BASE_ID, op_desc, ELM_OBJ_FILESELECTOR_BUTTON_SUB_ID_LAST),
     NULL,
     sizeof(Elm_Fileselector_Button_Smart_Data),
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(elm_obj_fileselector_button_class_get, &class_desc, ELM_OBJ_BUTTON_CLASS, ELM_INTERFACE_FILESELECTOR_CLASS, NULL);
