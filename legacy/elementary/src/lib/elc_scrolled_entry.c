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
 *
 * "changed" - The text within the entry was changed
 * "activated" - The entry has received focus and the cursor
 * "press" - The entry has been clicked
 * "longpressed" - The entry has been clicked for a couple seconds
 * "clicked" - The entry has been clicked
 * "clicked,double" - The entry has been double clicked
 * "focused" - The entry has received focus
 * "unfocused" - The entry has lost focus
 * "selection,paste" - A paste action has occurred
 * "selection,copy" - A copy action has occurred
 * "selection,cut" - A cut action has occurred
 * "selection,start" - A selection has begun
 * "selection,changed" - The selection has changed
 * "selection,cleared" - The selection has been cleared
 * "cursor,changed" - The cursor has changed
 * "anchor,clicked" - The anchor has been clicked
 */

typedef struct _Widget_Data Widget_Data;
typedef struct _Elm_Entry_Context_Menu_Item Elm_Entry_Context_Menu_Item;
typedef struct _Elm_Entry_Item_Provider Elm_Entry_Item_Provider;
typedef struct _Elm_Entry_Text_Filter Elm_Entry_Text_Filter;

struct _Widget_Data
{
   Evas_Object *scroller;
   Evas_Object *entry;
   Evas_Object *icon;
   Evas_Object *end;
   Elm_Scroller_Policy policy_h, policy_v;
   Eina_List *items;
   Eina_List *item_providers;
   Eina_List *text_filters;
   Eina_Bool single_line : 1;
};

struct _Elm_Entry_Context_Menu_Item
{
   Evas_Object *obj;
   Evas_Smart_Cb func;
   void *data;
};

struct _Elm_Entry_Item_Provider
{
   Evas_Object *(*func) (void *data, Evas_Object *entry, const char *item);
   void *data;
};

struct _Elm_Entry_Text_Filter
{
   void (*func) (void *data, Evas_Object *entry, char **text);
   void *data;
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
   Elm_Entry_Context_Menu_Item *ci;
   Elm_Entry_Item_Provider *ip;
   Elm_Entry_Text_Filter *tf;

   Widget_Data *wd = elm_widget_data_get(obj);

   EINA_LIST_FREE(wd->items, ci)
      free(ci);
   EINA_LIST_FREE(wd->item_providers, ip)
      free(ip);
   EINA_LIST_FREE(wd->text_filters, tf)
      free(tf);

   if (!wd) return;
   free(wd);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd;
   Evas_Coord minw, minh, minw_scr, minh_scr;
   wd = elm_widget_data_get(obj);
   if (!wd) return;

   evas_object_size_hint_min_get(obj, &minw, &minh);
   evas_object_size_hint_min_get(wd->scroller, &minw_scr, &minh_scr);
   if (minw < minw_scr) minw = minw_scr;
   if (minh < minh_scr) minh = minh_scr;

   evas_object_size_hint_min_set(obj, minw, minh);
   if (wd->single_line)
     evas_object_size_hint_max_set(obj, -1, minh);
   else
     evas_object_size_hint_max_set(obj, -1, -1);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_object_style_set(wd->entry, elm_widget_style_get(obj));
   elm_object_style_set(wd->scroller, elm_widget_style_get(obj));
   elm_object_disabled_set(wd->entry, elm_widget_disabled_get(obj));
   elm_object_disabled_set(wd->scroller, elm_widget_disabled_get(obj));
   _sizing_eval(obj);
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
_signal_emit_hook(Evas_Object *obj, const char *emission, const char *source)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_object_signal_emit(wd->entry, emission, source);
   elm_object_signal_emit(wd->scroller, emission, source);
}

static void
_signal_callback_add_hook(Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func_cb, void *data)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_object_signal_callback_add(wd->entry, emission, source, func_cb, data);
   elm_object_signal_callback_add(wd->scroller, emission, source, func_cb,
                                  data);
}

