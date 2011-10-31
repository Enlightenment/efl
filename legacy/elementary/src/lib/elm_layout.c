#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;
typedef struct _Subinfo Subinfo;
typedef struct _Part_Cursor Part_Cursor;

struct _Widget_Data
{
   Evas_Object *obj;
   Evas_Object *lay;
   Eina_List *subs;
   Eina_List *parts_cursors;
   Eina_Bool needs_size_calc:1;
   const char *clas, *group, *style;
};

struct _Subinfo
{
   const char *part;
   Evas_Object *obj;
   enum {
     SWALLOW,
     BOX_APPEND,
     BOX_PREPEND,
     BOX_INSERT_BEFORE,
     BOX_INSERT_AT,
     TABLE_PACK,
     TEXT
   } type;
   union {
      union {
         const Evas_Object *reference;
         unsigned int pos;
      } box;
      struct {
         unsigned short col, row, colspan, rowspan;
      } table;
      struct {
         const char *text;
      } text;
   } p;
};

struct _Part_Cursor
{
   Evas_Object *obj;
   const char *part;
   const char *cursor;
   const char *style;
   Eina_Bool engine_only:1;
};

static const char *widtype = NULL;
static void _del_hook(Evas_Object *obj);
static void _mirrored_set(Evas_Object *obj, Eina_Bool rtl);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Widget_Data *wd);
static void _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _sub_del(void *data, Evas_Object *obj, void *event_info);
static void _part_cursor_free(Part_Cursor *pc);

static const char SIG_THEME_CHANGED[] = "theme,changed";

static const Evas_Smart_Cb_Description _signals[] = {
   {SIG_THEME_CHANGED, ""},
   {NULL, NULL}
};

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Subinfo *si;
   Part_Cursor *pc;

   if (!wd) return;
   EINA_LIST_FREE(wd->subs, si)
     {
        eina_stringshare_del(si->part);
        if (si->type == TEXT)
          eina_stringshare_del(si->p.text.text);
        free(si);
     }
   EINA_LIST_FREE(wd->parts_cursors, pc) _part_cursor_free(pc);
   free(wd);
}

static void
_mirrored_set(Evas_Object *obj, Eina_Bool rtl)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   edje_object_mirrored_set(wd->lay, rtl);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   _elm_widget_mirrored_reload(obj);
   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   _elm_theme_object_set(obj, wd->lay, wd->clas, wd->group, wd->style);
   edje_object_scale_set(wd->lay, elm_widget_scale_get(obj) *
                         _elm_config->scale);
   evas_object_smart_callback_call(obj, SIG_THEME_CHANGED, NULL);
   _sizing_eval(wd);
}

static void
_changed_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->needs_size_calc)
     {
        _sizing_eval(wd);
        wd->needs_size_calc = 0;
     }
}

static void
_signal_emit_hook(Evas_Object *obj, const char *emission, const char *source)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   edje_object_signal_emit(wd->lay, emission, source);
}

static void
_signal_callback_add_hook(Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func_cb, void *data)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   edje_object_signal_callback_add(wd->lay, emission, source, func_cb, data);
}

static void
_signal_callback_del_hook(Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func_cb, void *data)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   edje_object_signal_callback_del_full(wd->lay, emission, source, func_cb,
                                        data);
}


static void *
_elm_layout_list_data_get(const Eina_List *list)
{
   Subinfo *si = eina_list_data_get(list);
   return si->obj;
}

static Eina_Bool
_elm_layout_focus_next_hook(const Evas_Object *obj, Elm_Focus_Direction dir, Evas_Object **next)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   const Eina_List *items;
   void *(*list_data_get) (const Eina_List *list);

   if ((!wd) || (!wd->subs))
     return EINA_FALSE;

   /* Focus chain (This block is diferent of elm_win cycle)*/
   if ((items = elm_widget_focus_custom_chain_get(obj)))
     list_data_get = eina_list_data_get;
   else
     {
        items = wd->subs;
        list_data_get = _elm_layout_list_data_get;

        if (!items) return EINA_FALSE;
     }

   return elm_widget_focus_list_next_get(obj, items, list_data_get, dir,
                                         next);
}

