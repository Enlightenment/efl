#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_layout.h"

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

EOLIAN static Eina_Bool
_elm_layout_elm_widget_disable(Eo *obj, Elm_Layout_Smart_Data *_pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   if (elm_object_disabled_get(obj))
     edje_object_signal_emit
       (wd->resize_obj, "elm,state,disabled", "elm");
   else
     edje_object_signal_emit
       (wd->resize_obj, "elm,state,enabled", "elm");

   return EINA_TRUE;
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

   eo_do(obj, ret = elm_obj_widget_disable());

   return ret;
}

EOLIAN static Eina_Bool
_elm_layout_elm_widget_theme_apply(Eo *obj, Elm_Layout_Smart_Data *sd)
{
   Eina_Bool int_ret = EINA_FALSE;

   eo_do_super(obj, MY_CLASS, int_ret = elm_obj_widget_theme_apply());
   if (!int_ret) return EINA_FALSE;
   /* The following lines are here to support entry design; the _theme function
    * of entry needs to call directly the widget _theme function */
   Eina_Bool enable = EINA_TRUE;
   eo_do(obj, enable = elm_obj_layout_theme_enable());
   if (!enable) return EINA_TRUE;

   return _elm_layout_theme_internal(obj, sd);
}

static void *
_elm_layout_list_data_get(const Eina_List *list)
{
   Elm_Layout_Sub_Object_Data *sub_d = eina_list_data_get(list);

   return sub_d->obj;
}