static void
_signal_callback_del_hook(Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func_cb, void *data __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   elm_object_signal_callback_del(wd->entry, emission, source, func_cb);
   elm_object_signal_callback_del(wd->scroller, emission, source, func_cb);
}

static void
_on_focus_region_hook(const Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   elm_widget_focus_region_get(wd->entry, x, y, w, h);
}

static void
_changed_size_hints(void *data __UNUSED__, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   _sizing_eval(obj);
}

static void
_entry_changed(void *data, Evas_Object *obj __UNUSED__, void *event_info)
{
   _sizing_eval(data);
   evas_object_smart_callback_call(data, SIG_CHANGED, event_info);
}

static void
_entry_activated(void *data, Evas_Object *obj __UNUSED__, void *event_info)
{
   evas_object_smart_callback_call(data, SIG_ACTIVATED, event_info);
}

static void
_entry_press(void *data, Evas_Object *obj __UNUSED__, void *event_info)
{
   evas_object_smart_callback_call(data, SIG_PRESS, event_info);
}

static void
_entry_clicked(void *data, Evas_Object *obj __UNUSED__, void *event_info)
{
   evas_object_smart_callback_call(data, SIG_CLICKED, event_info);
}

static void
_entry_clicked_double(void *data, Evas_Object *obj __UNUSED__, void *event_info)
{
   evas_object_smart_callback_call(data, SIG_CLICKED_DOUBLE, event_info);
}

static void
_entry_cursor_changed(void *data, Evas_Object *obj __UNUSED__, void *event_info)
{
   evas_object_smart_callback_call(data, SIG_CURSOR_CHANGED, event_info);
}

static void
_entry_anchor_clicked(void *data, Evas_Object *obj __UNUSED__, void *event_info)
{
   evas_object_smart_callback_call(data, SIG_ANCHOR_CLICKED, event_info);
}

static void
_entry_selection_start(void *data, Evas_Object *obj __UNUSED__, void *event_info)
{
   evas_object_smart_callback_call(data, SIG_SELECTION_START, event_info);
}

static void
_entry_selection_changed(void *data, Evas_Object *obj __UNUSED__, void *event_info)
{
   evas_object_smart_callback_call(data, SIG_SELECTION_CHANGED, event_info);
}

static void
_entry_selection_cleared(void *data, Evas_Object *obj __UNUSED__, void *event_info)
{
   evas_object_smart_callback_call(data, SIG_SELECTION_CLEARED, event_info);
}

static void
_entry_selection_paste(void *data, Evas_Object *obj __UNUSED__, void *event_info)
{
   evas_object_smart_callback_call(data, SIG_SELECTION_PASTE, event_info);
}

static void
_entry_selection_copy(void *data, Evas_Object *obj __UNUSED__, void *event_info)
{
   evas_object_smart_callback_call(data, SIG_SELECTION_COPY, event_info);
}

static void
_entry_selection_cut(void *data, Evas_Object *obj __UNUSED__, void *event_info)
{
   evas_object_smart_callback_call(data, SIG_SELECTION_CUT, event_info);
}

static void
_entry_longpressed(void *data, Evas_Object *obj __UNUSED__, void *event_info)
{
   evas_object_smart_callback_call(data, SIG_LONGPRESSED, event_info);
}

static void
_entry_focused(void *data, Evas_Object *obj __UNUSED__, void *event_info)
{
   evas_object_smart_callback_call(data, SIG_FOCUSED, event_info);
}

static void
_entry_unfocused(void *data, Evas_Object *obj __UNUSED__, void *event_info)
{
   evas_object_smart_callback_call(data, SIG_UNFOCUSED, event_info);
}

static void
_context_item_wrap_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info)
{
   Elm_Entry_Context_Menu_Item *ci = data;
   ci->func(ci->data, ci->obj, event_info);
}

static Evas_Object *
_item_provider_wrap_cb(void *data, Evas_Object *obj __UNUSED__, const char *item)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Eina_List *l;
   Elm_Entry_Item_Provider *ip;

   EINA_LIST_FOREACH(wd->item_providers, l, ip)
     {
        Evas_Object *o;
        o = ip->func(ip->data, data, item);
        if (o) return o;
     }
   return NULL;
}

