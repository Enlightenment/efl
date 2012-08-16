#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_layout.h"

EAPI const char ELM_LAYOUT_SMART_NAME[] = "elm_layout";

static const char SIG_THEME_CHANGED[] = "theme,changed";

/* no *direct* instantiation of this class, so far */
__UNUSED__ static Evas_Smart *_elm_layout_smart_class_new(void);

/* smart callbacks coming from elm layout objects: */
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_THEME_CHANGED, ""},
   {NULL, NULL}
};

/* these are data operated by layout's class functions internally, and
 * should not be messed up by inhering classes */
typedef struct _Elm_Layout_Sub_Object_Data   Elm_Layout_Sub_Object_Data;
typedef struct _Elm_Layout_Sub_Object_Cursor Elm_Layout_Sub_Object_Cursor;

struct _Elm_Layout_Sub_Object_Data
{
   const char  *part;
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
         unsigned int       pos;
      } box;
      struct
      {
         unsigned short col, row, colspan, rowspan;
      } table;
      struct
      {
         const char *text;
      } text;
   } p;
};

struct _Elm_Layout_Sub_Object_Cursor
{
   Evas_Object *obj;
   const char  *part;
   const char  *cursor;
   const char  *style;

   Eina_Bool    engine_only : 1;
};

/* layout's sizing evaluation is deferred. evaluation requests are
 * queued up and only flag the object as 'changed'. when it comes to
 * Evas's rendering phase, it will be addressed, finally (see
 * _elm_layout_smart_calculate()). */
static void
_elm_layout_smart_sizing_eval(Evas_Object *obj)
{
   ELM_LAYOUT_DATA_GET(obj, sd);

   if (sd->needs_size_calc) return;
   sd->needs_size_calc = EINA_TRUE;

   evas_object_smart_changed(obj);
}

static void
_on_sub_object_size_hint_change(void *data,
                                Evas *e __UNUSED__,
                                Evas_Object *obj __UNUSED__,
                                void *event_info __UNUSED__)
{
   ELM_LAYOUT_DATA_GET(data, sd);
   if (ELM_WIDGET_DATA(sd)->frozen) return;
   ELM_LAYOUT_CLASS(ELM_WIDGET_DATA(sd)->api)->sizing_eval(data);
}

static void
_part_cursor_free(Elm_Layout_Sub_Object_Cursor *pc)
{
   eina_stringshare_del(pc->part);
   eina_stringshare_del(pc->style);
   eina_stringshare_del(pc->cursor);

   free(pc);
}

/* Elementary smart class for all widgets having an Edje layout as a
 * building block */
EVAS_SMART_SUBCLASS_NEW
  (ELM_LAYOUT_SMART_NAME, _elm_layout, Elm_Layout_Smart_Class,
  Elm_Container_Smart_Class, elm_container_smart_class_get, _smart_callbacks);

EAPI const Elm_Layout_Smart_Class *
elm_layout_smart_class_get(void)
{
   static Elm_Layout_Smart_Class _sc =
     ELM_LAYOUT_SMART_CLASS_INIT_NAME_VERSION(ELM_LAYOUT_SMART_NAME);
   static const Elm_Layout_Smart_Class *class = NULL;
   Evas_Smart_Class *esc = (Evas_Smart_Class *)&_sc;

   if (class)
     return class;

   _elm_layout_smart_set(&_sc);
   esc->callbacks = _smart_callbacks;
   class = &_sc;

   return class;
}

static void
_sizing_eval(Evas_Object *obj, Elm_Layout_Smart_Data *sd)
{
   Evas_Coord minw = -1, minh = -1;

   edje_object_size_min_calc(ELM_WIDGET_DATA(sd)->resize_obj, &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, -1, -1);
}

/* common content cases for layout objects: icon and text */
static inline void
_icon_signal_emit(Elm_Layout_Smart_Data *sd,
                  Elm_Layout_Sub_Object_Data *sub_d,
                  Eina_Bool visible)
{
   char buf[1024];
   const char *type;

   if (sub_d->type != SWALLOW ||
       (strcmp("elm.swallow.icon", sub_d->part) &&
        (strcmp("elm.swallow.end", sub_d->part))))
     return;

   if (strncmp(sub_d->part, "elm.swallow.", sizeof("elm.swallow.") - 1) == 0)
     type = sub_d->part + sizeof("elm.swallow.") - 1;
   else
     type = sub_d->part;

   snprintf(buf, sizeof(buf), "elm,state,%s,%s", type,
            visible ? "visible" : "hidden");

   edje_object_signal_emit(ELM_WIDGET_DATA(sd)->resize_obj, buf, "elm");

   /* themes might need imediate action here */
   edje_object_message_signal_process(ELM_WIDGET_DATA(sd)->resize_obj);
}

static inline void
_text_signal_emit(Elm_Layout_Smart_Data *sd,
                  Elm_Layout_Sub_Object_Data *sub_d,
                  Eina_Bool visible)
{
   char buf[1024];
   const char *type;

   if (sub_d->type != TEXT || strcmp("elm.text", sub_d->part))
     return;

   if (strncmp(sub_d->part, "elm.text.", sizeof("elm.text.") - 1) == 0)
     type = sub_d->part + sizeof("elm.text.") - 1;
   else
     type = sub_d->part;

   snprintf(buf, sizeof(buf), "elm,state,%s,%s", type,
            visible ? "visible" : "hidden");
   edje_object_signal_emit(ELM_WIDGET_DATA(sd)->resize_obj, buf, "elm");

   /* TODO: is this right? It was like that, but IMO it should be removed: */
   snprintf(buf, sizeof(buf),
            visible ? "elm,state,text,visible" : "elm,state,text,hidden");

   edje_object_signal_emit(ELM_WIDGET_DATA(sd)->resize_obj, buf, "elm");

   /* themes might need imediate action here */
   edje_object_message_signal_process(ELM_WIDGET_DATA(sd)->resize_obj);
}

static void
_parts_signals_emit(Elm_Layout_Smart_Data *sd)
{
   const Eina_List *l;
   Elm_Layout_Sub_Object_Data *sub_d;

   EINA_LIST_FOREACH (sd->subs, l, sub_d)
     {
        _icon_signal_emit(sd, sub_d, EINA_TRUE);
        _text_signal_emit(sd, sub_d, EINA_TRUE);
     }
}

static void
_parts_text_fix(Elm_Layout_Smart_Data *sd)
{
   const Eina_List *l;
   Elm_Layout_Sub_Object_Data *sub_d;

   EINA_LIST_FOREACH (sd->subs, l, sub_d)
     {
        if (sub_d->type == TEXT)
          {
             edje_object_part_text_escaped_set
               (ELM_WIDGET_DATA(sd)->resize_obj, sub_d->part,
               sub_d->p.text.text);
          }
     }
}

static void
_part_cursor_part_apply(const Elm_Layout_Sub_Object_Cursor *pc)
{
   elm_object_cursor_set(pc->obj, pc->cursor);
   elm_object_cursor_style_set(pc->obj, pc->style);
   elm_object_cursor_theme_search_enabled_set(pc->obj, pc->engine_only);
}

