#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_layout.h"

EAPI Eo_Op ELM_OBJ_LAYOUT_BASE_ID = EO_NOOP;

#define MY_CLASS ELM_OBJ_LAYOUT_CLASS

#define MY_CLASS_NAME "Elm_Layout"
#define MY_CLASS_NAME_LEGACY "elm_layout"

static const char SIG_THEME_CHANGED[] = "theme,changed";
const char SIG_LAYOUT_FOCUSED[] = "focused";
const char SIG_LAYOUT_UNFOCUSED[] = "unfocused";

/* smart callbacks coming from elm layout objects: */
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_THEME_CHANGED, ""},
   {SIG_LAYOUT_FOCUSED, ""},
   {SIG_LAYOUT_UNFOCUSED, ""},
   {SIG_WIDGET_LANG_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_WIDGET_ACCESS_CHANGED, ""}, /**< handled by elm_widget */
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

static void
_on_sub_object_size_hint_change(void *data,
                                Evas *e EINA_UNUSED,
                                Evas_Object *obj EINA_UNUSED,
                                void *event_info EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(data, wd);
   eo_do(data, elm_obj_layout_sizing_eval());
}

static void
_part_cursor_free(Elm_Layout_Sub_Object_Cursor *pc)
{
   eina_stringshare_del(pc->part);
   eina_stringshare_del(pc->style);
   eina_stringshare_del(pc->cursor);

   free(pc);
}

static void
_sizing_eval(Evas_Object *obj, Elm_Layout_Smart_Data *sd)
{
   Evas_Coord minh = -1, minw = -1;
   Evas_Coord rest_w = 0, rest_h = 0;
   ELM_WIDGET_DATA_GET_OR_RETURN(sd->obj, wd);

   if (sd->restricted_calc_w)
     rest_w = wd->w;
   if (sd->restricted_calc_h)
     rest_h = wd->h;

   edje_object_size_min_restricted_calc(wd->resize_obj, &minw, &minh,
                                        rest_w, rest_h);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, -1, -1);

   sd->restricted_calc_w = sd->restricted_calc_h = EINA_FALSE;
}

/* common content cases for layout objects: icon and text */
static inline void
_icon_signal_emit(Elm_Layout_Smart_Data *sd,
                  Elm_Layout_Sub_Object_Data *sub_d,
                  Eina_Bool visible)
{
   char buf[1024];
   const char *type;

   //FIXME: Don't limit to the icon and end here.
   // send signals for all contents after elm 2.0
   if (sub_d->type != SWALLOW ||
       (strcmp("elm.swallow.icon", sub_d->part) &&
        (strcmp("elm.swallow.end", sub_d->part)))) return;

   if (!strncmp(sub_d->part, "elm.swallow.", strlen("elm.swallow.")))
     type = sub_d->part + strlen("elm.swallow.");
   else
     type = sub_d->part;

   snprintf(buf, sizeof(buf), "elm,state,%s,%s", type,
            visible ? "visible" : "hidden");

   ELM_WIDGET_DATA_GET_OR_RETURN(sd->obj, wd);
   edje_object_signal_emit(wd->resize_obj, buf, "elm");

   /* themes might need immediate action here */
   edje_object_message_signal_process(wd->resize_obj);
}

static inline void
_text_signal_emit(Elm_Layout_Smart_Data *sd,
                  Elm_Layout_Sub_Object_Data *sub_d,
                  Eina_Bool visible)
{
   char buf[1024];
   const char *type;

   //FIXME: Don't limit to "elm.text" prefix.
   //Send signals for all text parts after elm 2.0
   if (sub_d->type != TEXT || strcmp("elm.text", sub_d->part)) return;

   ELM_WIDGET_DATA_GET_OR_RETURN(sd->obj, wd);

   if (!strncmp(sub_d->part, "elm.text.", strlen("elm.text.")))
     type = sub_d->part + strlen("elm.text.");
   else
     type = sub_d->part;

   snprintf(buf, sizeof(buf), "elm,state,%s,%s", type,
            visible ? "visible" : "hidden");
   edje_object_signal_emit(wd->resize_obj, buf, "elm");

   /* TODO: is this right? It was like that, but IMO it should be removed: */
   snprintf(buf, sizeof(buf),
            visible ? "elm,state,text,visible" : "elm,state,text,hidden");

   edje_object_signal_emit(wd->resize_obj, buf, "elm");

   /* themes might need immediate action here */
   edje_object_message_signal_process(wd->resize_obj);
}

static void
_parts_signals_emit(Elm_Layout_Smart_Data *sd)
{
   const Eina_List *l;
   Elm_Layout_Sub_Object_Data *sub_d;

   EINA_LIST_FOREACH(sd->subs, l, sub_d)
     {
        _icon_signal_emit(sd, sub_d, EINA_TRUE);
        _text_signal_emit(sd, sub_d, EINA_TRUE);
     }
}

static void
_parts_swallow_fix(Elm_Layout_Smart_Data *sd, Elm_Widget_Smart_Data *wd)
{
   Eina_List *l;
   Elm_Layout_Sub_Object_Data *sub_d;

   EINA_LIST_FOREACH(sd->subs, l, sub_d)
     {
        if (sub_d->type == SWALLOW)
          {
             if (sub_d->part)
               edje_object_part_swallow(wd->resize_obj,
                                        sub_d->part, sub_d->obj);
          }
     }
}

static void
_parts_text_fix(Elm_Layout_Smart_Data *sd)
{
   const Eina_List *l;
   Elm_Layout_Sub_Object_Data *sub_d;
   ELM_WIDGET_DATA_GET_OR_RETURN(sd->obj, wd);

   EINA_LIST_FOREACH(sd->subs, l, sub_d)
     {
        if (sub_d->type == TEXT)
          {
             edje_object_part_text_escaped_set
               (wd->resize_obj, sub_d->part,
               sub_d->p.text.text);
          }
     }
}

static void
_part_cursor_part_apply(const Elm_Layout_Sub_Object_Cursor *pc)
{
   elm_object_cursor_set(pc->obj, pc->cursor);
   elm_object_cursor_style_set(pc->obj, pc->style);
   elm_object_cursor_theme_search_enabled_set(pc->obj, !pc->engine_only);
}

static void
_parts_cursors_apply(Elm_Layout_Smart_Data *sd)
{
   const Eina_List *l;
   const char *file, *group;
   Elm_Layout_Sub_Object_Cursor *pc;
   ELM_WIDGET_DATA_GET_OR_RETURN(sd->obj, wd);

   edje_object_file_get(wd->resize_obj, &file, &group);

   EINA_LIST_FOREACH(sd->parts_cursors, l, pc)
     {
        Evas_Object *obj = (Evas_Object *)edje_object_part_object_get
            (wd->resize_obj, pc->part);

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
              const char *emission EINA_UNUSED, const char *source EINA_UNUSED)
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

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   _parts_swallow_fix(sd, wd);
   _parts_text_fix(sd);
   _parts_signals_emit(sd);
   _parts_cursors_apply(sd);

   eo_do(obj, elm_obj_layout_sizing_eval());

   edje_object_signal_callback_del(wd->resize_obj,
                                   "edje,change,file", "edje",
                                   _reload_theme);
   edje_object_signal_callback_add(wd->resize_obj,
                                   "edje,change,file", "edje",
                                   _reload_theme, obj);
}

static void
_elm_layout_smart_disable(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   Eina_Bool *ret = va_arg(*list, Eina_Bool *);

   if (elm_object_disabled_get(obj))
     edje_object_signal_emit
       (wd->resize_obj, "elm,state,disabled", "elm");
   else
     edje_object_signal_emit
       (wd->resize_obj, "elm,state,enabled", "elm");

   if (ret) *ret = EINA_TRUE;
}

static void
_elm_layout_highlight_in_theme(Evas_Object *obj)
{
   const char *fh;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   fh = edje_object_data_get
       (wd->resize_obj, "focus_highlight");
   if ((fh) && (!strcmp(fh, "on")))
     elm_widget_highlight_in_theme_set(obj, EINA_TRUE);
   else
     elm_widget_highlight_in_theme_set(obj, EINA_FALSE);

   fh = edje_object_data_get
       (wd->resize_obj, "access_highlight");
   if ((fh) && (!strcmp(fh, "on")))
     elm_widget_access_highlight_in_theme_set(obj, EINA_TRUE);
   else
     elm_widget_access_highlight_in_theme_set(obj, EINA_FALSE);
}

static Eina_Bool
_elm_layout_theme_internal(Eo *obj, Elm_Layout_Smart_Data *sd)
{
   Eina_Bool ret = EINA_FALSE;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   /* function already prints error messages, if any */
   if (!elm_widget_theme_object_set(obj, wd->resize_obj, sd->klass, sd->group,
                                    elm_widget_style_get(obj)))
     return EINA_FALSE;

   edje_object_mirrored_set
     (wd->resize_obj, elm_widget_mirrored_get(obj));

   edje_object_scale_set
     (wd->resize_obj,
     elm_widget_scale_get(obj) * elm_config_scale_get());

   _elm_layout_highlight_in_theme(obj);

   evas_object_smart_callback_call(obj, SIG_THEME_CHANGED, NULL);

   _visuals_refresh(obj, sd);

   eo_do(obj, elm_obj_widget_disable(&ret));

   return ret;
}