static void
_sizing_eval(Widget_Data *wd)
{
   Evas_Coord minw = -1, minh = -1;
   edje_object_size_min_calc(wd->lay, &minw, &minh);
   evas_object_size_hint_min_set(wd->obj, minw, minh);
   evas_object_size_hint_max_set(wd->obj, -1, -1);
}

static void
_request_sizing_eval(Widget_Data *wd)
{
   if (wd->needs_size_calc) return;
   wd->needs_size_calc = 1;
   evas_object_smart_changed(wd->obj);
}

static void
_part_cursor_free(Part_Cursor *pc)
{
   eina_stringshare_del(pc->part);
   eina_stringshare_del(pc->style);
   eina_stringshare_del(pc->cursor);
   free(pc);
}

static void
_part_cursor_part_apply(const Part_Cursor *pc)
{
   elm_object_cursor_set(pc->obj, pc->cursor);
   elm_object_cursor_style_set(pc->obj, pc->style);
   elm_object_cursor_engine_only_set(pc->obj, pc->engine_only);
}

static Part_Cursor *
_parts_cursors_find(Widget_Data *wd, const char *part)
{
   const Eina_List *l;
   Part_Cursor *pc;
   EINA_LIST_FOREACH(wd->parts_cursors, l, pc)
     {
        if (!strcmp(pc->part, part))
          return pc;
     }
   return NULL;
}

static void
_parts_cursors_apply(Widget_Data *wd)
{
   const char *file, *group;
   const Eina_List *l;
   Part_Cursor *pc;

   edje_object_file_get(wd->lay, &file, &group);

   EINA_LIST_FOREACH(wd->parts_cursors, l, pc)
     {
        Evas_Object *obj = (Evas_Object *)edje_object_part_object_get
           (wd->lay, pc->part);

        if (!obj)
          {
             pc->obj = NULL;
             WRN("no part '%s' in group '%s' of file '%s'. "
                 "Cannot set cursor '%s'",
                 pc->part, group, file, pc->cursor);
             continue;
          }
        else if (evas_object_pass_events_get(obj))
          {
             pc->obj = NULL;
             WRN("part '%s' in group '%s' of file '%s' has mouse_events: 0. "
                 "Cannot set cursor '%s'",
                 pc->part, group, file, pc->cursor);
             continue;
          }

        pc->obj = obj;
        _part_cursor_part_apply(pc);
     }
}

static void
_changed_size_hints(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _request_sizing_eval(data);
}

static void
_sub_del(void *data __UNUSED__, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *sub = event_info;
   Eina_List *l;
   Subinfo *si;
   if (!wd) return;
   EINA_LIST_FOREACH(wd->subs, l, si)
     {
        if (si->obj == sub)
          {
             evas_object_event_callback_del_full(sub,
                                                 EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                                 _changed_size_hints,
                                                 wd);
             wd->subs = eina_list_remove_list(wd->subs, l);
             eina_stringshare_del(si->part);
             free(si);
             break;
          }
     }
}

static void
_signal_size_eval(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   _request_sizing_eval(data);
}

static void
_parts_text_fix(Widget_Data *wd)
{
   const Eina_List *l;
   Subinfo *si;

   EINA_LIST_FOREACH(wd->subs, l, si)
     {
        if (si->type == TEXT)
          edje_object_part_text_set(wd->lay, si->part, si->p.text.text);
     }
}

