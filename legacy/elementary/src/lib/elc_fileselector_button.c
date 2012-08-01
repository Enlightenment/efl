#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_fileselector_button.h"

EAPI const char ELM_FILESELECTOR_BUTTON_SMART_NAME[] =
  "elm_fileselector_button";

#define DEFAULT_WINDOW_TITLE "Select a file"

static const char SIG_FILE_CHOSEN[] = "file,chosen";
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_FILE_CHOSEN, "s"},
   {NULL, NULL}
};

EVAS_SMART_SUBCLASS_NEW
  (ELM_FILESELECTOR_BUTTON_SMART_NAME, _elm_fileselector_button,
  Elm_Fileselector_Button_Smart_Class, Elm_Button_Smart_Class,
  elm_button_smart_class_get, _smart_callbacks);

static Eina_Bool
_elm_fileselector_button_smart_theme(Evas_Object *obj)
{
   char buf[4096];
   const char *style;

   ELM_FILESELECTOR_BUTTON_DATA_GET(obj, sd);

   style = eina_stringshare_add(elm_widget_style_get(obj));

   snprintf(buf, sizeof(buf), "fileselector_button/%s", style);

   /* file selector button's style has an extra bit */
   eina_stringshare_replace(&(ELM_WIDGET_DATA(sd)->style), buf);

   if (!ELM_WIDGET_CLASS(_elm_fileselector_button_parent_sc)->theme(obj))
     return EINA_FALSE;

   eina_stringshare_replace(&(ELM_WIDGET_DATA(sd)->style), style);

   eina_stringshare_del(style);

   return EINA_TRUE;
}

static void
_selection_done(void *data,
                Evas_Object *obj __UNUSED__,
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
     (ELM_WIDGET_DATA(sd)->obj, SIG_FILE_CHOSEN, (void *)file);
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
   while ((obj) && (strcmp(elm_widget_type_get(obj), "elm_win")))
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
        sd->fsw = _parent_win_get(ELM_WIDGET_DATA(sd)->obj);

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
     (sd->fs, elm_widget_mirrored_get(ELM_WIDGET_DATA(sd)->obj));
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
                Evas_Object *obj __UNUSED__,
                void *event_info __UNUSED__)
{
   _activate(data);
}

static void
_elm_fileselector_button_smart_add(Evas_Object *obj)
{
   EVAS_SMART_DATA_ALLOC(obj, Elm_Fileselector_Button_Smart_Data);

   ELM_WIDGET_CLASS(_elm_fileselector_button_parent_sc)->base.add(obj);

   priv->window_title = eina_stringshare_add(DEFAULT_WINDOW_TITLE);
   if (getenv("HOME")) priv->fsd.path = eina_stringshare_add(getenv("HOME"));
   else priv->fsd.path = eina_stringshare_add("/");

   priv->fsd.expandable = _elm_config->fileselector_expand_enable;
   priv->inwin_mode = _elm_config->inwin_dialogs_enable;
   priv->w = 400;
   priv->h = 400;

   elm_widget_mirrored_automatic_set(obj, EINA_FALSE);

   evas_object_smart_callback_add(obj, "clicked", _button_clicked, priv);

   _elm_fileselector_button_smart_theme(obj);
   elm_widget_can_focus_set(obj, EINA_TRUE);
}

static void
_elm_fileselector_button_smart_del(Evas_Object *obj)
{
   Evas_Object *win;

   ELM_FILESELECTOR_BUTTON_DATA_GET(obj, sd);

   if (sd->window_title) eina_stringshare_del(sd->window_title);
   if (sd->fsd.path) eina_stringshare_del(sd->fsd.path);
   if (sd->fs)
     {
        win = evas_object_data_del(obj, "win");
        evas_object_del(win);
     }

   ELM_WIDGET_CLASS(_elm_fileselector_button_parent_sc)->base.del(obj);
}

static void
_elm_fileselector_button_smart_set_user(
  Elm_Fileselector_Button_Smart_Class *sc)
{
   ELM_WIDGET_CLASS(sc)->base.add = _elm_fileselector_button_smart_add;
   ELM_WIDGET_CLASS(sc)->base.del = _elm_fileselector_button_smart_del;

   ELM_WIDGET_CLASS(sc)->theme = _elm_fileselector_button_smart_theme;

   ELM_BUTTON_CLASS(sc)->admits_autorepeat = EINA_FALSE;
}

EAPI const Elm_Fileselector_Button_Smart_Class *
elm_fileselector_button_smart_class_get(void)
{
   static Elm_Fileselector_Button_Smart_Class _sc =
     ELM_FILESELECTOR_BUTTON_SMART_CLASS_INIT_NAME_VERSION
       (ELM_FILESELECTOR_BUTTON_SMART_NAME);
   static const Elm_Fileselector_Button_Smart_Class *class = NULL;
   Evas_Smart_Class *esc = (Evas_Smart_Class *)&_sc;

   if (class) return class;

   _elm_fileselector_button_smart_set(&_sc);
   esc->callbacks = _smart_callbacks;
   class = &_sc;

   return class;
}