static void
_elm_layout_smart_theme(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret = EINA_FALSE;

   Elm_Layout_Smart_Data *sd = _pd;

   eo_do_super(obj, MY_CLASS, elm_obj_widget_theme_apply(&int_ret));
   if (!int_ret) return;
   /* The following lines are here to support entry design; the _theme function
    * of entry needs to call directly the widget _theme function */
   Eina_Bool enable = EINA_TRUE;
   eo_do(obj, elm_obj_layout_theme_enable(&enable));
   if (!enable)
     {
        if (ret) *ret = EINA_TRUE;
        return;
     }

   int_ret = _elm_layout_theme_internal(obj, sd);

   if (ret) *ret = int_ret;
}

static void *
_elm_layout_list_data_get(const Eina_List *list)
{
   Elm_Layout_Sub_Object_Data *sub_d = eina_list_data_get(list);

   return sub_d->obj;
}

static void
_elm_layout_smart_on_focus(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   if (!elm_widget_can_focus_get(obj)) return;

   if (elm_widget_focus_get(obj))
     {
        elm_layout_signal_emit(obj, "elm,action,focus", "elm");
        evas_object_focus_set(wd->resize_obj, EINA_TRUE);
        evas_object_smart_callback_call(obj, SIG_LAYOUT_FOCUSED, NULL);
     }
   else
     {
        elm_layout_signal_emit(obj, "elm,action,unfocus", "elm");
        evas_object_focus_set(wd->resize_obj, EINA_FALSE);
        evas_object_smart_callback_call(obj, SIG_LAYOUT_UNFOCUSED, NULL);
     }

   if (ret) *ret = EINA_TRUE;
}

static void
_elm_layout_smart_focus_next_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (!elm_widget_can_focus_get(obj))
     *ret = EINA_TRUE;
   else
     *ret = EINA_FALSE;
}

static int
_access_focus_list_sort_cb(const void *data1, const void *data2)
{
   Evas_Coord_Point p1, p2;
   Evas_Object *obj1, *obj2;

   obj1 = ((Elm_Layout_Sub_Object_Data *)data1)->obj;
   obj2 = ((Elm_Layout_Sub_Object_Data *)data2)->obj;

   evas_object_geometry_get(obj1, &p1.x, &p1.y, NULL, NULL);
   evas_object_geometry_get(obj2, &p2.x, &p2.y, NULL, NULL);

   if (p1.y == p2.y)
     {
        return p1.x - p2.x;
     }

   return p1.y - p2.y;
}

static const Eina_List *
_access_focus_list_sort(Eina_List *origin)
{
   Eina_List *l, *temp = NULL;
   Elm_Layout_Sub_Object_Data *sub_d;

   EINA_LIST_FOREACH(origin, l, sub_d)
     temp = eina_list_sorted_insert(temp, _access_focus_list_sort_cb, sub_d);

   return temp;
}

/* WARNING: if you're making a widget *not* supposed to have focusable
 * child objects, but still inheriting from elm_layout, just set its
 * focus_next smart function back to NULL */
static void
_elm_layout_smart_focus_next(Eo *obj, void *_pd, va_list *list)
{
   Elm_Focus_Direction dir = va_arg(*list, Elm_Focus_Direction);
   Evas_Object **next = va_arg(*list, Evas_Object **);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret  = EINA_FALSE;

   const Eina_List *items;
   void *(*list_data_get)(const Eina_List *list);

   Elm_Layout_Smart_Data *sd = _pd;

   if ((items = elm_widget_focus_custom_chain_get(obj)))
     list_data_get = eina_list_data_get;
   else
     {
        items = sd->subs;
        list_data_get = _elm_layout_list_data_get;

        if (!items) return;

        if (_elm_config->access_mode)
          items = _access_focus_list_sort((Eina_List *)items);
     }

   int_ret = elm_widget_focus_list_next_get
            (obj, items, list_data_get, dir, next);
   if (ret) *ret = int_ret;
}

static void
_elm_layout_smart_sub_object_add(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Object *sobj = va_arg(*list, Evas_Object *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret = EINA_FALSE;

   if (evas_object_data_get(sobj, "elm-parent") == obj)
     goto end;

   eo_do_super(obj, MY_CLASS, elm_obj_widget_sub_object_add(sobj, &int_ret));
   if (!int_ret) return;

   Eina_Bool enable = EINA_TRUE;
   eo_do(obj, elm_obj_layout_sub_object_add_enable(&enable));

   if (EINA_TRUE == enable)
     evas_object_event_callback_add
       (sobj, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
         _on_sub_object_size_hint_change, obj);

end:
   if (ret) *ret = EINA_TRUE;
}

static void
_elm_layout_smart_sub_object_del(Eo *obj, void *_pd, va_list *list)
{
   Eina_List *l;
   Elm_Layout_Sub_Object_Data *sub_d;

   Evas_Object *sobj = va_arg(*list, Evas_Object *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret = EINA_FALSE;

   Elm_Layout_Smart_Data *sd = _pd;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   evas_object_event_callback_del_full
     (sobj, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
     _on_sub_object_size_hint_change, obj);

   eo_do_super(obj, MY_CLASS, elm_obj_widget_sub_object_del(sobj, &int_ret));
   if (!int_ret) return;

   EINA_LIST_FOREACH(sd->subs, l, sub_d)
     {
        if (sub_d->obj != sobj) continue;

        sd->subs = eina_list_remove_list(sd->subs, l);

        _icon_signal_emit(sd, sub_d, EINA_FALSE);

        eina_stringshare_del(sub_d->part);
        free(sub_d);

        break;
     }

   eo_do(obj, elm_obj_layout_sizing_eval());

   if (ret) *ret = EINA_TRUE;
}

static void
_elm_layout_smart_focus_direction_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (!elm_widget_can_focus_get(obj))
     *ret = EINA_TRUE;
   else
     *ret = EINA_FALSE;
}

static void
_elm_layout_smart_focus_direction(Eo *obj, void *_pd, va_list *list)
{
   const Evas_Object *base = va_arg(*list, const Evas_Object *);
   double degree = va_arg(*list, double);
   Evas_Object **direction = va_arg(*list, Evas_Object **);
   double *weight = va_arg(*list, double *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   Eina_Bool int_ret = EINA_FALSE;

   const Eina_List *items;
   void *(*list_data_get)(const Eina_List *list);

   Elm_Layout_Smart_Data *sd = _pd;

   if (!sd->subs) return;

   /* Focus chain (This block is different from elm_win cycle) */
   if ((items = elm_widget_focus_custom_chain_get(obj)))
     list_data_get = eina_list_data_get;
   else
     {
        items = sd->subs;
        list_data_get = _elm_layout_list_data_get;

        if (!items) return;
     }

   int_ret = elm_widget_focus_list_direction_get
            (obj, base, items, list_data_get, degree, direction, weight);
   if (ret) *ret = int_ret;
}

static void
_edje_signal_callback(void *data,
                      Evas_Object *obj EINA_UNUSED,
                      const char *emission,
                      const char *source)
{
   Edje_Signal_Data *esd = data;

   esd->func(esd->data, esd->obj, emission, source);
}

static void
_elm_layout_smart_text_aliases_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   const Elm_Layout_Part_Alias_Description **aliases = va_arg(*list, const Elm_Layout_Part_Alias_Description **);
   *aliases = NULL;
}

static void
_elm_layout_smart_content_aliases_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   const Elm_Layout_Part_Alias_Description **aliases = va_arg(*list, const Elm_Layout_Part_Alias_Description **);
   *aliases = NULL;
}

static void
_elm_layout_smart_sub_object_add_enable(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *enable = va_arg(*list, Eina_Bool *);
   *enable = EINA_TRUE;
}

static void
_elm_layout_smart_theme_enable(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *enable = va_arg(*list, Eina_Bool *);
   *enable = EINA_TRUE;
}

static Eina_Bool
_elm_layout_part_aliasing_eval(const Evas_Object *obj EINA_UNUSED,
                               Elm_Layout_Smart_Data *sd,
                               const char **part,
                               Eina_Bool is_text)
{
   const Elm_Layout_Part_Alias_Description *aliases = NULL;

   if (is_text)
     eo_do(sd->obj, elm_obj_layout_smart_text_aliases_get(&aliases));
   else
     eo_do(sd->obj, elm_obj_layout_smart_content_aliases_get(&aliases));

   while (aliases && aliases->alias && aliases->real_part)
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
            "part must not be NULL", sd->obj);
        return EINA_FALSE;
     }

   /* if no match, part goes on with the same value */

   return EINA_TRUE;
}

