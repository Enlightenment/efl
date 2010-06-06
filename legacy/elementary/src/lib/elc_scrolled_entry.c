#include <Elementary.h>
#include "elm_priv.h"

/**
 * @defgroup Scrolled_Entry Scrolled_Entry
 *
 * A scrolled entry is a convenience widget which shows
 * a box that the user can enter text into.  Unlike an
 * @ref Entry widget, scrolled entries scroll with user
 * input so that the window will not expand if the length
 * of text inside the entry exceeds the initial size of the
 * widget.
 *
 * Signals that you can add callbacks for are:
 * - "changed" - The text within the entry was changed
 * - "activated" - The entry has received focus and the cursor
 * - "press" - The entry has been clicked
 * - "longpressed" - The entry has been clicked for a couple seconds
 * - "clicked" - The entry has been clicked
 * - "clicked,double" - The entry has been double clicked
 * - "focused" - The entry has received focus
 * - "unfocused" - The entry has lost focus
 * - "selection,paste" - A paste action has occurred
 * - "selection,copy" - A copy action has occurred
 * - "selection,cut" - A cut action has occurred
 * - "selection,start" - A selection has begun
 * - "selection,changed" - The selection has changed
 * - "selection,cleared" - The selection has been cleared
 * - "cursor,changed" - The cursor has changed
 * - "anchor,clicked" - The anchor has been clicked
 */

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *scroller;
   Evas_Object *entry;
   Elm_Scroller_Policy policy_h, policy_v;
   Eina_Bool single_line : 1;
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
static const char SIG_SELECTION_START[] = "selection,start";
static const char SIG_SELECTION_CHANGED[] = "selection,changed";
static const char SIG_SELECTION_CLEARED[] = "selection,cleared";
static const char SIG_CURSOR_CHANGED[] = "cursor,changed";
static const char SIG_ANCHOR_CLICKED[] = "anchor,clicked";
static const Evas_Smart_Cb_Description _signals[] = {
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
  {SIG_SELECTION_START, ""},
  {SIG_SELECTION_CHANGED, ""},
  {SIG_SELECTION_CLEARED, ""},
  {SIG_CURSOR_CHANGED, ""},
  {SIG_ANCHOR_CLICKED, ""},
  {NULL, NULL}
};

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   free(wd);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_object_style_set(wd->entry, elm_widget_style_get(obj));
   elm_object_style_set(wd->scroller, elm_widget_style_get(obj));
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw, minh;
   if (!wd) return;
   evas_object_size_hint_min_get(wd->scroller, &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   if (wd->single_line)
     evas_object_size_hint_max_set(obj, -1, minh);
   else
     evas_object_size_hint_max_set(obj, -1, -1);
}

static void
_on_focus_hook(void *data __UNUSED__, Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (elm_widget_focus_get(obj))
     elm_widget_focus_steal(wd->entry);
}

static void
_disable_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_object_disabled_set(wd->entry, elm_widget_disabled_get(obj));
}

static void
_entry_changed(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _sizing_eval(data);
   evas_object_smart_callback_call(data, SIG_CHANGED, NULL);
}

static void
_entry_activated(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_ACTIVATED, NULL);
}

static void
_entry_press(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_PRESS, NULL);
}

static void
_entry_clicked(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_CLICKED, NULL);
}

static void
_entry_clicked_double(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_CLICKED_DOUBLE, NULL);
}

static void
_entry_cursor_changed(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_CURSOR_CHANGED, NULL);
}

static void
_entry_anchor_clicked(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_ANCHOR_CLICKED, NULL);
}

static void
_entry_selection_start(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_SELECTION_START, NULL);
}

static void
_entry_selection_changed(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_SELECTION_CHANGED, NULL);
}

static void
_entry_selection_cleared(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_SELECTION_CLEARED, NULL);
}

static void
_entry_selection_paste(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_SELECTION_PASTE, NULL);
}

static void
_entry_selection_copy(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_SELECTION_COPY, NULL);
}

static void
_entry_selection_cut(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_SELECTION_CUT, NULL);
}

static void
_entry_longpressed(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_LONGPRESSED, NULL);
}

static void
_entry_focused(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_FOCUSED, NULL);
}

static void
_entry_unfocused(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_UNFOCUSED, NULL);
}