static void
_parts_cursors_apply(Elm_Layout_Smart_Data *sd)
{
   const Eina_List *l;
   const char *file, *group;
   Elm_Layout_Sub_Object_Cursor *pc;

   edje_object_file_get(ELM_WIDGET_DATA(sd)->resize_obj, &file, &group);

   EINA_LIST_FOREACH (sd->parts_cursors, l, pc)
     {
        Evas_Object *obj = (Evas_Object *)edje_object_part_object_get
            (ELM_WIDGET_DATA(sd)->resize_obj, pc->part);

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
_reload_theme(void *data, Evas_Object *obj,
              const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Evas_Object *layout = data;
   const char *file;
   const char *group;

   edje_object_file_get(obj, &file, &group);
   elm_layout_file_set(layout, file, group);
}

static void
_visuals_refresh(Evas_Object *obj,
                 Elm_Layout_Smart_Data *sd)
{
   _parts_text_fix(sd);
   _parts_signals_emit(sd);
   _parts_cursors_apply(sd);

   ELM_LAYOUT_CLASS(ELM_WIDGET_DATA(sd)->api)->sizing_eval(obj);

   edje_object_signal_callback_del(ELM_WIDGET_DATA(sd)->resize_obj,
                                   "edje,change,file", "edje",
                                   _reload_theme);
   edje_object_signal_callback_add(ELM_WIDGET_DATA(sd)->resize_obj,
                                   "edje,change,file", "edje",
                                   _reload_theme, obj);
}

static Eina_Bool
_elm_layout_smart_disable(Evas_Object *obj)
{
   ELM_LAYOUT_DATA_GET(obj, sd);

   if (elm_object_disabled_get(obj))
     edje_object_signal_emit
       (ELM_WIDGET_DATA(sd)->resize_obj, "elm,state,disabled", "elm");
   else
     edje_object_signal_emit
       (ELM_WIDGET_DATA(sd)->resize_obj, "elm,state,enabled", "elm");

   return EINA_TRUE;
}

static Eina_Bool
_elm_layout_smart_theme(Evas_Object *obj)
{
   Eina_Bool ret;
   const char *fh;

   ELM_LAYOUT_DATA_GET(obj, sd);

   if (!ELM_WIDGET_CLASS(_elm_layout_parent_sc)->theme(obj)) return EINA_FALSE;

   /* function already prints error messages, if any */
   ret = elm_widget_theme_object_set
       (obj, ELM_WIDGET_DATA(sd)->resize_obj, sd->klass, sd->group,
       elm_widget_style_get(obj));

   edje_object_mirrored_set
     (ELM_WIDGET_DATA(sd)->resize_obj, elm_widget_mirrored_get(obj));

   edje_object_scale_set
     (ELM_WIDGET_DATA(sd)->resize_obj,
     elm_widget_scale_get(obj) * elm_config_scale_get());

   fh = edje_object_data_get
       (ELM_WIDGET_DATA(sd)->resize_obj, "focus_highlight");
   if ((fh) && (!strcmp(fh, "on")))
     elm_widget_highlight_in_theme_set(obj, EINA_TRUE);
   else
     elm_widget_highlight_in_theme_set(obj, EINA_FALSE);

   evas_object_smart_callback_call(obj, SIG_THEME_CHANGED, NULL);

   _visuals_refresh(obj, sd);

   return ret;
}

static void *
_elm_layout_list_data_get(const Eina_List *list)
{
   Elm_Layout_Sub_Object_Data *sub_d = eina_list_data_get(list);

   return sub_d->obj;
}

static Eina_Bool
_elm_layout_smart_on_focus(Evas_Object *obj)
{
   ELM_LAYOUT_DATA_GET(obj, sd);

   if (elm_widget_focus_get(obj))
     {
        elm_layout_signal_emit(obj, "elm,action,focus", "elm");
        evas_object_focus_set(ELM_WIDGET_DATA(sd)->resize_obj, EINA_TRUE);
     }
   else
     {
        elm_layout_signal_emit(obj, "elm,action,unfocus", "elm");
        evas_object_focus_set(ELM_WIDGET_DATA(sd)->resize_obj, EINA_FALSE);
     }

   return EINA_TRUE;
}

/* WARNING: if you're making a widget *not* supposed to have focusable
 * child objects, but still inheriting from elm_layout, just set its
 * focus_next smart function back to NULL */
static Eina_Bool
_elm_layout_smart_focus_next(const Evas_Object *obj,
                             Elm_Focus_Direction dir,
                             Evas_Object **next)
{
   const Eina_List *items;
   void *(*list_data_get)(const Eina_List *list);

   ELM_LAYOUT_DATA_GET(obj, sd);

   if (!sd->subs) return EINA_FALSE;

   if ((items = elm_widget_focus_custom_chain_get(obj)))
     list_data_get = eina_list_data_get;
   else
     {
        items = sd->subs;
        list_data_get = _elm_layout_list_data_get;
     }

   return elm_widget_focus_list_next_get
            (obj, items, list_data_get, dir, next);
}

static Eina_Bool
_elm_layout_smart_sub_object_add(Evas_Object *obj,
                                 Evas_Object *sobj)
{
   if (!ELM_WIDGET_CLASS(_elm_layout_parent_sc)->sub_object_add(obj, sobj))
     return EINA_FALSE;

   evas_object_event_callback_add
     (sobj, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
     _on_sub_object_size_hint_change, obj);

   return EINA_TRUE;
}

static Eina_Bool
_elm_layout_smart_sub_object_del(Evas_Object *obj,
                                 Evas_Object *sobj)
{
   Eina_List *l;
   Elm_Layout_Sub_Object_Data *sub_d;

   ELM_LAYOUT_DATA_GET(obj, sd);

   evas_object_event_callback_del_full
     (sobj, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
     _on_sub_object_size_hint_change, obj);

   if (!ELM_WIDGET_CLASS(_elm_layout_parent_sc)->sub_object_del(obj, sobj))
     return EINA_FALSE;

   EINA_LIST_FOREACH (sd->subs, l, sub_d)
     {
        if (sub_d->obj != sobj) continue;

        sd->subs = eina_list_remove_list(sd->subs, l);

        _icon_signal_emit(sd, sub_d, EINA_FALSE);

        eina_stringshare_del(sub_d->part);
        free(sub_d);

        break;
     }

   if (ELM_WIDGET_DATA(sd)->frozen) return EINA_TRUE;
   ELM_LAYOUT_CLASS(ELM_WIDGET_DATA(sd)->api)->sizing_eval(obj);

   return EINA_TRUE;
}

static Eina_Bool
_elm_layout_smart_focus_direction(const Evas_Object *obj,
                                  const Evas_Object *base,
                                  double degree,
                                  Evas_Object **direction,
                                  double *weight)
{
   const Eina_List *items;
   void *(*list_data_get)(const Eina_List *list);

   ELM_LAYOUT_DATA_GET(obj, sd);

   if (!sd->subs) return EINA_FALSE;

   /* Focus chain (This block is diferent of elm_win cycle) */
   if ((items = elm_widget_focus_custom_chain_get(obj)))
     list_data_get = eina_list_data_get;
   else
     {
        items = sd->subs;
        list_data_get = _elm_layout_list_data_get;

        if (!items) return EINA_FALSE;
     }

   return elm_widget_focus_list_direction_get
            (obj, base, items, list_data_get, degree, direction, weight);
}

static void
_elm_layout_smart_signal(Evas_Object *obj,
                         const char *emission,
                         const char *source)
{
   ELM_LAYOUT_DATA_GET(obj, sd);

   edje_object_signal_emit(ELM_WIDGET_DATA(sd)->resize_obj, emission, source);
}

static void
_edje_signal_callback(void *data,
                      Evas_Object *obj __UNUSED__,
                      const char *emission,
                      const char *source)
{
   Edje_Signal_Data *esd = data;

   esd->func(esd->data, esd->obj, emission, source);
}

static void
_elm_layout_smart_callback_add(Evas_Object *obj,
                               const char *emission,
                               const char *source,
                               Edje_Signal_Cb func_cb,
                               void *data)
{
   Edje_Signal_Data *esd;

   ELM_LAYOUT_DATA_GET(obj, sd);

   esd = ELM_NEW(Edje_Signal_Data);
   if (!esd) return;

   esd->obj = obj;
   esd->func = func_cb;
   esd->emission = eina_stringshare_add(emission);
   esd->source = eina_stringshare_add(source);
   esd->data = data;
   sd->edje_signals = eina_list_append(sd->edje_signals, esd);

   edje_object_signal_callback_add
     (ELM_WIDGET_DATA(sd)->resize_obj, emission, source,
     _edje_signal_callback, esd);
}

static void *
_elm_layout_smart_callback_del(Evas_Object *obj,
                               const char *emission,
                               const char *source,
                               Edje_Signal_Cb func_cb)
{
   Edje_Signal_Data *esd = NULL;
   void *data = NULL;
   Eina_List *l;

   ELM_LAYOUT_DATA_GET(obj, sd);

   EINA_LIST_FOREACH (sd->edje_signals, l, esd)
     {
        if ((esd->func == func_cb) && (!strcmp(esd->emission, emission)) &&
            (!strcmp(esd->source, source)))
          {
             sd->edje_signals = eina_list_remove_list(sd->edje_signals, l);
             eina_stringshare_del(esd->emission);
             eina_stringshare_del(esd->source);
             data = esd->data;
             free(esd);

             edje_object_signal_callback_del_full
               (ELM_WIDGET_DATA(sd)->resize_obj, emission, source,
               _edje_signal_callback, esd);

             return data; /* stop at 1st match */
          }
     }

   return data;
}

static Eina_Bool
_elm_layout_part_aliasing_eval(Elm_Layout_Smart_Data *sd,
                               const char **part,
                               Eina_Bool is_text)
{
#define ALIAS_LIST(_sd, _list) \
  ((ELM_LAYOUT_CLASS(ELM_WIDGET_DATA(_sd)->api))->_list)

   const Elm_Layout_Part_Alias_Description *aliases = is_text ?
     ALIAS_LIST(sd, text_aliases) : ALIAS_LIST(sd, content_aliases);

#undef ALIAS_LIST

   if (!aliases) return EINA_TRUE;

   while (aliases->alias && aliases->real_part)
     {
        /* NULL matches the 1st */
        if ((!*part) || (!strcmp(*part, aliases->alias)))
          {
             *part = aliases->real_part;
             break;
          }

        aliases++;
     }

   if (!*part)
     {
        ERR("no default content part set for object %p -- "
            "part must not be NULL", ELM_WIDGET_DATA(sd)->obj);
        return EINA_FALSE;
     }

   /* if no match, part goes on with the same value */

   return EINA_TRUE;
}

static Eina_Bool
_elm_layout_smart_text_set(Evas_Object *obj,
                           const char *part,
                           const char *text)
{
   Eina_List *l;
   Elm_Layout_Sub_Object_Data *sub_d = NULL;

   ELM_LAYOUT_DATA_GET(obj, sd);

   if (!_elm_layout_part_aliasing_eval(sd, &part, EINA_TRUE))
     return EINA_FALSE;

   EINA_LIST_FOREACH (sd->subs, l, sub_d)
     {
        if ((sub_d->type == TEXT) && (!strcmp(part, sub_d->part)))
          {
             if (!text)
               {
                  eina_stringshare_del(sub_d->part);
                  eina_stringshare_del(sub_d->p.text.text);
                  free(sub_d);
                  edje_object_part_text_escaped_set
                    (ELM_WIDGET_DATA(sd)->resize_obj, part, NULL);
                  sd->subs = eina_list_remove_list(sd->subs, l);
                  return EINA_TRUE;
               }
             else
               break;
          }
        sub_d = NULL;
     }

   if (!edje_object_part_text_escaped_set
         (ELM_WIDGET_DATA(sd)->resize_obj, part, text))
     return EINA_FALSE;

   if (!sub_d)
     {
        sub_d = ELM_NEW(Elm_Layout_Sub_Object_Data);
        if (!sub_d) return EINA_FALSE;
        sub_d->type = TEXT;
        sub_d->part = eina_stringshare_add(part);
        sd->subs = eina_list_append(sd->subs, sub_d);
     }

   eina_stringshare_replace(&sub_d->p.text.text, text);

   _text_signal_emit(sd, sub_d, !!text);

   if (!ELM_WIDGET_DATA(sd)->frozen)
     {
        ELM_LAYOUT_CLASS(ELM_WIDGET_DATA(sd)->api)->sizing_eval(obj);
     }

   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON &&
       ELM_WIDGET_DATA(sd)->can_access && !(sub_d->obj))
     sub_d->obj = _elm_access_edje_object_part_object_register
         (obj, elm_layout_edje_get(obj), part);

   return EINA_TRUE;
}

static const char *
_elm_layout_smart_text_get(const Evas_Object *obj,
                           const char *part)
{
   ELM_LAYOUT_DATA_GET(obj, sd);

   if (!_elm_layout_part_aliasing_eval(sd, &part, EINA_TRUE))
     return EINA_FALSE;

   return edje_object_part_text_get(ELM_WIDGET_DATA(sd)->resize_obj, part);
}

static Eina_Bool
_elm_layout_smart_content_set(Evas_Object *obj,
                              const char *part,
                              Evas_Object *content)
{
   Elm_Layout_Sub_Object_Data *sub_d;
   const Eina_List *l;

   ELM_LAYOUT_DATA_GET(obj, sd);

   if (!_elm_layout_part_aliasing_eval(sd, &part, EINA_FALSE))
     return EINA_FALSE;

   EINA_LIST_FOREACH (sd->subs, l, sub_d)
     {
        if ((sub_d->type == SWALLOW))
          {
             if (!strcmp(part, sub_d->part))
               {
                  if (content == sub_d->obj) return EINA_TRUE;
                  evas_object_del(sub_d->obj);
                  break;
               }
             /* was previously swallowed at another part -- mimic
              * edje_object_part_swallow()'s behavior, then */
             else if (content == sub_d->obj)
               {
                  elm_widget_sub_object_del(obj, content);
                  break;
               }
          }
     }

   if (content)
     {
        if (!elm_widget_sub_object_add(obj, content))
          {
             ERR("could not add %p as sub object of %p", content, obj);
             return EINA_FALSE;
          }

        if (!edje_object_part_swallow
              (ELM_WIDGET_DATA(sd)->resize_obj, part, content))
          {
             ERR("could not swallow %p into part '%s'", content, part);
             return EINA_FALSE;
          }

        sub_d = ELM_NEW(Elm_Layout_Sub_Object_Data);
        sub_d->type = SWALLOW;
        sub_d->part = eina_stringshare_add(part);
        sub_d->obj = content;
        sd->subs = eina_list_append(sd->subs, sub_d);

        _icon_signal_emit(sd, sub_d, EINA_TRUE);
     }

   if (ELM_WIDGET_DATA(sd)->frozen) return EINA_TRUE;
   ELM_LAYOUT_CLASS(ELM_WIDGET_DATA(sd)->api)->sizing_eval(obj);

   return EINA_TRUE;
}

static Evas_Object *
_elm_layout_smart_content_get(const Evas_Object *obj,
                              const char *part)
{
   const Eina_List *l;
   Elm_Layout_Sub_Object_Data *sub_d;

   ELM_LAYOUT_DATA_GET(obj, sd);

   if (!_elm_layout_part_aliasing_eval(sd, &part, EINA_FALSE))
     return EINA_FALSE;

   EINA_LIST_FOREACH (sd->subs, l, sub_d)
     {
        if ((sub_d->type == SWALLOW) && !strcmp(part, sub_d->part))
          return sub_d->obj;
     }
   return NULL;
}

static Evas_Object *
_elm_layout_smart_content_unset(Evas_Object *obj,
                                const char *part)
{
   Elm_Layout_Sub_Object_Data *sub_d;
   const Eina_List *l;

   ELM_LAYOUT_DATA_GET(obj, sd);

   if (!_elm_layout_part_aliasing_eval(sd, &part, EINA_FALSE))
     return EINA_FALSE;

   EINA_LIST_FOREACH (sd->subs, l, sub_d)
     {
        if ((sub_d->type == SWALLOW) && (!strcmp(part, sub_d->part)))
          {
             Evas_Object *content;

             if (!sub_d->obj) return NULL;

             content = sub_d->obj; /* sub_d will die in
                                    * _elm_layout_smart_sub_object_del */

             if (!elm_widget_sub_object_del(obj, content))
               {
                  ERR("could not remove sub object %p from %p", content, obj);
                  return NULL;
               }

             edje_object_part_unswallow
               (ELM_WIDGET_DATA(sd)->resize_obj, content);
             return content;
          }
     }

   return NULL;
}

static Eina_Bool
_elm_layout_smart_box_append(Evas_Object *obj,
                             const char *part,
                             Evas_Object *child)
{
   Elm_Layout_Sub_Object_Data *sub_d;

   ELM_LAYOUT_DATA_GET(obj, sd);

   if (!edje_object_part_box_append
         (ELM_WIDGET_DATA(sd)->resize_obj, part, child))
     {
        ERR("child %p could not be appended to box part '%s'", child, part);
        return EINA_FALSE;
     }

   if (!elm_widget_sub_object_add(obj, child))
     {
        ERR("could not add %p as sub object of %p", child, obj);
        edje_object_part_box_remove
          (ELM_WIDGET_DATA(sd)->resize_obj, part, child);
        return EINA_FALSE;
     }

   sub_d = ELM_NEW(Elm_Layout_Sub_Object_Data);
   sub_d->type = BOX_APPEND;
   sub_d->part = eina_stringshare_add(part);
   sub_d->obj = child;
   sd->subs = eina_list_append(sd->subs, sub_d);

   if (ELM_WIDGET_DATA(sd)->frozen) return EINA_TRUE;
   ELM_LAYOUT_CLASS(ELM_WIDGET_DATA(sd)->api)->sizing_eval(obj);

   return EINA_TRUE;
}

static Eina_Bool
_elm_layout_smart_box_prepend(Evas_Object *obj,
                              const char *part,
                              Evas_Object *child)
{
   Elm_Layout_Sub_Object_Data *sub_d;

   ELM_LAYOUT_DATA_GET(obj, sd);

   if (!edje_object_part_box_prepend
         (ELM_WIDGET_DATA(sd)->resize_obj, part, child))
     {
        ERR("child %p could not be prepended to box part '%s'", child, part);
        return EINA_FALSE;
     }

   if (!elm_widget_sub_object_add(obj, child))
     {
        ERR("could not add %p as sub object of %p", child, obj);
        edje_object_part_box_remove
          (ELM_WIDGET_DATA(sd)->resize_obj, part, child);
        return EINA_FALSE;
     }

   sub_d = ELM_NEW(Elm_Layout_Sub_Object_Data);
   sub_d->type = BOX_PREPEND;
   sub_d->part = eina_stringshare_add(part);
   sub_d->obj = child;
   sd->subs = eina_list_prepend(sd->subs, sub_d);

   if (ELM_WIDGET_DATA(sd)->frozen) return EINA_TRUE;
   ELM_LAYOUT_CLASS(ELM_WIDGET_DATA(sd)->api)->sizing_eval(obj);

   return EINA_TRUE;
}

static void
_box_reference_del(void *data,
                   Evas *e __UNUSED__,
                   Evas_Object *obj __UNUSED__,
                   void *event_info __UNUSED__)
{
   Elm_Layout_Sub_Object_Data *sub_d = data;
   sub_d->p.box.reference = NULL;
}

static Eina_Bool
_elm_layout_smart_box_insert_before(Evas_Object *obj,
                                    const char *part,
                                    Evas_Object *child,
                                    const Evas_Object *reference)
{
   Elm_Layout_Sub_Object_Data *sub_d;

   ELM_LAYOUT_DATA_GET(obj, sd);

   if (!edje_object_part_box_insert_before
         (ELM_WIDGET_DATA(sd)->resize_obj, part, child, reference))
     {
        ERR("child %p could not be inserted before %p inf box part '%s'",
            child, reference, part);
        return EINA_FALSE;
     }

   if (!elm_widget_sub_object_add(obj, child))
     {
        ERR("could not add %p as sub object of %p", child, obj);
        edje_object_part_box_remove
          (ELM_WIDGET_DATA(sd)->resize_obj, part, child);
        return EINA_FALSE;
     }

   sub_d = ELM_NEW(Elm_Layout_Sub_Object_Data);
   sub_d->type = BOX_INSERT_BEFORE;
   sub_d->part = eina_stringshare_add(part);
   sub_d->obj = child;
   sub_d->p.box.reference = reference;
   sd->subs = eina_list_append(sd->subs, sub_d);

   evas_object_event_callback_add
     ((Evas_Object *)reference, EVAS_CALLBACK_DEL, _box_reference_del, sub_d);

   if (ELM_WIDGET_DATA(sd)->frozen) return EINA_TRUE;
   ELM_LAYOUT_CLASS(ELM_WIDGET_DATA(sd)->api)->sizing_eval(obj);

   return EINA_TRUE;
}

static Eina_Bool
_elm_layout_smart_box_insert_at(Evas_Object *obj,
                                const char *part,
                                Evas_Object *child,
                                unsigned int pos)
{
   Elm_Layout_Sub_Object_Data *sub_d;

   ELM_LAYOUT_DATA_GET(obj, sd);

   if (!edje_object_part_box_insert_at
         (ELM_WIDGET_DATA(sd)->resize_obj, part, child, pos))
     {
        ERR("child %p could not be inserted at %u to box part '%s'",
            child, pos, part);
        return EINA_FALSE;
     }

   if (!elm_widget_sub_object_add(obj, child))
     {
        ERR("could not add %p as sub object of %p", child, obj);
        edje_object_part_box_remove
          (ELM_WIDGET_DATA(sd)->resize_obj, part, child);
        return EINA_FALSE;
     }

   sub_d = ELM_NEW(Elm_Layout_Sub_Object_Data);
   sub_d->type = BOX_INSERT_AT;
   sub_d->part = eina_stringshare_add(part);
   sub_d->obj = child;
   sub_d->p.box.pos = pos;
   sd->subs = eina_list_append(sd->subs, sub_d);

   if (ELM_WIDGET_DATA(sd)->frozen) return EINA_TRUE;
   ELM_LAYOUT_CLASS(ELM_WIDGET_DATA(sd)->api)->sizing_eval(obj);

   return EINA_TRUE;
}

static Evas_Object *
_sub_box_remove(Evas_Object *obj,
                Elm_Layout_Smart_Data *sd,
                Elm_Layout_Sub_Object_Data *sub_d)
{
   Evas_Object *child = sub_d->obj; /* sub_d will die in
                                     * _elm_layout_smart_sub_object_del */

   if (sub_d->type == BOX_INSERT_BEFORE)
     evas_object_event_callback_del_full
       ((Evas_Object *)sub_d->p.box.reference,
       EVAS_CALLBACK_DEL, _box_reference_del, sub_d);

   edje_object_part_box_remove
     (ELM_WIDGET_DATA(sd)->resize_obj, sub_d->part, child);

   if (!elm_widget_sub_object_del(obj, child))
     {
        ERR("could not remove sub object %p from %p", child, obj);
        return NULL;
     }

   return child;
}

static Eina_Bool
_sub_box_is(const Elm_Layout_Sub_Object_Data *sub_d)
{
   switch (sub_d->type)
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

static Evas_Object *
_elm_layout_smart_box_remove(Evas_Object *obj,
                             const char *part,
                             Evas_Object *child)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(part, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(child, NULL);

   ELM_LAYOUT_DATA_GET(obj, sd);

   const Eina_List *l;
   Elm_Layout_Sub_Object_Data *sub_d;

   EINA_LIST_FOREACH (sd->subs, l, sub_d)
     {
        if (!_sub_box_is(sub_d)) continue;
        if ((sub_d->obj == child) && (!strcmp(sub_d->part, part)))
          return _sub_box_remove(obj, sd, sub_d);
     }

   return NULL;
}

static Eina_Bool
_elm_layout_smart_box_remove_all(Evas_Object *obj,
                                 const char *part,
                                 Eina_Bool clear)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(part, EINA_FALSE);

   ELM_LAYOUT_DATA_GET(obj, sd);

   Elm_Layout_Sub_Object_Data *sub_d;
   Eina_List *lst;

   lst = eina_list_clone(sd->subs);
   EINA_LIST_FREE (lst, sub_d)
     {
        if (!_sub_box_is(sub_d)) continue;
        if (!strcmp(sub_d->part, part))
          {
             /* original item's deletion handled at sub-obj-del */
             Evas_Object *child = _sub_box_remove(obj, sd, sub_d);
             if ((clear) && (child)) evas_object_del(child);
          }
     }

   /* eventually something may not be added with elm_layout, delete them
    * as well */
   edje_object_part_box_remove_all
     (ELM_WIDGET_DATA(sd)->resize_obj, part, clear);

   return EINA_TRUE;
}

static Eina_Bool
_elm_layout_smart_table_pack(Evas_Object *obj,
                             const char *part,
                             Evas_Object *child,
                             unsigned short col,
                             unsigned short row,
                             unsigned short colspan,
                             unsigned short rowspan)
{
   Elm_Layout_Sub_Object_Data *sub_d;

   ELM_LAYOUT_DATA_GET(obj, sd);

   if (!edje_object_part_table_pack
         (ELM_WIDGET_DATA(sd)->resize_obj, part, child, col,
         row, colspan, rowspan))
     {
        ERR("child %p could not be packed into box part '%s' col=%uh, row=%hu,"
            " colspan=%hu, rowspan=%hu", child, part, col, row, colspan,
            rowspan);
        return EINA_FALSE;
     }

   if (!elm_widget_sub_object_add(obj, child))
     {
        ERR("could not add %p as sub object of %p", child, obj);
        edje_object_part_table_unpack
          (ELM_WIDGET_DATA(sd)->resize_obj, part, child);
        return EINA_FALSE;
     }

   sub_d = ELM_NEW(Elm_Layout_Sub_Object_Data);
   sub_d->type = TABLE_PACK;
   sub_d->part = eina_stringshare_add(part);
   sub_d->obj = child;
   sub_d->p.table.col = col;
   sub_d->p.table.row = row;
   sub_d->p.table.colspan = colspan;
   sub_d->p.table.rowspan = rowspan;
   sd->subs = eina_list_append(sd->subs, sub_d);

   if (ELM_WIDGET_DATA(sd)->frozen) return EINA_TRUE;
   ELM_LAYOUT_CLASS(ELM_WIDGET_DATA(sd)->api)->sizing_eval(obj);

   return EINA_TRUE;
}

static Evas_Object *
_sub_table_remove(Evas_Object *obj,
                  Elm_Layout_Smart_Data *sd,
                  Elm_Layout_Sub_Object_Data *sub_d)
{
   Evas_Object *child;

   child = sub_d->obj; /* sub_d will die in _elm_layout_smart_sub_object_del */

   edje_object_part_table_unpack
     (ELM_WIDGET_DATA(sd)->resize_obj, sub_d->part, child);

   if (!elm_widget_sub_object_del(obj, child))
     {
        ERR("could not remove sub object %p from %p", child, obj);
        return NULL;
     }

   return child;
}

static Evas_Object *
_elm_layout_smart_table_unpack(Evas_Object *obj,
                               const char *part,
                               Evas_Object *child)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(part, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(child, NULL);

   ELM_LAYOUT_DATA_GET(obj, sd);

   const Eina_List *l;
   Elm_Layout_Sub_Object_Data *sub_d;

   EINA_LIST_FOREACH (sd->subs, l, sub_d)
     {
        if (sub_d->type != TABLE_PACK) continue;
        if ((sub_d->obj == child) && (!strcmp(sub_d->part, part)))
          return _sub_table_remove(obj, sd, sub_d);
     }

   return NULL;
}

static Eina_Bool
_elm_layout_smart_table_clear(Evas_Object *obj,
                              const char *part,
                              Eina_Bool clear)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(part, EINA_FALSE);
   ELM_LAYOUT_DATA_GET(obj, sd);

   Elm_Layout_Sub_Object_Data *sub_d;
   Eina_List *lst;

   lst = eina_list_clone(sd->subs);
   EINA_LIST_FREE (lst, sub_d)
     {
        if (sub_d->type != TABLE_PACK) continue;
        if (!strcmp(sub_d->part, part))
          {
             /* original item's deletion handled at sub-obj-del */
             Evas_Object *child = _sub_table_remove(obj, sd, sub_d);
             if ((clear) && (child)) evas_object_del(child);
          }
     }

   /* eventually something may not be added with elm_layout, delete them
    * as well */
   edje_object_part_table_clear(ELM_WIDGET_DATA(sd)->resize_obj, part, clear);

   return EINA_TRUE;
}