static void
_box_reference_del(void *data,
                   Evas *e EINA_UNUSED,
                   Evas_Object *obj EINA_UNUSED,
                   void *event_info EINA_UNUSED)
{
   Elm_Layout_Sub_Object_Data *sub_d = data;
   sub_d->p.box.reference = NULL;
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

   ELM_WIDGET_DATA_GET_OR_RETURN(sd->obj, wd, NULL);
   edje_object_part_box_remove
     (wd->resize_obj, sub_d->part, child);

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
_sub_table_remove(Evas_Object *obj,
                  Elm_Layout_Smart_Data *sd,
                  Elm_Layout_Sub_Object_Data *sub_d)
{
   Evas_Object *child;
   ELM_WIDGET_DATA_GET_OR_RETURN(sd->obj, wd, NULL);

   child = sub_d->obj; /* sub_d will die in _elm_layout_smart_sub_object_del */

   edje_object_part_table_unpack
     (wd->resize_obj, sub_d->part, child);

   if (!elm_widget_sub_object_del(obj, child))
     {
        ERR("could not remove sub object %p from %p", child, obj);
        return NULL;
     }

   return child;
}

static void
_on_size_evaluate_signal(void *data,
                         Evas_Object *obj EINA_UNUSED,
                         const char *emission EINA_UNUSED,
                         const char *source EINA_UNUSED)
{
   eo_do(data, elm_obj_layout_sizing_eval());
}

static void
_elm_layout_smart_add(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   Evas_Object *edje;

   elm_widget_sub_object_parent_add(obj);

   /* has to be there *before* parent's smart_add() */
   edje = edje_object_add(evas_object_evas_get(obj));
   elm_widget_resize_object_set(obj, edje, EINA_TRUE);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());

   elm_widget_can_focus_set(obj, EINA_FALSE);

   edje_object_signal_callback_add
     (edje, "size,eval", "elm", _on_size_evaluate_signal, obj);

   eo_do(obj, elm_obj_layout_sizing_eval());
}

static void
_elm_layout_smart_del(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Layout_Sub_Object_Data *sub_d;
   Elm_Layout_Sub_Object_Cursor *pc;
   Edje_Signal_Data *esd;
   Evas_Object *child;
   Eina_List *l;

   Elm_Layout_Smart_Data *sd = _pd;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   elm_layout_freeze(obj);

   EINA_LIST_FREE(sd->subs, sub_d)
     {
        eina_stringshare_del(sub_d->part);

        if (sub_d->type == TEXT)
          eina_stringshare_del(sub_d->p.text.text);

        free(sub_d);
     }

   EINA_LIST_FREE(sd->parts_cursors, pc)
     _part_cursor_free(pc);

   EINA_LIST_FREE(sd->edje_signals, esd)
     {
        edje_object_signal_callback_del_full
           (wd->resize_obj, esd->emission, esd->source,
            _edje_signal_callback, esd);
        eina_stringshare_del(esd->emission);
        eina_stringshare_del(esd->source);
        free(esd);
     }

   eina_stringshare_del(sd->klass);
   eina_stringshare_del(sd->group);

   /* let's make our Edje object the *last* to be processed, since it
    * may (smart) parent other sub objects here */
   EINA_LIST_FOREACH(wd->subobjs, l, child)
     {
        if (child == wd->resize_obj)
          {
             wd->subobjs =
               eina_list_demote_list(wd->subobjs, l);
             break;
          }
     }

   eo_do_super(obj, MY_CLASS, evas_obj_smart_del());
}

/* rewrite or extend this one on your derived class as to suit your
 * needs */
static void
_elm_layout_smart_calculate(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Layout_Smart_Data *sd = _pd;

   if (sd->needs_size_calc)
     {
        _sizing_eval(obj, sd);
        sd->needs_size_calc = EINA_FALSE;
     }
}

static Elm_Layout_Sub_Object_Cursor *
_parts_cursors_find(Elm_Layout_Smart_Data *sd,
                    const char *part)
{
   const Eina_List *l;
   Elm_Layout_Sub_Object_Cursor *pc;

   EINA_LIST_FOREACH(sd->parts_cursors, l, pc)
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
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, elm_obj_layout_file_set(file, group, &ret));
   return ret;
}

static void
_elm_layout_smart_file_set(Eo *obj, void *_pd, va_list *list)
{
   const char *file = va_arg(*list, const char *);
   const char *group = va_arg(*list, const char *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Eina_Bool int_ret = EINA_FALSE;

   Elm_Layout_Smart_Data *sd = _pd;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   int_ret =
     edje_object_file_set(wd->resize_obj, file, group);

   if (int_ret) _visuals_refresh(obj, sd);
   else
     ERR("failed to set edje file '%s', group '%s': %s",
         file, group,
         edje_load_error_str
           (edje_object_load_error_get(wd->resize_obj)));

   if (ret) *ret = int_ret;
}

EAPI Eina_Bool
elm_layout_theme_set(Evas_Object *obj,
                     const char *klass,
                     const char *group,
                     const char *style)
{
   ELM_LAYOUT_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, elm_obj_layout_theme_set(klass, group, style, &ret));
   return ret;
}

static void
_elm_layout_smart_theme_set(Eo *obj, void *_pd, va_list *list)
{
   const char *klass = va_arg(*list, const char *);
   const char *group = va_arg(*list, const char *);
   const char *style = va_arg(*list, const char *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Eina_Bool int_ret = EINA_FALSE;

   Elm_Layout_Smart_Data *sd = _pd;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   eina_stringshare_replace(&(sd->klass), klass);
   eina_stringshare_replace(&(sd->group), group);
   eina_stringshare_replace(&(wd->style), style);

   int_ret = _elm_layout_theme_internal(obj, sd);

   if (ret) *ret = int_ret;
}

EAPI void
elm_layout_signal_emit(Evas_Object *obj,
                       const char *emission,
                       const char *source)
{
   ELM_LAYOUT_CHECK(obj);
   eo_do(obj, elm_obj_layout_signal_emit(emission, source));
}

static void
_elm_layout_smart_signal_emit(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   const char *emission = va_arg(*list, const char *);
   const char *source = va_arg(*list, const char *);

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   edje_object_signal_emit(wd->resize_obj, emission, source);
}

EAPI void
elm_layout_signal_callback_add(Evas_Object *obj,
                               const char *emission,
                               const char *source,
                               Edje_Signal_Cb func,
                               void *data)
{
   ELM_LAYOUT_CHECK(obj);
   eo_do(obj, elm_obj_layout_signal_callback_add(emission, source, func, data));
}

static void
_elm_layout_smart_signal_callback_add(Eo *obj, void *_pd, va_list *list)
{
   const char *emission = va_arg(*list, const char *);
   const char *source = va_arg(*list, const char *);
   Edje_Signal_Cb func_cb = va_arg(*list, Edje_Signal_Cb);
   void *data = va_arg(*list, void *);

   Edje_Signal_Data *esd;

   Elm_Layout_Smart_Data *sd = _pd;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   esd = ELM_NEW(Edje_Signal_Data);
   if (!esd) return;

   esd->obj = obj;
   esd->func = func_cb;
   esd->emission = eina_stringshare_add(emission);
   esd->source = eina_stringshare_add(source);
   esd->data = data;
   sd->edje_signals = eina_list_append(sd->edje_signals, esd);

   edje_object_signal_callback_add
     (wd->resize_obj, emission, source,
     _edje_signal_callback, esd);
}

EAPI void *
elm_layout_signal_callback_del(Evas_Object *obj,
                               const char *emission,
                               const char *source,
                               Edje_Signal_Cb func)
{
   ELM_LAYOUT_CHECK(obj) NULL;
   void *ret = NULL;
   eo_do(obj, elm_obj_layout_signal_callback_del(emission, source, func, &ret));
   return ret;
}

static void
_elm_layout_smart_signal_callback_del(Eo *obj, void *_pd, va_list *list)
{
   Edje_Signal_Data *esd = NULL;
   void *data = NULL;
   Eina_List *l;

   const char *emission = va_arg(*list, const char *);
   const char *source = va_arg(*list, const char *);
   Edje_Signal_Cb func_cb = va_arg(*list, Edje_Signal_Cb);
   void **ret = va_arg(*list, void **);
   if (ret) *ret = NULL;

   Elm_Layout_Smart_Data *sd = _pd;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   EINA_LIST_FOREACH(sd->edje_signals, l, esd)
     {
        if ((esd->func == func_cb) && (!strcmp(esd->emission, emission)) &&
            (!strcmp(esd->source, source)))
          {
             sd->edje_signals = eina_list_remove_list(sd->edje_signals, l);
             eina_stringshare_del(esd->emission);
             eina_stringshare_del(esd->source);
             data = esd->data;

             edje_object_signal_callback_del_full
               (wd->resize_obj, emission, source,
               _edje_signal_callback, esd);

             free(esd);

             if (ret) *ret = data;
             return; /* stop at 1st match */
          }
     }
}

EAPI Eina_Bool
elm_layout_content_set(Evas_Object *obj,
                       const char *swallow,
                       Evas_Object *content)
{
   ELM_LAYOUT_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, elm_obj_container_content_set(swallow, content, &ret));
   return ret;
}