static void
_elm_layout_label_set(Evas_Object *obj, const char *part, const char *text)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Subinfo *si = NULL;
   Eina_List *l;
   ELM_CHECK_WIDTYPE(obj, widtype);
   if (!part) part = "elm.text";

   EINA_LIST_FOREACH(wd->subs, l, si)
     {
        if ((si->type == TEXT) && (!strcmp(part, si->part)))
          {
             if (!text)
               {
                  eina_stringshare_del(si->part);
                  eina_stringshare_del(si->p.text.text);
                  free(si);
                  edje_object_part_text_set(wd->lay, part, NULL);
                  wd->subs = eina_list_remove_list(wd->subs, l);
                  return;
               }
             else
               break;
          }
        si = NULL;
     }

   if (!si)
     {
        si = ELM_NEW(Subinfo);
        if (!si) return;
        si->type = TEXT;
        si->part = eina_stringshare_add(part);
        wd->subs = eina_list_append(wd->subs, si);
     }

   eina_stringshare_replace(&si->p.text.text, text);
   edje_object_part_text_set(wd->lay, part, text);
   _request_sizing_eval(wd);
}

static const char *
_elm_layout_label_get(const Evas_Object *obj, const char *part)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!part) part = "elm.text";
   return edje_object_part_text_get(wd->lay, part);
}

static void
_content_set_hook(Evas_Object *obj, const char *part, Evas_Object *content)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   Subinfo *si;
   const Eina_List *l;
   if (!wd) return;
   EINA_LIST_FOREACH(wd->subs, l, si)
     {
        if ((si->type == SWALLOW) && (!strcmp(part, si->part)))
          {
             if (content == si->obj) return;
             evas_object_del(si->obj);
             break;
          }
     }
   if (content)
     {
        elm_widget_sub_object_add(obj, content);
        evas_object_event_callback_add(content,
                                       EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                       _changed_size_hints, wd);
        if (!edje_object_part_swallow(wd->lay, part, content))
          WRN("could not swallow %p into part '%s'", content, part);
        si = ELM_NEW(Subinfo);
        si->type = SWALLOW;
        si->part = eina_stringshare_add(part);
        si->obj = content;
        wd->subs = eina_list_append(wd->subs, si);
     }
   _request_sizing_eval(wd);
}

static Evas_Object *
_content_get_hook(const Evas_Object *obj, const char *part)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   const Eina_List *l;
   Subinfo *si;
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;

   EINA_LIST_FOREACH(wd->subs, l, si)
     {
        if ((si->type == SWALLOW) && !strcmp(part, si->part))
          return si->obj;
     }
   return NULL;
}

static Evas_Object *
_content_unset_hook(Evas_Object *obj, const char *part)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   Subinfo *si;
   const Eina_List *l;
   if (!wd) return NULL;
   EINA_LIST_FOREACH(wd->subs, l, si)
     {
        if ((si->type == SWALLOW) && (!strcmp(part, si->part)))
          {
             Evas_Object *content;
             if (!si->obj) return NULL;
             content = si->obj; /* si will die in _sub_del due elm_widget_sub_object_del() */
             elm_widget_sub_object_del(obj, content);
             evas_object_event_callback_del_full(content,
                                                 EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                                 _changed_size_hints, wd);
             edje_object_part_unswallow(wd->lay, content);
             return content;
          }
     }
   return NULL;
}

EAPI Evas_Object *
elm_layout_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);

   ELM_SET_WIDTYPE(widtype, "layout");
   elm_widget_type_set(obj, "layout");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_changed_hook_set(obj, _changed_hook);
   elm_widget_can_focus_set(obj, EINA_FALSE);
   elm_widget_focus_next_hook_set(obj, _elm_layout_focus_next_hook);
   elm_widget_signal_emit_hook_set(obj, _signal_emit_hook);
   elm_widget_signal_callback_add_hook_set(obj, _signal_callback_add_hook);
   elm_widget_signal_callback_del_hook_set(obj, _signal_callback_del_hook);
   elm_widget_text_set_hook_set(obj, _elm_layout_label_set);
   elm_widget_text_get_hook_set(obj, _elm_layout_label_get);
   elm_widget_content_set_hook_set(obj, _content_set_hook);
   elm_widget_content_get_hook_set(obj, _content_get_hook);
   elm_widget_content_unset_hook_set(obj, _content_unset_hook);

   wd->obj = obj;
   wd->lay = edje_object_add(e);
   elm_widget_resize_object_set(obj, wd->lay);
   edje_object_signal_callback_add(wd->lay, "size,eval", "elm",
                                   _signal_size_eval, wd);

   evas_object_smart_callback_add(obj, "sub-object-del", _sub_del, obj);
   evas_object_smart_callbacks_descriptions_set(obj, _signals);

   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   _request_sizing_eval(wd);
   return obj;
}