static void
_on_size_evaluate_signal(void *data,
                         Evas_Object *obj __UNUSED__,
                         const char *emission __UNUSED__,
                         const char *source __UNUSED__)
{
   ELM_LAYOUT_DATA_GET(data, sd);
   ELM_LAYOUT_CLASS(ELM_WIDGET_DATA(sd)->api)->sizing_eval(data);
}

static void
_elm_layout_smart_add(Evas_Object *obj)
{
   EVAS_SMART_DATA_ALLOC(obj, Elm_Layout_Smart_Data);

   /* has to be there *before* parent's smart_add() */
   ELM_WIDGET_DATA(priv)->resize_obj =
     edje_object_add(evas_object_evas_get(obj));

   ELM_WIDGET_CLASS(_elm_layout_parent_sc)->base.add(obj);

   elm_widget_can_focus_set(obj, EINA_FALSE);

   edje_object_signal_callback_add
     (ELM_WIDGET_DATA(priv)->resize_obj, "size,eval", "elm",
     _on_size_evaluate_signal, obj);

   if (ELM_WIDGET_DATA(priv)->frozen) return;
   ELM_LAYOUT_CLASS(ELM_WIDGET_DATA(priv)->api)->sizing_eval(obj);
}

static void
_elm_layout_smart_del(Evas_Object *obj)
{
   Elm_Layout_Sub_Object_Data *sub_d;
   Elm_Layout_Sub_Object_Cursor *pc;
   Edje_Signal_Data *esd;
   Evas_Object *child;
   Eina_List *l;

   ELM_LAYOUT_DATA_GET(obj, sd);

   EINA_LIST_FREE (sd->subs, sub_d)
     {
        eina_stringshare_del(sub_d->part);

        if (sub_d->type == TEXT)
          eina_stringshare_del(sub_d->p.text.text);

        free(sub_d);
     }

   EINA_LIST_FREE (sd->parts_cursors, pc)
     _part_cursor_free(pc);

   EINA_LIST_FREE (sd->edje_signals, esd)
     {
        eina_stringshare_del(esd->emission);
        eina_stringshare_del(esd->source);
        free(esd);
     }

   eina_stringshare_del(sd->klass);
   eina_stringshare_del(sd->group);

   /* let's make our Edje object the *last* to be processed, since it
    * may (smart) parent other sub objects here */
   EINA_LIST_FOREACH (ELM_WIDGET_DATA(sd)->subobjs, l, child)
     {
        if (child == ELM_WIDGET_DATA(sd)->resize_obj)
          {
             ELM_WIDGET_DATA(sd)->subobjs =
               eina_list_demote_list(ELM_WIDGET_DATA(sd)->subobjs, l);
             break;
          }
     }

   ELM_WIDGET_CLASS(_elm_layout_parent_sc)->base.del(obj);
}