static void
_elm_layout_smart_content_set(Eo *obj, void *_pd, va_list *list)
{
   Elm_Layout_Sub_Object_Data *sub_d;
   const Eina_List *l;

   const char *part = va_arg(*list, const char *);
   Evas_Object *content = va_arg(*list, Evas_Object *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   Elm_Layout_Smart_Data *sd = _pd;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (!_elm_layout_part_aliasing_eval(obj, sd, &part, EINA_FALSE))
     return;

   EINA_LIST_FOREACH(sd->subs, l, sub_d)
     {
        if (sub_d->type == SWALLOW)
          {
             if (!strcmp(part, sub_d->part))
               {
                  if (content == sub_d->obj) goto end;
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
          return;

        if (!edje_object_part_swallow
              (wd->resize_obj, part, content))
          {
             ERR("could not swallow %p into part '%s'", content, part);
             return;
          }

        sub_d = ELM_NEW(Elm_Layout_Sub_Object_Data);
        sub_d->type = SWALLOW;
        sub_d->part = eina_stringshare_add(part);
        sub_d->obj = content;
        sd->subs = eina_list_append(sd->subs, sub_d);

        _icon_signal_emit(sd, sub_d, EINA_TRUE);
     }

   eo_do(obj, elm_obj_layout_sizing_eval());

end:
   if (ret) *ret = EINA_TRUE;
}

EAPI Evas_Object *
elm_layout_content_get(const Evas_Object *obj,
                       const char *swallow)
{
   ELM_LAYOUT_CHECK(obj) NULL;
   Evas_Object *ret = NULL;
   eo_do((Eo *) obj, elm_obj_container_content_get(swallow, &ret));
   return ret;
}

static void
_elm_layout_smart_content_get(Eo *obj, void *_pd, va_list *list)
{
   const Eina_List *l;
   Elm_Layout_Sub_Object_Data *sub_d;

   const char *part = va_arg(*list, const char *);
   Evas_Object **content = va_arg(*list, Evas_Object **);
   *content = NULL;

   Elm_Layout_Smart_Data *sd = _pd;

   if (!_elm_layout_part_aliasing_eval(obj, sd, &part, EINA_FALSE))
     return;

   EINA_LIST_FOREACH(sd->subs, l, sub_d)
     {
        if ((sub_d->type == SWALLOW) && !strcmp(part, sub_d->part))
          {
             *content = sub_d->obj;
             return;
          }
     }
}

EAPI Evas_Object *
elm_layout_content_unset(Evas_Object *obj,
                         const char *swallow)
{
   ELM_LAYOUT_CHECK(obj) NULL;
   Evas_Object *ret = NULL;
   eo_do(obj, elm_obj_container_content_unset(swallow, &ret));
   return ret;
}

static void
_elm_layout_smart_content_unset(Eo *obj, void *_pd, va_list *list)
{
   Elm_Layout_Sub_Object_Data *sub_d;
   const Eina_List *l;

   const char *part = va_arg(*list, const char *);
   Evas_Object **ret = va_arg(*list, Evas_Object **);
   if (ret) *ret = NULL;

   Elm_Layout_Smart_Data *sd = _pd;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (!_elm_layout_part_aliasing_eval(obj, sd, &part, EINA_FALSE))
     return;

   EINA_LIST_FOREACH(sd->subs, l, sub_d)
     {
        if ((sub_d->type == SWALLOW) && (!strcmp(part, sub_d->part)))
          {
             Evas_Object *content;

             if (!sub_d->obj) return;

             content = sub_d->obj; /* sub_d will die in
                                    * _elm_layout_smart_sub_object_del */

             if (!elm_widget_sub_object_del(obj, content))
               {
                  ERR("could not remove sub object %p from %p", content, obj);
                  return;
               }

             edje_object_part_unswallow
               (wd->resize_obj, content);
             if (ret) *ret = content;
             return;
          }
     }
}

EAPI Eina_List *
elm_layout_content_swallow_list_get(const Evas_Object *obj)
{
   ELM_LAYOUT_CHECK(obj) NULL;
   Eina_List *ret = NULL;
   eo_do(obj, elm_obj_container_content_swallow_list_get(&ret));
   return ret;
}

static void
_elm_layout_smart_content_swallow_list_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Layout_Smart_Data *sd = _pd;

   Eina_List **ret = va_arg(*list, Eina_List **);
   if (ret) *ret = NULL;

   Elm_Layout_Sub_Object_Data *sub_d = NULL;
   Eina_List *l = NULL;

   EINA_LIST_FOREACH(sd->subs, l, sub_d)
     {
        if ((sub_d->type == SWALLOW) && ret)
          *ret = eina_list_append(*ret, sub_d->obj);
     }
}

EAPI Eina_Bool
elm_layout_text_set(Evas_Object *obj,
                    const char *part,
                    const char *text)
{
   ELM_LAYOUT_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   text = elm_widget_part_text_translate(obj, part, text);
   eo_do(obj, elm_obj_layout_text_set(part, text, &ret));
   return ret;
}

static void
_elm_layout_smart_text_set(Eo *obj, void *_pd, va_list *list)
{
   Elm_Layout_Smart_Data *sd = _pd;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   const char *part = va_arg(*list, const char *);
   const char *text = va_arg(*list, const char *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   Eina_List *l;
   Elm_Layout_Sub_Object_Data *sub_d = NULL;

   if (!_elm_layout_part_aliasing_eval(obj, sd, &part, EINA_TRUE))
     return;

   EINA_LIST_FOREACH(sd->subs, l, sub_d)
     {
        if ((sub_d->type == TEXT) && (!strcmp(part, sub_d->part)))
          {
             if (!text)
               {
                  eina_stringshare_del(sub_d->part);
                  eina_stringshare_del(sub_d->p.text.text);
                  free(sub_d);
                  edje_object_part_text_escaped_set
                    (wd->resize_obj, part, NULL);
                  sd->subs = eina_list_remove_list(sd->subs, l);
                  goto end;
               }
             else
               break;
          }
        sub_d = NULL;
     }

   if (!edje_object_part_text_escaped_set
         (wd->resize_obj, part, text))
     return;

   if (!sub_d)
     {
        sub_d = ELM_NEW(Elm_Layout_Sub_Object_Data);
        if (!sub_d) return;
        sub_d->type = TEXT;
        sub_d->part = eina_stringshare_add(part);
        sd->subs = eina_list_append(sd->subs, sub_d);
     }

   eina_stringshare_replace(&sub_d->p.text.text, text);

   _text_signal_emit(sd, sub_d, !!text);

   eo_do(obj, elm_obj_layout_sizing_eval());

   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON &&
       sd->can_access && !(sub_d->obj))
     sub_d->obj = _elm_access_edje_object_part_object_register
         (obj, elm_layout_edje_get(obj), part);

end:
   if (ret) *ret = EINA_TRUE;
}

EAPI const char *
elm_layout_text_get(const Evas_Object *obj,
                    const char *part)
{
   ELM_LAYOUT_CHECK(obj) NULL;

   const char *ret = NULL;
   eo_do((Eo *) obj, elm_obj_layout_text_get(part, &ret));
   return ret;
}

static void
_elm_layout_smart_text_get(Eo *obj, void *_pd, va_list *list)
{
   Elm_Layout_Smart_Data *sd = _pd;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   const char *part = va_arg(*list, const char *);
   const char **text = va_arg(*list, const char **);
   *text = NULL;

   if (!_elm_layout_part_aliasing_eval(obj, sd, &part, EINA_TRUE))
     return;

   *text = edje_object_part_text_get(wd->resize_obj, part);
}

EAPI Eina_Bool
elm_layout_box_append(Evas_Object *obj,
                      const char *part,
                      Evas_Object *child)
{
   ELM_LAYOUT_CHECK(obj) EINA_FALSE;
   EINA_SAFETY_ON_NULL_RETURN_VAL(child, EINA_FALSE);

   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, elm_obj_layout_box_append(part, child, &ret));
   return ret;
}

static void
_elm_layout_smart_box_append(Eo *obj, void *_pd, va_list *list)
{
   Elm_Layout_Sub_Object_Data *sub_d;

   const char *part = va_arg(*list, const char *);
   Evas_Object *child = va_arg(*list, Evas_Object *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   Elm_Layout_Smart_Data *sd = _pd;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (!edje_object_part_box_append
         (wd->resize_obj, part, child))
     {
        ERR("child %p could not be appended to box part '%s'", child, part);
        return;
     }

   if (!elm_widget_sub_object_add(obj, child))
     {
        edje_object_part_box_remove
          (wd->resize_obj, part, child);
        return;
     }

   sub_d = ELM_NEW(Elm_Layout_Sub_Object_Data);
   sub_d->type = BOX_APPEND;
   sub_d->part = eina_stringshare_add(part);
   sub_d->obj = child;
   sd->subs = eina_list_append(sd->subs, sub_d);

   eo_do(obj, elm_obj_layout_sizing_eval());

   if (ret) *ret = EINA_TRUE;
}

EAPI Eina_Bool
elm_layout_box_prepend(Evas_Object *obj,
                       const char *part,
                       Evas_Object *child)
{
   ELM_LAYOUT_CHECK(obj) EINA_FALSE;
   EINA_SAFETY_ON_NULL_RETURN_VAL(child, EINA_FALSE);

   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, elm_obj_layout_box_prepend(part, child, &ret));
   return ret;
}

static void
_elm_layout_smart_box_prepend(Eo *obj, void *_pd, va_list *list)
{
   Elm_Layout_Sub_Object_Data *sub_d;

   const char *part = va_arg(*list, const char *);
   Evas_Object *child = va_arg(*list, Evas_Object *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   Elm_Layout_Smart_Data *sd = _pd;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (!edje_object_part_box_prepend
         (wd->resize_obj, part, child))
     {
        ERR("child %p could not be prepended to box part '%s'", child, part);
        return;
     }

   if (!elm_widget_sub_object_add(obj, child))
     {
        edje_object_part_box_remove
          (wd->resize_obj, part, child);
        return;
     }

   sub_d = ELM_NEW(Elm_Layout_Sub_Object_Data);
   sub_d->type = BOX_PREPEND;
   sub_d->part = eina_stringshare_add(part);
   sub_d->obj = child;
   sd->subs = eina_list_prepend(sd->subs, sub_d);

   eo_do(obj, elm_obj_layout_sizing_eval());

   if (ret) *ret = EINA_TRUE;
}

EAPI Eina_Bool
elm_layout_box_insert_before(Evas_Object *obj,
                             const char *part,
                             Evas_Object *child,
                             const Evas_Object *reference)
{
   ELM_LAYOUT_CHECK(obj) EINA_FALSE;
   EINA_SAFETY_ON_NULL_RETURN_VAL(child, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(reference, EINA_FALSE);

   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, elm_obj_layout_box_insert_before(part, child, reference, &ret));
   return ret;
}

static void
_elm_layout_smart_box_insert_before(Eo *obj, void *_pd, va_list *list)
{
   Elm_Layout_Sub_Object_Data *sub_d;

   const char *part = va_arg(*list, const char *);
   Evas_Object *child = va_arg(*list, Evas_Object *);
   const Evas_Object *reference = va_arg(*list, const Evas_Object *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   Elm_Layout_Smart_Data *sd = _pd;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (!edje_object_part_box_insert_before
         (wd->resize_obj, part, child, reference))
     {
        ERR("child %p could not be inserted before %p inf box part '%s'",
            child, reference, part);
        return;
     }

   if (!elm_widget_sub_object_add(obj, child))
     {
        edje_object_part_box_remove
          (wd->resize_obj, part, child);
        return;
     }

   sub_d = ELM_NEW(Elm_Layout_Sub_Object_Data);
   sub_d->type = BOX_INSERT_BEFORE;
   sub_d->part = eina_stringshare_add(part);
   sub_d->obj = child;
   sub_d->p.box.reference = reference;
   sd->subs = eina_list_append(sd->subs, sub_d);

   evas_object_event_callback_add
     ((Evas_Object *)reference, EVAS_CALLBACK_DEL, _box_reference_del, sub_d);

   eo_do(obj, elm_obj_layout_sizing_eval());

   if (ret) *ret = EINA_TRUE;
}

EAPI Eina_Bool
elm_layout_box_insert_at(Evas_Object *obj,
                         const char *part,
                         Evas_Object *child,
                         unsigned int pos)
{
   ELM_LAYOUT_CHECK(obj) EINA_FALSE;
   EINA_SAFETY_ON_NULL_RETURN_VAL(child, EINA_FALSE);

   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, elm_obj_layout_box_insert_at(part, child, pos, &ret));
   return ret;
}

static void
_elm_layout_smart_box_insert_at(Eo *obj, void *_pd, va_list *list)
{
   Elm_Layout_Sub_Object_Data *sub_d;

   const char *part = va_arg(*list, const char *);
   Evas_Object *child = va_arg(*list, Evas_Object *);
   unsigned int pos = va_arg(*list, unsigned int);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   Elm_Layout_Smart_Data *sd = _pd;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (!edje_object_part_box_insert_at
         (wd->resize_obj, part, child, pos))
     {
        ERR("child %p could not be inserted at %u to box part '%s'",
            child, pos, part);
        return;
     }

   if (!elm_widget_sub_object_add(obj, child))
     {
        edje_object_part_box_remove
          (wd->resize_obj, part, child);
        return;
     }

   sub_d = ELM_NEW(Elm_Layout_Sub_Object_Data);
   sub_d->type = BOX_INSERT_AT;
   sub_d->part = eina_stringshare_add(part);
   sub_d->obj = child;
   sub_d->p.box.pos = pos;
   sd->subs = eina_list_append(sd->subs, sub_d);

   eo_do(obj, elm_obj_layout_sizing_eval());

   if (ret) *ret = EINA_TRUE;
}

EAPI Evas_Object *
elm_layout_box_remove(Evas_Object *obj,
                      const char *part,
                      Evas_Object *child)
{
   ELM_LAYOUT_CHECK(obj) NULL;

   Evas_Object *ret = NULL;
   eo_do(obj, elm_obj_layout_box_remove(part, child, &ret));
   return ret;
}

static void
_elm_layout_smart_box_remove(Eo *obj, void *_pd, va_list *list)
{

   const char *part = va_arg(*list, const char *);
   Evas_Object *child = va_arg(*list, Evas_Object *);
   Evas_Object **ret = va_arg(*list, Evas_Object **);
   if (ret) *ret = NULL;
   Evas_Object *int_ret = NULL;

   EINA_SAFETY_ON_NULL_RETURN(part);
   EINA_SAFETY_ON_NULL_RETURN(child);

   Elm_Layout_Smart_Data *sd = _pd;

   const Eina_List *l;
   Elm_Layout_Sub_Object_Data *sub_d;

   EINA_LIST_FOREACH(sd->subs, l, sub_d)
     {
        if (!_sub_box_is(sub_d)) continue;
        if ((sub_d->obj == child) && (!strcmp(sub_d->part, part)))
          {
             int_ret = _sub_box_remove(obj, sd, sub_d);
             if (ret) *ret = int_ret;
             return;
          }
     }
}

EAPI Eina_Bool
elm_layout_box_remove_all(Evas_Object *obj,
                          const char *part,
                          Eina_Bool clear)
{
   ELM_LAYOUT_CHECK(obj) EINA_FALSE;

   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, elm_obj_layout_box_remove_all(part, clear, &ret));
   return ret;
}

static void
_elm_layout_smart_box_remove_all(Eo *obj, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Eina_Bool clear = va_arg(*list, int);
   Eina_Bool *ret= va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   EINA_SAFETY_ON_NULL_RETURN(part);

   Elm_Layout_Smart_Data *sd = _pd;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   Elm_Layout_Sub_Object_Data *sub_d;
   Eina_List *lst;

   lst = eina_list_clone(sd->subs);
   EINA_LIST_FREE(lst, sub_d)
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
     (wd->resize_obj, part, clear);

   if (ret) *ret = EINA_TRUE;
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

   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, elm_obj_layout_table_pack(part, child, col, row, colspan, rowspan, &ret));
   return ret;
}