static void
_text_filter_wrap_cb(void *data, Evas_Object *obj __UNUSED__, char **text)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Eina_List *l;
   Elm_Entry_Text_Filter *tf;

   EINA_LIST_FOREACH(wd->text_filters, l, tf)
     {
        tf->func(tf->data, data, text);
        if (!*text) break;
     }
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

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);

   ELM_SET_WIDTYPE(widtype, "scrolled_entry");
   elm_widget_type_set(obj, "scrolled_entry");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_on_focus_hook_set(obj, _on_focus_hook, NULL);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_disable_hook_set(obj, _disable_hook);
   elm_widget_can_focus_set(obj, EINA_TRUE);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_on_focus_region_hook_set(obj, _on_focus_region_hook);
   elm_widget_signal_emit_hook_set(obj, _signal_emit_hook);
   elm_widget_signal_callback_add_hook_set(obj, _signal_callback_add_hook);
   elm_widget_signal_callback_del_hook_set(obj, _signal_callback_del_hook);

   wd->scroller = elm_scroller_add(obj);
   elm_scroller_custom_widget_base_theme_set(wd->scroller, "scroller", "entry");
   elm_widget_resize_object_set(obj, wd->scroller);
   evas_object_size_hint_weight_set(wd->scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(wd->scroller, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_scroller_bounce_set(wd->scroller, EINA_FALSE, EINA_FALSE);
   elm_scroller_propagate_events_set(wd->scroller, EINA_TRUE);
   evas_object_show(wd->scroller);

   wd->entry = elm_entry_add(obj);
   evas_object_size_hint_weight_set(wd->entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(wd->entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_scroller_content_set(wd->scroller, wd->entry);
   evas_object_show(wd->entry);

   elm_entry_text_filter_prepend(wd->entry, _text_filter_wrap_cb, obj);
   elm_entry_item_provider_prepend(wd->entry, _item_provider_wrap_cb, obj);

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

   evas_object_event_callback_add(obj, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                  _changed_size_hints, NULL);

   _sizing_eval(obj);

   // TODO: convert Elementary to subclassing of Evas_Smart_Class
   // TODO: and save some bytes, making descriptions per-class and not instance!
   evas_object_smart_callbacks_descriptions_set(obj, _signals);
   return obj;
}

/**
 * This sets a widget to be displayed to the left of a scrolled entry.
 *
 * @param obj The scrolled entry object
 * @param icon The widget to display on the left side of the scrolled
 * entry.
 *
 * @note A previously set widget will be destroyed.
 * @note If the object being set does not have minimum size hints set,
 * it won't get properly displayed.
 *
 * @ingroup Scrolled_Entry
 * @see elm_scrolled_entry_end_set
 */
EAPI void
elm_scrolled_entry_icon_set(Evas_Object *obj, Evas_Object *icon)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *edje;
   if (!wd) return;
   EINA_SAFETY_ON_NULL_RETURN(icon);
   if (wd->icon == icon) return;
   if (wd->icon) evas_object_del(wd->icon);
   wd->icon = icon;
   edje = _elm_scroller_edje_object_get(wd->scroller);
   if (!edje) return;
   edje_object_part_swallow(edje, "elm.swallow.icon", wd->icon);
   edje_object_signal_emit(edje, "elm,action,show,icon", "elm");
   _sizing_eval(obj);
}

/**
 * Gets the leftmost widget of the scrolled entry. This object is
 * owned by the scrolled entry and should not be modified.
 *
 * @param obj The scrolled entry object
 * @return the left widget inside the scroller
 *
 * @ingroup Scrolled_Entry
 */
EAPI Evas_Object *
elm_scrolled_entry_icon_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->icon;
}

/**
 * Unset the leftmost widget of the scrolled entry, unparenting and
 * returning it.
 *
 * @param obj The scrolled entry object
 * @return the previously set icon sub-object of this entry, on
 * success.
 *
 * @see elm_scrolled_entry_icon_set()
 *
 * @ingroup Scrolled_Entry
 */
EAPI Evas_Object *
elm_scrolled_entry_icon_unset(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *ret = NULL;
   if (!wd) return NULL;
   if (wd->icon)
     {
        Evas_Object *edje = _elm_scroller_edje_object_get(wd->scroller);
        if (!edje) return NULL;
        ret = wd->icon;
        edje_object_part_unswallow(edje, wd->icon);
        edje_object_signal_emit(edje, "elm,action,hide,icon", "elm");
        wd->icon = NULL;
        _sizing_eval(obj);
     }
   return ret;
}

/**
 * Sets the visibility of the left-side widget of the scrolled entry,
 * set by @elm_scrolled_entry_icon_set().
 *
 * @param obj The scrolled entry object
 * @param setting EINA_TRUE if the object should be displayed,
 * EINA_FALSE if not.
 *
 * @ingroup Scrolled_Entry
 */
EAPI void
elm_scrolled_entry_icon_visible_set(Evas_Object *obj, Eina_Bool setting)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if ((!wd) || (!wd->icon)) return;
   if (setting)
     evas_object_hide(wd->icon);
   else
     evas_object_show(wd->icon);
   _sizing_eval(obj);
}