EOLIAN static Eina_Bool
_elm_layout_elm_widget_on_focus(Eo *obj, Elm_Layout_Smart_Data *_pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   if (!elm_widget_can_focus_get(obj)) return EINA_FALSE;

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

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_layout_elm_widget_focus_next_manager_is(Eo *obj EINA_UNUSED, Elm_Layout_Smart_Data *_pd EINA_UNUSED)
{
   if (!elm_widget_can_focus_get(obj))
     return EINA_TRUE;
   else
     return EINA_FALSE;
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
EOLIAN static Eina_Bool
_elm_layout_elm_widget_focus_next(Eo *obj, Elm_Layout_Smart_Data *sd, Elm_Focus_Direction dir, Evas_Object **next)
{
   const Eina_List *items;
   void *(*list_data_get)(const Eina_List *list);

   if ((items = elm_widget_focus_custom_chain_get(obj)))
     list_data_get = eina_list_data_get;
   else
     {
        items = sd->subs;
        list_data_get = _elm_layout_list_data_get;

        if (!items) return EINA_FALSE;

        if (_elm_config->access_mode)
          items = _access_focus_list_sort((Eina_List *)items);
     }

   return elm_widget_focus_list_next_get
            (obj, items, list_data_get, dir, next);
}

EOLIAN static Eina_Bool
_elm_layout_elm_widget_sub_object_add(Eo *obj, Elm_Layout_Smart_Data *_pd EINA_UNUSED, Evas_Object *sobj)
{
   Eina_Bool int_ret = EINA_FALSE;

   if (evas_object_data_get(sobj, "elm-parent") == obj) return EINA_TRUE;

   eo_do_super(obj, MY_CLASS, int_ret = elm_obj_widget_sub_object_add(sobj));
   if (!int_ret) return EINA_FALSE;

   Eina_Bool enable = EINA_TRUE;
   eo_do(obj, enable = elm_obj_layout_sub_object_add_enable());

   if (EINA_TRUE == enable)
     evas_object_event_callback_add
       (sobj, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
         _on_sub_object_size_hint_change, obj);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_layout_elm_widget_sub_object_del(Eo *obj, Elm_Layout_Smart_Data *sd, Evas_Object *sobj)
{
   Eina_List *l;
   Elm_Layout_Sub_Object_Data *sub_d;

   Eina_Bool int_ret = EINA_FALSE;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   evas_object_event_callback_del_full
     (sobj, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
     _on_sub_object_size_hint_change, obj);

   eo_do_super(obj, MY_CLASS, int_ret = elm_obj_widget_sub_object_del(sobj));
   if (!int_ret) return EINA_FALSE;

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

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_layout_elm_widget_focus_direction_manager_is(Eo *obj EINA_UNUSED, Elm_Layout_Smart_Data *_pd EINA_UNUSED)
{
   if (!elm_widget_can_focus_get(obj))
     return EINA_TRUE;
   else
     return EINA_FALSE;
}

EOLIAN static Eina_Bool
_elm_layout_elm_widget_focus_direction(Eo *obj, Elm_Layout_Smart_Data *sd, const Evas_Object *base, double degree, Evas_Object **direction, double *weight)
{
   const Eina_List *items;
   void *(*list_data_get)(const Eina_List *list);

   if (!sd->subs) return EINA_FALSE;

   /* Focus chain (This block is different from elm_win cycle) */
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
_edje_signal_callback(void *data,
                      Evas_Object *obj EINA_UNUSED,
                      const char *emission,
                      const char *source)
{
   Edje_Signal_Data *esd = data;

   esd->func(esd->data, esd->obj, emission, source);
}

EOLIAN static const Elm_Layout_Part_Alias_Description*
_elm_layout_text_aliases_get(Eo *obj EINA_UNUSED, Elm_Layout_Smart_Data *_pd EINA_UNUSED)
{
   return NULL;
}

EOLIAN static const Elm_Layout_Part_Alias_Description*
_elm_layout_content_aliases_get(Eo *obj EINA_UNUSED, Elm_Layout_Smart_Data *_pd EINA_UNUSED)
{
   return NULL;
}

EOLIAN static Eina_Bool
_elm_layout_sub_object_add_enable(Eo *obj EINA_UNUSED, Elm_Layout_Smart_Data *_pd EINA_UNUSED)
{
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_layout_theme_enable(Eo *obj EINA_UNUSED, Elm_Layout_Smart_Data *_pd EINA_UNUSED)
{
   return EINA_TRUE;
}

static Eina_Bool
_elm_layout_part_aliasing_eval(const Evas_Object *obj EINA_UNUSED,
                               Elm_Layout_Smart_Data *sd,
                               const char **part,
                               Eina_Bool is_text)
{
   const Elm_Layout_Part_Alias_Description *aliases = NULL;

   if (is_text)
     eo_do(sd->obj, aliases = elm_obj_layout_text_aliases_get());
   else
     eo_do(sd->obj, aliases =  elm_obj_layout_content_aliases_get());

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

EOLIAN static void
_elm_layout_evas_smart_add(Eo *obj, Elm_Layout_Smart_Data *_pd EINA_UNUSED)
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

EOLIAN static void
_elm_layout_evas_smart_del(Eo *obj, Elm_Layout_Smart_Data *sd)
{
   Elm_Layout_Sub_Object_Data *sub_d;
   Elm_Layout_Sub_Object_Cursor *pc;
   Edje_Signal_Data *esd;
   Evas_Object *child;
   Eina_List *l;

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
EOLIAN static void
_elm_layout_evas_smart_calculate(Eo *obj, Elm_Layout_Smart_Data *sd)
{
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

EOLIAN static Eina_Bool
_elm_layout_file_set(Eo *obj, Elm_Layout_Smart_Data *sd, const char *file, const char *group)
{
   Eina_Bool int_ret = EINA_FALSE;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   int_ret =
     edje_object_file_set(wd->resize_obj, file, group);

   if (int_ret) _visuals_refresh(obj, sd);
   else
     ERR("failed to set edje file '%s', group '%s': %s",
         file, group,
         edje_load_error_str
           (edje_object_load_error_get(wd->resize_obj)));

   return int_ret;
}

EOLIAN static Eina_Bool
_elm_layout_theme_set(Eo *obj, Elm_Layout_Smart_Data *sd, const char *klass, const char *group, const char *style)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   eina_stringshare_replace(&(sd->klass), klass);
   eina_stringshare_replace(&(sd->group), group);
   eina_stringshare_replace(&(wd->style), style);

   return _elm_layout_theme_internal(obj, sd);
}

EOLIAN static void
_elm_layout_signal_emit(Eo *obj, Elm_Layout_Smart_Data *_pd EINA_UNUSED, const char *emission, const char *source)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   edje_object_signal_emit(wd->resize_obj, emission, source);
}

EOLIAN static void
_elm_layout_signal_callback_add(Eo *obj, Elm_Layout_Smart_Data *sd, const char *emission, const char *source, Edje_Signal_Cb func_cb, void *data)
{
   Edje_Signal_Data *esd;

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

EOLIAN static void*
_elm_layout_signal_callback_del(Eo *obj, Elm_Layout_Smart_Data *sd, const char *emission, const char *source, Edje_Signal_Cb func_cb)
{
   Edje_Signal_Data *esd = NULL;
   void *data = NULL;
   Eina_List *l;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

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

             return data; /* stop at 1st match */

          }
     }

   return NULL;
}

EAPI Eina_Bool
elm_layout_content_set(Evas_Object *obj,
                       const char *swallow,
                       Evas_Object *content)
{
   ELM_LAYOUT_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, ret = elm_obj_container_content_set(swallow, content));
   return ret;
}

EOLIAN static Eina_Bool
_elm_layout_elm_container_content_set(Eo *obj, Elm_Layout_Smart_Data *sd, const char *part, Evas_Object *content)
{
   Elm_Layout_Sub_Object_Data *sub_d;
   const Eina_List *l;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   if (!_elm_layout_part_aliasing_eval(obj, sd, &part, EINA_FALSE))
     return EINA_FALSE;

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
          return EINA_FALSE;

        if (!edje_object_part_swallow
              (wd->resize_obj, part, content))
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

   eo_do(obj, elm_obj_layout_sizing_eval());

end:
   return EINA_TRUE;
}

EAPI Evas_Object *
elm_layout_content_get(const Evas_Object *obj,
                       const char *swallow)
{
   ELM_LAYOUT_CHECK(obj) NULL;
   Evas_Object *ret = NULL;
   eo_do((Eo *) obj, ret = elm_obj_container_content_get(swallow));
   return ret;
}

EOLIAN static Evas_Object*
_elm_layout_elm_container_content_get(Eo *obj, Elm_Layout_Smart_Data *sd, const char *part)
{
   const Eina_List *l;
   Elm_Layout_Sub_Object_Data *sub_d;

   if (!_elm_layout_part_aliasing_eval(obj, sd, &part, EINA_FALSE))
     return NULL;

   EINA_LIST_FOREACH(sd->subs, l, sub_d)
     {
        if ((sub_d->type == SWALLOW) && !strcmp(part, sub_d->part)) return sub_d->obj;
     }

   return NULL;
}

EAPI Evas_Object *
elm_layout_content_unset(Evas_Object *obj,
                         const char *swallow)
{
   ELM_LAYOUT_CHECK(obj) NULL;
   Evas_Object *ret = NULL;
   eo_do(obj, ret = elm_obj_container_content_unset(swallow));
   return ret;
}

EOLIAN static Evas_Object*
_elm_layout_elm_container_content_unset(Eo *obj, Elm_Layout_Smart_Data *sd, const char *part)
{
   Elm_Layout_Sub_Object_Data *sub_d;
   const Eina_List *l;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   if (!_elm_layout_part_aliasing_eval(obj, sd, &part, EINA_FALSE))
     return NULL;

   EINA_LIST_FOREACH(sd->subs, l, sub_d)
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
               (wd->resize_obj, content);
             return content;
          }
     }

   return NULL;
}

EAPI Eina_List *
elm_layout_content_swallow_list_get(const Evas_Object *obj)
{
   ELM_LAYOUT_CHECK(obj) NULL;
   Eina_List *ret = NULL;
   eo_do(obj, ret = elm_obj_container_content_swallow_list_get());
   return ret;
}

EOLIAN static Eina_List*
_elm_layout_elm_container_content_swallow_list_get(Eo *obj EINA_UNUSED, Elm_Layout_Smart_Data *sd)
{
   Eina_List *ret = NULL;

   Elm_Layout_Sub_Object_Data *sub_d = NULL;
   Eina_List *l = NULL;

   EINA_LIST_FOREACH(sd->subs, l, sub_d)
     {
        if (sub_d->type == SWALLOW)
          ret = eina_list_append(ret, sub_d->obj);
     }

   return ret;
}

EOLIAN static Eina_Bool
_elm_layout_text_set(Eo *obj, Elm_Layout_Smart_Data *sd, const char *part, const char *text)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   Eina_List *l;
   Elm_Layout_Sub_Object_Data *sub_d = NULL;

   if (!_elm_layout_part_aliasing_eval(obj, sd, &part, EINA_TRUE))
     return EINA_FALSE;

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
                  return EINA_TRUE;
               }
             else
               break;
          }
        sub_d = NULL;
     }

   if (!edje_object_part_text_escaped_set
         (wd->resize_obj, part, text))
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

   eo_do(obj, elm_obj_layout_sizing_eval());

   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON &&
       sd->can_access && !(sub_d->obj))
     sub_d->obj = _elm_access_edje_object_part_object_register
         (obj, elm_layout_edje_get(obj), part);

   return EINA_TRUE;
}