/**
 * This adds a scrolled entry to @p parent object.
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Scrolled_Entry
 */
EAPI Evas_Object *
elm_scrolled_entry_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   obj = elm_widget_add(e);
   ELM_SET_WIDTYPE(widtype, "scrolled_entry");
   elm_widget_type_set(obj, "scrolled_entry");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_on_focus_hook_set(obj, _on_focus_hook, NULL);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_disable_hook_set(obj, _disable_hook);
   elm_widget_can_focus_set(obj, 1);
   elm_widget_theme_hook_set(obj, _theme_hook);

   wd->scroller = elm_scroller_add(parent);
   elm_widget_resize_object_set(obj, wd->scroller);
   elm_scroller_bounce_set(wd->scroller, 0, 0);
   
   wd->entry = elm_entry_add(parent);
   evas_object_size_hint_weight_set(wd->entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(wd->entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_scroller_content_set(wd->scroller, wd->entry);
   evas_object_show(wd->entry);

   evas_object_smart_callback_add(wd->entry, "changed", _entry_changed, obj);
   evas_object_smart_callback_add(wd->entry, "activated", _entry_activated, obj);
   evas_object_smart_callback_add(wd->entry, "press", _entry_press, obj);
   evas_object_smart_callback_add(wd->entry, "clicked", _entry_clicked, obj);
   evas_object_smart_callback_add(wd->entry, "clicked,double", _entry_clicked_double, obj);
   evas_object_smart_callback_add(wd->entry, "cursor,changed", _entry_cursor_changed, obj);
   evas_object_smart_callback_add(wd->entry, "anchor,clicked", _entry_anchor_clicked, obj);
   evas_object_smart_callback_add(wd->entry, "selection,start", _entry_selection_start, obj);
   evas_object_smart_callback_add(wd->entry, "selection,changed", _entry_selection_changed, obj);
   evas_object_smart_callback_add(wd->entry, "selection,cleared", _entry_selection_cleared, obj);
   evas_object_smart_callback_add(wd->entry, "selection,paste", _entry_selection_paste, obj);
   evas_object_smart_callback_add(wd->entry, "selection,copy", _entry_selection_copy, obj);
   evas_object_smart_callback_add(wd->entry, "selection,cut", _entry_selection_cut, obj);
   evas_object_smart_callback_add(wd->entry, "longpressed", _entry_longpressed, obj);
   evas_object_smart_callback_add(wd->entry, "focused", _entry_focused, obj);
   evas_object_smart_callback_add(wd->entry, "unfocused", _entry_unfocused, obj);

   _sizing_eval(obj);

   // TODO: convert Elementary to subclassing of Evas_Smart_Class
   // TODO: and save some bytes, making descriptions per-class and not instance!
   evas_object_smart_callbacks_descriptions_set(obj, _signals);
   return obj;
}

/**
 * This sets the scrolled entry object not to line wrap.  All input will
 * be on a single line, and the entry box will scroll with user input.
 *
 * @param obj The scrolled entry object
 * @param single_line If true, the text in the scrolled entry
 * will be on a single line.
 *
 * @ingroup Scrolled_Entry
 */
EAPI void
elm_scrolled_entry_single_line_set(Evas_Object *obj, Eina_Bool single_line)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->single_line == single_line) return;
   elm_entry_single_line_set(wd->entry, single_line);
   wd->single_line = single_line;
   if (single_line)
     {
	elm_scroller_policy_set(wd->scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
	elm_scroller_content_min_limit(wd->scroller, 0, 1);
     }
   else
     {
	elm_scroller_policy_set(wd->scroller, wd->policy_h, wd->policy_v);
	elm_scroller_content_min_limit(wd->scroller, 0, 0);
     }
   _sizing_eval(obj);
}

/**
 * This returns true if the scrolled entry has been set to single line mode.
 * See also elm_scrolled_entry_single_line_set().
 *
 * @param obj The scrolled entry object
 * @return single_line If true, the text in the scrolled entry is set to display
 * on a single line.
 *
 * @ingroup Scrolled_Entry
 */
EAPI Eina_Bool
elm_scrolled_entry_single_line_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return elm_entry_single_line_get(wd->entry);
}


