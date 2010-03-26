#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *scroller;
   Evas_Object *entry;
   Elm_Scroller_Policy policy_h, policy_v;
   Eina_Bool single_line : 1;
};

static const char *widtype = NULL;
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
   evas_object_smart_callback_call(data, "changed", NULL);
}

static void
_entry_activated(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, "activated", NULL);
}

static void
_entry_press(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, "press", NULL);
}

static void
_entry_clicked(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, "clicked", NULL);
}

static void
_entry_clicked_double(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, "clicked,double", NULL);
}

static void
_entry_cursor_changed(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, "cursor,changed", NULL);
}

static void
_entry_anchor_clicked(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, "anchor,clicked", NULL);
}

static void
_entry_selection_start(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, "selection,start", NULL);
}

static void
_entry_selection_changed(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, "selection,changed", NULL);
}

static void
_entry_selection_cleared(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, "selection,cleared", NULL);
}

static void
_entry_selection_paste(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, "selection,paste", NULL);
}

static void
_entry_selection_copy(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, "selection,copy", NULL);
}

static void
_entry_selection_cut(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, "selection,cut", NULL);
}

static void
_entry_longpressed(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, "longpressed", NULL);
}

static void
_entry_focused(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, "focused", NULL);
}

static void
_entry_unfocused(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, "unfocused", NULL);
}

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

   return obj;
}

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

EAPI Eina_Bool
elm_scrolled_entry_single_line_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return elm_entry_single_line_get(wd->entry);
}

EAPI void
elm_scrolled_entry_password_set(Evas_Object *obj, Eina_Bool password)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_password_set(wd->entry, password);
}

EAPI Eina_Bool
elm_scrolled_entry_password_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return elm_entry_password_get(wd->entry);
}

EAPI void
elm_scrolled_entry_entry_set(Evas_Object *obj, const char *entry)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_entry_set(wd->entry, entry);
}

EAPI const char *
elm_scrolled_entry_entry_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return elm_entry_entry_get(wd->entry);
}

EAPI const char *
elm_scrolled_entry_selection_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return elm_entry_selection_get(wd->entry);
}

EAPI void
elm_scrolled_entry_entry_insert(Evas_Object *obj, const char *entry)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_entry_insert(wd->entry, entry);
}

EAPI void
elm_scrolled_entry_line_wrap_set(Evas_Object *obj, Eina_Bool wrap)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_line_wrap_set(wd->entry, wrap);
}

EAPI void
elm_scrolled_entry_line_char_wrap_set(Evas_Object *obj, Eina_Bool wrap)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_line_char_wrap_set(wd->entry, wrap);
}

EAPI void
elm_scrolled_entry_editable_set(Evas_Object *obj, Eina_Bool editable)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_editable_set(wd->entry, editable);
}

EAPI Eina_Bool
elm_scrolled_entry_editable_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return elm_entry_editable_get(wd->entry);
}

EAPI void
elm_scrolled_entry_select_none(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_select_none(wd->entry);
}

EAPI void
elm_scrolled_entry_select_all(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_select_all(wd->entry);
}

EAPI Eina_Bool
elm_scrolled_entry_cursor_next(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) 0;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return 0;
   return elm_entry_cursor_next(wd->entry);
}

EAPI Eina_Bool
elm_scrolled_entry_cursor_prev(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) 0;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return 0;
   return elm_entry_cursor_prev(wd->entry);
}

EAPI Eina_Bool
elm_scrolled_entry_cursor_up(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) 0;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return 0;
   return elm_entry_cursor_up(wd->entry);
}

EAPI Eina_Bool
elm_scrolled_entry_cursor_down(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) 0;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return 0;
   return elm_entry_cursor_down(wd->entry);
}

EAPI void
elm_scrolled_entry_cursor_begin_set(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_cursor_begin_set(wd->entry);
}

EAPI void
elm_scrolled_entry_cursor_end_set(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_cursor_end_set(wd->entry);
}

EAPI void
elm_scrolled_entry_cursor_line_begin_set(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_cursor_line_begin_set(wd->entry);
}

EAPI void
elm_scrolled_entry_cursor_line_end_set(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_cursor_line_end_set(wd->entry);
}

EAPI void
elm_scrolled_entry_cursor_selection_begin(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_cursor_selection_begin(wd->entry);
}

EAPI void
elm_scrolled_entry_cursor_selection_end(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_cursor_selection_end(wd->entry);
}

EAPI Eina_Bool
elm_scrolled_entry_cursor_is_format_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) 0;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return 0;
   return elm_entry_cursor_is_format_get(wd->entry);
}

EAPI Eina_Bool
elm_scrolled_entry_cursor_is_visible_format_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) 0;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return 0;
   return elm_entry_cursor_is_visible_format_get(wd->entry);
}

EAPI const char *
elm_scrolled_entry_cursor_content_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return elm_entry_cursor_content_get(wd->entry);
}

EAPI void
elm_scrolled_entry_selection_cut(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_selection_cut(wd->entry);
}

EAPI void
elm_scrolled_entry_selection_copy(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_selection_copy(wd->entry);
}

EAPI void
elm_scrolled_entry_selection_paste(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_selection_paste(wd->entry);
}

EAPI void
elm_scrolled_entry_context_menu_clear(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_context_menu_clear(wd->entry);
}

EAPI void
elm_scrolled_entry_context_menu_item_add(Evas_Object *obj, const char *label, const char *icon_file, Elm_Icon_Type icon_type, Evas_Smart_Cb func, const void *data)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_context_menu_item_add(wd->entry, label, icon_file, icon_type, func, data);
}

EAPI void
elm_scrolled_entry_context_menu_disabled_set(Evas_Object *obj, Eina_Bool disabled)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_context_menu_disabled_set(wd->entry, disabled);
}

EAPI Eina_Bool
elm_scrolled_entry_context_menu_disabled_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) 0;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return 0;
   return elm_entry_context_menu_disabled_get(wd->entry);
}

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

EAPI void
elm_scrolled_entry_bounce_set(Evas_Object *obj, Eina_Bool h_bounce, Eina_Bool v_bounce)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_scroller_bounce_set(wd->scroller, h_bounce, v_bounce);
}
