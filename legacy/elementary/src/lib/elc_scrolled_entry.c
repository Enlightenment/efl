#include <Elementary.h>
#include "elm_priv.h"
EINA_DEPRECATED EAPI Evas_Object *
elm_scrolled_entry_add(Evas_Object *parent)
{
   Evas_Object *obj;
   obj = elm_entry_add(parent);
   elm_entry_scrollable_set(obj, EINA_TRUE);
   return obj;
}
EINA_DEPRECATED EAPI void
elm_scrolled_entry_icon_set(Evas_Object *obj, Evas_Object *icon)
{elm_entry_icon_set(obj, icon);}
EINA_DEPRECATED EAPI Evas_Object *
elm_scrolled_entry_icon_get(const Evas_Object *obj)
{return elm_entry_icon_get(obj);}
EINA_DEPRECATED EAPI Evas_Object *
elm_scrolled_entry_icon_unset(Evas_Object *obj)
{return elm_entry_icon_unset(obj);}
EINA_DEPRECATED EAPI void
elm_scrolled_entry_icon_visible_set(Evas_Object *obj, Eina_Bool setting)
{elm_entry_icon_visible_set(obj, setting);}
EINA_DEPRECATED EAPI void
elm_scrolled_entry_end_set(Evas_Object *obj, Evas_Object *end)
{elm_entry_end_set(obj, end);}
EINA_DEPRECATED EAPI Evas_Object *
elm_scrolled_entry_end_get(const Evas_Object *obj)
{return elm_entry_end_get(obj);}
EINA_DEPRECATED EAPI Evas_Object *
elm_scrolled_entry_end_unset(Evas_Object *obj)
{return elm_entry_end_unset(obj);}
EINA_DEPRECATED EAPI void
elm_scrolled_entry_end_visible_set(Evas_Object *obj, Eina_Bool setting)
{elm_entry_end_visible_set(obj, setting);}
EINA_DEPRECATED EAPI void
elm_scrolled_entry_single_line_set(Evas_Object *obj, Eina_Bool single_line)
{elm_entry_single_line_set(obj, single_line);}
EINA_DEPRECATED EAPI Eina_Bool
elm_scrolled_entry_single_line_get(const Evas_Object *obj)
{return elm_entry_single_line_get(obj);}
EINA_DEPRECATED EAPI void
elm_scrolled_entry_password_set(Evas_Object *obj, Eina_Bool password)
{elm_entry_password_set(obj, password);}
EINA_DEPRECATED EAPI Eina_Bool
elm_scrolled_entry_password_get(const Evas_Object *obj)
{return elm_entry_password_get(obj);}
EINA_DEPRECATED EAPI void
elm_scrolled_entry_entry_set(Evas_Object *obj, const char *entry)
{elm_entry_entry_set(obj, entry);}
EINA_DEPRECATED EAPI void
elm_scrolled_entry_entry_append(Evas_Object *obj, const char *entry)
{elm_entry_entry_append(obj, entry);}
EINA_DEPRECATED EAPI const char *
elm_scrolled_entry_entry_get(const Evas_Object *obj)
{return elm_entry_entry_get(obj);}
EINA_DEPRECATED EAPI Eina_Bool
elm_scrolled_entry_is_empty(const Evas_Object *obj)
{return elm_entry_is_empty(obj);}
EINA_DEPRECATED EAPI const char *
elm_scrolled_entry_selection_get(const Evas_Object *obj)
{return elm_entry_selection_get(obj);}
EINA_DEPRECATED EAPI void
elm_scrolled_entry_entry_insert(Evas_Object *obj, const char *entry)
{elm_entry_entry_insert(obj, entry);}
EINA_DEPRECATED EAPI void
elm_scrolled_entry_line_wrap_set(Evas_Object *obj, Elm_Wrap_Type wrap)
{elm_entry_line_wrap_set(obj, wrap);}
EINA_DEPRECATED EAPI void
elm_scrolled_entry_editable_set(Evas_Object *obj, Eina_Bool editable)
{elm_entry_editable_set(obj, editable);}
EINA_DEPRECATED EAPI Eina_Bool
elm_scrolled_entry_editable_get(const Evas_Object *obj)
{return elm_entry_editable_get(obj);}
EINA_DEPRECATED EAPI void
elm_scrolled_entry_select_none(Evas_Object *obj)
{elm_entry_select_none(obj);}
EINA_DEPRECATED EAPI void
elm_scrolled_entry_select_all(Evas_Object *obj)
{return elm_entry_select_all(obj);}
EINA_DEPRECATED EAPI Eina_Bool
elm_scrolled_entry_cursor_next(Evas_Object *obj)
{return elm_entry_cursor_next(obj);}
EINA_DEPRECATED EAPI Eina_Bool
elm_scrolled_entry_cursor_prev(Evas_Object *obj)
{return elm_entry_cursor_prev(obj);}
EINA_DEPRECATED EAPI Eina_Bool
elm_scrolled_entry_cursor_up(Evas_Object *obj)
{return elm_entry_cursor_up(obj);}
EINA_DEPRECATED EAPI Eina_Bool
elm_scrolled_entry_cursor_down(Evas_Object *obj)
{return elm_entry_cursor_down(obj);}
EINA_DEPRECATED EAPI void
elm_scrolled_entry_cursor_begin_set(Evas_Object *obj)
{elm_entry_cursor_begin_set(obj);}
EINA_DEPRECATED EAPI void
elm_scrolled_entry_cursor_end_set(Evas_Object *obj)
{elm_entry_cursor_end_set(obj);}
EINA_DEPRECATED EAPI void
elm_scrolled_entry_cursor_line_begin_set(Evas_Object *obj)
{elm_entry_cursor_line_begin_set(obj);}
EINA_DEPRECATED EAPI void
elm_scrolled_entry_cursor_line_end_set(Evas_Object *obj)
{elm_entry_cursor_line_end_set(obj);}
EINA_DEPRECATED EAPI void
elm_scrolled_entry_cursor_selection_begin(Evas_Object *obj)
{elm_entry_cursor_selection_begin(obj);}
EINA_DEPRECATED EAPI void
elm_scrolled_entry_cursor_selection_end(Evas_Object *obj)
{return elm_entry_cursor_selection_end(obj);}
EINA_DEPRECATED EAPI Eina_Bool
elm_scrolled_entry_cursor_is_format_get(const Evas_Object *obj)
{return elm_entry_cursor_is_format_get(obj);}
EINA_DEPRECATED EAPI Eina_Bool
elm_scrolled_entry_cursor_is_visible_format_get(const Evas_Object *obj)
{return elm_entry_cursor_is_visible_format_get(obj);}
EINA_DEPRECATED EAPI const char *
elm_scrolled_entry_cursor_content_get(const Evas_Object *obj)
{return elm_entry_cursor_content_get(obj);}
EINA_DEPRECATED EAPI void
elm_scrolled_entry_cursor_pos_set(Evas_Object *obj, int pos)
{elm_entry_cursor_pos_set(obj, pos);}
EINA_DEPRECATED EAPI int
elm_scrolled_entry_cursor_pos_get(const Evas_Object *obj)
{return elm_entry_cursor_pos_get(obj);}
EINA_DEPRECATED EAPI void
elm_scrolled_entry_selection_cut(Evas_Object *obj)
{elm_entry_selection_cut(obj);}
EINA_DEPRECATED EAPI void
elm_scrolled_entry_selection_copy(Evas_Object *obj)
{elm_entry_selection_copy(obj);}
EINA_DEPRECATED EAPI void
elm_scrolled_entry_selection_paste(Evas_Object *obj)
{elm_entry_selection_paste(obj);}
EINA_DEPRECATED EAPI void
elm_scrolled_entry_context_menu_clear(Evas_Object *obj)
{elm_entry_context_menu_clear(obj);}
EINA_DEPRECATED EAPI void
elm_scrolled_entry_context_menu_item_add(Evas_Object *obj, const char *label, const char *icon_file, Elm_Icon_Type icon_type, Evas_Smart_Cb func, const void *data)
{elm_entry_context_menu_item_add(obj, label, icon_file, icon_type, func, data);}
EINA_DEPRECATED EAPI void
elm_scrolled_entry_context_menu_disabled_set(Evas_Object *obj, Eina_Bool disabled)
{elm_entry_context_menu_disabled_set(obj, disabled);}
EINA_DEPRECATED EAPI Eina_Bool
elm_scrolled_entry_context_menu_disabled_get(const Evas_Object *obj)
{return elm_entry_context_menu_disabled_get(obj);}
EINA_DEPRECATED EAPI void
elm_scrolled_entry_scrollbar_policy_set(Evas_Object *obj, Elm_Scroller_Policy h, Elm_Scroller_Policy v)
{elm_entry_scrollbar_policy_set(obj, h, v);}
EINA_DEPRECATED EAPI void
elm_scrolled_entry_bounce_set(Evas_Object *obj, Eina_Bool h_bounce, Eina_Bool v_bounce)
{elm_entry_bounce_set(obj, h_bounce, v_bounce);}
EINA_DEPRECATED EAPI void
elm_scrolled_entry_bounce_get(const Evas_Object *obj, Eina_Bool *h_bounce, Eina_Bool *v_bounce)
{elm_entry_bounce_get(obj, h_bounce, v_bounce);}
EINA_DEPRECATED EAPI void
elm_scrolled_entry_item_provider_append(Evas_Object *obj, Evas_Object *(*func) (void *data, Evas_Object *entry, const char *item), void *data)
{elm_entry_item_provider_append(obj, func, data);}
EINA_DEPRECATED EAPI void
elm_scrolled_entry_item_provider_prepend(Evas_Object *obj, Evas_Object *(*func) (void *data, Evas_Object *entry, const char *item), void *data)
{elm_entry_item_provider_prepend(obj, func, data);}
EINA_DEPRECATED EAPI void
elm_scrolled_entry_item_provider_remove(Evas_Object *obj, Evas_Object *(*func) (void *data, Evas_Object *entry, const char *item), void *data)
{elm_entry_item_provider_remove(obj, func, data);}
EINA_DEPRECATED EAPI void
elm_scrolled_entry_text_filter_append(Evas_Object *obj, void (*func) (void *data, Evas_Object *entry, char **text), void *data)
{elm_entry_text_filter_append(obj, func, data);}
EINA_DEPRECATED EAPI void
elm_scrolled_entry_text_filter_prepend(Evas_Object *obj, void (*func) (void *data, Evas_Object *entry, char **text), void *data)
{elm_entry_text_filter_prepend(obj, func, data);}
EINA_DEPRECATED EAPI void
elm_scrolled_entry_text_filter_remove(Evas_Object *obj, void (*func) (void *data, Evas_Object *entry, char **text), void *data)
{elm_entry_text_filter_remove(obj, func, data);}
EINA_DEPRECATED EAPI void
elm_scrolled_entry_file_set(Evas_Object *obj, const char *file, Elm_Text_Format format)
{elm_entry_file_set(obj, file, format);}
EINA_DEPRECATED EAPI void
elm_scrolled_entry_file_get(const Evas_Object *obj, const char **file, Elm_Text_Format *format)
{elm_entry_file_get(obj, file, format);}
EINA_DEPRECATED EAPI void
elm_scrolled_entry_file_save(Evas_Object *obj)
{elm_entry_file_save(obj);}
EINA_DEPRECATED EAPI void
elm_scrolled_entry_autosave_set(Evas_Object *obj, Eina_Bool autosave)
{elm_entry_autosave_set(obj, autosave);}
EINA_DEPRECATED EAPI Eina_Bool
elm_scrolled_entry_autosave_get(const Evas_Object *obj)
{return elm_entry_autosave_get(obj);}
EINA_DEPRECATED EAPI void
elm_scrolled_entry_cnp_textonly_set(Evas_Object *obj, Eina_Bool textonly)
{elm_entry_cnp_textonly_set(obj, textonly);}
EINA_DEPRECATED EAPI Eina_Bool
elm_scrolled_entry_cnp_textonly_get(Evas_Object *obj)
{return elm_entry_cnp_textonly_get(obj);}