EAPI Eina_Bool
elm_layout_file_set(Evas_Object *obj, const char *file, const char *group)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   Eina_Bool ret = edje_object_file_set(wd->lay, file, group);
   if (ret)
     {
        _parts_text_fix(wd);
        _request_sizing_eval(wd);
        _parts_cursors_apply(wd);
     }
   else DBG("failed to set edje file '%s', group '%s': %s",
            file, group,
            edje_load_error_str(edje_object_load_error_get(wd->lay)));
   return ret;
}

EAPI Eina_Bool
elm_layout_theme_set(Evas_Object *obj, const char *clas, const char *group, const char *style)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   Eina_Bool ret = _elm_theme_object_set(obj, wd->lay, clas, group, style);
   wd->clas = clas;
   wd->group = group;
   wd->style = style;
   if (ret)
     {
        _parts_text_fix(wd);
        _request_sizing_eval(wd);
        _parts_cursors_apply(wd);
     }
   return ret;
}

EAPI void
elm_layout_content_set(Evas_Object *obj, const char *swallow, Evas_Object *content)
{
   _content_set_hook(obj, swallow, content);
}


EAPI Evas_Object *
elm_layout_content_get(const Evas_Object *obj, const char *swallow)
{
   return _content_get_hook(obj, swallow);
}

EAPI Evas_Object *
elm_layout_content_unset(Evas_Object *obj, const char *swallow)
{
   return _content_unset_hook(obj, swallow);
}

EAPI void
elm_layout_text_set(Evas_Object *obj, const char *part, const char *text)
{
   _elm_layout_label_set(obj, part, text);
}

EAPI const char *
elm_layout_text_get(const Evas_Object *obj, const char *part)
{
   return _elm_layout_label_get(obj, part);
}

EAPI void
elm_layout_box_append(Evas_Object *obj, const char *part, Evas_Object *child)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   Subinfo *si;
   if (!wd) return;

   if (!edje_object_part_box_append(wd->lay, part, child))
     WRN("child %p could not be appended to box part '%s'", child, part);
   elm_widget_sub_object_add(obj, child);
   evas_object_event_callback_add
      (child, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _changed_size_hints, wd);

   si = ELM_NEW(Subinfo);
   si->type = BOX_APPEND;
   si->part = eina_stringshare_add(part);
   si->obj = child;
   wd->subs = eina_list_append(wd->subs, si);
   _request_sizing_eval(wd);
}

EAPI void
elm_layout_box_prepend(Evas_Object *obj, const char *part, Evas_Object *child)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   Subinfo *si;
   if (!wd) return;

   if (!edje_object_part_box_prepend(wd->lay, part, child))
     WRN("child %p could not be prepended to box part '%s'", child, part);
   elm_widget_sub_object_add(obj, child);
   evas_object_event_callback_add
      (child, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _changed_size_hints, wd);

   si = ELM_NEW(Subinfo);
   si->type = BOX_PREPEND;
   si->part = eina_stringshare_add(part);
   si->obj = child;
   wd->subs = eina_list_prepend(wd->subs, si);
   _request_sizing_eval(wd);
}

static void
_box_reference_del(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Subinfo *si = data;
   si->p.box.reference = NULL;
}

