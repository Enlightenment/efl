#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *self, *btn, *fs, *fsw;
   const char  *window_title;
   Evas_Coord   w, h;
   struct
   {
      const char *path;
      Eina_Bool   expandable : 1;
      Eina_Bool   folder_only : 1;
      Eina_Bool   is_save : 1;
   } fsd;
   Eina_Bool inwin_mode : 1;
};

#define DEFAULT_WINDOW_TITLE "Select a file"

static const char *widtype = NULL;

static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _disable_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _changed_size_hints(void        *data,
                                Evas        *e,
                                Evas_Object *obj,
                                void        *event_info);
static void _on_focus_hook(void        *data,
                           Evas_Object *obj);
static void _selection_done(void        *data,
                            Evas_Object *obj,
                            void        *event_info);
static void _activate(Widget_Data *wd);

static const char SIG_FILE_CHOSEN[] = "file,chosen";
static const Evas_Smart_Cb_Description _signals[] = {
       {SIG_FILE_CHOSEN, "s"},
       {NULL, NULL}
};

static void
_del_hook(Evas_Object *obj)
{
   Evas_Object *win;
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd) return;

   if (wd->window_title) eina_stringshare_del(wd->window_title);
   if (wd->fsd.path) eina_stringshare_del(wd->fsd.path);
   if (wd->fs)
     {
        win = evas_object_data_del(obj, "win");
        evas_object_del(win);
     }
   free(wd);
}

static void
_on_focus_hook(void *data   __UNUSED__,
               Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (elm_widget_focus_get(obj))
     elm_widget_focus_steal(wd->btn);
}

static void
_mirrored_set(Evas_Object *obj, Eina_Bool rtl)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_widget_mirrored_set(wd->btn, rtl);
   elm_widget_mirrored_set(wd->fs, rtl);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   char buf[4096];
   if (!wd) return;
   _elm_widget_mirrored_reload(obj);
   _mirrored_set(obj, elm_widget_mirrored_get(obj));

   snprintf(buf, sizeof(buf), "fileselector_button/%s",
            elm_widget_style_get(obj));
   elm_object_style_set(wd->btn, buf);
   _sizing_eval(obj);
}

static void
_disable_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_widget_disabled_set(wd->btn, elm_widget_disabled_get(obj));
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   if (!wd) return;
   evas_object_size_hint_min_get(wd->btn, &minw, &minh);
   evas_object_size_hint_max_get(wd->btn, &maxw, &maxh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_changed_size_hints(void            *data,
                    Evas *e          __UNUSED__,
                    Evas_Object *obj __UNUSED__,
                    void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   _sizing_eval(data);
}

static void
_activate_hook(Evas_Object *obj)
{
   Widget_Data *wd;
   wd = elm_widget_data_get(obj);
   if (!wd) return;
   _activate(wd);
}

static void
_button_clicked(void            *data,
                Evas_Object *obj __UNUSED__,
                void *event_info __UNUSED__)
{
   _activate(data);
}

static Evas_Object *
_parent_win_get(Evas_Object *obj)
{
   while ((obj) && (strcmp(elm_widget_type_get(obj), "win")))
     obj = elm_object_parent_widget_get(obj);

   return obj;
}

static Evas_Object *
_new_window_add(Widget_Data *wd)
{
   Evas_Object *win, *bg;

   win = elm_win_add(NULL, "fileselector_button", ELM_WIN_DIALOG_BASIC);
   elm_win_title_set(win, wd->window_title);
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   evas_object_resize(win, wd->w, wd->h);
   return win;
}

static void
_activate(Widget_Data *wd)
{
   Eina_Bool is_inwin = EINA_FALSE;

   if (wd->fs) return;

   if (wd->inwin_mode)
     {
        wd->fsw = _parent_win_get(wd->self);

        if (!wd->fsw)
          wd->fsw = _new_window_add(wd);
        else
          {
             wd->fsw = elm_win_inwin_add(wd->fsw);
             is_inwin = EINA_TRUE;
          }
     }
   else
     wd->fsw = _new_window_add(wd);

   wd->fs = elm_fileselector_add(wd->fsw);
   elm_widget_mirrored_set(wd->fs, elm_widget_mirrored_get(wd->self));
   elm_widget_mirrored_automatic_set(wd->fs, EINA_FALSE);
   elm_fileselector_expandable_set(wd->fs, wd->fsd.expandable);
   elm_fileselector_folder_only_set(wd->fs, wd->fsd.folder_only);
   elm_fileselector_is_save_set(wd->fs, wd->fsd.is_save);
   elm_fileselector_selected_set(wd->fs, wd->fsd.path);
   evas_object_size_hint_weight_set(wd->fs, EVAS_HINT_EXPAND,
                                    EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(wd->fs, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(wd->fs, "done", _selection_done, wd);
   evas_object_show(wd->fs);

   if (is_inwin)
     {
        elm_win_inwin_content_set(wd->fsw, wd->fs);
        elm_win_inwin_activate(wd->fsw);
     }
   else
     {
        elm_win_resize_object_add(wd->fsw, wd->fs);
        evas_object_show(wd->fsw);
     }
}

static void
_selection_done(void            *data,
                Evas_Object *obj __UNUSED__,
                void            *event_info)
{
   const char *file = event_info;
   Widget_Data *wd = data;
   Evas_Object *del;
   if (!wd) return;

   if (file) eina_stringshare_replace(&wd->fsd.path, file);

   del = wd->fsw;
   wd->fs = NULL;
   wd->fsw = NULL;
   evas_object_del(del);

   evas_object_smart_callback_call(wd->self, SIG_FILE_CHOSEN,
                                   (void *)wd->fsd.path);
}

static void
_elm_fileselector_button_label_set(Evas_Object *obj, const char *item,
                                  const char  *label)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   if (item && strcmp(item, "default")) return;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_object_text_set(wd->btn, label);
}

static const char *
_elm_fileselector_button_label_get(const Evas_Object *obj, const char *item)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (item && strcmp(item, "default")) return NULL;
   if (!wd) return NULL;
   return elm_object_text_get(wd->btn);
}

EAPI Evas_Object *
elm_fileselector_button_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);

   ELM_SET_WIDTYPE(widtype, "fileselector_button");
   elm_widget_type_set(obj, "fileselector_button");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_on_focus_hook_set(obj, _on_focus_hook, NULL);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_disable_hook_set(obj, _disable_hook);
   elm_widget_can_focus_set(obj, EINA_TRUE);
   elm_widget_activate_hook_set(obj, _activate_hook);
   elm_widget_text_set_hook_set(obj, _elm_fileselector_button_label_set);
   elm_widget_text_get_hook_set(obj, _elm_fileselector_button_label_get);

   wd->self = obj;
   wd->window_title = eina_stringshare_add(DEFAULT_WINDOW_TITLE);
   if (getenv("HOME")) wd->fsd.path = eina_stringshare_add(getenv("HOME"));
   else wd->fsd.path = eina_stringshare_add("/");
   wd->fsd.expandable = _elm_config->fileselector_expand_enable;
   wd->inwin_mode = _elm_config->inwin_dialogs_enable;
   wd->w = 400;
   wd->h = 400;

   wd->btn = elm_button_add(parent);
   elm_widget_mirrored_automatic_set(wd->btn, EINA_FALSE);
   elm_widget_resize_object_set(obj, wd->btn);
   evas_object_event_callback_add(wd->btn, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                  _changed_size_hints, obj);
   evas_object_smart_callback_add(wd->btn, "clicked", _button_clicked, wd);
   elm_widget_sub_object_add(obj, wd->btn);

   _theme_hook(obj);
   evas_object_smart_callbacks_descriptions_set(obj, _signals);
   return obj;
}

