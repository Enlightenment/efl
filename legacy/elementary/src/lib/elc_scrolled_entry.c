#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *scroller;
   Evas_Object *entry;
   Elm_Scroller_Policy policy_h, policy_v;
};

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

   evas_object_size_hint_min_set(obj, -1, -1);
   evas_object_size_hint_max_set(obj, -1, -1);
}

static void
_on_focus_hook(void *data, Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (elm_widget_focus_get(obj))
     elm_widget_focus_steal(wd->entry);
}

static void
_disable_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   elm_object_disabled_set(wd->entry, elm_widget_disabled_get(obj));
}

static void
_entry_changed(void *data, Evas_Object *obj, void *event_info)
{
   _sizing_eval(data);
   evas_object_smart_callback_call(data, "changed", NULL);
}

static void
_entry_activated(void *data, Evas_Object *obj, void *event_info)
{
   evas_object_smart_callback_call(data, "activated", NULL);
}

static void
_entry_press(void *data, Evas_Object *obj, void *event_info)
{
   evas_object_smart_callback_call(data, "press", NULL);
}

static void
_entry_clicked(void *data, Evas_Object *obj, void *event_info)
{
   evas_object_smart_callback_call(data, "clicked", NULL);
}

static void
_entry_clicked_double(void *data, Evas_Object *obj, void *event_info)
{
   evas_object_smart_callback_call(data, "clicked,double", NULL);
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
   elm_widget_type_set(obj, "scrolled_entry");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_on_focus_hook_set(obj, _on_focus_hook, NULL);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_disable_hook_set(obj, _disable_hook);
   elm_widget_can_focus_set(obj, 1);

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

   _sizing_eval(obj);

   return obj;
}

EAPI void
elm_scrolled_entry_single_line_set(Evas_Object *obj, Eina_Bool single_line)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_single_line_set(wd->entry, single_line);
   if (single_line)
     elm_scroller_policy_set(wd->scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
   else
     elm_scroller_policy_set(wd->scroller, wd->policy_h, wd->policy_v);
}

EAPI void
elm_scrolled_entry_password_set(Evas_Object *obj, Eina_Bool password)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_password_set(wd->entry, password);
}

EAPI void
elm_scrolled_entry_entry_set(Evas_Object *obj, const char *entry)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_entry_set(wd->entry, entry);
}

EAPI const char *
elm_scrolled_entry_entry_get(const Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return elm_entry_entry_get(wd->entry);
}

EAPI const char *
elm_scrolled_entry_selection_get(const Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return elm_entry_selection_get(wd->entry);
}

EAPI void
elm_scrolled_entry_entry_insert(Evas_Object *obj, const char *entry)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_entry_insert(wd->entry, entry);
}

EAPI void
elm_scrolled_entry_line_wrap_set(Evas_Object *obj, Eina_Bool wrap)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_line_wrap_set(wd->entry, wrap);
}

EAPI void
elm_scrolled_entry_line_char_wrap_set(Evas_Object *obj, Eina_Bool wrap)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_line_char_wrap_set(wd->entry, wrap);
}

EAPI void
elm_scrolled_entry_editable_set(Evas_Object *obj, Eina_Bool editable)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_editable_set(wd->entry, editable);
}

EAPI void
elm_scrolled_entry_select_none(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_select_none(wd->entry);
}

EAPI void
elm_scrolled_entry_select_all(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_select_all(wd->entry);
}

EAPI Eina_Bool
elm_scrolled_entry_cursor_next(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return 0;
   return elm_entry_cursor_next(wd->entry);
}

EAPI Eina_Bool
elm_scrolled_entry_cursor_prev(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return 0;
   return elm_entry_cursor_prev(wd->entry);
}

EAPI Eina_Bool
elm_scrolled_entry_cursor_up(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return 0;
   return elm_entry_cursor_up(wd->entry);
}

EAPI Eina_Bool
elm_scrolled_entry_cursor_down(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return 0;
   return elm_entry_cursor_down(wd->entry);
}

EAPI void
elm_scrolled_entry_cursor_begin_set(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_cursor_begin_set(wd->entry);
}

EAPI void
elm_scrolled_entry_cursor_end_set(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_cursor_end_set(wd->entry);
}

EAPI void
elm_scrolled_entry_cursor_line_begin_set(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_cursor_line_begin_set(wd->entry);
}

EAPI void
elm_scrolled_entry_cursor_line_end_set(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_cursor_line_end_set(wd->entry);
}

EAPI void
elm_scrolled_entry_cursor_selection_begin(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_cursor_selection_begin(wd->entry);
}

EAPI void
elm_scrolled_entry_cursor_selection_end(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_cursor_selection_end(wd->entry);
}

EAPI Eina_Bool
elm_scrolled_entry_cursor_is_format_get(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return 0;
   return elm_entry_cursor_is_format_get(wd->entry);
}

EAPI Eina_Bool
elm_scrolled_entry_cursor_is_visible_format_get(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return 0;
   return elm_entry_cursor_is_visible_format_get(wd->entry);
}

EAPI const char *
elm_scrolled_entry_cursor_content_get(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return elm_entry_cursor_content_get(wd->entry);
}

EAPI void
elm_scrolled_entry_selection_cut(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_selection_cut(wd->entry);
}

EAPI void
elm_scrolled_entry_selection_copy(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_selection_copy(wd->entry);
}

EAPI void
elm_scrolled_entry_selection_paste(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_selection_paste(wd->entry);
}

EAPI void
elm_scrolled_entry_context_menu_clear(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_context_menu_clear(wd->entry);
}

EAPI void
elm_scrolled_entry_context_menu_item_add(Evas_Object *obj, const char *label, const char *icon_file, Elm_Icon_Type icon_type, void (*func) (void *data, Evas_Object *obj, void *event_info), const void *data)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_context_menu_item_add(wd->entry, label, icon_file, icon_type, func, data);
}

EAPI void
elm_scrolled_entry_context_menu_disabled_set(Evas_Object *obj, Eina_Bool disabled)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_context_menu_disabled_set(wd->entry, disabled);
}

EAPI Eina_Bool
elm_scrolled_entry_context_menu_disabled_get(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return 0;
   return elm_entry_context_menu_disabled_get(wd->entry);
}

EAPI void
elm_scrolled_entry_scrollbar_policy_set(Evas_Object *obj, Elm_Scroller_Policy h, Elm_Scroller_Policy v)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->policy_h = h;
   wd->policy_v = v;
   elm_scroller_policy_set(wd->scroller, h, v);
}