/**
 * This sets a widget to be displayed to the end of a scrolled entry.
 *
 * @param obj The scrolled entry object
 * @param end The widget to display on the right side of the scrolled
 * entry.
 *
 * @note A previously set widget will be destroyed.
 * @note If the object being set does not have minimum size hints set,
 * it won't get properly displayed.
 *
 * @ingroup Scrolled_Entry
 * @see elm_scrolled_entry_icon_set
 */
EAPI void
elm_scrolled_entry_end_set(Evas_Object *obj, Evas_Object *end)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *edje;
   if (!wd) return;
   EINA_SAFETY_ON_NULL_RETURN(end);
   if (wd->end == end) return;
   if (wd->end) evas_object_del(wd->end);
   wd->end = end;
   edje = _elm_scroller_edje_object_get(wd->scroller);
   if (!edje) return;
   edje_object_part_swallow(edje, "elm.swallow.end", wd->end);
   edje_object_signal_emit(edje, "elm,action,show,end", "elm");
   _sizing_eval(obj);
}

/**
 * Gets the endmost widget of the scrolled entry. This object is owned
 * by the scrolled entry and should not be modified.
 *
 * @param obj The scrolled entry object
 * @return the right widget inside the scroller
 *
 * @ingroup Scrolled_Entry
 */
EAPI Evas_Object *
elm_scrolled_entry_end_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->end;
}

/**
 * Unset the endmost widget of the scrolled entry, unparenting and
 * returning it.
 *
 * @param obj The scrolled entry object
 * @return the previously set icon sub-object of this entry, on
 * success.
 *
 * @see elm_scrolled_entry_icon_set()
 *
 * @ingroup Scrolled_Entry
 */
EAPI Evas_Object *
elm_scrolled_entry_end_unset(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *ret = NULL;
   if (!wd) return NULL;
   if (wd->end)
     {
        Evas_Object *edje = _elm_scroller_edje_object_get(wd->scroller);
        if (!edje) return NULL;
        ret = wd->end;
        edje_object_part_unswallow(edje, wd->end);
        edje_object_signal_emit(edje, "elm,action,hide,end", "elm");
        wd->end = NULL;
        _sizing_eval(obj);
     }
   return ret;
}

/**
 * Sets the visibility of the end widget of the scrolled entry, set by
 * @elm_scrolled_entry_end_set().
 *
 * @param obj The scrolled entry object
 * @param setting EINA_TRUE if the object should be displayed,
 * EINA_FALSE if not.
 *
 * @ingroup Scrolled_Entry
 */
EAPI void
elm_scrolled_entry_end_visible_set(Evas_Object *obj, Eina_Bool setting)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if ((!wd) || (!wd->end)) return;
   if (setting)
     evas_object_hide(wd->end);
   else
     evas_object_show(wd->end);
   _sizing_eval(obj);
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
 * This appends @p entry to the scrolled entry
 *
 * @param obj The scrolled entry object
 * @param entry The text to be displayed
 *
 * @ingroup Scrolled_Entry
 */