/**
 * This sets the scrolled entry object to password mode.  All text entered
 * and/or displayed within the widget will be replaced with asterisks (*).
 *
 * @param obj The scrolled entry object
 * @param password If true, password mode is enabled.
 *
 * @ingroup Scrolled_Entry
 */
EAPI void
elm_scrolled_entry_password_set(Evas_Object *obj, Eina_Bool password)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_password_set(wd->entry, password);
}

/**
 * This returns whether password mode is enabled.
 * See also elm_scrolled_entry_password_set().
 *
 * @param obj The scrolled entry object
 * @return If true, the scrolled entry is set to display all characters
 * as asterisks (*).
 *
 * @ingroup Scrolled_Entry
 */
EAPI Eina_Bool
elm_scrolled_entry_password_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return elm_entry_password_get(wd->entry);
}


/**
 * This sets the text displayed within the scrolled entry to @p entry.
 *
 * @param obj The scrolled entry object
 * @param entry The text to be displayed
 *
 * @ingroup Scrolled_Entry
 */
EAPI void
elm_scrolled_entry_entry_set(Evas_Object *obj, const char *entry)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_entry_set(wd->entry, entry);
}

/**
 * This returns the text currently shown in object @p entry.
 * See also elm_scrolled_entry_entry_set().
 *
 * @param obj The scrolled entry object
 * @return The currently displayed text or NULL on failure
 *
 * @ingroup Scrolled_Entry
 */
EAPI const char *
elm_scrolled_entry_entry_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return elm_entry_entry_get(wd->entry);
}

/**
 * This returns all selected text within the scrolled entry.
 *
 * @param obj The scrolled entry object
 * @return The selected text within the scrolled entry or NULL on failure
 *
 * @ingroup Scrolled_Entry
 */
EAPI const char *
elm_scrolled_entry_selection_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return elm_entry_selection_get(wd->entry);
}

/**
 * This inserts text in @p entry at the beginning of the scrolled entry
 * object.
 *
 * @param obj The scrolled entry object
 * @param entry The text to insert
 *
 * @ingroup Scrolled_Entry
 */
EAPI void
elm_scrolled_entry_entry_insert(Evas_Object *obj, const char *entry)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_entry_insert(wd->entry, entry);
}

/**
 * This enables word line wrapping in the scrolled entry object.  It is the opposite
 * of elm_scrolled_entry_single_line_set().  Additionally, setting this disables
 * character line wrapping.
 * See also elm_scrolled_entry_line_char_wrap_set().
 *
 * @param obj The scrolled entry object
 * @param wrap If true, the scrolled entry will be wrapped once it reaches the end
 * of the object. Wrapping will occur at the end of the word before the end of the
 * object.
 *
 * @ingroup Scrolled_Entry
 */
EAPI void
elm_scrolled_entry_line_wrap_set(Evas_Object *obj, Eina_Bool wrap)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_line_wrap_set(wd->entry, wrap);
}

/**
 * This enables character line wrapping in the scrolled entry object.  It is the opposite
 * of elm_scrolled_entry_single_line_set().  Additionally, setting this disables
 * word line wrapping.
 * See also elm_scrolled_entry_line_wrap_set().
 *
 * @param obj The scrolled entry object
 * @param wrap If true, the scrolled entry will be wrapped once it reaches the end
 * of the object. Wrapping will occur immediately upon reaching the end of the object.
 *
 * @ingroup Scrolled_Entry
 */
EAPI void
elm_scrolled_entry_line_char_wrap_set(Evas_Object *obj, Eina_Bool wrap)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_line_char_wrap_set(wd->entry, wrap);
}

/**
 * This sets the editable attribute of the scrolled entry.
 *
 * @param obj The scrolled entry object
 * @param editable If true, the scrolled entry will be editable by the user.
 * If false, it will be set to the disabled state.
 *
 * @ingroup Scrolled_Entry
 */
EAPI void
elm_scrolled_entry_editable_set(Evas_Object *obj, Eina_Bool editable)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_editable_set(wd->entry, editable);
}

/**
 * This gets the editable attribute of the scrolled entry.
 * See also elm_scrolled_entry_editable_set().
 *
 * @param obj The scrolled entry object
 * @return If true, the scrolled entry is editable by the user.
 * If false, it is not editable by the user
 *
 * @ingroup Scrolled_Entry
 */