EAPI void
elm_fileselector_button_label_set(Evas_Object *obj,
                                  const char  *label)
{
   _elm_fileselector_button_label_set(obj, NULL, label);
}

EAPI const char *
elm_fileselector_button_label_get(const Evas_Object *obj)
{
   return _elm_fileselector_button_label_get(obj, NULL);
}

EAPI void
elm_fileselector_button_window_title_set(Evas_Object *obj,
                                         const char  *title)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   eina_stringshare_replace(&wd->window_title, title);

   if (wd->fsw)
     elm_win_title_set(wd->fsw, wd->window_title);
}

EAPI const char *
elm_fileselector_button_window_title_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return NULL;
   return wd->window_title;
}

EAPI void
elm_fileselector_button_window_size_set(Evas_Object *obj,
                                        Evas_Coord   width,
                                        Evas_Coord   height)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   wd->w = width;
   wd->h = height;

   if (wd->fsw)
     evas_object_resize(wd->fsw, wd->w, wd->h);
}

EAPI void
elm_fileselector_button_window_size_get(const Evas_Object *obj,
                                        Evas_Coord        *width,
                                        Evas_Coord        *height)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   if (width) *width = wd->w;
   if (height) *height = wd->h;
}

EAPI void
elm_fileselector_button_path_set(Evas_Object *obj,
                                 const char  *path)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   eina_stringshare_replace(&wd->fsd.path, path);

   if (wd->fs)
     elm_fileselector_selected_set(wd->fs, wd->fsd.path);
}

EAPI const char *
elm_fileselector_button_path_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->fsd.path;
}

EAPI void
elm_fileselector_button_expandable_set(Evas_Object *obj,
                                       Eina_Bool    value)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   wd->fsd.expandable = value;

   if (wd->fs)
     elm_fileselector_expandable_set(wd->fs, wd->fsd.expandable);
}

EAPI Eina_Bool
elm_fileselector_button_expandable_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return EINA_FALSE;
   return wd->fsd.expandable;
}

EAPI void
elm_fileselector_button_folder_only_set(Evas_Object *obj,
                                        Eina_Bool    value)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   wd->fsd.folder_only = value;

   if (wd->fs)
     elm_fileselector_folder_only_set(wd->fs, wd->fsd.folder_only);
}

EAPI Eina_Bool
elm_fileselector_button_folder_only_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return EINA_FALSE;
   return wd->fsd.folder_only;
}

EAPI void
elm_fileselector_button_is_save_set(Evas_Object *obj,
                                    Eina_Bool    value)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   wd->fsd.is_save = value;

   if (wd->fs)
     elm_fileselector_is_save_set(wd->fs, wd->fsd.is_save);
}

EAPI Eina_Bool
elm_fileselector_button_is_save_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return EINA_FALSE;
   return wd->fsd.is_save;
}

EAPI void
elm_fileselector_button_inwin_mode_set(Evas_Object *obj,
                                       Eina_Bool    value)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   wd->inwin_mode = value;
}

EAPI Eina_Bool
elm_fileselector_button_inwin_mode_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return EINA_FALSE;
   return wd->inwin_mode;
}

EAPI void
elm_fileselector_button_icon_set(Evas_Object *obj,
                                 Evas_Object *icon)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd)
     {
        evas_object_del(icon);
        return;
     }
   elm_object_content_set(wd->btn, icon);
}

EAPI Evas_Object *
elm_fileselector_button_icon_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return elm_object_content_get(wd->btn);
}

EAPI Evas_Object *
elm_fileselector_button_icon_unset(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return elm_object_content_unset(wd->btn);
}