EAPI void
elm_scrolled_entry_entry_append(Evas_Object *obj, const char *entry)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_entry_append(wd->entry, entry);
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
 * This returns EINA_TRUE if the entry is empty/there was an error
 * and EINA_FALSE if it is not empty.
 *
 * @param obj The entry object
 * @return If the entry is empty or not.
 *
 * @ingroup Entry
 */
EAPI Eina_Bool
elm_scrolled_entry_is_empty(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_TRUE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_TRUE;
   return elm_entry_is_empty(wd->entry);
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
 * @param wrap wrap according to Elm_Wrap_Type
 * of the object. Wrapping will occur at the end of the word before the end of the
 * object.
 *
 * @ingroup Scrolled_Entry
 */
EAPI void
elm_scrolled_entry_line_wrap_set(Evas_Object *obj, Elm_Wrap_Type wrap)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_line_wrap_set(wd->entry, wrap);
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
   int x, y, w, h;
   elm_entry_cursor_end_set(wd->entry);
   elm_widget_show_region_get(wd->entry, &x, &y, &w, &h);
   elm_scroller_region_show(wd->scroller, x, y, w, h);
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
 * Sets the cursor position in the scrolled entry to the given value
 *
 * @param obj The scrolled entry object
 * @param pos the position of the cursor
 *
 * @ingroup Scrolled_Entry
 */
EAPI void
elm_scrolled_entry_cursor_pos_set(Evas_Object *obj, int pos)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_cursor_pos_set(wd->entry, pos);
}

/**
 * Retrieves the current position of the cursor in the scrolled entry
 *
 * @param obj The entry object
 * @return the cursor position
 *
 * @ingroup Scrolled_Entry
 */
EAPI int
elm_scrolled_entry_cursor_pos_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) 0;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return 0;
   return elm_entry_cursor_pos_get(wd->entry);
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
   Elm_Entry_Context_Menu_Item *ci;
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   ci = malloc(sizeof(Elm_Entry_Context_Menu_Item));
   if (!ci) return;
   ci->func = func;
   ci->data = (void *)data;
   ci->obj = obj;
   wd->items = eina_list_append(wd->items, ci);
   elm_entry_context_menu_item_add(wd->entry, label, icon_file, icon_type, _context_item_wrap_cb, ci);
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

/**
 * Get the bounce mode
 *
 * @param obj The Scrolled_Entry object
 * @param h_bounce Allow bounce horizontally
 * @param v_bounce Allow bounce vertically
 *
 * @ingroup Scrolled_Entry
 */
EAPI void
elm_scrolled_entry_bounce_get(const Evas_Object *obj, Eina_Bool *h_bounce, Eina_Bool *v_bounce)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_scroller_bounce_get(wd->scroller, h_bounce, v_bounce);
}

/**
 * This appends a custom item provider to the list for that entry
 *
 * This appends the given callback. The list is walked from beginning to end
 * with each function called given the item href string in the text. If the
 * function returns an object handle other than NULL (it should create an
 * and object to do this), then this object is used to replace that item. If
 * not the next provider is called until one provides an item object, or the
 * default provider in entry does.
 *
 * @param obj The entry object
 * @param func The function called to provide the item object
 * @param data The data passed to @p func
 *
 * @ingroup Scrolled_Entry
 */
EAPI void
elm_scrolled_entry_item_provider_append(Evas_Object *obj, Evas_Object *(*func) (void *data, Evas_Object *entry, const char *item), void *data)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   EINA_SAFETY_ON_NULL_RETURN(func);
   Elm_Entry_Item_Provider *ip = calloc(1, sizeof(Elm_Entry_Item_Provider));
   if (!ip) return;
   ip->func = func;
   ip->data = data;
   wd->item_providers = eina_list_append(wd->item_providers, ip);
}