static void
_elm_layout_smart_table_pack(Eo *obj, void *_pd, va_list *list)
{
   Elm_Layout_Sub_Object_Data *sub_d;

   const char *part = va_arg(*list, const char *);
   Evas_Object *child = va_arg(*list, Evas_Object *);
   unsigned short col = va_arg(*list, unsigned int);
   unsigned short row = va_arg(*list, unsigned int);
   unsigned short colspan = va_arg(*list, unsigned int);
   unsigned short rowspan = va_arg(*list, unsigned int);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   Elm_Layout_Smart_Data *sd = _pd;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (!edje_object_part_table_pack
         (wd->resize_obj, part, child, col,
         row, colspan, rowspan))
     {
        ERR("child %p could not be packed into box part '%s' col=%uh, row=%hu,"
            " colspan=%hu, rowspan=%hu", child, part, col, row, colspan,
            rowspan);
        return;
     }

   if (!elm_widget_sub_object_add(obj, child))
     {
        edje_object_part_table_unpack
          (wd->resize_obj, part, child);
        return;
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

   eo_do(obj, elm_obj_layout_sizing_eval());

   if (ret) *ret = EINA_TRUE;
}

EAPI Evas_Object *
elm_layout_table_unpack(Evas_Object *obj,
                        const char *part,
                        Evas_Object *child)
{
   ELM_LAYOUT_CHECK(obj) NULL;

   Evas_Object *ret = NULL;
   eo_do(obj, elm_obj_layout_table_unpack(part, child, &ret));
   return ret;
}

static void
_elm_layout_smart_table_unpack(Eo *obj, void *_pd, va_list *list)
{

   const char *part = va_arg(*list, const char *);
   Evas_Object *child = va_arg(*list, Evas_Object *);
   Evas_Object **ret = va_arg(*list, Evas_Object **);
   if (ret) *ret = NULL;
   Evas_Object *int_ret = NULL;

   EINA_SAFETY_ON_NULL_RETURN(part);
   EINA_SAFETY_ON_NULL_RETURN(child);

   Elm_Layout_Smart_Data *sd = _pd;

   const Eina_List *l;
   Elm_Layout_Sub_Object_Data *sub_d;

   EINA_LIST_FOREACH(sd->subs, l, sub_d)
     {
        if (sub_d->type != TABLE_PACK) continue;
        if ((sub_d->obj == child) && (!strcmp(sub_d->part, part)))
          {
             int_ret = _sub_table_remove(obj, sd, sub_d);
             if (ret) *ret = int_ret;
             return;
          }
     }
}

EAPI Eina_Bool
elm_layout_table_clear(Evas_Object *obj,
                       const char *part,
                       Eina_Bool clear)
{
   ELM_LAYOUT_CHECK(obj) EINA_FALSE;

   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, elm_obj_layout_table_clear(part, clear, &ret));
   return ret;
}