/* rewrite or extend this one on your derived class as to suit your
 * needs */
static void
_elm_layout_smart_calculate(Evas_Object *obj)
{
   ELM_LAYOUT_DATA_GET(obj, sd);

   if (sd->needs_size_calc)
     {
        _sizing_eval(obj, sd);
        sd->needs_size_calc = EINA_FALSE;
     }
}

static void
_elm_layout_smart_set_user(Elm_Layout_Smart_Class *sc)
{
   ELM_WIDGET_CLASS(sc)->base.add = _elm_layout_smart_add;
   ELM_WIDGET_CLASS(sc)->base.del = _elm_layout_smart_del;
   ELM_WIDGET_CLASS(sc)->base.calculate = _elm_layout_smart_calculate;

   ELM_WIDGET_CLASS(sc)->theme = _elm_layout_smart_theme;
   ELM_WIDGET_CLASS(sc)->disable = _elm_layout_smart_disable;
   ELM_WIDGET_CLASS(sc)->focus_next = _elm_layout_smart_focus_next;
   ELM_WIDGET_CLASS(sc)->focus_direction = _elm_layout_smart_focus_direction;
   ELM_WIDGET_CLASS(sc)->on_focus = _elm_layout_smart_on_focus;

   ELM_WIDGET_CLASS(sc)->sub_object_add = _elm_layout_smart_sub_object_add;
   ELM_WIDGET_CLASS(sc)->sub_object_del = _elm_layout_smart_sub_object_del;

   ELM_CONTAINER_CLASS(sc)->content_set = _elm_layout_smart_content_set;
   ELM_CONTAINER_CLASS(sc)->content_get = _elm_layout_smart_content_get;
   ELM_CONTAINER_CLASS(sc)->content_unset = _elm_layout_smart_content_unset;

   sc->sizing_eval = _elm_layout_smart_sizing_eval;
   sc->signal = _elm_layout_smart_signal;
   sc->callback_add = _elm_layout_smart_callback_add;
   sc->callback_del = _elm_layout_smart_callback_del;
   sc->text_set = _elm_layout_smart_text_set;
   sc->text_get = _elm_layout_smart_text_get;
   sc->box_append = _elm_layout_smart_box_append;
   sc->box_prepend = _elm_layout_smart_box_prepend;
   sc->box_insert_before = _elm_layout_smart_box_insert_before;
   sc->box_insert_at = _elm_layout_smart_box_insert_at;
   sc->box_remove = _elm_layout_smart_box_remove;
   sc->box_remove_all = _elm_layout_smart_box_remove_all;
   sc->table_pack = _elm_layout_smart_table_pack;
   sc->table_unpack = _elm_layout_smart_table_unpack;
   sc->table_clear = _elm_layout_smart_table_clear;
}

