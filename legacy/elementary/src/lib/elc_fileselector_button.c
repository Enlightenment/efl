#include <Elementary.h>
#include "elm_priv.h"

 /**
 * @defgroup File_Selector_Button File Selector Button
 *
 * A button that, when clicked, creates an Elementary window (or inner
 * window) with an Elementary File Selector within. When a file is
 * chosen, the (inner) window is closed and the selected file is
 * exposed as an evas_object_smart_callback_call() of the button.
 */

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *btn, *icon, *fs;
   const char *window_title;
   const char *btn_label;
   Evas_Coord w, h;
   struct {
      const char *path;
      Eina_Bool expandable:1;
      Eina_Bool folder_only:1;
      Eina_Bool is_save:1;
   } fsd;
   Eina_Bool inwin_mode:1;
};

static const char DEFAULT_WINDOW_TITLE[] = "Select a file";

static const char *widtype = NULL;
static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _disable_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _sub_del(void *data, Evas_Object *obj, void *event_info);
static void _signal_clicked(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _signal_pressed(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _signal_unpressed(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _on_focus_hook(void *data, Evas_Object *obj);
static void _selection_done(void *data, Evas_Object *obj, void *event_info);

static const char SIG_CLICKED[] = "clicked";
static const char SIG_UNPRESSED[] = "unpressed";
static const char SIG_FILE_CHOSEN[] = "file,chosen";
static const Evas_Smart_Cb_Description _signals[] = 
{
   {SIG_CLICKED, ""},
   {SIG_UNPRESSED, ""},
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

   if (wd->btn_label) eina_stringshare_del(wd->btn_label);
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
_on_focus_hook(void *data __UNUSED__, Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (elm_widget_focus_get(obj))
     {
	edje_object_signal_emit(wd->btn, "elm,action,focus", "elm");
	evas_object_focus_set(wd->btn, 1);
     }
   else
     {
	edje_object_signal_emit(wd->btn, "elm,action,unfocus", "elm");
	evas_object_focus_set(wd->btn, 0);
     }
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   _elm_theme_object_set(obj, wd->btn, "button", "base", elm_widget_style_get(obj));
   if (wd->icon)
     edje_object_part_swallow(wd->btn, "elm.swallow.content", wd->icon);
   if (wd->btn_label)
     edje_object_signal_emit(wd->btn, "elm,state,text,visible", "elm");
   else
     edje_object_signal_emit(wd->btn, "elm,state,text,hidden", "elm");
   if (wd->icon)
     edje_object_signal_emit(wd->btn, "elm,state,icon,visible", "elm");
   else
     edje_object_signal_emit(wd->btn, "elm,state,icon,hidden", "elm");
   edje_object_part_text_set(wd->btn, "elm.text", wd->btn_label);
   edje_object_message_signal_process(wd->btn);
   edje_object_scale_set(wd->btn, 
			 elm_widget_scale_get(obj) * _elm_config->scale);
   _sizing_eval(obj);
}

static void
_disable_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (elm_widget_disabled_get(obj))
     edje_object_signal_emit(wd->btn, "elm,state,disabled", "elm");
   else
     edje_object_signal_emit(wd->btn, "elm,state,enabled", "elm");
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;

   if (!wd) return;
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   edje_object_size_min_restricted_calc(wd->btn, &minw, &minh, minw, minh);
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_changed_size_hints(void *data, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   if (obj != wd->icon) return;
   _sizing_eval(data);
}

static void
_sub_del(void *data __UNUSED__, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *sub = event_info;
   if (!wd) return;
   if (sub == wd->icon)
     {
	edje_object_signal_emit(wd->btn, "elm,state,icon,hidden", "elm");
	evas_object_event_callback_del_full(sub,
					    EVAS_CALLBACK_CHANGED_SIZE_HINTS,
					    _changed_size_hints, obj);
	wd->icon = NULL;
	edje_object_message_signal_process(wd->btn);
	_sizing_eval(obj);
     }
}

static void
_signal_clicked(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;

   evas_object_smart_callback_call(data, SIG_CLICKED, NULL);

   /* safe guard when the theme does not emit the 'unpress' signal */
   _signal_unpressed(data, obj, emission, source);
}

static Evas_Object *
_parent_win_get(Evas_Object *obj)
{
   Evas_Object *ret;

   ret = elm_object_parent_widget_get(obj);
   if (!ret) return NULL;

   if (strcmp(elm_widget_type_get(ret), "win") != 0)
     {
	DBG("Widget type (%s) is not \"win\", going up\n",
	    elm_widget_type_get(ret));
	return _parent_win_get(ret);
     }

   return ret;
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
_fs_launch(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Evas_Object *fs_btn, *win, *iw;
   Eina_Bool win_fallback;
   Widget_Data *wd;

   fs_btn = data;
   wd = elm_widget_data_get(fs_btn);

   if (!wd) return;
   if (wd->fs) return;

   win_fallback = EINA_FALSE;
   if (wd->inwin_mode)
     {
	win = _parent_win_get(fs_btn);

	if (!win)
	  {
	     ERR("No elementary window found as parent of the file selector "
		 "button! Launching the file selector inside a new elementary"
		 " window, then.");
	     win_fallback = EINA_TRUE;
	  }
	else
	  {
	     iw = elm_win_inwin_add(win);
	     evas_object_data_set(fs_btn, "win", iw);
	  }
     }

   if ((!wd->inwin_mode) || (win_fallback))
     {
	win = _new_window_add(wd);
	evas_object_data_set(fs_btn, "win", win);
     }

   wd->fs = elm_fileselector_add(win);
   elm_fileselector_expandable_set(wd->fs, wd->fsd.expandable);
   elm_fileselector_folder_only_set(wd->fs, wd->fsd.folder_only);
   elm_fileselector_is_save_set(wd->fs, wd->fsd.is_save);
   elm_fileselector_selected_set(wd->fs, wd->fsd.path);
   evas_object_size_hint_weight_set(wd->fs, EVAS_HINT_EXPAND,
				    EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(wd->fs, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(wd->fs, "done", _selection_done, fs_btn);
   evas_object_show(wd->fs);

   if ((wd->inwin_mode) && (!win_fallback))
     {
	elm_win_inwin_content_set(iw, wd->fs);
	elm_win_inwin_activate(iw);
     }
   else
     {
	elm_win_resize_object_add(win, wd->fs);
	evas_object_show(win);
     }
}

static void
_selection_done(void *data, Evas_Object *obj __UNUSED__, void *event_info)
{
   Evas_Object *fs_btn, *win;
   const char *file;
   Widget_Data *wd;

   file = event_info;
   fs_btn = data;

   wd = elm_widget_data_get(fs_btn);
   if (!wd) return;

   win = evas_object_data_del(fs_btn, "win");

   evas_object_smart_callback_call(fs_btn, SIG_FILE_CHOSEN, event_info);
   if (file) eina_stringshare_replace(&wd->fsd.path, file);

   wd->fs = NULL;
   evas_object_del(win);
}

static void
_signal_pressed(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
}

static void
_signal_unpressed(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   evas_object_smart_callback_call(data, SIG_UNPRESSED, NULL);
}

/**
 * Add a new file selector button into the parent object.
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup File_Selector_Button
 */
EAPI Evas_Object *
elm_fileselector_button_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   wd = ELM_NEW(Widget_Data);
   wd->window_title = eina_stringshare_add(DEFAULT_WINDOW_TITLE);
   wd->fsd.path = eina_stringshare_add(getenv("HOME"));
   wd->w = 400;
   wd->h = 400;

   e = evas_object_evas_get(parent);
   obj = elm_widget_add(e);
   ELM_SET_WIDTYPE(widtype, "fileselector_button");
   elm_widget_type_set(obj, "fileselector_button");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_on_focus_hook_set(obj, _on_focus_hook, NULL);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_disable_hook_set(obj, _disable_hook);
   elm_widget_can_focus_set(obj, 1);

   wd->btn = edje_object_add(e);
   _elm_theme_object_set(obj, wd->btn, "button", "base", "default");
   edje_object_signal_callback_add(wd->btn, "elm,action,click", "",
                                   _signal_clicked, obj);
   edje_object_signal_callback_add(wd->btn, "elm,action,click", "",
                                   _fs_launch, obj);
   edje_object_signal_callback_add(wd->btn, "elm,action,press", "",
                                   _signal_pressed, obj);
   edje_object_signal_callback_add(wd->btn, "elm,action,unpress", "",
                                   _signal_unpressed, obj);
   elm_widget_resize_object_set(obj, wd->btn);

   evas_object_smart_callback_add(obj, "sub-object-del", _sub_del, obj);

   _sizing_eval(obj);

   // TODO: convert Elementary to subclassing of Evas_Smart_Class
   // TODO: and save some bytes, making descriptions per-class and not instance!
   evas_object_smart_callbacks_descriptions_set(obj, _signals);
   return obj;
}

/**
 * Set the label used in the file selector button.
 *
 * @param obj The button object
 * @param label The text label text to be displayed on the button
 *
 * @ingroup File_Selector_Button
 */
EAPI void
elm_fileselector_button_label_set(Evas_Object *obj, const char *label)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   eina_stringshare_replace(&wd->btn_label, label);
   if (label)
     edje_object_signal_emit(wd->btn, "elm,state,text,visible", "elm");
   else
     edje_object_signal_emit(wd->btn, "elm,state,text,hidden", "elm");
   edje_object_message_signal_process(wd->btn);
   edje_object_part_text_set(wd->btn, "elm.text", label);
   _sizing_eval(obj);
}

EAPI const char *
elm_fileselector_button_label_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->btn_label;
}

/**
 * Set the path to start the button's file selector with, when clicked.
 *
 * @param obj The button object
 * @param path Path to a file/directory
 *
 * Default path is "HOME" environment variable's value.
 *
 * @ingroup File_Selector_Button
 */
EAPI void
elm_fileselector_button_selected_set(Evas_Object *obj, const char *path)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   eina_stringshare_replace(&wd->fsd.path, path);
}

/**
 * Get the <b>last</b> path which the button's file selector was set to.
 *
 * @param obj The button object
 * @param path Path to a file/directory
 *
 * Default path is "HOME" environment variable's value.
 *
 * @ingroup File_Selector_Button
 */
EAPI const char *
elm_fileselector_button_selected_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->fsd.path;
}

/**
 * Set the title of the file selector button's window.
 *
 * @param obj The button object
 * @param title The title string
 *
 * Note that it will only take any effect if the fileselector button
 * not at "inwin mode".
 *
 * @ingroup File_Selector_Button
 */
EAPI void
elm_fileselector_button_window_title_set(Evas_Object *obj, const char *title)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   eina_stringshare_replace(&wd->window_title, title);
}

/**
 * Get the title of the file selector button's window.
 *
 * @param obj The button object
 *
 * @ingroup File_Selector_Button
 */
EAPI const char *
elm_fileselector_button_window_title_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->window_title;
}