static void
_elm_layout_smart_table_clear(Eo *obj, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Eina_Bool clear = va_arg(*list, int);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   EINA_SAFETY_ON_NULL_RETURN(part);

   Elm_Layout_Smart_Data *sd = _pd;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   Elm_Layout_Sub_Object_Data *sub_d;
   Eina_List *lst;

   lst = eina_list_clone(sd->subs);
   EINA_LIST_FREE(lst, sub_d)
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
   edje_object_part_table_clear(wd->resize_obj, part, clear);

   if (ret) *ret = EINA_TRUE;
}

EAPI Evas_Object *
elm_layout_edje_get(const Evas_Object *obj)
{
   ELM_LAYOUT_CHECK(obj) NULL;
   Evas_Object *ret = NULL;
   eo_do((Eo *) obj, elm_obj_layout_edje_get(&ret));
   return ret;
}

static void
_elm_layout_smart_edje_get(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Object **ret = va_arg(*list, Evas_Object **);

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   *ret = wd->resize_obj;
}

EAPI const char *
elm_layout_data_get(const Evas_Object *obj,
                    const char *key)
{
   ELM_LAYOUT_CHECK(obj) NULL;
   const char *ret = NULL;
   eo_do((Eo *) obj, elm_obj_layout_data_get(key, &ret));
   return ret;
}

static void
_elm_layout_smart_data_get(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   const char *key = va_arg(*list, const char *);
   const char **ret = va_arg(*list, const char **);

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   *ret = edje_object_data_get(wd->resize_obj, key);
}

EAPI void
elm_layout_sizing_eval(Evas_Object *obj)
{
   ELM_LAYOUT_CHECK(obj);
   eo_do(obj, elm_obj_layout_sizing_eval());
}

/* layout's sizing evaluation is deferred. evaluation requests are
 * queued up and only flag the object as 'changed'. when it comes to
 * Evas's rendering phase, it will be addressed, finally (see
 * _elm_layout_smart_calculate()). */
static void
_elm_layout_smart_sizing_eval(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Layout_Smart_Data *sd = _pd;
   if (sd->frozen) return;
   if (sd->needs_size_calc) return;
   sd->needs_size_calc = EINA_TRUE;

   evas_object_smart_changed(obj);
}

EAPI void
elm_layout_sizing_restricted_eval(Evas_Object *obj, Eina_Bool w, Eina_Bool h)
{
   ELM_LAYOUT_CHECK(obj);
   eo_do(obj, elm_obj_layout_sizing_restricted_eval(w, h));
}

static void
_elm_layout_smart_sizing_restricted_eval(Eo *obj, void *_pd, va_list *list)
{
   Elm_Layout_Smart_Data *sd = _pd;
   Eina_Bool w = va_arg(*list, int);
   Eina_Bool h = va_arg(*list, int);

   sd->restricted_calc_w = !!w;
   sd->restricted_calc_h = !!h;

   evas_object_smart_changed(obj);
}

EAPI int
elm_layout_freeze(Evas_Object *obj)
{
   ELM_LAYOUT_CHECK(obj) 0;
   int ret = 0;
   eo_do(obj, elm_obj_layout_freeze(&ret));
   return ret;
}

static void
_elm_layout_smart_freeze(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   int *ret = va_arg(*list, int *);
   int int_ret = 1;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   ELM_LAYOUT_DATA_GET(obj, sd);

   if ((sd->frozen)++ != 0)
     {
        int_ret = sd->frozen;
        goto end;
     }

   edje_object_freeze(wd->resize_obj);

end:
   if (ret) *ret = int_ret;
}

EAPI int
elm_layout_thaw(Evas_Object *obj)
{
   ELM_LAYOUT_CHECK(obj) 0;
   int ret = 0;
   eo_do(obj, elm_obj_layout_thaw(&ret));
   return ret;
}

static void
_elm_layout_smart_thaw(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   int *ret = va_arg(*list, int *);
   int int_ret = 0;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   ELM_LAYOUT_DATA_GET(obj, sd);

   if (--(sd->frozen) != 0)
     {
        int_ret = sd->frozen;
        goto end;
     }

   edje_object_thaw(wd->resize_obj);

   eo_do(obj, elm_obj_layout_sizing_eval());

end:
   if (ret) *ret = int_ret;
}

EAPI Eina_Bool
elm_layout_part_cursor_set(Evas_Object *obj,
                           const char *part_name,
                           const char *cursor)
{
   ELM_LAYOUT_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, elm_obj_layout_part_cursor_set(part_name, cursor, &ret));
   return ret;
}

static void
_elm_layout_smart_part_cursor_set(Eo *obj, void *_pd, va_list *list)
{
   const char *part_name = va_arg(*list, const char *);
   const char *cursor = va_arg(*list, const char *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   Elm_Layout_Smart_Data *sd = _pd;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   EINA_SAFETY_ON_NULL_RETURN(part_name);

   Evas_Object *part_obj;
   Elm_Layout_Sub_Object_Cursor *pc;

   part_obj = (Evas_Object *)edje_object_part_object_get
       (wd->resize_obj, part_name);
   if (!part_obj)
     {
        const char *group, *file;

        edje_object_file_get(wd->resize_obj, &file, &group);
        ERR("no part '%s' in group '%s' of file '%s'. Cannot set cursor '%s'",
            part_name, group, file, cursor);
        return;
     }
   if (evas_object_pass_events_get(part_obj))
     {
        const char *group, *file;

        edje_object_file_get(wd->resize_obj, &file, &group);
        ERR("part '%s' in group '%s' of file '%s' has mouse_events: 0. "
            "Cannot set cursor '%s'",
            part_name, group, file, cursor);
        return;
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

   if (ret) *ret = EINA_TRUE;
}

EAPI const char *
elm_layout_part_cursor_get(const Evas_Object *obj,
                           const char *part_name)
{
   ELM_LAYOUT_CHECK(obj) NULL;
   const char *ret = NULL;
   eo_do((Eo *) obj, elm_obj_layout_part_cursor_get(part_name, &ret));
   return ret;
}

static void
_elm_layout_smart_part_cursor_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part_name = va_arg(*list, const char *);
   const char **ret = va_arg(*list, const char **);
   *ret = NULL;

   Elm_Layout_Smart_Data *sd = _pd;
   EINA_SAFETY_ON_NULL_RETURN(part_name);

   Elm_Layout_Sub_Object_Cursor *pc = _parts_cursors_find(sd, part_name);
   EINA_SAFETY_ON_NULL_RETURN(pc);
   EINA_SAFETY_ON_NULL_RETURN(pc->obj);

   *ret = elm_object_cursor_get(pc->obj);
}

EAPI Eina_Bool
elm_layout_part_cursor_unset(Evas_Object *obj,
                             const char *part_name)
{
   ELM_LAYOUT_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, elm_obj_layout_part_cursor_unset(part_name, &ret));
   return ret;
}

static void
_elm_layout_smart_part_cursor_unset(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part_name = va_arg(*list, const char *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   Elm_Layout_Smart_Data *sd = _pd;
   EINA_SAFETY_ON_NULL_RETURN(part_name);

   Eina_List *l;
   Elm_Layout_Sub_Object_Cursor *pc;

   EINA_LIST_FOREACH(sd->parts_cursors, l, pc)
     {
        if (!strcmp(part_name, pc->part))
          {
             if (pc->obj) elm_object_cursor_unset(pc->obj);
             _part_cursor_free(pc);
             sd->parts_cursors = eina_list_remove_list(sd->parts_cursors, l);
             if (ret) *ret = EINA_TRUE;
             return;
          }
     }
}

EAPI Eina_Bool
elm_layout_part_cursor_style_set(Evas_Object *obj,
                                 const char *part_name,
                                 const char *style)
{
   ELM_LAYOUT_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, elm_obj_layout_part_cursor_style_set(part_name, style, &ret));
   return ret;
}