EAPI void
elm_layout_box_insert_before(Evas_Object *obj, const char *part, Evas_Object *child, const Evas_Object *reference)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   Subinfo *si;
   if (!wd) return;

   if (!edje_object_part_box_insert_before(wd->lay, part, child, reference))
     WRN("child %p could not be inserted before %p inf box part '%s'",
         child, reference, part);

   si = ELM_NEW(Subinfo);
   si->type = BOX_INSERT_BEFORE;
   si->part = eina_stringshare_add(part);
   si->obj = child;
   si->p.box.reference = reference;

   elm_widget_sub_object_add(obj, child);
   evas_object_event_callback_add
      (child, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _changed_size_hints, wd);
   evas_object_event_callback_add
      ((Evas_Object *)reference, EVAS_CALLBACK_DEL, _box_reference_del, si);

   wd->subs = eina_list_append(wd->subs, si);
   _request_sizing_eval(wd);
}

EAPI void
elm_layout_box_insert_at(Evas_Object *obj, const char *part, Evas_Object *child, unsigned int pos)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   Subinfo *si;
   if (!wd) return;

   if (!edje_object_part_box_insert_at(wd->lay, part, child, pos))
     WRN("child %p could not be inserted at %u to box part '%s'",
         child, pos, part);

   elm_widget_sub_object_add(obj, child);
   evas_object_event_callback_add
      (child, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _changed_size_hints, wd);

   si = ELM_NEW(Subinfo);
   si->type = BOX_INSERT_AT;
   si->part = eina_stringshare_add(part);
   si->obj = child;
   si->p.box.pos = pos;
   wd->subs = eina_list_append(wd->subs, si);
   _request_sizing_eval(wd);
}

static Evas_Object *
_sub_box_remove(Widget_Data *wd, Subinfo *si)
{
   Evas_Object *child;

   if (si->type == BOX_INSERT_BEFORE)
     evas_object_event_callback_del_full
        ((Evas_Object *)si->p.box.reference,
         EVAS_CALLBACK_DEL, _box_reference_del, si);

   child = si->obj; /* si will die in _sub_del due elm_widget_sub_object_del() */
   edje_object_part_box_remove(wd->lay, si->part, child);
   elm_widget_sub_object_del(wd->obj, child);
   return child;
}

static Evas_Object *
_sub_table_remove(Widget_Data *wd, Subinfo *si)
{
   Evas_Object *child;

   child = si->obj; /* si will die in _sub_del due elm_widget_sub_object_del() */
   edje_object_part_table_unpack(wd->lay, si->part, child);
   elm_widget_sub_object_del(wd->obj, child);
   return child;
}

static Eina_Bool
_sub_box_is(const Subinfo *si)
{
   switch (si->type)
     {
      case BOX_APPEND:
      case BOX_PREPEND:
      case BOX_INSERT_BEFORE:
      case BOX_INSERT_AT:
         return EINA_TRUE;
      default:
         return EINA_FALSE;
     }
}

EAPI Evas_Object *
elm_layout_box_remove(Evas_Object *obj, const char *part, Evas_Object *child)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   const Eina_List *l;
   Subinfo *si;

   if (!wd) return NULL;

   EINA_SAFETY_ON_NULL_RETURN_VAL(part, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(child, NULL);
   EINA_LIST_FOREACH(wd->subs, l, si)
     {
        if (!_sub_box_is(si)) continue;
        if ((si->obj == child) && (!strcmp(si->part, part)))
          return _sub_box_remove(wd, si);
     }
   return NULL;
}

EAPI void
elm_layout_box_remove_all(Evas_Object *obj, const char *part, Eina_Bool clear)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   Subinfo *si;
   Eina_List *lst;

   if (!wd) return;
   EINA_SAFETY_ON_NULL_RETURN(part);

   lst = eina_list_clone(wd->subs);
   EINA_LIST_FREE(lst, si)
     {
        if (!_sub_box_is(si)) continue;
        if (!strcmp(si->part, part))
          {
             Evas_Object *child = _sub_box_remove(wd, si);
             if ((clear) && (child)) evas_object_del(child);
          }
     }
   /* eventually something may not be added with layout, del them as well */
   edje_object_part_box_remove_all(wd->lay, part, clear);
}