/**
 * This prepends a custom item provider to the list for that entry
 *
 * This prepends the given callback. See elm_scrolled_entry_item_provider_append() for
 * more information
 *
 * @param obj The entry object
 * @param func The function called to provide the item object
 * @param data The data passed to @p func
 *
 * @ingroup Scrolled_Entry
 */
EAPI void
elm_scrolled_entry_item_provider_prepend(Evas_Object *obj, Evas_Object *(*func) (void *data, Evas_Object *entry, const char *item), void *data)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   EINA_SAFETY_ON_NULL_RETURN(func);
   Elm_Entry_Item_Provider *ip = calloc(1, sizeof(Elm_Entry_Item_Provider));
   if (!ip) return;
   ip->func = func;
   ip->data = data;
   wd->item_providers = eina_list_prepend(wd->item_providers, ip);
}

/**
 * This removes a custom item provider to the list for that entry
 *
 * This removes the given callback. See elm_scrolled_entry_item_provider_append() for
 * more information
 *
 * @param obj The entry object
 * @param func The function called to provide the item object
 * @param data The data passed to @p func
 *
 * @ingroup Scrolled_Entry
 */
EAPI void
elm_scrolled_entry_item_provider_remove(Evas_Object *obj, Evas_Object *(*func) (void *data, Evas_Object *entry, const char *item), void *data)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   Eina_List *l;
   Elm_Entry_Item_Provider *ip;
   if (!wd) return;
   EINA_SAFETY_ON_NULL_RETURN(func);
   EINA_LIST_FOREACH(wd->item_providers, l, ip)
     {
        if ((ip->func == func) && (ip->data == data))
          {
             wd->item_providers = eina_list_remove_list(wd->item_providers, l);
             free(ip);
             return;
          }
     }
}

/**
 * Append a filter function for text inserted in the entry
 *
 * Append the given callback to the list. This functions will be called
 * whenever any text is inserted into the entry, with the text to be inserted
 * as a parameter. The callback function is free to alter the text in any way
 * it wants, but it must remember to free the given pointer and update it.
 * If the new text is to be discarded, the function can free it and set it text
 * parameter to NULL. This will also prevent any following filters from being
 * called.
 *
 * @param obj The entry object
 * @param func The function to use as text filter
 * @param data User data to pass to @p func
 *
 * @ingroup Scrolled_Entry
 */
EAPI void
elm_scrolled_entry_text_filter_append(Evas_Object *obj, void (*func) (void *data, Evas_Object *entry, char **text), void *data)
{
   Widget_Data *wd;
   Elm_Entry_Text_Filter *tf;
   ELM_CHECK_WIDTYPE(obj, widtype);

   wd = elm_widget_data_get(obj);

   EINA_SAFETY_ON_NULL_RETURN(func);

   tf = ELM_NEW(Elm_Entry_Text_Filter);
   if (!tf) return;
   tf->func = func;
   tf->data = data;
   wd->text_filters = eina_list_append(wd->text_filters, tf);
}

/**
 * Prepend a filter function for text insdrted in the entry
 *
 * Prepend the given callback to the list. See elm_scrolled_entry_text_filter_append()
 * for more information
 *
 * @param obj The entry object
 * @param func The function to use as text filter
 * @param data User data to pass to @p func
 *
 * @ingroup Scrolled_Entry
 */
EAPI void
elm_scrolled_entry_text_filter_prepend(Evas_Object *obj, void (*func) (void *data, Evas_Object *entry, char **text), void *data)
{
   Widget_Data *wd;
   Elm_Entry_Text_Filter *tf;
   ELM_CHECK_WIDTYPE(obj, widtype);

   wd = elm_widget_data_get(obj);

   EINA_SAFETY_ON_NULL_RETURN(func);

   tf = ELM_NEW(Elm_Entry_Text_Filter);
   if (!tf) return;
   tf->func = func;
   tf->data = data;
   wd->text_filters = eina_list_prepend(wd->text_filters, tf);
}