/**
 * Set the size of the file selector button's window.
 *
 * @param obj The button object
 * @param width The width
 * @param height The height
 *
 * Note that it will only take any effect if the fileselector button not at
 * "inwin mode". Default size for the window (when applicable) is 400x400.
 *
 * @ingroup File_Selector_Button
 */
EAPI void
elm_fileselector_button_window_size_set(Evas_Object *obj, Evas_Coord width, Evas_Coord height)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->w = width;
   wd->h = height;
}

/**
 * Get the size of the file selector button's window.
 *
 * @param obj The button object
 * @param width Pointer into which to store the width value
 * @param height Pointer into which to store the height value
 *
 * @ingroup File_Selector_Button
 */
EAPI void
elm_fileselector_button_window_size_get(const Evas_Object *obj, Evas_Coord *width, Evas_Coord *height)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (width) *width = wd->w;
   if (height) *height = wd->h;
}

/**
 * Set the starting path of the file selector button's window.
 *
 * @param obj The button object
 * @param path The path string
 *
 * It must be a <b>directory</b> path.
 *
 * @ingroup File_Selector_Button
 */
EAPI void
elm_fileselector_button_path_set(Evas_Object *obj, const char *path)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   eina_stringshare_replace(&wd->fsd.path, path);
}

/**
 * Get the <b>last</b> path of the file selector button's window.
 *
 * @param obj The button object
 *
 * @ingroup File_Selector_Button
 */