static void
_elm_layout_smart_part_cursor_style_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part_name = va_arg(*list, const char *);
   const char *style = va_arg(*list, const char *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   Elm_Layout_Smart_Data *sd = _pd;
   EINA_SAFETY_ON_NULL_RETURN(part_name);

   Elm_Layout_Sub_Object_Cursor *pc = _parts_cursors_find(sd, part_name);
   EINA_SAFETY_ON_NULL_RETURN(pc);
   EINA_SAFETY_ON_NULL_RETURN(pc->obj);

   eina_stringshare_replace(&pc->style, style);
   elm_object_cursor_style_set(pc->obj, pc->style);

   if (ret) *ret = EINA_TRUE;
}

EAPI const char *
elm_layout_part_cursor_style_get(const Evas_Object *obj,
                                 const char *part_name)
{
   ELM_LAYOUT_CHECK(obj) NULL;
   const char *ret = NULL;
   eo_do((Eo *) obj, elm_obj_layout_part_cursor_style_get(part_name, &ret));
   return ret;
}

static void
_elm_layout_smart_part_cursor_style_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part_name = va_arg(*list, const char *);
   const char **ret = va_arg(*list, const char **);
   *ret = NULL;

   Elm_Layout_Smart_Data *sd = _pd;
   EINA_SAFETY_ON_NULL_RETURN(part_name);

   Elm_Layout_Sub_Object_Cursor *pc = _parts_cursors_find(sd, part_name);
   EINA_SAFETY_ON_NULL_RETURN(pc);
   EINA_SAFETY_ON_NULL_RETURN(pc->obj);

   *ret = elm_object_cursor_style_get(pc->obj);
}

EAPI Eina_Bool
elm_layout_part_cursor_engine_only_set(Evas_Object *obj,
                                       const char *part_name,
                                       Eina_Bool engine_only)
{
   ELM_LAYOUT_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, elm_obj_layout_part_cursor_engine_only_set(part_name, engine_only, &ret));
   return ret;
}

static void
_elm_layout_smart_part_cursor_engine_only_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part_name = va_arg(*list, const char *);
   Eina_Bool engine_only = va_arg(*list, int);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   Elm_Layout_Smart_Data *sd = _pd;
   EINA_SAFETY_ON_NULL_RETURN(part_name);

   Elm_Layout_Sub_Object_Cursor *pc = _parts_cursors_find(sd, part_name);
   EINA_SAFETY_ON_NULL_RETURN(pc);
   EINA_SAFETY_ON_NULL_RETURN(pc->obj);

   pc->engine_only = !!engine_only;
   elm_object_cursor_theme_search_enabled_set(pc->obj, !pc->engine_only);

   if (ret) *ret = EINA_TRUE;
}

EAPI Eina_Bool
elm_layout_part_cursor_engine_only_get(const Evas_Object *obj,
                                       const char *part_name)
{
   ELM_LAYOUT_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_layout_part_cursor_engine_only_get(part_name, &ret));
   return ret;
}

static void
_elm_layout_smart_part_cursor_engine_only_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part_name = va_arg(*list, const char *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_FALSE;

   Elm_Layout_Smart_Data *sd = _pd;
   EINA_SAFETY_ON_NULL_RETURN(part_name);

   Elm_Layout_Sub_Object_Cursor *pc = _parts_cursors_find(sd, part_name);
   EINA_SAFETY_ON_NULL_RETURN(pc);
   EINA_SAFETY_ON_NULL_RETURN(pc->obj);

   *ret = !elm_object_cursor_theme_search_enabled_get(pc->obj);
}

static const Elm_Layout_Part_Alias_Description _text_aliases[] =
{
   {"default", "elm.text"},
   {NULL, NULL}
};

EAPI Eina_Bool
elm_layout_edje_object_can_access_set(Evas_Object *obj,
                                      Eina_Bool can_access)
{
   ELM_LAYOUT_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, elm_obj_layout_edje_object_can_access_set(can_access, &ret));
   return ret;
}

static void
_elm_layout_smart_edje_object_can_access_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Layout_Smart_Data *sd = _pd;
   Eina_Bool can_access = va_arg(*list, int);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   sd->can_access = !!can_access;
   if (ret) *ret = EINA_TRUE;
}

EAPI Eina_Bool
elm_layout_edje_object_can_access_get(Evas_Object *obj)
{
   ELM_LAYOUT_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, elm_obj_layout_edje_object_can_access_get(&ret));
   return ret;
}

static void
_elm_layout_smart_edje_object_can_access_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Layout_Smart_Data *sd = _pd;
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);

   *ret = sd->can_access;
}

static void
_dbg_info_get(Eo *eo_obj, void *_pd EINA_UNUSED, va_list *list)
{
   Eo_Dbg_Info *root = (Eo_Dbg_Info *) va_arg(*list, Eo_Dbg_Info *);
   eo_do_super(eo_obj, MY_CLASS, eo_dbg_info_get(root));
   ELM_WIDGET_DATA_GET_OR_RETURN(eo_obj, wd);

   if (wd->resize_obj && eo_isa(wd->resize_obj, EDJE_OBJ_CLASS))
     {
        Eo_Dbg_Info *group = EO_DBG_INFO_LIST_APPEND(root, MY_CLASS_NAME);
        const char *file, *edje_group;
        Evas_Object *edje_obj = wd->resize_obj;

        eo_do(edje_obj, edje_obj_file_get(&file, &edje_group));
        EO_DBG_INFO_APPEND(group, "File", EINA_VALUE_TYPE_STRING, file);
        EO_DBG_INFO_APPEND(group, "Group", EINA_VALUE_TYPE_STRING, edje_group);

        Edje_Load_Error error;
        eo_do(edje_obj, edje_obj_load_error_get(&error));
        if (error != EDJE_LOAD_ERROR_NONE)
          {
             EO_DBG_INFO_APPEND(group, "Error", EINA_VALUE_TYPE_STRING,
                                edje_load_error_str(error));
          }
     }
}

EAPI Evas_Object *
elm_layout_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = eo_add(MY_CLASS, parent);
   eo_unref(obj);
   return obj;
}