EOLIAN static const char*
_elm_layout_text_get(Eo *obj, Elm_Layout_Smart_Data *sd, const char *part)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   if (!_elm_layout_part_aliasing_eval(obj, sd, &part, EINA_TRUE))
     return NULL;

   return edje_object_part_text_get(wd->resize_obj, part);
}

EOLIAN static Eina_Bool
_elm_layout_box_append(Eo *obj, Elm_Layout_Smart_Data *sd, const char *part, Evas_Object *child)
{
   Elm_Layout_Sub_Object_Data *sub_d;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   if (!edje_object_part_box_append
         (wd->resize_obj, part, child))
     {
        ERR("child %p could not be appended to box part '%s'", child, part);
        return EINA_FALSE;
     }

   if (!elm_widget_sub_object_add(obj, child))
     {
        edje_object_part_box_remove
          (wd->resize_obj, part, child);
        return EINA_FALSE;
     }

   sub_d = ELM_NEW(Elm_Layout_Sub_Object_Data);
   sub_d->type = BOX_APPEND;
   sub_d->part = eina_stringshare_add(part);
   sub_d->obj = child;
   sd->subs = eina_list_append(sd->subs, sub_d);

   eo_do(obj, elm_obj_layout_sizing_eval());

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_layout_box_prepend(Eo *obj, Elm_Layout_Smart_Data *sd, const char *part, Evas_Object *child)
{
   Elm_Layout_Sub_Object_Data *sub_d;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   if (!edje_object_part_box_prepend
         (wd->resize_obj, part, child))
     {
        ERR("child %p could not be prepended to box part '%s'", child, part);
        return EINA_FALSE;
     }

   if (!elm_widget_sub_object_add(obj, child))
     {
        edje_object_part_box_remove
          (wd->resize_obj, part, child);
        return EINA_FALSE;
     }

   sub_d = ELM_NEW(Elm_Layout_Sub_Object_Data);
   sub_d->type = BOX_PREPEND;
   sub_d->part = eina_stringshare_add(part);
   sub_d->obj = child;
   sd->subs = eina_list_prepend(sd->subs, sub_d);

   eo_do(obj, elm_obj_layout_sizing_eval());

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_layout_box_insert_before(Eo *obj, Elm_Layout_Smart_Data *sd, const char *part, Evas_Object *child, const Evas_Object *reference)
{
   Elm_Layout_Sub_Object_Data *sub_d;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   if (!edje_object_part_box_insert_before
         (wd->resize_obj, part, child, reference))
     {
        ERR("child %p could not be inserted before %p inf box part '%s'",
            child, reference, part);
        return EINA_FALSE;
     }

   if (!elm_widget_sub_object_add(obj, child))
     {
        edje_object_part_box_remove
          (wd->resize_obj, part, child);
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

   eo_do(obj, elm_obj_layout_sizing_eval());

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_layout_box_insert_at(Eo *obj, Elm_Layout_Smart_Data *sd, const char *part, Evas_Object *child, unsigned int pos)
{
   Elm_Layout_Sub_Object_Data *sub_d;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   if (!edje_object_part_box_insert_at
         (wd->resize_obj, part, child, pos))
     {
        ERR("child %p could not be inserted at %u to box part '%s'",
            child, pos, part);
        return EINA_FALSE;
     }

   if (!elm_widget_sub_object_add(obj, child))
     {
        edje_object_part_box_remove
          (wd->resize_obj, part, child);
        return EINA_FALSE;
     }

   sub_d = ELM_NEW(Elm_Layout_Sub_Object_Data);
   sub_d->type = BOX_INSERT_AT;
   sub_d->part = eina_stringshare_add(part);
   sub_d->obj = child;
   sub_d->p.box.pos = pos;
   sd->subs = eina_list_append(sd->subs, sub_d);

   eo_do(obj, elm_obj_layout_sizing_eval());

   return EINA_TRUE;
}

EOLIAN static Evas_Object*
_elm_layout_box_remove(Eo *obj, Elm_Layout_Smart_Data *sd, const char *part, Evas_Object *child)
{

   EINA_SAFETY_ON_NULL_RETURN_VAL(part, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(child, NULL);


   const Eina_List *l;
   Elm_Layout_Sub_Object_Data *sub_d;

   EINA_LIST_FOREACH(sd->subs, l, sub_d)
     {
        if (!_sub_box_is(sub_d)) continue;
        if ((sub_d->obj == child) && (!strcmp(sub_d->part, part)))
           return _sub_box_remove(obj, sd, sub_d);
     }

   return NULL;
}

EOLIAN static Eina_Bool
_elm_layout_box_remove_all(Eo *obj, Elm_Layout_Smart_Data *sd, const char *part, Eina_Bool clear)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(part, EINA_FALSE);

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

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

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_layout_table_pack(Eo *obj, Elm_Layout_Smart_Data *sd, const char *part, Evas_Object *child, unsigned short col, unsigned short row, unsigned short colspan, unsigned short rowspan)
{
   Elm_Layout_Sub_Object_Data *sub_d;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   if (!edje_object_part_table_pack
         (wd->resize_obj, part, child, col,
         row, colspan, rowspan))
     {
        ERR("child %p could not be packed into box part '%s' col=%uh, row=%hu,"
            " colspan=%hu, rowspan=%hu", child, part, col, row, colspan,
            rowspan);
        return EINA_FALSE;
     }

   if (!elm_widget_sub_object_add(obj, child))
     {
        edje_object_part_table_unpack
          (wd->resize_obj, part, child);
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

   eo_do(obj, elm_obj_layout_sizing_eval());

   return EINA_TRUE;
}

EOLIAN static Evas_Object*
_elm_layout_table_unpack(Eo *obj, Elm_Layout_Smart_Data *sd, const char *part, Evas_Object *child)
{

   EINA_SAFETY_ON_NULL_RETURN_VAL(part, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(child, NULL);

   const Eina_List *l;
   Elm_Layout_Sub_Object_Data *sub_d;

   EINA_LIST_FOREACH(sd->subs, l, sub_d)
     {
        if (sub_d->type != TABLE_PACK) continue;
        if ((sub_d->obj == child) && (!strcmp(sub_d->part, part)))
          {
             return _sub_table_remove(obj, sd, sub_d);
          }
     }

   return NULL;
}

EOLIAN static Eina_Bool
_elm_layout_table_clear(Eo *obj, Elm_Layout_Smart_Data *sd, const char *part, Eina_Bool clear)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(part, EINA_FALSE);

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

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

   return EINA_TRUE;
}

EOLIAN static Evas_Object*
_elm_layout_edje_get(Eo *obj, Elm_Layout_Smart_Data *_pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   return wd->resize_obj;
}

EOLIAN static const char*
_elm_layout_data_get(Eo *obj, Elm_Layout_Smart_Data *_pd EINA_UNUSED, const char *key)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   return edje_object_data_get(wd->resize_obj, key);
}

/* layout's sizing evaluation is deferred. evaluation requests are
 * queued up and only flag the object as 'changed'. when it comes to
 * Evas's rendering phase, it will be addressed, finally (see
 * _elm_layout_smart_calculate()). */
EOLIAN static void
_elm_layout_sizing_eval(Eo *obj, Elm_Layout_Smart_Data *sd)
{
   if (sd->frozen) return;
   if (sd->needs_size_calc) return;
   sd->needs_size_calc = EINA_TRUE;

   evas_object_smart_changed(obj);
}

EOLIAN static void
_elm_layout_sizing_restricted_eval(Eo *obj, Elm_Layout_Smart_Data *sd, Eina_Bool w, Eina_Bool h)
{
   sd->restricted_calc_w = !!w;
   sd->restricted_calc_h = !!h;

   evas_object_smart_changed(obj);
}

EOLIAN static int
_elm_layout_freeze(Eo *obj, Elm_Layout_Smart_Data *_pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, 1);
   ELM_LAYOUT_DATA_GET(obj, sd);

   if ((sd->frozen)++ != 0) return sd->frozen;

   edje_object_freeze(wd->resize_obj);

   return 1;
}

EOLIAN static int
_elm_layout_thaw(Eo *obj, Elm_Layout_Smart_Data *_pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, 0);
   ELM_LAYOUT_DATA_GET(obj, sd);

   if (--(sd->frozen) != 0) return sd->frozen;

   edje_object_thaw(wd->resize_obj);

   eo_do(obj, elm_obj_layout_sizing_eval());

   return 0;
}

EOLIAN static Eina_Bool
_elm_layout_part_cursor_set(Eo *obj, Elm_Layout_Smart_Data *sd, const char *part_name, const char *cursor)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(part_name, EINA_FALSE);

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
        return EINA_FALSE;
     }
   if (evas_object_pass_events_get(part_obj))
     {
        const char *group, *file;

        edje_object_file_get(wd->resize_obj, &file, &group);
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

EOLIAN static const char*
_elm_layout_part_cursor_get(Eo *obj EINA_UNUSED, Elm_Layout_Smart_Data *sd, const char *part_name)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(part_name, NULL);

   Elm_Layout_Sub_Object_Cursor *pc = _parts_cursors_find(sd, part_name);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pc, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pc->obj, NULL);

   return elm_object_cursor_get(pc->obj);
}

EOLIAN static Eina_Bool
_elm_layout_part_cursor_unset(Eo *obj EINA_UNUSED, Elm_Layout_Smart_Data *sd, const char *part_name)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(part_name, EINA_FALSE);

   Eina_List *l;
   Elm_Layout_Sub_Object_Cursor *pc;

   EINA_LIST_FOREACH(sd->parts_cursors, l, pc)
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

EOLIAN static Eina_Bool
_elm_layout_part_cursor_style_set(Eo *obj EINA_UNUSED, Elm_Layout_Smart_Data *sd, const char *part_name, const char *style)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(part_name, EINA_FALSE);

   Elm_Layout_Sub_Object_Cursor *pc = _parts_cursors_find(sd, part_name);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pc, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pc->obj, EINA_FALSE);

   eina_stringshare_replace(&pc->style, style);
   elm_object_cursor_style_set(pc->obj, pc->style);

   return EINA_TRUE;
}

