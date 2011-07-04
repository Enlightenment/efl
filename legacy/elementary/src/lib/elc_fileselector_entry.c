#include <Elementary.h>
#include "elm_priv.h"

 /**
 * @defgroup File_Selector_Entry File Selector Entry
 *
 * An entry that shows to enter/display path and have an associated
 * button to allow selecting the file from a dialog.
 *
 * The button, when clicked, creates an Elementary window (or inner
 * window) with an Elementary File Selector within. When a file is
 * chosen, the (inner) window is closed and the selected file is
 * exposed as an evas_object_smart_callback_call() of the button.
 */

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *edje;
   Evas_Object *button;
   Evas_Object *entry;
};

static const char *widtype = NULL;

static const char SIG_CHANGED[] = "changed";
static const char SIG_ACTIVATED[] = "activated";
static const char SIG_PRESS[] = "press";
static const char SIG_LONGPRESSED[] = "longpressed";
static const char SIG_CLICKED[] = "clicked";
static const char SIG_CLICKED_DOUBLE[] = "clicked,double";
static const char SIG_FOCUSED[] = "focused";
static const char SIG_UNFOCUSED[] = "unfocused";
static const char SIG_SELECTION_PASTE[] = "selection,paste";
static const char SIG_SELECTION_COPY[] = "selection,copy";
static const char SIG_SELECTION_CUT[] = "selection,cut";
static const char SIG_UNPRESSED[] = "unpressed";
static const char SIG_FILE_CHOSEN[] = "file,chosen";
static const Evas_Smart_Cb_Description _signals[] =
{
  {SIG_CHANGED, ""},
  {SIG_ACTIVATED, ""},
  {SIG_PRESS, ""},
  {SIG_LONGPRESSED, ""},
  {SIG_CLICKED, ""},
  {SIG_CLICKED_DOUBLE, ""},
  {SIG_FOCUSED, ""},
  {SIG_UNFOCUSED, ""},
  {SIG_SELECTION_PASTE, ""},
  {SIG_SELECTION_COPY, ""},
  {SIG_SELECTION_CUT, ""},
  {SIG_UNPRESSED, ""},
  {SIG_FILE_CHOSEN, "s"},
  {NULL, NULL}
};

#define SIG_FWD(name)                                                    \
static void                                                              \
_##name##_fwd(void *data, Evas_Object *obj __UNUSED__, void *event_info) \
{                                                                        \
   evas_object_smart_callback_call(data, SIG_##name, event_info);        \
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

static void _mirrored_set(Evas_Object *obj, Eina_Bool rtl);

static void
_FILE_CHOSEN_fwd(void *data, Evas_Object *obj __UNUSED__, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   const char *file = event_info;
   elm_entry_entry_set(wd->entry, file);
   evas_object_smart_callback_call(data, SIG_FILE_CHOSEN, event_info);
}

static void
_ACTIVATED_fwd(void *data, Evas_Object *obj __UNUSED__, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   const char *file = elm_entry_entry_get(wd->entry);
   elm_fileselector_button_path_set(wd->button, file);
   evas_object_smart_callback_call(data, SIG_ACTIVATED, event_info);
}

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   free(wd);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1;
   if (!wd) return;
   edje_object_size_min_calc(wd->edje, &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, -1, -1);
}

static Eina_Bool
_elm_fileselector_entry_focus_next_hook(const Evas_Object *obj, Elm_Focus_Direction dir, Evas_Object **next)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd)
     return EINA_FALSE;

   Evas_Object *chain[2];

   /* Direction */
   if (dir == ELM_FOCUS_PREVIOUS)
     {
        chain[0] = wd->button;
        chain[1] = wd->entry;
     }
   else if (dir == ELM_FOCUS_NEXT)
     {
        chain[0] = wd->entry;
        chain[1] = wd->button;
     }
   else
     return EINA_FALSE;

   unsigned char i = elm_widget_focus_get(chain[1]);

   if (elm_widget_focus_next_get(chain[i], dir, next))
     return EINA_TRUE;

   i = !i;

   Evas_Object *to_focus;
   if (elm_widget_focus_next_get(chain[i], dir, &to_focus))
     {
        *next = to_focus;
        return !!i;
     }

   return EINA_FALSE;
}