EAPI Eina_Bool
elm_scrolled_entry_editable_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return elm_entry_editable_get(wd->entry);
}


/**
 * This drops any existing text selection within the scrolled entry.
 *
 * @param obj The scrolled entry object
 *
 * @ingroup Scrolled_Entry
 */
EAPI void
elm_scrolled_entry_select_none(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_select_none(wd->entry);
}

/**
 * This selects all text within the scrolled entry.
 *
 * @param obj The scrolled entry object
 *
 * @ingroup Scrolled_Entry
 */
EAPI void
elm_scrolled_entry_select_all(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_select_all(wd->entry);
}

/**
 * This moves the cursor one place to the right within the entry.
 *
 * @param obj The scrolled entry object
 * @return EINA_TRUE upon success, EINA_FALSE upon failure
 *
 * @ingroup Scrolled_Entry
 */
EAPI Eina_Bool
elm_scrolled_entry_cursor_next(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return elm_entry_cursor_next(wd->entry);
}

/**
 * This moves the cursor one place to the left within the entry.
 *
 * @param obj The scrolled entry object
 * @return EINA_TRUE upon success, EINA_FALSE upon failure
 *
 * @ingroup Scrolled_Entry
 */
EAPI Eina_Bool
elm_scrolled_entry_cursor_prev(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return elm_entry_cursor_prev(wd->entry);
}

/**
 * This moves the cursor one line up within the entry.
 *
 * @param obj The scrolled entry object
 * @return EINA_TRUE upon success, EINA_FALSE upon failure
 *
 * @ingroup Scrolled_Entry
 */
EAPI Eina_Bool
elm_scrolled_entry_cursor_up(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return elm_entry_cursor_up(wd->entry);
}

/**
 * This moves the cursor one line down within the entry.
 *
 * @param obj The scrolled entry object
 * @return EINA_TRUE upon success, EINA_FALSE upon failure
 *
 * @ingroup Scrolled_Entry
 */
EAPI Eina_Bool
elm_scrolled_entry_cursor_down(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return elm_entry_cursor_down(wd->entry);
}

/**
 * This moves the cursor to the beginning of the entry.
 *
 * @param obj The scrolled entry object
 *
 * @ingroup Scrolled_Entry
 */
EAPI void
elm_scrolled_entry_cursor_begin_set(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_cursor_begin_set(wd->entry);
}

/**
 * This moves the cursor to the end of the entry.
 *
 * @param obj The scrolled entry object
 *
 * @ingroup Scrolled_Entry
 */
EAPI void
elm_scrolled_entry_cursor_end_set(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_cursor_end_set(wd->entry);
}

/**
 * This moves the cursor to the beginning of the current line.
 *
 * @param obj The scrolled entry object
 *
 * @ingroup Scrolled_Entry
 */
EAPI void
elm_scrolled_entry_cursor_line_begin_set(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_cursor_line_begin_set(wd->entry);
}

/**
 * This moves the cursor to the end of the current line.
 *
 * @param obj The scrolled entry object
 *
 * @ingroup Scrolled_Entry
 */
EAPI void
elm_scrolled_entry_cursor_line_end_set(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_cursor_line_end_set(wd->entry);
}

/**
 * This begins a selection within the scrolled entry as though
 * the user were holding down the mouse button to make a selection.
 *
 * @param obj The scrolled entry object
 *
 * @ingroup Scrolled_Entry
 */
EAPI void
elm_scrolled_entry_cursor_selection_begin(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_cursor_selection_begin(wd->entry);
}

/**
 * This ends a selection within the scrolled entry as though
 * the user had just released the mouse button while making a selection.
 *
 * @param obj The scrolled entry object
 *
 * @ingroup Scrolled_Entry
 */
EAPI void
elm_scrolled_entry_cursor_selection_end(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_cursor_selection_end(wd->entry);
}

/**
 * TODO: fill this in
 *
 * @param obj The scrolled entry object
 * @return TODO: fill this in
 *
 * @ingroup Scrolled_Entry
 */
EAPI Eina_Bool
elm_scrolled_entry_cursor_is_format_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return elm_entry_cursor_is_format_get(wd->entry);
}