EAPI const char *
elm_fileselector_button_path_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->fsd.path;
}

/**
 * Set whether the button's file selector is to present itself as an
 * Elementary Generic List (which will expand its entries for nested
 * directories) or as canonical list, which will be rendered again
 * with the contents of each selected directory.
 *
 * @param obj The button object
 * @param value The expandable flag
 *
 * @ingroup File_Selector_Button
 */
EAPI void
elm_fileselector_button_expandable_set(Evas_Object *obj, Eina_Bool value)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->fsd.expandable = value;
}

/**
 * Get the button's file selector expandable flag.
 *
 * @param obj The button object
 * @return value The expandable flag
 *
 * @ingroup File_Selector_Button
 */
EAPI Eina_Bool
elm_fileselector_button_expandable_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->fsd.expandable;
}

/**
 * Set whether the button's file selector list is to display folders
 * only or the directory contents, as well.
 *
 * @param obj The button object
 * @param value The "folder only" flag
 *
 * @ingroup File_Selector_Button
 */
EAPI void
elm_fileselector_button_folder_only_set(Evas_Object *obj, Eina_Bool value)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->fsd.folder_only = value;
}

/**
 * Get the button's file selector "folder only" flag.
 *
 * @param obj The button object
 * @return value The "folder only" flag
 *
 * @ingroup File_Selector_Button
 */