static void
_constructor(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Layout_Smart_Data *sd = _pd;
   sd->obj = obj;
   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME_LEGACY),
         evas_obj_smart_callbacks_descriptions_set(_smart_callbacks, NULL));
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_DBG_INFO_GET), _dbg_info_get),

        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_ADD), _elm_layout_smart_add),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_DEL), _elm_layout_smart_del),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_CALCULATE), _elm_layout_smart_calculate),

        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_ON_FOCUS), _elm_layout_smart_on_focus),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_DISABLE), _elm_layout_smart_disable),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_THEME_APPLY), _elm_layout_smart_theme),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_NEXT_MANAGER_IS), _elm_layout_smart_focus_next_manager_is),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_NEXT), _elm_layout_smart_focus_next),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_DIRECTION_MANAGER_IS), _elm_layout_smart_focus_direction_manager_is),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_DIRECTION), _elm_layout_smart_focus_direction),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_SUB_OBJECT_ADD), _elm_layout_smart_sub_object_add),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_SUB_OBJECT_DEL), _elm_layout_smart_sub_object_del),

        EO_OP_FUNC(ELM_OBJ_CONTAINER_ID(ELM_OBJ_CONTAINER_SUB_ID_CONTENT_SET), _elm_layout_smart_content_set),
        EO_OP_FUNC(ELM_OBJ_CONTAINER_ID(ELM_OBJ_CONTAINER_SUB_ID_CONTENT_GET), _elm_layout_smart_content_get),
        EO_OP_FUNC(ELM_OBJ_CONTAINER_ID(ELM_OBJ_CONTAINER_SUB_ID_CONTENT_UNSET), _elm_layout_smart_content_unset),
        EO_OP_FUNC(ELM_OBJ_CONTAINER_ID(ELM_OBJ_CONTAINER_SUB_ID_CONTENT_SWALLOW_LIST_GET), _elm_layout_smart_content_swallow_list_get),

        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_FILE_SET), _elm_layout_smart_file_set),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_THEME_SET), _elm_layout_smart_theme_set),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_SIGNAL_EMIT), _elm_layout_smart_signal_emit),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_SIGNAL_CALLBACK_ADD), _elm_layout_smart_signal_callback_add),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_SIGNAL_CALLBACK_DEL), _elm_layout_smart_signal_callback_del),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_TEXT_SET), _elm_layout_smart_text_set),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_TEXT_GET), _elm_layout_smart_text_get),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_BOX_APPEND), _elm_layout_smart_box_append),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_BOX_PREPEND), _elm_layout_smart_box_prepend),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_BOX_INSERT_BEFORE), _elm_layout_smart_box_insert_before),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_BOX_INSERT_AT), _elm_layout_smart_box_insert_at),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_BOX_REMOVE), _elm_layout_smart_box_remove),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_BOX_REMOVE_ALL), _elm_layout_smart_box_remove_all),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_TABLE_PACK), _elm_layout_smart_table_pack),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_TABLE_UNPACK), _elm_layout_smart_table_unpack),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_TABLE_CLEAR), _elm_layout_smart_table_clear),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_EDJE_GET), _elm_layout_smart_edje_get),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_DATA_GET), _elm_layout_smart_data_get),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_SIZING_EVAL), _elm_layout_smart_sizing_eval),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_SIZING_RESTRICTED_EVAL), _elm_layout_smart_sizing_restricted_eval),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_PART_CURSOR_SET), _elm_layout_smart_part_cursor_set),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_PART_CURSOR_GET), _elm_layout_smart_part_cursor_get),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_PART_CURSOR_UNSET), _elm_layout_smart_part_cursor_unset),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_PART_CURSOR_STYLE_SET), _elm_layout_smart_part_cursor_style_set),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_PART_CURSOR_STYLE_GET), _elm_layout_smart_part_cursor_style_get),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_PART_CURSOR_ENGINE_ONLY_SET), _elm_layout_smart_part_cursor_engine_only_set),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_PART_CURSOR_ENGINE_ONLY_GET), _elm_layout_smart_part_cursor_engine_only_get),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_EDJE_OBJECT_CAN_ACCESS_SET), _elm_layout_smart_edje_object_can_access_set),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_EDJE_OBJECT_CAN_ACCESS_GET), _elm_layout_smart_edje_object_can_access_get),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_TEXT_ALIASES_GET), _elm_layout_smart_text_aliases_get),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_CONTENT_ALIASES_GET), _elm_layout_smart_content_aliases_get),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_SUB_OBJECT_ADD_ENABLE), _elm_layout_smart_sub_object_add_enable),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_THEME_ENABLE), _elm_layout_smart_theme_enable),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_FREEZE), _elm_layout_smart_freeze),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_THAW), _elm_layout_smart_thaw),

        EO_OP_FUNC_SENTINEL
   };
   eo_class_funcs_set(klass, func_desc);

   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(ELM_OBJ_LAYOUT_SUB_ID_FILE_SET, "Set the file that will be used as layout."),
     EO_OP_DESCRIPTION(ELM_OBJ_LAYOUT_SUB_ID_THEME_SET, "Set the edje group from the elementary theme that will be used as layout."),
     EO_OP_DESCRIPTION(ELM_OBJ_LAYOUT_SUB_ID_SIGNAL_EMIT, "Send a (Edje) signal to a given layout widget's underlying Edje object."),
     EO_OP_DESCRIPTION(ELM_OBJ_LAYOUT_SUB_ID_SIGNAL_CALLBACK_ADD, "Add a callback for a (Edje) signal emitted by a layout widget's underlying Edje object."),
     EO_OP_DESCRIPTION(ELM_OBJ_LAYOUT_SUB_ID_SIGNAL_CALLBACK_DEL, "Remove a signal-triggered callback from a given layout widget."),
     EO_OP_DESCRIPTION(ELM_OBJ_LAYOUT_SUB_ID_TEXT_SET, "Set the text of the given part."),
     EO_OP_DESCRIPTION(ELM_OBJ_LAYOUT_SUB_ID_TEXT_GET, "Get the text set in the given part."),
     EO_OP_DESCRIPTION(ELM_OBJ_LAYOUT_SUB_ID_BOX_APPEND, "Append child to layout box part."),
     EO_OP_DESCRIPTION(ELM_OBJ_LAYOUT_SUB_ID_BOX_PREPEND, "Prepend child to layout box part."),
     EO_OP_DESCRIPTION(ELM_OBJ_LAYOUT_SUB_ID_BOX_INSERT_BEFORE, "Insert child to layout box part before a reference object."),
     EO_OP_DESCRIPTION(ELM_OBJ_LAYOUT_SUB_ID_BOX_INSERT_AT, "Insert child to layout box part at a given position."),
     EO_OP_DESCRIPTION(ELM_OBJ_LAYOUT_SUB_ID_BOX_REMOVE, "Remove a child of the given part box."),
     EO_OP_DESCRIPTION(ELM_OBJ_LAYOUT_SUB_ID_BOX_REMOVE_ALL, "Remove all children of the given part box."),
     EO_OP_DESCRIPTION(ELM_OBJ_LAYOUT_SUB_ID_TABLE_PACK, "Insert child to layout table part."),
     EO_OP_DESCRIPTION(ELM_OBJ_LAYOUT_SUB_ID_TABLE_UNPACK, "Unpack (remove) a child of the given part table."),
     EO_OP_DESCRIPTION(ELM_OBJ_LAYOUT_SUB_ID_TABLE_CLEAR, "Remove all the child objects of the given part table."),
     EO_OP_DESCRIPTION(ELM_OBJ_LAYOUT_SUB_ID_EDJE_GET, "Get the edje layout."),
     EO_OP_DESCRIPTION(ELM_OBJ_LAYOUT_SUB_ID_DATA_GET, "Get the edje data from the given layout."),
     EO_OP_DESCRIPTION(ELM_OBJ_LAYOUT_SUB_ID_SIZING_EVAL, "Eval sizing."),
     EO_OP_DESCRIPTION(ELM_OBJ_LAYOUT_SUB_ID_PART_CURSOR_SET, "Sets a specific cursor for an edje part."),
     EO_OP_DESCRIPTION(ELM_OBJ_LAYOUT_SUB_ID_PART_CURSOR_GET, "Get the cursor to be shown when mouse is over an edje part."),
     EO_OP_DESCRIPTION(ELM_OBJ_LAYOUT_SUB_ID_PART_CURSOR_UNSET, "Unsets a cursor previously set with elm_layout_part_cursor_set()."),
     EO_OP_DESCRIPTION(ELM_OBJ_LAYOUT_SUB_ID_PART_CURSOR_STYLE_SET, "Sets a specific cursor style for an edje part."),
     EO_OP_DESCRIPTION(ELM_OBJ_LAYOUT_SUB_ID_PART_CURSOR_STYLE_GET, "Get a specific cursor style for an edje part."),
     EO_OP_DESCRIPTION(ELM_OBJ_LAYOUT_SUB_ID_PART_CURSOR_ENGINE_ONLY_SET, "Sets if the cursor set should be searched on the theme or should use the provided by the engine, only ."),
     EO_OP_DESCRIPTION(ELM_OBJ_LAYOUT_SUB_ID_PART_CURSOR_ENGINE_ONLY_GET, "Get a specific cursor engine_only for an edje part."),
     EO_OP_DESCRIPTION(ELM_OBJ_LAYOUT_SUB_ID_EDJE_OBJECT_CAN_ACCESS_SET, "Checks whenever 'property' is impemented in current class."),
     EO_OP_DESCRIPTION(ELM_OBJ_LAYOUT_SUB_ID_EDJE_OBJECT_CAN_ACCESS_GET, "Checks whenever 'property' is impemented in current class."),
     EO_OP_DESCRIPTION(ELM_OBJ_LAYOUT_SUB_ID_TEXT_ALIASES_GET, "Checks whenever 'text aliases' are impemented in current class."),
     EO_OP_DESCRIPTION(ELM_OBJ_LAYOUT_SUB_ID_CONTENT_ALIASES_GET, "Checks whenever 'content aliases' are impemented in current class."),
     EO_OP_DESCRIPTION(ELM_OBJ_LAYOUT_SUB_ID_SUB_OBJECT_ADD_ENABLE, "Checks whenever sub object handling impemented in current class."),
     EO_OP_DESCRIPTION(ELM_OBJ_LAYOUT_SUB_ID_THEME_ENABLE, "Checks whenever 'theme' is impemented in current class."),
     EO_OP_DESCRIPTION(ELM_OBJ_LAYOUT_SUB_ID_FREEZE, "Freezes the Elementary layout object."),
     EO_OP_DESCRIPTION(ELM_OBJ_LAYOUT_SUB_ID_THAW, "Thaws the Elementary layout object."),
     EO_OP_DESCRIPTION(ELM_OBJ_LAYOUT_SUB_ID_SIZING_RESTRICTED_EVAL, "Eval sizing, restricted to current width/height size."),

     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     MY_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&ELM_OBJ_LAYOUT_BASE_ID, op_desc, ELM_OBJ_LAYOUT_SUB_ID_LAST),
     NULL,
     sizeof(Elm_Layout_Smart_Data),
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(elm_obj_layout_class_get, &class_desc, ELM_OBJ_CONTAINER_CLASS, NULL);