EAPI void
elm_layout_table_pack(Evas_Object *obj, const char *part, Evas_Object *child, unsigned short col, unsigned short row, unsigned short colspan, unsigned short rowspan)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   Subinfo *si;
   if (!wd) return;

   if (!edje_object_part_table_pack
       (wd->lay, part, child, col, row, colspan, rowspan))
     WRN("child %p could not be packed into box part '%s' col=%uh, row=%hu, "
         "colspan=%hu, rowspan=%hu", child, part, col, row, colspan, rowspan);

   elm_widget_sub_object_add(obj, child);
   evas_object_event_callback_add
      (child, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _changed_size_hints, wd);

   si = ELM_NEW(Subinfo);
   si->type = TABLE_PACK;
   si->part = eina_stringshare_add(part);
   si->obj = child;
   si->p.table.col = col;
   si->p.table.row = row;
   si->p.table.colspan = colspan;
   si->p.table.rowspan = rowspan;
   wd->subs = eina_list_append(wd->subs, si);
   _request_sizing_eval(wd);
}

EAPI Evas_Object *
elm_layout_table_unpack(Evas_Object *obj, const char *part, Evas_Object *child)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   const Eina_List *l;
   Subinfo *si;

   if (!wd) return NULL;

   EINA_SAFETY_ON_NULL_RETURN_VAL(part, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(child, NULL);
   EINA_LIST_FOREACH(wd->subs, l, si)
     {
        if (si->type != TABLE_PACK) continue;
        if ((si->obj == child) && (!strcmp(si->part, part)))
          return _sub_table_remove(wd, si);
     }
   return NULL;
}

EAPI void
elm_layout_table_clear(Evas_Object *obj, const char *part, Eina_Bool clear)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   Subinfo *si;
   Eina_List *lst;

   if (!wd) return;
   EINA_SAFETY_ON_NULL_RETURN(part);

   lst = eina_list_clone(wd->subs);
   EINA_LIST_FREE(lst, si)
     {
        if (si->type != TABLE_PACK) continue;
        if (!strcmp(si->part, part))
          {
             Evas_Object *child = _sub_table_remove(wd, si);
             if ((clear) && (child)) evas_object_del(child);
          }
     }
   /* eventually something may not be added with layout, del them as well */
   edje_object_part_table_clear(wd->lay, part, clear);
}

EAPI Evas_Object *
elm_layout_edje_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->lay;
}

EAPI const char *
elm_layout_data_get(const Evas_Object *obj, const char *key)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   return edje_object_data_get(wd->lay, key);
}

EAPI void
elm_layout_sizing_eval(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   EINA_SAFETY_ON_NULL_RETURN(wd);
   _request_sizing_eval(wd);
}

EAPI Eina_Bool
elm_layout_part_cursor_set(Evas_Object *obj, const char *part_name, const char *cursor)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   EINA_SAFETY_ON_NULL_RETURN_VAL(part_name, EINA_FALSE);
   Widget_Data *wd = elm_widget_data_get(obj);
   EINA_SAFETY_ON_NULL_RETURN_VAL(wd, EINA_FALSE);
   Evas_Object *part_obj;
   Part_Cursor *pc;

   part_obj = (Evas_Object *)edje_object_part_object_get(wd->lay, part_name);
   if (!part_obj)
     {
        const char *group, *file;
        edje_object_file_get(wd->lay, &file, &group);
        WRN("no part '%s' in group '%s' of file '%s'. Cannot set cursor '%s'",
            part_name, group, file, cursor);
        return EINA_FALSE;
     }
   if (evas_object_pass_events_get(part_obj))
     {
        const char *group, *file;
        edje_object_file_get(wd->lay, &file, &group);
        WRN("part '%s' in group '%s' of file '%s' has mouse_events: 0. "
            "Cannot set cursor '%s'",
            part_name, group, file, cursor);
        return EINA_FALSE;
     }

   pc = _parts_cursors_find(wd, part_name);
   if (pc) eina_stringshare_replace(&pc->cursor, cursor);
   else
     {
        pc = calloc(1, sizeof(*pc));
        pc->part = eina_stringshare_add(part_name);
        pc->cursor = eina_stringshare_add(cursor);
     }

   pc->obj = part_obj;
   elm_object_sub_cursor_set(part_obj, obj, pc->cursor);
   return EINA_TRUE;
}