EOLIAN static const char*
_elm_layout_part_cursor_style_get(Eo *obj EINA_UNUSED, Elm_Layout_Smart_Data *sd, const char *part_name)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(part_name, NULL);

   Elm_Layout_Sub_Object_Cursor *pc = _parts_cursors_find(sd, part_name);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pc, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pc->obj, NULL);

   return elm_object_cursor_style_get(pc->obj);
}

EOLIAN static Eina_Bool
_elm_layout_part_cursor_engine_only_set(Eo *obj EINA_UNUSED, Elm_Layout_Smart_Data *sd, const char *part_name, Eina_Bool engine_only)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(part_name, EINA_FALSE);

   Elm_Layout_Sub_Object_Cursor *pc = _parts_cursors_find(sd, part_name);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pc, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pc->obj, EINA_FALSE);

   pc->engine_only = !!engine_only;
   elm_object_cursor_theme_search_enabled_set(pc->obj, !pc->engine_only);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_layout_part_cursor_engine_only_get(Eo *obj EINA_UNUSED, Elm_Layout_Smart_Data *sd, const char *part_name)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(part_name, EINA_FALSE);

   Elm_Layout_Sub_Object_Cursor *pc = _parts_cursors_find(sd, part_name);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pc, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pc->obj, EINA_FALSE);

   return !elm_object_cursor_theme_search_enabled_get(pc->obj);
}