static Elm_Layout_Sub_Object_Cursor *
_parts_cursors_find(Elm_Layout_Smart_Data *sd,
                    const char *part)
{
   const Eina_List *l;
   Elm_Layout_Sub_Object_Cursor *pc;

   EINA_LIST_FOREACH (sd->parts_cursors, l, pc)
     {
        if (!strcmp(pc->part, part))
          return pc;
     }

   return NULL;
}

/* The public functions down here are meant to operate on whichever
 * widget inheriting from elm_layout */

EAPI Eina_Bool
elm_layout_file_set(Evas_Object *obj,
                    const char *file,
                    const char *group)
{
   ELM_LAYOUT_CHECK(obj) EINA_FALSE;
   ELM_LAYOUT_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);

   Eina_Bool ret =
     edje_object_file_set(ELM_WIDGET_DATA(sd)->resize_obj, file, group);

   if (ret) _visuals_refresh(obj, sd);
   else
     ERR("failed to set edje file '%s', group '%s': %s",
         file, group,
         edje_load_error_str
           (edje_object_load_error_get(ELM_WIDGET_DATA(sd)->resize_obj)));

   return ret;
}

EAPI Eina_Bool
elm_layout_theme_set(Evas_Object *obj,
                     const char *klass,
                     const char *group,
                     const char *style)
{
   Eina_Bool ret;

   ELM_LAYOUT_CHECK(obj) EINA_FALSE;
   ELM_LAYOUT_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);

   eina_stringshare_replace(&(sd->klass), klass);
   eina_stringshare_replace(&(sd->group), group);
   eina_stringshare_replace(&(ELM_WIDGET_DATA(sd)->style), style);

   /* not issuing smart theme directly here, because one may want to
      use this function inside a smart theme routine of its own */
   ret = elm_widget_theme_object_set
       (obj, ELM_WIDGET_DATA(sd)->resize_obj, sd->klass, sd->group,
       elm_widget_style_get(obj));
   evas_object_smart_callback_call(obj, SIG_THEME_CHANGED, NULL);

   return ret;
}