EAPI const char *
elm_layout_part_cursor_get(const Evas_Object *obj, const char *part_name)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   EINA_SAFETY_ON_NULL_RETURN_VAL(part_name, NULL);
   Widget_Data *wd = elm_widget_data_get(obj);
   EINA_SAFETY_ON_NULL_RETURN_VAL(wd, NULL);
   Part_Cursor *pc = _parts_cursors_find(wd, part_name);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pc, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pc->obj, NULL);
   return elm_object_cursor_get(pc->obj);
}

EAPI void
elm_layout_part_cursor_unset(Evas_Object *obj, const char *part_name)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   EINA_SAFETY_ON_NULL_RETURN(part_name);
   Widget_Data *wd = elm_widget_data_get(obj);
   EINA_SAFETY_ON_NULL_RETURN(wd);
   Eina_List *l;
   Part_Cursor *pc;

   EINA_LIST_FOREACH(wd->parts_cursors, l, pc)
     {
        if (!strcmp(part_name, pc->part))
          {
             if (pc->obj) elm_object_cursor_unset(pc->obj);
             _part_cursor_free(pc);
             wd->parts_cursors = eina_list_remove_list(wd->parts_cursors, l);
             return;
          }
     }
}

EAPI Eina_Bool
elm_layout_part_cursor_style_set(Evas_Object *obj, const char *part_name, const char *style)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   EINA_SAFETY_ON_NULL_RETURN_VAL(part_name, EINA_FALSE);
   Widget_Data *wd = elm_widget_data_get(obj);
   EINA_SAFETY_ON_NULL_RETURN_VAL(wd, EINA_FALSE);
   Part_Cursor *pc = _parts_cursors_find(wd, part_name);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pc, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pc->obj, EINA_FALSE);

   eina_stringshare_replace(&pc->style, style);
   elm_object_cursor_style_set(pc->obj, pc->style);
   return EINA_TRUE;
}

EAPI const char *
elm_layout_part_cursor_style_get(const Evas_Object *obj, const char *part_name)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   EINA_SAFETY_ON_NULL_RETURN_VAL(part_name, NULL);
   Widget_Data *wd = elm_widget_data_get(obj);
   EINA_SAFETY_ON_NULL_RETURN_VAL(wd, NULL);
   Part_Cursor *pc = _parts_cursors_find(wd, part_name);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pc, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pc->obj, NULL);
   return elm_object_cursor_style_get(pc->obj);
}

EAPI Eina_Bool
elm_layout_part_cursor_engine_only_set(Evas_Object *obj, const char *part_name, Eina_Bool engine_only)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   EINA_SAFETY_ON_NULL_RETURN_VAL(part_name, EINA_FALSE);
   Widget_Data *wd = elm_widget_data_get(obj);
   EINA_SAFETY_ON_NULL_RETURN_VAL(wd, EINA_FALSE);
   Part_Cursor *pc = _parts_cursors_find(wd, part_name);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pc, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pc->obj, EINA_FALSE);

   pc->engine_only = !!engine_only;
   elm_object_cursor_engine_only_set(pc->obj, pc->engine_only);
   return EINA_TRUE;
}

EAPI Eina_Bool
elm_layout_part_cursor_engine_only_get(const Evas_Object *obj, const char *part_name)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   EINA_SAFETY_ON_NULL_RETURN_VAL(part_name, EINA_FALSE);
   Widget_Data *wd = elm_widget_data_get(obj);
   EINA_SAFETY_ON_NULL_RETURN_VAL(wd, EINA_FALSE);
   Part_Cursor *pc = _parts_cursors_find(wd, part_name);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pc, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pc->obj, EINA_FALSE);
   return elm_object_cursor_engine_only_get(pc->obj);
}