static void
_mirrored_set(Evas_Object *obj, Eina_Bool rtl)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_widget_mirrored_set(wd->button, rtl);
   edje_object_mirrored_set(wd->edje, rtl);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   const char *style = elm_widget_style_get(obj);
   char buf[1024];

   if (!wd) return;
   _elm_widget_mirrored_reload(obj);
   _mirrored_set(obj, elm_widget_mirrored_get(obj));

   _elm_theme_object_set(obj, wd->edje, "fileselector_entry", "base", style);
   if (elm_object_disabled_get(obj))
      edje_object_signal_emit(wd->edje, "elm,state,disabled", "elm");

   if (!style) style = "default";
   snprintf(buf, sizeof(buf), "fileselector_entry/%s", style);
   elm_widget_style_set(wd->button, buf);
   elm_widget_style_set(wd->entry, buf);

   edje_object_part_swallow(obj, "elm.swallow.button", wd->button);
   edje_object_part_swallow(obj, "elm.swallow.entry", wd->entry);

   edje_object_message_signal_process(wd->edje);
   edje_object_scale_set
     (wd->edje, elm_widget_scale_get(obj) * _elm_config->scale);
   _sizing_eval(obj);
}

static void
_disable_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Eina_Bool val = elm_widget_disabled_get(obj);
   if (!wd) return;
   if (val)
     edje_object_signal_emit(wd->edje, "elm,state,disabled", "elm");
   else
     edje_object_signal_emit(wd->edje, "elm,state,enabled", "elm");

   elm_widget_disabled_set(wd->button, val);
   elm_widget_disabled_set(wd->entry, val);
}

static void
_changed_size_hints(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _sizing_eval(data);
}

static void
_elm_fileselector_entry_button_label_set(Evas_Object *obj, const char *item, const char *label)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (item) return;
   if (!wd) return;
   elm_object_text_set(wd->button, label);
}

static const char *
_elm_fileselector_entry_button_label_get(const Evas_Object *obj, const char *item)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (item) return NULL;
   if (!wd) return NULL;
   return elm_object_text_get(wd->button);
}

/**
 * Add a new file selector entry into the parent object.
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup File_Selector_Entry
 */
EAPI Evas_Object *
elm_fileselector_entry_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);

   ELM_SET_WIDTYPE(widtype, "fileselector_entry");
   elm_widget_type_set(obj, "fileselector_entry");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_disable_hook_set(obj, _disable_hook);
   elm_widget_focus_next_hook_set(obj, _elm_fileselector_entry_focus_next_hook);
   elm_widget_can_focus_set(obj, EINA_FALSE);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_text_set_hook_set(obj, _elm_fileselector_entry_button_label_set);
   elm_widget_text_get_hook_set(obj, _elm_fileselector_entry_button_label_get);

   wd->edje = edje_object_add(e);
   _elm_theme_object_set(obj, wd->edje, "fileselector_entry", "base", "default");
   elm_widget_resize_object_set(obj, wd->edje);

   wd->button = elm_fileselector_button_add(obj);
   elm_widget_mirrored_automatic_set(wd->button, EINA_FALSE);
   ELM_SET_WIDTYPE(widtype, "fileselector_entry");
   elm_widget_style_set(wd->button, "fileselector_entry/default");
   edje_object_part_swallow(wd->edje, "elm.swallow.button", wd->button);
   elm_widget_sub_object_add(obj, wd->button);
   evas_object_event_callback_add
      (wd->button, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _changed_size_hints, obj);
   elm_fileselector_button_expandable_set(wd->button,
                                          _elm_config->fileselector_expand_enable);