/**
 * This returns whether the cursor is visible.
 *
 * @param obj The scrolled entry object
 * @return If true, the cursor is visible.
 *
 * @ingroup Scrolled_Entry
 */
EAPI Eina_Bool
elm_scrolled_entry_cursor_is_visible_format_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return elm_entry_cursor_is_visible_format_get(wd->entry);
}

/**
 * TODO: fill this in
 *
 * @param obj The scrolled entry object
 * @return TODO: fill this in
 *
 * @ingroup Scrolled_Entry
 */
EAPI const char *
elm_scrolled_entry_cursor_content_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return elm_entry_cursor_content_get(wd->entry);
}

/**
 * This executes a "cut" action on the selected text in the scrolled entry.
 *
 * @param obj The scrolled entry object
 *
 * @ingroup Scrolled_Entry
 */
EAPI void
elm_scrolled_entry_selection_cut(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_selection_cut(wd->entry);
}

/**
 * This executes a "copy" action on the selected text in the scrolled entry.
 *
 * @param obj The scrolled entry object
 *
 * @ingroup Scrolled_Entry
 */
EAPI void
elm_scrolled_entry_selection_copy(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_selection_copy(wd->entry);
}

/**
 * This executes a "paste" action in the scrolled entry.
 *
 * @param obj The scrolled entry object
 *
 * @ingroup Scrolled_Entry
 */
EAPI void
elm_scrolled_entry_selection_paste(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_selection_paste(wd->entry);
}

/**
 * This clears and frees the items in a scrolled entry's contextual (right click) menu.
 *
 * @param obj The scrolled entry object
 *
 * @ingroup Scrolled_Entry
 */
EAPI void
elm_scrolled_entry_context_menu_clear(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_context_menu_clear(wd->entry);
}

/**
 * This adds an item to the scrolled entry's contextual menu.
 *
 * @param obj The scrolled entry object
 * @param label The item's text label
 * @param icon_file The item's icon file
 * @param icon_type The item's icon type
 * @param func The callback to execute when the item is clicked
 * @param data The data to associate with the item for related functions
 *
 * @ingroup Scrolled_Entry
 */
EAPI void
elm_scrolled_entry_context_menu_item_add(Evas_Object *obj, const char *label, const char *icon_file, Elm_Icon_Type icon_type, Evas_Smart_Cb func, const void *data)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_context_menu_item_add(wd->entry, label, icon_file, icon_type, func, data);
}

/**
 * This disables the scrolled entry's contextual (right click) menu.
 *
 * @param obj The scrolled entry object
 * @param disabled If true, the menu is disabled
 *
 * @ingroup Scrolled_Entry
 */
EAPI void
elm_scrolled_entry_context_menu_disabled_set(Evas_Object *obj, Eina_Bool disabled)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_context_menu_disabled_set(wd->entry, disabled);
}

/**
 * This returns whether the scrolled entry's contextual (right click) menu is disabled.
 *
 * @param obj The scrolled entry object
 * @return If true, the menu is disabled
 *
 * @ingroup Scrolled_Entry
 */
EAPI Eina_Bool
elm_scrolled_entry_context_menu_disabled_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return elm_entry_context_menu_disabled_get(wd->entry);
}

/**
 * This sets the scrolled entry's scrollbar policy (ie. enabling/disabling them).
 *
 * @param obj The scrolled entry object
 * @param h The horizontal scrollbar policy to apply
 * @param v The vertical scrollbar policy to apply
 *
 * @ingroup Scrolled_Entry
 */
EAPI void
elm_scrolled_entry_scrollbar_policy_set(Evas_Object *obj, Elm_Scroller_Policy h, Elm_Scroller_Policy v)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->policy_h = h;
   wd->policy_v = v;
   elm_scroller_policy_set(wd->scroller, h, v);
}

/**
 * This enables/disables bouncing within the entry.
 *
 * @param obj The scrolled entry object
 * @param h The horizontal bounce state
 * @param v The vertical bounce state
 *
 * @ingroup Scrolled_Entry
 */
EAPI void
elm_scrolled_entry_bounce_set(Evas_Object *obj, Eina_Bool h_bounce, Eina_Bool v_bounce)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_scroller_bounce_set(wd->scroller, h_bounce, v_bounce);
}