EAPI Evas_Object *
elm_fileselector_button_add(Evas_Object *parent)
{
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   obj = elm_widget_add(_elm_fileselector_button_smart_class_new(), parent);
   if (!obj) return NULL;

   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);

   return obj;
}

EAPI void
elm_fileselector_button_window_title_set(Evas_Object *obj,
                                         const char *title)
{
   ELM_FILESELECTOR_BUTTON_CHECK(obj);
   ELM_FILESELECTOR_BUTTON_DATA_GET(obj, sd);

   eina_stringshare_replace(&sd->window_title, title);

   if (sd->fsw) elm_win_title_set(sd->fsw, sd->window_title);
}

EAPI const char *
elm_fileselector_button_window_title_get(const Evas_Object *obj)
{
   ELM_FILESELECTOR_BUTTON_CHECK(obj) NULL;
   ELM_FILESELECTOR_BUTTON_DATA_GET(obj, sd);

   return sd->window_title;
}

EAPI void
elm_fileselector_button_window_size_set(Evas_Object *obj,
                                        Evas_Coord width,
                                        Evas_Coord height)
{
   ELM_FILESELECTOR_BUTTON_CHECK(obj);
   ELM_FILESELECTOR_BUTTON_DATA_GET(obj, sd);

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
   ELM_FILESELECTOR_BUTTON_DATA_GET(obj, sd);

   if (width) *width = sd->w;
   if (height) *height = sd->h;
}

EAPI void
elm_fileselector_button_path_set(Evas_Object *obj,
                                 const char *path)
{
   ELM_FILESELECTOR_BUTTON_CHECK(obj);
   ELM_FILESELECTOR_BUTTON_DATA_GET(obj, sd);

   eina_stringshare_replace(&sd->fsd.path, path);

   if (sd->fs) elm_fileselector_selected_set(sd->fs, sd->fsd.path);
}

EAPI const char *
elm_fileselector_button_path_get(const Evas_Object *obj)
{
   ELM_FILESELECTOR_BUTTON_CHECK(obj) NULL;
   ELM_FILESELECTOR_BUTTON_DATA_GET(obj, sd);

   return sd->fsd.path;
}

EAPI void
elm_fileselector_button_expandable_set(Evas_Object *obj,
                                       Eina_Bool value)
{
   ELM_FILESELECTOR_BUTTON_CHECK(obj);
   ELM_FILESELECTOR_BUTTON_DATA_GET(obj, sd);

   sd->fsd.expandable = value;

   if (sd->fs) elm_fileselector_expandable_set(sd->fs, sd->fsd.expandable);
}

EAPI Eina_Bool
elm_fileselector_button_expandable_get(const Evas_Object *obj)
{
   ELM_FILESELECTOR_BUTTON_CHECK(obj) EINA_FALSE;
   ELM_FILESELECTOR_BUTTON_DATA_GET(obj, sd);

   return sd->fsd.expandable;
}

EAPI void
elm_fileselector_button_folder_only_set(Evas_Object *obj,
                                        Eina_Bool value)
{
   ELM_FILESELECTOR_BUTTON_CHECK(obj);
   ELM_FILESELECTOR_BUTTON_DATA_GET(obj, sd);

   sd->fsd.folder_only = value;

   if (sd->fs) elm_fileselector_folder_only_set(sd->fs, sd->fsd.folder_only);
}

EAPI Eina_Bool
elm_fileselector_button_folder_only_get(const Evas_Object *obj)
{
   ELM_FILESELECTOR_BUTTON_CHECK(obj) EINA_FALSE;
   ELM_FILESELECTOR_BUTTON_DATA_GET(obj, sd);

   return sd->fsd.folder_only;
}

EAPI void
elm_fileselector_button_is_save_set(Evas_Object *obj,
                                    Eina_Bool value)
{
   ELM_FILESELECTOR_BUTTON_CHECK(obj);
   ELM_FILESELECTOR_BUTTON_DATA_GET(obj, sd);

   sd->fsd.is_save = value;

   if (sd->fs) elm_fileselector_is_save_set(sd->fs, sd->fsd.is_save);
}

EAPI Eina_Bool
elm_fileselector_button_is_save_get(const Evas_Object *obj)
{
   ELM_FILESELECTOR_BUTTON_CHECK(obj) EINA_FALSE;
   ELM_FILESELECTOR_BUTTON_DATA_GET(obj, sd);

   return sd->fsd.is_save;
}

EAPI void
elm_fileselector_button_inwin_mode_set(Evas_Object *obj,
                                       Eina_Bool value)
{
   ELM_FILESELECTOR_BUTTON_CHECK(obj);
   ELM_FILESELECTOR_BUTTON_DATA_GET(obj, sd);

   sd->inwin_mode = value;
}

EAPI Eina_Bool
elm_fileselector_button_inwin_mode_get(const Evas_Object *obj)
{
   ELM_FILESELECTOR_BUTTON_CHECK(obj) EINA_FALSE;
   ELM_FILESELECTOR_BUTTON_DATA_GET(obj, sd);

   return sd->inwin_mode;
}