#define SIG_FWD(name)                                                   \
   evas_object_smart_callback_add(wd->button, SIG_##name, _##name##_fwd, obj)
   SIG_FWD(CLICKED);
   SIG_FWD(UNPRESSED);
   SIG_FWD(FILE_CHOSEN);
#undef SIG_FWD

   wd->entry = elm_entry_add(obj);
   elm_entry_scrollable_set(wd->entry, EINA_TRUE);
   elm_widget_mirrored_automatic_set(wd->entry, EINA_FALSE);
   elm_widget_style_set(wd->entry, "fileselector_entry/default");
   elm_entry_single_line_set(wd->entry, EINA_TRUE);
   elm_entry_editable_set(wd->entry, EINA_TRUE);
   edje_object_part_swallow(wd->edje, "elm.swallow.entry", wd->entry);
   elm_widget_sub_object_add(obj, wd->entry);
   evas_object_event_callback_add
      (wd->entry, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _changed_size_hints, obj);

#define SIG_FWD(name)                                                   \
   evas_object_smart_callback_add(wd->entry, SIG_##name, _##name##_fwd, obj)
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

   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   _sizing_eval(obj);

   // TODO: convert Elementary to subclassing of Evas_Smart_Class
   // TODO: and save some bytes, making descriptions per-class and not instance!
   evas_object_smart_callbacks_descriptions_set(obj, _signals);
   return obj;
}

/**
 * Set the label used in the file selector entry.
 *
 * @param obj The entry object
 * @param label The text label text to be displayed on the entry
 *
 * @ingroup File_Selector_Entry
 * @deprecated use elm_object_text_set() instead.
 */
EAPI void
elm_fileselector_entry_button_label_set(Evas_Object *obj, const char *label)
{
   _elm_fileselector_entry_button_label_set(obj, NULL, label);
}

EAPI const char *
elm_fileselector_entry_button_label_get(const Evas_Object *obj)
{
   return _elm_fileselector_entry_button_label_get(obj, NULL);
}

/**
 * Set the path to start the entry's file selector with, when clicked.
 *
 * @param obj The entry object
 * @param path Path to a file/directory
 *
 * Default path is "HOME" environment variable's value.
 *
 * @ingroup File_Selector_Entry
 */
EAPI void
elm_fileselector_entry_selected_set(Evas_Object *obj, const char *path)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_fileselector_button_path_set(wd->button, path);
}

/**
 * Get the <b>last</b> path which the entry's file selector was set to.
 *
 * @param obj The entry object
 * @param path Path to a file/directory
 *
 * Default path is "HOME" environment variable's value.
 *
 * @ingroup File_Selector_Entry
 */
EAPI const char *
elm_fileselector_entry_selected_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return elm_fileselector_button_path_get(wd->button);
}

/**
 * Set the title of the file selector entry's window.
 *
 * @param obj The entry object
 * @param title The title string
 *
 * Note that it will only take any effect if the fileselector entry
 * not at "inwin mode".
 *
 * @ingroup File_Selector_Entry
 */
EAPI void
elm_fileselector_entry_window_title_set(Evas_Object *obj, const char *title)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_fileselector_button_window_title_set(wd->button, title);
}

/**
 * Get the title of the file selector entry's window.
 *
 * @param obj The entry object
 *
 * @ingroup File_Selector_Entry
 */
EAPI const char *
elm_fileselector_entry_window_title_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return elm_fileselector_button_window_title_get(wd->button);
}

/**
 * Set the size of the file selector entry's window.
 *
 * @param obj The entry object
 * @param width The width
 * @param height The height
 *
 * Note that it will only take any effect if the fileselector entry not at
 * "inwin mode". Default size for the window (when applicable) is 400x400.
 *
 * @ingroup File_Selector_Entry
 */
EAPI void
elm_fileselector_entry_window_size_set(Evas_Object *obj, Evas_Coord width, Evas_Coord height)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_fileselector_button_window_size_set(wd->button, width, height);
}

/**
 * Get the size of the file selector entry's window.
 *
 * @param obj The entry object
 * @param width Pointer into which to store the width value
 * @param height Pointer into which to store the height value
 *
 * @ingroup File_Selector_Entry
 */
EAPI void
elm_fileselector_entry_window_size_get(const Evas_Object *obj, Evas_Coord *width, Evas_Coord *height)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_fileselector_button_window_size_get(wd->button, width, height);
}

/**
 * Set the starting path of the file selector entry's window.
 *
 * @param obj The entry object
 * @param path The path string
 *
 * It must be a <b>directory</b> path.
 *
 * @ingroup File_Selector_Entry
 */
EAPI void
elm_fileselector_entry_path_set(Evas_Object *obj, const char *path)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_fileselector_button_path_set(wd->button, path);
   elm_entry_entry_set(wd->entry, path);
}

/**
 * Get the <b>last</b> path of the file selector entry's window.
 *
 * @param obj The entry object
 *
 * @ingroup File_Selector_Entry
 */