EAPI void
elm_layout_signal_emit(Evas_Object *obj,
                       const char *emission,
                       const char *source)
{
   ELM_LAYOUT_CHECK(obj);
   ELM_LAYOUT_DATA_GET_OR_RETURN(obj, sd);

   ELM_LAYOUT_CLASS(ELM_WIDGET_DATA(sd)->api)->signal(obj, emission, source);
}

EAPI void
elm_layout_signal_callback_add(Evas_Object *obj,
                               const char *emission,
                               const char *source,
                               Edje_Signal_Cb func,
                               void *data)
{
   ELM_LAYOUT_CHECK(obj);
   ELM_LAYOUT_DATA_GET_OR_RETURN(obj, sd);

   ELM_LAYOUT_CLASS(ELM_WIDGET_DATA(sd)->api)->callback_add
     (obj, emission, source, func, data);
}

EAPI void *
elm_layout_signal_callback_del(Evas_Object *obj,
                               const char *emission,
                               const char *source,
                               Edje_Signal_Cb func)
{
   ELM_LAYOUT_CHECK(obj) NULL;
   ELM_LAYOUT_DATA_GET_OR_RETURN_VAL(obj, sd, NULL);

   return ELM_LAYOUT_CLASS(ELM_WIDGET_DATA(sd)->api)->callback_del
            (obj, emission, source, func);
}