EAPI Eina_Bool
elm_fileselector_button_folder_only_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->fsd.folder_only;
}

/**
 * Set whether the button's file selector has an editable text entry
 * which will hold its current selection.
 *
 * @param obj The button object
 * @param value The "is save" flag
 *
 * @ingroup File_Selector_Button
 */
EAPI void
elm_fileselector_button_is_save_set(Evas_Object *obj, Eina_Bool value)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->fsd.is_save = value;
}

/**
 * Get the button's file selector "is save" flag.
 *
 * @param obj The button object
 * @return value The "is save" flag
 *
 * @ingroup File_Selector_Button
 */
EAPI Eina_Bool
elm_fileselector_button_is_save_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->fsd.is_save;
}

/**
 * Set whether the button's file selector will raise an Elementary
 * Inner Window, instead of a dedicated Elementary Window. By default,
 * it won't.
 *
 * @param obj The button object
 * @param value The "inwin mode" flag
 *
 * @ingroup File_Selector_Button
 */
EAPI void
elm_fileselector_button_inwin_mode_set(Evas_Object *obj, Eina_Bool value)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->inwin_mode = value;
}

/**
 * Get the button's file selector "inwin mode" flag.
 *
 * @param obj The button object
 * @return value The "inwin mode" flag
 *
 * @ingroup File_Selector_Button
 */
EAPI Eina_Bool
elm_fileselector_button_inwin_mode_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->inwin_mode;
}

/**
 * Set the icon used for the button
 *
 * @param obj The button object
 * @param icon  The image for the button
 *
 * @ingroup File_Selector_Button
 */
EAPI void
elm_fileselector_button_icon_set(Evas_Object *obj, Evas_Object *icon)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if ((wd->icon != icon) && (wd->icon))
     elm_widget_sub_object_del(obj, wd->icon);
   if ((icon) && (wd->icon != icon))
     {
	wd->icon = icon;
	elm_widget_sub_object_add(obj, icon);
	evas_object_event_callback_add(icon, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
				       _changed_size_hints, obj);
	edje_object_part_swallow(wd->btn, "elm.swallow.content", icon);
	edje_object_signal_emit(wd->btn, "elm,state,icon,visible", "elm");
	edje_object_message_signal_process(wd->btn);
	_sizing_eval(obj);
     }
   else
     wd->icon = icon;
}

/**
 * Get the icon used for the button
 *
 * @param obj The button object
 * @return The image for the button
 *
 * @ingroup File_Selector_Button
 */
EAPI Evas_Object *
elm_fileselector_button_icon_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->icon;
}