EAPI const char *
elm_fileselector_entry_path_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return elm_entry_entry_get(wd->entry);
}

/**
 * Set whether the entry's file selector is to present itself as an
 * Elementary Generic List (which will expand its entries for nested
 * directories) or as canonical list, which will be rendered again
 * with the contents of each selected directory.
 *
 * @param obj The entry object
 * @param value The expandable flag
 *
 * @ingroup File_Selector_Entry
 */
EAPI void
elm_fileselector_entry_expandable_set(Evas_Object *obj, Eina_Bool value)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_fileselector_button_expandable_set(wd->button, value);
}

/**
 * Get the entry's file selector expandable flag.
 *
 * @param obj The entry object
 * @return value The expandable flag
 *
 * @ingroup File_Selector_Entry
 */
EAPI Eina_Bool
elm_fileselector_entry_expandable_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return elm_fileselector_button_expandable_get(wd->button);
}

/**
 * Set whether the entry's file selector list is to display folders
 * only or the directory contents, as well.
 *
 * @param obj The entry object
 * @param value The "folder only" flag
 *
 * @ingroup File_Selector_Entry
 */
EAPI void
elm_fileselector_entry_folder_only_set(Evas_Object *obj, Eina_Bool value)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_fileselector_button_folder_only_set(wd->button, value);
}

/**
 * Get the entry's file selector "folder only" flag.
 *
 * @param obj The entry object
 * @return value The "folder only" flag
 *
 * @ingroup File_Selector_Entry
 */
EAPI Eina_Bool
elm_fileselector_entry_folder_only_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return elm_fileselector_button_folder_only_get(wd->button);
}

/**
 * Set whether the entry's file selector has an editable text entry
 * which will hold its current selection.
 *
 * @param obj The entry object
 * @param value The "is save" flag
 *
 * @ingroup File_Selector_Entry
 */
EAPI void
elm_fileselector_entry_is_save_set(Evas_Object *obj, Eina_Bool value)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_fileselector_button_is_save_set(wd->button, value);
}

/**
 * Get the entry's file selector "is save" flag.
 *
 * @param obj The entry object
 * @return value The "is save" flag
 *
 * @ingroup File_Selector_Entry
 */
EAPI Eina_Bool
elm_fileselector_entry_is_save_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return elm_fileselector_button_is_save_get(wd->button);
}

/**
 * Set whether the entry's file selector will raise an Elementary
 * Inner Window, instead of a dedicated Elementary Window. By default,
 * it won't.
 *
 * @param obj The entry object
 * @param value The "inwin mode" flag
 *
 * @ingroup File_Selector_Entry
 */
EAPI void
elm_fileselector_entry_inwin_mode_set(Evas_Object *obj, Eina_Bool value)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_fileselector_button_inwin_mode_set(wd->button, value);
}

/**
 * Get the entry's file selector "inwin mode" flag.
 *
 * @param obj The entry object
 * @return value The "inwin mode" flag
 *
 * @ingroup File_Selector_Entry
 */
EAPI Eina_Bool
elm_fileselector_entry_inwin_mode_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return elm_fileselector_button_inwin_mode_get(wd->button);
}

/**
 * Set the icon used for the entry button
 *
 * Once the icon object is set, a previously set one will be deleted.
 *
 * @param obj The entry object
 * @param icon  The image for the entry
 *
 * @ingroup File_Selector_Entry
 */
EAPI void
elm_fileselector_entry_button_icon_set(Evas_Object *obj, Evas_Object *icon)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_fileselector_button_icon_set(wd->button, icon);
}

/**
 * Get the icon used for the entry button
 *
 * @param obj The entry object
 * @return The image for the entry
 *
 * @ingroup File_Selector_Entry
 */
EAPI Evas_Object *
elm_fileselector_entry_button_icon_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return elm_fileselector_button_icon_get(wd->button);
}

/**
 * Unset the icon used for the entry button
 *
 * Unparent and return the icon object which was set for this widget.
 *
 * @param obj The entry object
 * @return The icon object that was being used
 *
 * @ingroup File_Selector_Entry
 */
EAPI Evas_Object *
elm_fileselector_entry_button_icon_unset(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return elm_fileselector_button_icon_unset(wd->button);
}