EAPI Eina_Bool
elm_layout_content_set(Evas_Object *obj,
                       const char *swallow,
                       Evas_Object *content)
{
   ELM_LAYOUT_CHECK(obj) EINA_FALSE;
   ELM_LAYOUT_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);

   return ELM_CONTAINER_CLASS(ELM_WIDGET_DATA(sd)->api)->content_set
            (obj, swallow, content);
}

EAPI Evas_Object *
elm_layout_content_get(const Evas_Object *obj,
                       const char *swallow)
{
   ELM_LAYOUT_CHECK(obj) NULL;
   ELM_LAYOUT_DATA_GET_OR_RETURN_VAL(obj, sd, NULL);

   return ELM_CONTAINER_CLASS(ELM_WIDGET_DATA(sd)->api)->content_get
            (obj, swallow);
}

EAPI Evas_Object *
elm_layout_content_unset(Evas_Object *obj,
                         const char *swallow)
{
   ELM_LAYOUT_CHECK(obj) NULL;
   ELM_LAYOUT_DATA_GET_OR_RETURN_VAL(obj, sd, NULL);

   return ELM_CONTAINER_CLASS(ELM_WIDGET_DATA(sd)->api)->content_unset
            (obj, swallow);
}

EAPI Eina_Bool
elm_layout_text_set(Evas_Object *obj,
                    const char *part,
                    const char *text)
{
   ELM_LAYOUT_CHECK(obj) EINA_FALSE;
   ELM_LAYOUT_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);

   return ELM_LAYOUT_CLASS(ELM_WIDGET_DATA(sd)->api)->text_set
            (obj, part, text);
}

EAPI const char *
elm_layout_text_get(const Evas_Object *obj,
                    const char *part)
{
   ELM_LAYOUT_CHECK(obj) NULL;
   ELM_LAYOUT_DATA_GET_OR_RETURN_VAL(obj, sd, NULL);

   return ELM_LAYOUT_CLASS(ELM_WIDGET_DATA(sd)->api)->text_get(obj, part);
}

EAPI Eina_Bool
elm_layout_box_append(Evas_Object *obj,
                      const char *part,
                      Evas_Object *child)
{
   ELM_LAYOUT_CHECK(obj) EINA_FALSE;
   ELM_LAYOUT_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);

   return ELM_LAYOUT_CLASS(ELM_WIDGET_DATA(sd)->api)->box_append
            (obj, part, child);
}

EAPI Eina_Bool
elm_layout_box_prepend(Evas_Object *obj,
                       const char *part,
                       Evas_Object *child)
{
   ELM_LAYOUT_CHECK(obj) EINA_FALSE;
   ELM_LAYOUT_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);

   return ELM_LAYOUT_CLASS(ELM_WIDGET_DATA(sd)->api)->box_prepend
            (obj, part, child);
}

EAPI Eina_Bool
elm_layout_box_insert_before(Evas_Object *obj,
                             const char *part,
                             Evas_Object *child,
                             const Evas_Object *reference)
{
   ELM_LAYOUT_CHECK(obj) EINA_FALSE;
   ELM_LAYOUT_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);

   return ELM_LAYOUT_CLASS(ELM_WIDGET_DATA(sd)->api)->box_insert_before
            (obj, part, child, reference);
}

EAPI Eina_Bool
elm_layout_box_insert_at(Evas_Object *obj,
                         const char *part,
                         Evas_Object *child,
                         unsigned int pos)
{
   ELM_LAYOUT_CHECK(obj) EINA_FALSE;
   ELM_LAYOUT_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);

   return ELM_LAYOUT_CLASS(ELM_WIDGET_DATA(sd)->api)->box_insert_at
            (obj, part, child, pos);
}

EAPI Evas_Object *
elm_layout_box_remove(Evas_Object *obj,
                      const char *part,
                      Evas_Object *child)
{
   ELM_LAYOUT_CHECK(obj) NULL;
   ELM_LAYOUT_DATA_GET_OR_RETURN_VAL(obj, sd, NULL);

   return ELM_LAYOUT_CLASS(ELM_WIDGET_DATA(sd)->api)->box_remove
            (obj, part, child);
}

EAPI Eina_Bool
elm_layout_box_remove_all(Evas_Object *obj,
                          const char *part,
                          Eina_Bool clear)
{
   ELM_LAYOUT_CHECK(obj) EINA_FALSE;
   ELM_LAYOUT_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);

   return ELM_LAYOUT_CLASS(ELM_WIDGET_DATA(sd)->api)->box_remove_all
            (obj, part, clear);
}

EAPI Eina_Bool
elm_layout_table_pack(Evas_Object *obj,
                      const char *part,
                      Evas_Object *child,
                      unsigned short col,
                      unsigned short row,
                      unsigned short colspan,
                      unsigned short rowspan)
{
   ELM_LAYOUT_CHECK(obj) EINA_FALSE;
   ELM_LAYOUT_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);

   return ELM_LAYOUT_CLASS(ELM_WIDGET_DATA(sd)->api)->table_pack
            (obj, part, child, col, row, colspan, rowspan);
}

EAPI Evas_Object *
elm_layout_table_unpack(Evas_Object *obj,
                        const char *part,
                        Evas_Object *child)
{
   ELM_LAYOUT_CHECK(obj) NULL;
   ELM_LAYOUT_DATA_GET_OR_RETURN_VAL(obj, sd, NULL);

   return ELM_LAYOUT_CLASS(ELM_WIDGET_DATA(sd)->api)->table_unpack
            (obj, part, child);
}

EAPI Eina_Bool
elm_layout_table_clear(Evas_Object *obj,
                       const char *part,
                       Eina_Bool clear)
{
   ELM_LAYOUT_CHECK(obj) EINA_FALSE;
   ELM_LAYOUT_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);

   return ELM_LAYOUT_CLASS(ELM_WIDGET_DATA(sd)->api)->table_clear
            (obj, part, clear);
}

EAPI Evas_Object *
elm_layout_edje_get(const Evas_Object *obj)
{
   ELM_LAYOUT_CHECK(obj) NULL;
   ELM_LAYOUT_DATA_GET_OR_RETURN_VAL(obj, sd, NULL);

   return ELM_WIDGET_DATA(sd)->resize_obj;
}

EAPI const char *
elm_layout_data_get(const Evas_Object *obj,
                    const char *key)
{
   ELM_LAYOUT_CHECK(obj) NULL;
   ELM_LAYOUT_DATA_GET_OR_RETURN_VAL(obj, sd, NULL);

   return edje_object_data_get(ELM_WIDGET_DATA(sd)->resize_obj, key);
}

EAPI void
elm_layout_sizing_eval(Evas_Object *obj)
{
   ELM_LAYOUT_CHECK(obj);
   ELM_LAYOUT_DATA_GET(obj, sd);

   ELM_LAYOUT_CLASS(ELM_WIDGET_DATA(sd)->api)->sizing_eval(obj);
}

EAPI int
elm_layout_freeze(Evas_Object *obj)
{
   ELM_LAYOUT_CHECK(obj) 0;
   ELM_LAYOUT_DATA_GET(obj, sd);

   if ((ELM_WIDGET_DATA(sd)->frozen)++ != 0)
     return ELM_WIDGET_DATA(sd)->frozen;

   edje_object_freeze(ELM_WIDGET_DATA(sd)->resize_obj);

   return 1;
}