static const Elm_Layout_Part_Alias_Description _text_aliases[] =
{
   {"default", "elm.text"},
   {NULL, NULL}
};

EOLIAN static Eina_Bool
_elm_layout_edje_object_can_access_set(Eo *obj EINA_UNUSED, Elm_Layout_Smart_Data *sd, Eina_Bool can_access)
{
   sd->can_access = !!can_access;
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_layout_edje_object_can_access_get(Eo *obj EINA_UNUSED, Elm_Layout_Smart_Data *sd)
{
   return sd->can_access;
}

EOLIAN static void
_elm_layout_eo_base_dbg_info_get(Eo *eo_obj, Elm_Layout_Smart_Data *_pd EINA_UNUSED, Eo_Dbg_Info *root)
{
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

        Edje_Load_Error error = EDJE_LOAD_ERROR_GENERIC;
        eo_do(edje_obj, error = edje_obj_load_error_get());
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

EOLIAN static void
_elm_layout_eo_base_constructor(Eo *obj, Elm_Layout_Smart_Data *sd)
{
   sd->obj = obj;
   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME_LEGACY),
         evas_obj_smart_callbacks_descriptions_set(_smart_callbacks));
}

EOLIAN static void _elm_layout_class_constructor(Eo_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

#include "elm_layout.eo.c"