/**
 * Remove a filter from the list
 *
 * Removes the given callback from the filter list. See elm_scrolled_entry_text_filter_append()
 * for more information.
 *
 * @param obj The entry object
 * @param func The filter function to remove
 * @param data The user data passed when adding the function
 *
 * @ingroup Scrolled_Entry
 */
EAPI void
elm_scrolled_entry_text_filter_remove(Evas_Object *obj, void (*func) (void *data, Evas_Object *entry, char **text), void *data)
{
   Widget_Data *wd;
   Eina_List *l;
   Elm_Entry_Text_Filter *tf;
   ELM_CHECK_WIDTYPE(obj, widtype);

   wd = elm_widget_data_get(obj);

   EINA_SAFETY_ON_NULL_RETURN(func);

   EINA_LIST_FOREACH(wd->text_filters, l, tf)
     {
        if ((tf->func == func) && (tf->data == data))
          {
             wd->text_filters = eina_list_remove_list(wd->text_filters, l);
             free(tf);
             return;
          }
     }
}

/**
 * This sets the file (and implicitly loads it) for the text to display and
 * then edit. All changes are written back to the file after a short delay if
 * the entry object is set to autosave.
 *
 * @param obj The scrolled entry object
 * @param file The path to the file to load and save
 * @param format The file format
 *
 * @ingroup Scrolled_Entry
 */
EAPI void
elm_scrolled_entry_file_set(Evas_Object *obj, const char *file, Elm_Text_Format format)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_file_set(wd->entry, file, format);
}

/**
 * Gets the file to load and save and the file format
 *
 * @param obj The scrolled entry object
 * @param file The path to the file to load and save
 * @param format The file format
 *
 * @ingroup Scrolled_Entry
 */
EAPI void
elm_scrolled_entry_file_get(const Evas_Object *obj, const char **file, Elm_Text_Format *format)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_file_get(wd->entry, file, format);
}

/**
 * This function writes any changes made to the file set with
 * elm_scrolled_entry_file_set()
 *
 * @param obj The scrolled entry object
 *
 * @ingroup Scrolled_Entry
 */
EAPI void
elm_scrolled_entry_file_save(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_file_save(wd->entry);
}

/**
 * This sets the entry object to 'autosave' the loaded text file or not.
 *
 * @param obj The scrolled entry object
 * @param autosave Autosave the loaded file or not
 *
 * @ingroup Scrolled_Entry
 */
EAPI void
elm_scrolled_entry_autosave_set(Evas_Object *obj, Eina_Bool autosave)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_autosave_set(wd->entry, autosave);
}

/**
 * This gets the entry object's 'autosave' status.
 *
 * @param obj The scrolled entry object
 * @return Autosave the loaded file or not
 *
 * @ingroup Scrolled_Entry
 */
EAPI Eina_Bool
elm_scrolled_entry_autosave_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return elm_entry_autosave_get(wd->entry);
}

/**
 * Control pasting of text and images for the widget.
 *
 * Normally the scrolled entry allows both text and images to be pasted.
 * By setting textonly to be true, this prevents images from being pasted.
 *
 * Note this only changes the behaviour of text.
 *
 * @param obj The scrolled entry object
 * @param textonly paste mode - EINA_TRUE is text only, EINA_FALSE is text+image+other.
 *
 * @see elm_entry_cnp_textonly_set
 * @ingroup Scrolled_Entry
 */
EAPI void
elm_scrolled_entry_cnp_textonly_set(Evas_Object *obj, Eina_Bool textonly)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_cnp_textonly_set(wd->entry, textonly);
}

/**
 * Getting elm_scrolled_entry text paste/drop mode.
 *
 * In textonly mode, only text may be pasted or dropped into the widget.
 *
 * @param obj The scrolled entry object
 * @return If the widget only accepts text from pastes.
 *
 * @see elm_entry_cnp_textonly_get
 * @ingroup Scrolled_Entry
 */
EAPI Eina_Bool
elm_scrolled_entry_cnp_textonly_get(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return elm_entry_cnp_textonly_get(wd->entry);
}