EAPI int
elm_layout_thaw(Evas_Object *obj)
{
   ELM_LAYOUT_CHECK(obj) 0;
   ELM_LAYOUT_DATA_GET(obj, sd);

   if (--(ELM_WIDGET_DATA(sd)->frozen) != 0)
     return ELM_WIDGET_DATA(sd)->frozen;

   edje_object_thaw(ELM_WIDGET_DATA(sd)->resize_obj);

   ELM_LAYOUT_CLASS(ELM_WIDGET_DATA(sd)->api)->sizing_eval(obj);

   return 0;
}

EAPI Eina_Bool
elm_layout_part_cursor_set(Evas_Object *obj,
                           const char *part_name,
                           const char *cursor)
{
   ELM_LAYOUT_CHECK(obj) EINA_FALSE;
   ELM_LAYOUT_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(part_name, EINA_FALSE);

   Evas_Object *part_obj;
   Elm_Layout_Sub_Object_Cursor *pc;

   part_obj = (Evas_Object *)edje_object_part_object_get
       (ELM_WIDGET_DATA(sd)->resize_obj, part_name);
   if (!part_obj)
     {
        const char *group, *file;

        edje_object_file_get(ELM_WIDGET_DATA(sd)->resize_obj, &file, &group);
        ERR("no part '%s' in group '%s' of file '%s'. Cannot set cursor '%s'",
            part_name, group, file, cursor);
        return EINA_FALSE;
     }
   if (evas_object_pass_events_get(part_obj))
     {
        const char *group, *file;

        edje_object_file_get(ELM_WIDGET_DATA(sd)->resize_obj, &file, &group);
        ERR("part '%s' in group '%s' of file '%s' has mouse_events: 0. "
            "Cannot set cursor '%s'",
            part_name, group, file, cursor);
        return EINA_FALSE;
     }

   pc = _parts_cursors_find(sd, part_name);
   if (pc) eina_stringshare_replace(&pc->cursor, cursor);
   else
     {
        pc = calloc(1, sizeof(*pc));
        pc->part = eina_stringshare_add(part_name);
        pc->cursor = eina_stringshare_add(cursor);
        pc->style = eina_stringshare_add("default");
        sd->parts_cursors = eina_list_append(sd->parts_cursors, pc);
     }

   pc->obj = part_obj;
   elm_object_sub_cursor_set(part_obj, obj, pc->cursor);

   return EINA_TRUE;
}

EAPI const char *
elm_layout_part_cursor_get(const Evas_Object *obj,
                           const char *part_name)
{
   ELM_LAYOUT_CHECK(obj) NULL;
   ELM_LAYOUT_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(part_name, NULL);

   Elm_Layout_Sub_Object_Cursor *pc = _parts_cursors_find(sd, part_name);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pc, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pc->obj, NULL);

   return elm_object_cursor_get(pc->obj);
}

EAPI Eina_Bool
elm_layout_part_cursor_unset(Evas_Object *obj,
                             const char *part_name)
{
   ELM_LAYOUT_CHECK(obj) EINA_FALSE;
   ELM_LAYOUT_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(part_name, EINA_FALSE);

   Eina_List *l;
   Elm_Layout_Sub_Object_Cursor *pc;

   EINA_LIST_FOREACH (sd->parts_cursors, l, pc)
     {
        if (!strcmp(part_name, pc->part))
          {
             if (pc->obj) elm_object_cursor_unset(pc->obj);
             _part_cursor_free(pc);
             sd->parts_cursors = eina_list_remove_list(sd->parts_cursors, l);
             return EINA_TRUE;
          }
     }

   return EINA_FALSE;
}

EAPI Eina_Bool
elm_layout_part_cursor_style_set(Evas_Object *obj,
                                 const char *part_name,
                                 const char *style)
{
   ELM_LAYOUT_CHECK(obj) EINA_FALSE;
   ELM_LAYOUT_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(part_name, EINA_FALSE);

   Elm_Layout_Sub_Object_Cursor *pc = _parts_cursors_find(sd, part_name);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pc, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pc->obj, EINA_FALSE);

   eina_stringshare_replace(&pc->style, style);
   elm_object_cursor_style_set(pc->obj, pc->style);

   return EINA_TRUE;
}

EAPI const char *
elm_layout_part_cursor_style_get(const Evas_Object *obj,
                                 const char *part_name)
{
   ELM_LAYOUT_CHECK(obj) NULL;
   ELM_LAYOUT_DATA_GET_OR_RETURN_VAL(obj, sd, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(part_name, NULL);

   Elm_Layout_Sub_Object_Cursor *pc = _parts_cursors_find(sd, part_name);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pc, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pc->obj, NULL);

   return elm_object_cursor_style_get(pc->obj);
}

EAPI Eina_Bool
elm_layout_part_cursor_engine_only_set(Evas_Object *obj,
                                       const char *part_name,
                                       Eina_Bool engine_only)
{
   ELM_LAYOUT_CHECK(obj) EINA_FALSE;
   ELM_LAYOUT_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(part_name, EINA_FALSE);

   Elm_Layout_Sub_Object_Cursor *pc = _parts_cursors_find(sd, part_name);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pc, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pc->obj, EINA_FALSE);

   pc->engine_only = !!engine_only;
   elm_object_cursor_theme_search_enabled_set(pc->obj, pc->engine_only);

   return EINA_TRUE;
}

EAPI Eina_Bool
elm_layout_part_cursor_engine_only_get(const Evas_Object *obj,
                                       const char *part_name)
{
   ELM_LAYOUT_CHECK(obj) EINA_FALSE;
   ELM_LAYOUT_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(part_name, EINA_FALSE);

   Elm_Layout_Sub_Object_Cursor *pc = _parts_cursors_find(sd, part_name);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pc, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pc->obj, EINA_FALSE);

   return elm_object_cursor_theme_search_enabled_get(pc->obj);
}

EVAS_SMART_SUBCLASS_NEW
  (ELM_LAYOUT_SMART_NAME, _elm_layout_widget, Elm_Layout_Smart_Class,
  Elm_Layout_Smart_Class, elm_layout_smart_class_get, NULL);

static const Elm_Layout_Part_Alias_Description _text_aliases[] =
{
   {"default", "elm.text"},
   {NULL, NULL}
};

/* the layout widget (not the base layout) has this extra bit */
static void
_elm_layout_widget_smart_set_user(Elm_Layout_Smart_Class *sc)
{
   sc->text_aliases = _text_aliases;
}

EAPI Eina_Bool
elm_layout_edje_object_can_access_set(Evas_Object *obj,
                                      Eina_Bool can_access)
{
   ELM_LAYOUT_CHECK(obj) EINA_FALSE;
   ELM_LAYOUT_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);

   ELM_WIDGET_DATA(sd)->can_access = !!can_access;
   return EINA_TRUE;
}

EAPI Eina_Bool
elm_layout_edje_object_can_access_get(Evas_Object *obj)
{
   ELM_LAYOUT_CHECK(obj) EINA_FALSE;
   ELM_LAYOUT_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);

   return ELM_WIDGET_DATA(sd)->can_access;
}

/* And now the basic layout widget itself */
EAPI Evas_Object *
elm_layout_add(Evas_Object *parent)
{
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   obj = elm_widget_add(_elm_layout_widget_smart_class_new(), parent);
   if (!obj) return NULL;

   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);

   return obj;
}
