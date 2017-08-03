#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
#define ELM_LAYOUT_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_layout.h"

#include "elm_layout_internal_part.eo.h"
#include "elm_part_helper.h"

#define EDJE_EDIT_IS_UNSTABLE_AND_I_KNOW_ABOUT_IT
#include <Edje_Edit.h>

#define MY_CLASS ELM_LAYOUT_CLASS
#define MY_CLASS_PFX elm_layout

#define MY_CLASS_NAME "Elm_Layout"
#define MY_CLASS_NAME_LEGACY "elm_layout"

Eo *_elm_layout_pack_proxy_get(Elm_Layout *obj, Edje_Part_Type type, const char *part);
static void _efl_model_properties_changed_cb(void *, const Efl_Event *);
static Eina_Bool _elm_layout_part_cursor_unset(Elm_Layout_Smart_Data *sd, const char *part_name);

static const char SIG_THEME_CHANGED[] = "theme,changed";
const char SIG_LAYOUT_FOCUSED[] = "focused";
const char SIG_LAYOUT_UNFOCUSED[] = "unfocused";

const char SIGNAL_PREFIX[] = "signal/";

/* smart callbacks coming from elm layout objects: */
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_THEME_CHANGED, ""},
   {SIG_LAYOUT_FOCUSED, ""},
   {SIG_LAYOUT_UNFOCUSED, ""},
   {SIG_WIDGET_LANG_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_WIDGET_ACCESS_CHANGED, ""}, /**< handled by elm_widget */
   {NULL, NULL}
};

static const Elm_Layout_Part_Alias_Description _text_aliases[] =
{
   {"default", "elm.text"},
   {NULL, NULL}
};

static const Elm_Layout_Part_Alias_Description _content_aliases[] =
{
   {"default", "elm.swallow.content"},
   {NULL, NULL}
};

static const char *_elm_layout_swallow_parts[] = {
   "elm.swallow.icon",
   "elm.swallow.end",
   "elm.swallow.background",
   NULL
};

/* these are data operated by layout's class functions internally, and
 * should not be messed up by inhering classes */
typedef struct _Elm_Layout_Sub_Object_Data   Elm_Layout_Sub_Object_Data;
typedef struct _Elm_Layout_Sub_Object_Cursor Elm_Layout_Sub_Object_Cursor;
typedef struct _Elm_Layout_Sub_Iterator      Elm_Layout_Sub_Iterator;

struct _Elm_Layout_Sub_Iterator
{
   Eina_Iterator  iterator;
   Eina_Iterator *real_iterator;
   Elm_Layout    *object;
};

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

typedef struct _Elm_Layout_Sub_Property_Future Elm_Layout_Sub_Property_Future;
struct _Elm_Layout_Sub_Property_Future
{
   Elm_Layout_Smart_Data *pd;
   Eina_Array            *name_arr;
};

static void
_on_sub_object_size_hint_change(void *data,
                                Evas *e EINA_UNUSED,
                                Evas_Object *obj EINA_UNUSED,
                                void *event_info EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(data, wd);
   elm_obj_layout_sizing_eval(data);
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
   int i;

   //FIXME: Don't limit to the icon and end here.
   // send signals for all contents after elm 2.0
   if (sub_d->type != SWALLOW) return;
   for (i = 0;; i++)
     {
        if (!_elm_layout_swallow_parts[i]) return;
        if (!strcmp(sub_d->part, _elm_layout_swallow_parts[i])) break;
     }

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
   if ((sub_d->type != TEXT) ||
       (!((!strcmp("elm.text", sub_d->part)) ||
          (!strncmp("elm.text.", sub_d->part, 9)))))
     return;

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
_visuals_refresh(Evas_Object *obj,
                 Elm_Layout_Smart_Data *sd)
{
   Eina_Bool ret = EINA_FALSE;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   _parts_signals_emit(sd);
   _parts_cursors_apply(sd);

   edje_object_mirrored_set
     (wd->resize_obj, efl_ui_mirrored_get(obj));

   edje_object_scale_set
     (wd->resize_obj,
     efl_ui_scale_get(obj) * elm_config_scale_get());

   _elm_layout_highlight_in_theme(obj);

   ret = elm_obj_widget_disable(obj);

   elm_obj_layout_sizing_eval(obj);

   return ret;
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

static Elm_Theme_Apply
_elm_layout_theme_internal(Eo *obj, Elm_Layout_Smart_Data *sd)
{
   Elm_Theme_Apply ret = ELM_THEME_APPLY_FAILED;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, ELM_THEME_APPLY_FAILED);

   /* function already prints error messages, if any */
   if (!sd->file_set)
     {
        ret = elm_widget_theme_object_set
                (obj, wd->resize_obj, sd->klass, sd->group,
                 elm_widget_style_get(obj));
     }

   if (ret)
     efl_event_callback_legacy_call(obj, ELM_LAYOUT_EVENT_THEME_CHANGED, NULL);

   if (!_visuals_refresh(obj, sd))
     ret = ELM_THEME_APPLY_FAILED;

   return ret;
}

EOLIAN static Elm_Theme_Apply
_elm_layout_elm_widget_theme_apply(Eo *obj, Elm_Layout_Smart_Data *sd)
{
   Elm_Theme_Apply theme_apply = ELM_THEME_APPLY_FAILED;

   theme_apply = elm_obj_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (!theme_apply) return ELM_THEME_APPLY_FAILED;

   theme_apply &= _elm_layout_theme_internal(obj, sd);
   return theme_apply;
}

static void *
_elm_layout_list_data_get(const Eina_List *list)
{
   Elm_Layout_Sub_Object_Data *sub_d = eina_list_data_get(list);

   return sub_d->obj;
}

EOLIAN static Eina_Bool
_elm_layout_elm_widget_on_focus(Eo *obj, Elm_Layout_Smart_Data *_pd EINA_UNUSED, Elm_Object_Item *item EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   if (!elm_widget_can_focus_get(obj)) return EINA_FALSE;

   if (elm_widget_focus_get(obj))
     {
        elm_layout_signal_emit(obj, "elm,action,focus", "elm");
        evas_object_focus_set(wd->resize_obj, EINA_TRUE);
        efl_event_callback_legacy_call(obj, ELM_WIDGET_EVENT_FOCUSED, NULL);
        if (_elm_config->atspi_mode && !elm_widget_child_can_focus_get(obj))
          elm_interface_atspi_accessible_state_changed_signal_emit(obj, ELM_ATSPI_STATE_FOCUSED, EINA_TRUE);
     }
   else
     {
        elm_layout_signal_emit(obj, "elm,action,unfocus", "elm");
        evas_object_focus_set(wd->resize_obj, EINA_FALSE);
        efl_event_callback_legacy_call(obj, ELM_WIDGET_EVENT_UNFOCUSED, NULL);
        if (_elm_config->atspi_mode && !elm_widget_child_can_focus_get(obj))
          elm_interface_atspi_accessible_state_changed_signal_emit(obj, ELM_ATSPI_STATE_FOCUSED, EINA_FALSE);
     }
   if (efl_isa(wd->resize_obj, EDJE_OBJECT_CLASS))
     edje_object_message_signal_process(wd->resize_obj);

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
_elm_layout_elm_widget_focus_next(Eo *obj, Elm_Layout_Smart_Data *sd, Elm_Focus_Direction dir, Evas_Object **next, Elm_Object_Item **next_item)
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
            (obj, items, list_data_get, dir, next, next_item);
}

EOLIAN static Eina_Bool
_elm_layout_elm_widget_sub_object_add(Eo *obj, Elm_Layout_Smart_Data *_pd EINA_UNUSED, Evas_Object *sobj)
{
   Eina_Bool int_ret = EINA_FALSE;

   if (evas_object_data_get(sobj, "elm-parent") == obj) return EINA_TRUE;

   int_ret = elm_obj_widget_sub_object_add(efl_super(obj, MY_CLASS), sobj);
   if (!int_ret) return EINA_FALSE;

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

   int_ret = elm_obj_widget_sub_object_del(efl_super(obj, MY_CLASS), sobj);
   if (!int_ret) return EINA_FALSE;
   if (sd->destructed_is) return EINA_TRUE;

   EINA_LIST_FOREACH(sd->subs, l, sub_d)
     {
        if (sub_d->obj != sobj) continue;

        sd->subs = eina_list_remove_list(sd->subs, l);

        _icon_signal_emit(sd, sub_d, EINA_FALSE);

        eina_stringshare_del(sub_d->part);
        free(sub_d);

        break;
     }

   elm_obj_layout_sizing_eval(obj);

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
_elm_layout_elm_widget_focus_direction(Eo *obj, Elm_Layout_Smart_Data *sd, const Evas_Object *base, double degree, Evas_Object **direction, Elm_Object_Item **direction_item, double *weight)
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
            (obj, base, items, list_data_get, degree, direction, direction_item, weight);
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

EAPI Eina_Bool
_elm_layout_part_aliasing_eval(const Evas_Object *obj,
                               const char **part,
                               Eina_Bool is_text)
{
   const Elm_Layout_Part_Alias_Description *aliases = NULL;

   if (is_text)
     aliases = elm_layout_text_aliases_get(obj);
   else
     aliases =  elm_layout_content_aliases_get(obj);

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
            "part must not be NULL", obj);
        return EINA_FALSE;
     }

   /* if no match, part goes on with the same value */

   return EINA_TRUE;
}

static void
_eo_unparent_helper(Eo *child, Eo *parent)
{
   if (efl_parent_get(child) == parent)
     {
        efl_parent_set(child, evas_object_evas_get(parent));
     }
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

   _eo_unparent_helper(child, obj);
   if (!_elm_widget_sub_object_redirect_to_top(obj, child))
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

   _eo_unparent_helper(child, obj);

   if (!_elm_widget_sub_object_redirect_to_top(obj, child))
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
   elm_obj_layout_sizing_eval(data);
}

EOLIAN static void
_elm_layout_efl_canvas_group_group_add(Eo *obj, Elm_Layout_Smart_Data *_pd EINA_UNUSED)
{
   Evas_Object *edje;

   elm_widget_sub_object_parent_add(obj);

   /* has to be there *before* parent's smart_add() */
   edje = edje_object_add(evas_object_evas_get(obj));
   elm_widget_resize_object_set(obj, edje, EINA_TRUE);

   efl_canvas_group_add(efl_super(obj, MY_CLASS));

   elm_widget_can_focus_set(obj, EINA_FALSE);

   edje_object_signal_callback_add
     (edje, "size,eval", "elm", _on_size_evaluate_signal, obj);

   elm_obj_layout_sizing_eval(obj);
}

EOLIAN static void
_elm_layout_efl_canvas_group_group_del(Eo *obj, Elm_Layout_Smart_Data *sd)
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

   if(sd->model)
     {
         efl_event_callback_del(sd->model, EFL_MODEL_EVENT_PROPERTIES_CHANGED, _efl_model_properties_changed_cb, sd);
         efl_unref(sd->model);
         sd->model = NULL;
     }
   eina_hash_free(sd->prop_connect);
   sd->prop_connect = NULL;
   eina_hash_free(sd->factories);
   sd->factories = NULL;

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

   sd->destructed_is = EINA_TRUE;

   efl_canvas_group_del(efl_super(obj, MY_CLASS));
}

/* rewrite or extend this one on your derived class as to suit your
 * needs */
EOLIAN static void
_elm_layout_efl_canvas_group_group_calculate(Eo *obj, Elm_Layout_Smart_Data *sd)
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
_elm_layout_efl_file_file_set(Eo *obj, Elm_Layout_Smart_Data *sd, const char *file, const char *group)
{
   Eina_Bool int_ret = EINA_FALSE;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   int_ret =
     edje_object_file_set(wd->resize_obj, file, group);

   if (int_ret)
     {
        sd->file_set = EINA_TRUE;
        _visuals_refresh(obj, sd);
     }
   else
     ERR("failed to set edje file '%s', group '%s': %s",
         file, group,
         edje_load_error_str
           (edje_object_load_error_get(wd->resize_obj)));

   return int_ret;
}

EOLIAN static void
_elm_layout_efl_file_file_get(Eo *obj, Elm_Layout_Smart_Data *sd EINA_UNUSED, const char **file, const char **group)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   edje_object_file_get(wd->resize_obj, file, group);
}


EOLIAN static Eina_Bool
_elm_layout_efl_file_mmap_set(Eo *obj, Elm_Layout_Smart_Data *sd, const Eina_File *file, const char *group)
{
   Eina_Bool int_ret = EINA_FALSE;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   int_ret =
     edje_object_mmap_set(wd->resize_obj, file, group);

   if (int_ret)
     {
        sd->file_set = EINA_TRUE;
        _visuals_refresh(obj, sd);
     }
   else
     ERR("failed to set edje mmap file %p, group '%s': %s",
         file, group,
         edje_load_error_str
           (edje_object_load_error_get(wd->resize_obj)));

   return int_ret;
}

EOLIAN static void
_elm_layout_efl_file_mmap_get(Eo *obj, Elm_Layout_Smart_Data *sd EINA_UNUSED, const Eina_File **file, const char **group)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   efl_file_mmap_get(wd->resize_obj, file, group);
}

EOLIAN static Eina_Bool
_elm_layout_theme_set(Eo *obj, Elm_Layout_Smart_Data *sd, const char *klass, const char *group, const char *style)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   if (sd->file_set) sd->file_set = EINA_FALSE;
   eina_stringshare_replace(&(sd->klass), klass);
   eina_stringshare_replace(&(sd->group), group);
   eina_stringshare_replace(&(wd->style), style);

   if (_elm_layout_theme_internal(obj, sd))
     return EINA_TRUE;
   else
     return EINA_FALSE;
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
   if (!swallow)
     {
        swallow = elm_widget_default_content_part_get(obj);
        if (!swallow) return EINA_FALSE;
     }
   return efl_content_set(efl_part(obj, swallow), content);
}

static Eina_Bool
_elm_layout_content_set(Eo *obj, Elm_Layout_Smart_Data *sd, const char *part, Evas_Object *content)
{
   Elm_Layout_Sub_Object_Data *sub_d;
   const Eina_List *l;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   if (!_elm_layout_part_aliasing_eval(obj, &part, EINA_FALSE))
     return EINA_FALSE;

   EINA_LIST_FOREACH(sd->subs, l, sub_d)
     {
        if (sub_d->type == SWALLOW)
          {
             if (!strcmp(part, sub_d->part))
               {
                  if (content == sub_d->obj) goto end;
                  _eo_unparent_helper(sub_d->obj, obj);
                  evas_object_del(sub_d->obj);
                  break;
               }
             /* was previously swallowed at another part -- mimic
              * edje_object_part_swallow()'s behavior, then */
             else if (content == sub_d->obj)
               {
                  _elm_widget_sub_object_redirect_to_top(obj, content);
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
             _elm_widget_sub_object_redirect_to_top(obj, content);
             return EINA_FALSE;
          }

        sub_d = ELM_NEW(Elm_Layout_Sub_Object_Data);
        if (!sub_d)
          {
             ERR("failed to allocate memory!");
             edje_object_part_unswallow(wd->resize_obj, content);
             _elm_widget_sub_object_redirect_to_top(obj, content);
             return EINA_FALSE;
          }
        sub_d->type = SWALLOW;
        sub_d->part = eina_stringshare_add(part);
        sub_d->obj = content;
        sd->subs = eina_list_append(sd->subs, sub_d);

        efl_parent_set(content, obj);
        _icon_signal_emit(sd, sub_d, EINA_TRUE);
     }

   elm_obj_layout_sizing_eval(obj);

end:
   return EINA_TRUE;
}

EAPI Evas_Object *
elm_layout_content_get(const Evas_Object *obj,
                       const char *swallow)
{
   ELM_LAYOUT_CHECK(obj) NULL;
   if (!swallow)
     {
        swallow = elm_widget_default_content_part_get(obj);
        if (!swallow) return NULL;
     }
   return efl_content_get(efl_part(obj, swallow));
}

static Evas_Object*
_elm_layout_content_get(Eo *obj, Elm_Layout_Smart_Data *sd, const char *part)
{
   const Eina_List *l;
   Elm_Layout_Sub_Object_Data *sub_d;

   if (!_elm_layout_part_aliasing_eval(obj, &part, EINA_FALSE))
     return NULL;

   EINA_LIST_FOREACH(sd->subs, l, sub_d)
     {
        if ((sub_d->type != TEXT) && !strcmp(part, sub_d->part))
          {
             if (sub_d->type == SWALLOW)
               return sub_d->obj;
          }
     }

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);
   return efl_content_get(efl_part(wd->resize_obj, part));
}

EAPI Evas_Object *
elm_layout_content_unset(Evas_Object *obj,
                         const char *swallow)
{
   ELM_LAYOUT_CHECK(obj) NULL;
   return efl_content_unset(efl_part(obj, swallow));
}

static Evas_Object*
_elm_layout_content_unset(Eo *obj, Elm_Layout_Smart_Data *sd, const char *part)
{
   Elm_Layout_Sub_Object_Data *sub_d;
   const Eina_List *l;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   if (!_elm_layout_part_aliasing_eval(obj, &part, EINA_FALSE))
     return NULL;

   EINA_LIST_FOREACH(sd->subs, l, sub_d)
     {
        if ((sub_d->type == SWALLOW) && (!strcmp(part, sub_d->part)))
          {
             Evas_Object *content;

             if (!sub_d->obj) return NULL;

             content = sub_d->obj; /* sub_d will die in
                                    * _elm_layout_smart_sub_object_del */

             if (!_elm_widget_sub_object_redirect_to_top(obj, content))
               {
                  ERR("could not remove sub object %p from %p", content, obj);
                  return NULL;
               }

             edje_object_part_unswallow
               (wd->resize_obj, content);
             _eo_unparent_helper(content, obj);
             return content;
          }
     }

   return NULL;
}

EOLIAN static Eina_Bool
_elm_layout_efl_container_content_set(Eo *obj, Elm_Layout_Smart_Data *sd, Evas_Object *content)
{
   return _elm_layout_content_set(obj, sd, NULL, content);
}

EOLIAN static Evas_Object*
_elm_layout_efl_container_content_get(Eo *obj EINA_UNUSED, Elm_Layout_Smart_Data *sd)
{
   return _elm_layout_content_get(obj, sd, NULL);
}

EOLIAN static Evas_Object*
_elm_layout_efl_container_content_unset(Eo *obj, Elm_Layout_Smart_Data *sd)
{
   return _elm_layout_content_unset(obj, sd, NULL);
}

EOLIAN static Eina_Bool
_elm_layout_efl_container_content_remove(Eo *obj, Elm_Layout_Smart_Data *sd EINA_UNUSED,
                                         Efl_Gfx *content)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   if (!_elm_widget_sub_object_redirect_to_top(obj, content))
     {
        ERR("could not remove sub object %p from %p", content, obj);
        return EINA_FALSE;
     }
   edje_object_part_unswallow(wd->resize_obj, content);
   _eo_unparent_helper(content, obj);

   return EINA_TRUE;
}

/* legacy only - eo is iterator */
EAPI Eina_List *
elm_layout_content_swallow_list_get(const Evas_Object *obj)
{
   ELM_LAYOUT_CHECK(obj) NULL;
   Eina_List *ret = NULL;
   Elm_Layout_Sub_Object_Data *sub_d = NULL;
   Elm_Layout_Smart_Data *sd = efl_data_scope_get(obj, MY_CLASS);
   Eina_List *l = NULL;

   EINA_LIST_FOREACH(sd->subs, l, sub_d)
     {
        if (sub_d->type == SWALLOW)
          ret = eina_list_append(ret, sub_d->obj);
     }

   return ret;
}

static Eina_Bool
_sub_iterator_next(Elm_Layout_Sub_Iterator *it, void **data)
{
   Elm_Layout_Sub_Object_Data *sub;

   if (!eina_iterator_next(it->real_iterator, (void **)&sub))
     return EINA_FALSE;

   if (data) *data = sub->obj;
   return EINA_TRUE;
}

static Elm_Layout *
_sub_iterator_get_container(Elm_Layout_Sub_Iterator *it)
{
   return it->object;
}

static void
_sub_iterator_free(Elm_Layout_Sub_Iterator *it)
{
   eina_iterator_free(it->real_iterator);
   free(it);
}

static Eina_Iterator *
_sub_iterator_create(Eo *eo_obj, Elm_Layout_Smart_Data *sd)
{
   Elm_Layout_Sub_Iterator *it;

   it = calloc(1, sizeof(*it));
   if (!it) return NULL;

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);

   it->real_iterator = eina_list_iterator_new(sd->subs);
   it->iterator.version = EINA_ITERATOR_VERSION;
   it->iterator.next = FUNC_ITERATOR_NEXT(_sub_iterator_next);
   it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(_sub_iterator_get_container);
   it->iterator.free = FUNC_ITERATOR_FREE(_sub_iterator_free);
   it->object = eo_obj;

   return &it->iterator;
}

EOLIAN static Eina_Iterator *
_elm_layout_efl_container_content_iterate(Eo *eo_obj EINA_UNUSED, Elm_Layout_Smart_Data *sd)
{
   return _sub_iterator_create(eo_obj, sd);
}

EOLIAN static int
_elm_layout_efl_container_content_count(Eo *eo_obj EINA_UNUSED, Elm_Layout_Smart_Data *sd)
{
   return eina_list_count(sd->subs);
}

EOLIAN static Eina_Bool
_elm_layout_text_set(Eo *obj, Elm_Layout_Smart_Data *sd, const char *part, const char *text)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   Eina_List *l;
   Elm_Layout_Sub_Object_Data *sub_d = NULL;

   if (!_elm_layout_part_aliasing_eval(obj, &part, EINA_TRUE))
     return EINA_FALSE;

   EINA_LIST_FOREACH(sd->subs, l, sub_d)
     {
        if ((sub_d->type == TEXT) && (!strcmp(part, sub_d->part)))
          {
             if (!text)
               {
                  eina_stringshare_del(sub_d->part);
                  free(sub_d);
                  edje_object_part_text_escaped_set
                    (wd->resize_obj, part, NULL);
                  sd->subs = eina_list_remove_list(sd->subs, l);
                  return EINA_TRUE;
               }
             else
               break;
          }
     }

   if (!text) return EINA_TRUE;

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

   _text_signal_emit(sd, sub_d, !!text);

   elm_obj_layout_sizing_eval(obj);

   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON &&
       sd->can_access && !(sub_d->obj))
     sub_d->obj = _elm_access_edje_object_part_object_register
         (obj, elm_layout_edje_get(obj), part);

   if (sd->model && !sd->view_updated)
     {
        Eina_Stringshare *prop = eina_hash_find(sd->prop_connect, sub_d->part);
        if (prop)
          {
             Eina_Value v;
             eina_value_setup(&v, EINA_VALUE_TYPE_STRING);
             eina_value_set(&v, text);
             efl_model_property_set(sd->model, prop, &v);
             eina_value_flush(&v);
          }
     }

   sd->view_updated = EINA_FALSE;
   return EINA_TRUE;
}

EOLIAN static const char*
_elm_layout_text_get(Eo *obj, Elm_Layout_Smart_Data *sd EINA_UNUSED, const char *part)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   if (!_elm_layout_part_aliasing_eval(obj, &part, EINA_TRUE))
     return NULL;

   return edje_object_part_text_get(wd->resize_obj, part);
}

static void
_layout_box_subobj_init(Elm_Layout_Smart_Data *sd, Elm_Layout_Sub_Object_Data *sub_d, const char *part, Evas_Object *child)
{
   sub_d->part = eina_stringshare_add(part);
   sub_d->obj = child;
   sd->subs = eina_list_append(sd->subs, sub_d);
   efl_parent_set(child, sd->obj);
}

Eina_Bool
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
   if (!sub_d)
     {
        ERR("failed to allocate memory!");
        _elm_widget_sub_object_redirect_to_top(obj, child);
        edje_object_part_box_remove(wd->resize_obj, part, child);
        return EINA_FALSE;
     }
   sub_d->type = BOX_APPEND;
   _layout_box_subobj_init(sd, sub_d, part, child);

   elm_obj_layout_sizing_eval(obj);

   return EINA_TRUE;
}

Eina_Bool
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
   if (!sub_d)
     {
        ERR("failed to allocate memory!");
        _elm_widget_sub_object_redirect_to_top(obj, child);
        edje_object_part_box_remove(wd->resize_obj, part, child);
        return EINA_FALSE;
     }
   sub_d->type = BOX_PREPEND;
   _layout_box_subobj_init(sd, sub_d, part, child);

   elm_obj_layout_sizing_eval(obj);

   return EINA_TRUE;
}

Eina_Bool
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
   if (!sub_d)
     {
        ERR("failed to allocate memory!");
        _elm_widget_sub_object_redirect_to_top(obj, child);
        edje_object_part_box_remove(wd->resize_obj, part, child);
        return EINA_FALSE;
     }
   sub_d->type = BOX_INSERT_BEFORE;
   sub_d->p.box.reference = reference;
   _layout_box_subobj_init(sd, sub_d, part, child);

   evas_object_event_callback_add
     ((Evas_Object *)reference, EVAS_CALLBACK_DEL, _box_reference_del, sub_d);

   elm_obj_layout_sizing_eval(obj);

   return EINA_TRUE;
}

Eina_Bool
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
   if (!sub_d)
     {
        ERR("failed to allocate memory!");
        _elm_widget_sub_object_redirect_to_top(obj, child);
        edje_object_part_box_remove(wd->resize_obj, part, child);
        return EINA_FALSE;
     }
   sub_d->type = BOX_INSERT_AT;
   sub_d->p.box.pos = pos;
   _layout_box_subobj_init(sd, sub_d, part, child);

   elm_obj_layout_sizing_eval(obj);

   return EINA_TRUE;
}

Evas_Object *
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

Eina_Bool
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

Eina_Bool
_elm_layout_table_pack(Eo *obj, Elm_Layout_Smart_Data *sd, const char *part, Evas_Object *child, unsigned short col, unsigned short row, unsigned short colspan, unsigned short rowspan)
{
   Elm_Layout_Sub_Object_Data *sub_d;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   if (!edje_object_part_table_pack
         (wd->resize_obj, part, child, col,
         row, colspan, rowspan))
     {
        ERR("child %p could not be packed into table part '%s' col=%uh, row=%hu,"
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
   if (!sub_d)
     {
        ERR("failed to allocate memory!");
        _elm_widget_sub_object_redirect_to_top(obj, child);
        edje_object_part_table_unpack(wd->resize_obj, part, child);
        return EINA_FALSE;
     }
   sub_d->type = TABLE_PACK;
   sub_d->part = eina_stringshare_add(part);
   sub_d->obj = child;
   sub_d->p.table.col = col;
   sub_d->p.table.row = row;
   sub_d->p.table.colspan = colspan;
   sub_d->p.table.rowspan = rowspan;
   sd->subs = eina_list_append(sd->subs, sub_d);
   efl_parent_set(child, obj);

   elm_obj_layout_sizing_eval(obj);

   return EINA_TRUE;
}

Evas_Object *
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

Eina_Bool
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

EAPI Evas_Object*
elm_layout_edje_get(const Eo *obj)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL(efl_isa(obj, MY_CLASS), NULL);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   return wd->resize_obj;
}

EOLIAN static const char*
_elm_layout_data_get(const Eo *obj, Elm_Layout_Smart_Data *_pd EINA_UNUSED, const char *key)
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
_elm_layout_efl_canvas_layout_calc_calc_freeze(Eo *obj, Elm_Layout_Smart_Data *sd)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, 0);

   if ((sd->frozen)++ != 0) return sd->frozen;

   edje_object_freeze(wd->resize_obj);

   return 1;
}

EOLIAN static int
_elm_layout_efl_canvas_layout_calc_calc_thaw(Eo *obj, Elm_Layout_Smart_Data *sd)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, 0);

   if (--(sd->frozen) != 0) return sd->frozen;

   edje_object_thaw(wd->resize_obj);

   elm_obj_layout_sizing_eval(obj);

   return 0;
}

static Eina_Bool
_elm_layout_part_cursor_set(Elm_Layout_Smart_Data *sd, const char *part_name, const char *cursor)
{
   Evas_Object *part_obj;
   Elm_Layout_Sub_Object_Cursor *pc;
   Eo *obj = sd->obj;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(part_name, EINA_FALSE);

   if (!cursor) return _elm_layout_part_cursor_unset(sd, part_name);

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
        if (!pc)
          {
             ERR("failed to allocate memory!");
             return EINA_FALSE;
          }
        pc->part = eina_stringshare_add(part_name);
        pc->cursor = eina_stringshare_add(cursor);
        pc->style = eina_stringshare_add("default");
        sd->parts_cursors = eina_list_append(sd->parts_cursors, pc);
     }

   pc->obj = part_obj;
   elm_object_sub_cursor_set(part_obj, obj, pc->cursor);

   return EINA_TRUE;
}

static const char *
_elm_layout_part_cursor_get(Elm_Layout_Smart_Data *sd, const char *part_name)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(part_name, NULL);

   Elm_Layout_Sub_Object_Cursor *pc = _parts_cursors_find(sd, part_name);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pc, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pc->obj, NULL);

   return elm_object_cursor_get(pc->obj);
}

static Eina_Bool
_elm_layout_part_cursor_unset(Elm_Layout_Smart_Data *sd, const char *part_name)
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

static Eina_Bool
_elm_layout_part_cursor_style_set(Elm_Layout_Smart_Data *sd, const char *part_name, const char *style)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(part_name, EINA_FALSE);

   Elm_Layout_Sub_Object_Cursor *pc = _parts_cursors_find(sd, part_name);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pc, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pc->obj, EINA_FALSE);

   eina_stringshare_replace(&pc->style, style);
   elm_object_cursor_style_set(pc->obj, pc->style);

   return EINA_TRUE;
}

static const char*
_elm_layout_part_cursor_style_get(Elm_Layout_Smart_Data *sd, const char *part_name)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(part_name, NULL);

   Elm_Layout_Sub_Object_Cursor *pc = _parts_cursors_find(sd, part_name);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pc, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pc->obj, NULL);

   return elm_object_cursor_style_get(pc->obj);
}

static Eina_Bool
_elm_layout_part_cursor_engine_only_set(Elm_Layout_Smart_Data *sd, const char *part_name, Eina_Bool engine_only)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(part_name, EINA_FALSE);

   Elm_Layout_Sub_Object_Cursor *pc = _parts_cursors_find(sd, part_name);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pc, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pc->obj, EINA_FALSE);

   pc->engine_only = !!engine_only;
   elm_object_cursor_theme_search_enabled_set(pc->obj, !pc->engine_only);

   return EINA_TRUE;
}

static Eina_Bool
_elm_layout_part_cursor_engine_only_get(Elm_Layout_Smart_Data *sd, const char *part_name)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(part_name, EINA_FALSE);

   Elm_Layout_Sub_Object_Cursor *pc = _parts_cursors_find(sd, part_name);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pc, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pc->obj, EINA_FALSE);

   return !elm_object_cursor_theme_search_enabled_get(pc->obj);
}

EAPI Eina_Bool
elm_layout_edje_object_can_access_set(Eo *obj, Eina_Bool can_access)
{
   Elm_Layout_Smart_Data *sd = efl_data_scope_safe_get(obj, MY_CLASS);
   EINA_SAFETY_ON_NULL_RETURN_VAL(sd, EINA_FALSE);
   sd->can_access = !!can_access;
   return EINA_TRUE;
}

EAPI Eina_Bool
elm_layout_edje_object_can_access_get(const Eo *obj)
{
   Elm_Layout_Smart_Data *sd = efl_data_scope_safe_get(obj, MY_CLASS);
   EINA_SAFETY_ON_NULL_RETURN_VAL(sd, EINA_FALSE);
   return sd->can_access;
}

EOLIAN static void
_elm_layout_efl_object_dbg_info_get(Eo *eo_obj, Elm_Layout_Smart_Data *_pd EINA_UNUSED, Efl_Dbg_Info *root)
{
   efl_dbg_info_get(efl_super(eo_obj, MY_CLASS), root);
   ELM_WIDGET_DATA_GET_OR_RETURN(eo_obj, wd);

   if (wd->resize_obj && efl_isa(wd->resize_obj, EDJE_OBJECT_CLASS))
     {
        Efl_Dbg_Info *group = EFL_DBG_INFO_LIST_APPEND(root, MY_CLASS_NAME);
        const char *file, *edje_group;
        Evas_Object *edje_obj = wd->resize_obj;
        Edje_Load_Error error;

        efl_file_get(edje_obj, &file, &edje_group);
        EFL_DBG_INFO_APPEND(group, "File", EINA_VALUE_TYPE_STRING, file);
        EFL_DBG_INFO_APPEND(group, "Group", EINA_VALUE_TYPE_STRING, edje_group);

        error = edje_object_load_error_get(edje_obj);
        if (error != EDJE_LOAD_ERROR_NONE)
          {
             EFL_DBG_INFO_APPEND(group, "Error", EINA_VALUE_TYPE_STRING,
                                edje_load_error_str(error));
          }
     }
}

static void
_prop_future_error_cb(void* data, Efl_Event const*event EINA_UNUSED)
{
   Elm_Layout_Sub_Property_Future *sub_pp = data;
   Eina_Array_Iterator iterator;
   Eina_Stringshare  *name;
   unsigned int i = 0;

   EINA_ARRAY_ITER_NEXT(sub_pp->name_arr, i, name, iterator)
     eina_stringshare_del(name);

   eina_array_free(sub_pp->name_arr);
   free(sub_pp);
}

static void
_view_update(Elm_Layout_Smart_Data *pd, const char *name, const char *property)
{
   Eina_Strbuf *buf;

   if (strncmp(SIGNAL_PREFIX, name, sizeof(SIGNAL_PREFIX) -1) != 0)
     {
         elm_layout_text_set(pd->obj, name, property);
         return;
     }

   ELM_WIDGET_DATA_GET_OR_RETURN(pd->obj, wd);

   buf = eina_strbuf_new();
   eina_strbuf_append(buf, name);
   eina_strbuf_remove(buf, 0, sizeof(SIGNAL_PREFIX)-1);
   eina_strbuf_replace_all(buf, "%v", property);

   edje_object_signal_emit(wd->resize_obj, eina_strbuf_string_get(buf), "elm");
   eina_strbuf_free(buf);
}

static void
_prop_future_then_cb(void* data, Efl_Event const*event)
{
   Elm_Layout_Sub_Property_Future *sub_pp = data;
   Elm_Layout_Smart_Data *pd = sub_pp->pd;
   Eina_Accessor *value_acc = (Eina_Accessor *)((Efl_Future_Event_Success*)event->info)->value;
   Eina_Value *value;
   Eina_Stringshare *name;
   char *text;
   unsigned int i = 0;
   unsigned int acc_i = 0;

   while (eina_accessor_data_get(value_acc, acc_i, (void **)&value))
     {
         const Eina_Value_Type *vtype = eina_value_type_get(value);
         name = eina_array_data_get(sub_pp->name_arr, i);

         pd->view_updated = EINA_TRUE;
         if (vtype == EINA_VALUE_TYPE_STRING || vtype == EINA_VALUE_TYPE_STRINGSHARE)
           {
               eina_value_get(value, &text);
               _view_update(pd, name, text);
           }
         else
           {
               text = eina_value_to_string(value);
               _view_update(pd, name, text);
               free(text);
           }
        eina_stringshare_del(name);
        ++acc_i;
     }
   eina_array_free(sub_pp->name_arr);
   free(sub_pp);
}

static void
_elm_layout_view_model_update(Elm_Layout_Smart_Data *pd)
{
   Elm_Layout_Sub_Property_Future *sub_pp;
   Efl_Future **future_arr, **f, *future_all;
   Eina_Hash_Tuple *tuple;
   Eina_Iterator *it_p;
   int size;

   if (!pd->prop_connect || !pd->model) return;

   size = eina_hash_population(pd->prop_connect);
   if (size == 0) return;

   future_arr = alloca((size + 1) * sizeof(Efl_Future*));
   f = future_arr;

   sub_pp = ELM_NEW(Elm_Layout_Sub_Property_Future);
   sub_pp->pd = pd;
   sub_pp->name_arr = eina_array_new(size);

   it_p = eina_hash_iterator_tuple_new(pd->prop_connect);
   while (eina_iterator_next(it_p, (void **)&tuple))
     {
         *f = efl_model_property_get(pd->model, tuple->data);
         eina_array_push(sub_pp->name_arr, eina_stringshare_ref(tuple->key));
         f++;
     }
   eina_iterator_free(it_p);
   *f = NULL;

   future_all = efl_future_iterator_all(eina_carray_iterator_new((void**)future_arr));

   efl_future_then(future_all, &_prop_future_then_cb, &_prop_future_error_cb, NULL, sub_pp);
}

static void
_efl_model_properties_changed_cb(void *data, const Efl_Event *event)
{
   Elm_Layout_Smart_Data *pd = data;
   Efl_Model_Property_Event *evt = event->info;
   Eina_Stringshare *ss_prop;
   Eina_Hash_Tuple *tuple;
   Eina_Array *names, *futures;
   Eina_Iterator *it_p;
   const char *prop;
   Eina_Array_Iterator it;
   unsigned int i;

   if (!evt->changed_properties || !pd->prop_connect) return;

   names = eina_array_new(1);
   futures = eina_array_new(1);

   EINA_ARRAY_ITER_NEXT(evt->changed_properties, i, prop, it)
     {
         ss_prop = eina_stringshare_add(prop);
         it_p = eina_hash_iterator_tuple_new(pd->prop_connect);
         while (eina_iterator_next(it_p, (void **)&tuple))
           {
              if (tuple->data == ss_prop)
                {
                    eina_array_push(names, eina_stringshare_ref(tuple->key));
                    eina_array_push(futures, efl_model_property_get(pd->model, prop));
                }
           }
         eina_iterator_free(it_p);
         eina_stringshare_del(ss_prop);
     }

   if (eina_array_count(names))
     {
         Elm_Layout_Sub_Property_Future *sub_pp;
         Efl_Future *future_all;

         sub_pp = ELM_NEW(Elm_Layout_Sub_Property_Future);
         sub_pp->pd = pd;
         sub_pp->name_arr = names;

         future_all = efl_future_iterator_all(eina_array_iterator_new(futures));
         efl_future_then(future_all, &_prop_future_then_cb, &_prop_future_error_cb, NULL, sub_pp);
     }
   else
     eina_array_free(names);

   eina_array_free(futures);
}

EOLIAN static void
_elm_layout_efl_ui_view_model_set(Eo *obj EINA_UNUSED, Elm_Layout_Smart_Data *pd, Efl_Model *model)
{
   if (pd->model)
     {
         efl_event_callback_del(pd->model, EFL_MODEL_EVENT_PROPERTIES_CHANGED, _efl_model_properties_changed_cb, pd);
         efl_unref(pd->model);
         pd->model = NULL;
     }

   if (model)
     {
         pd->model = model;
         efl_ref(pd->model);
         efl_event_callback_add(pd->model, EFL_MODEL_EVENT_PROPERTIES_CHANGED, _efl_model_properties_changed_cb, pd);
     }
   else
     return;

   if (pd->prop_connect)
     _elm_layout_view_model_update(pd);

   if (pd->factories)
     {
         Eina_Hash_Tuple *tuple;
         Eina_Stringshare *name;
         Efl_Ui_Factory *factory;
         Efl_Gfx *content;

         Eina_Iterator *it_p = eina_hash_iterator_tuple_new(pd->factories);
         while (eina_iterator_next(it_p, (void **)&tuple))
           {
               name = tuple->key;
               factory = tuple->data;
               content = elm_layout_content_get(pd->obj, name);

               if (content && efl_isa(content, EFL_UI_VIEW_INTERFACE))
                 {
                     efl_ui_view_model_set(content, pd->model);
                 }
               else
                 {
                     efl_ui_factory_release(factory, content);
                     content = efl_ui_factory_create(factory, pd->model, pd->obj);
                     elm_layout_content_set(pd->obj, name, content);
                 }
           }
         eina_iterator_free(it_p);
     }
}

EOLIAN static Efl_Model *
_elm_layout_efl_ui_view_model_get(Eo *obj EINA_UNUSED, Elm_Layout_Smart_Data *pd)
{
   return pd->model;
}

EOLIAN static void
_elm_layout_efl_ui_model_connect_connect(Eo *obj EINA_UNUSED, Elm_Layout_Smart_Data *pd, const char *name, const char *property)
{
   EINA_SAFETY_ON_NULL_RETURN(name);
   Eina_Stringshare *ss_name, *ss_prop;

   if (property == NULL && pd->prop_connect)
     {
        ss_name = eina_stringshare_add(name);
        eina_hash_del(pd->prop_connect, ss_name, NULL);
        return;
     }

   if (!_elm_layout_part_aliasing_eval(obj, &name, EINA_TRUE))
     return;

   ss_name = eina_stringshare_add(name);
   ss_prop = eina_stringshare_add(property);
   if (!pd->prop_connect)
     {
         pd->prop_connect = eina_hash_stringshared_new(EINA_FREE_CB(eina_stringshare_del));
     }

   eina_stringshare_del(eina_hash_set(pd->prop_connect, ss_name, ss_prop));

   if (pd->model)
     {
         Elm_Layout_Sub_Property_Future *sub_pp = ELM_NEW(Elm_Layout_Sub_Property_Future);
         Efl_Future *futures[2] = {NULL,};
         Efl_Future *future_all = NULL;

         sub_pp->pd = pd;
         sub_pp->name_arr = eina_array_new(1);
         eina_array_push(sub_pp->name_arr, eina_stringshare_ref(ss_name));
         futures[0] = efl_model_property_get(pd->model, ss_prop);

         future_all = efl_future_iterator_all(eina_carray_iterator_new((void**)futures));
         efl_future_then(future_all, &_prop_future_then_cb, &_prop_future_error_cb, NULL, sub_pp);
     }
}


EOLIAN static void
_elm_layout_efl_ui_model_factory_connect_connect(Eo *obj EINA_UNUSED, Elm_Layout_Smart_Data *pd,
                const char *name, Efl_Ui_Factory *factory)
{
   EINA_SAFETY_ON_NULL_RETURN(name);
   Eina_Stringshare *ss_name;
   Efl_Ui_Factory *old_factory;
   Evas_Object *new_ev, *old_ev;

   if (!_elm_layout_part_aliasing_eval(obj, &name, EINA_TRUE))
     return;

   ss_name = eina_stringshare_add(name);

   if (!pd->factories)
     pd->factories = eina_hash_stringshared_new(EINA_FREE_CB(efl_unref));

   new_ev = efl_ui_factory_create(factory, pd->model, obj);
   EINA_SAFETY_ON_NULL_RETURN(new_ev);

   old_factory = eina_hash_set(pd->factories, ss_name, efl_ref(factory));
   if (old_factory)
     {
         old_ev = elm_layout_content_get(obj, name);
         if (old_ev)
           efl_ui_factory_release(old_factory, old_ev);
         efl_unref(old_factory);
     }

   elm_layout_content_set(obj, name, new_ev);
}

EAPI Evas_Object *
elm_layout_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   return efl_add(MY_CLASS, parent, efl_canvas_object_legacy_ctor(efl_added));
}

EOLIAN static Eo *
_elm_layout_efl_object_constructor(Eo *obj, Elm_Layout_Smart_Data *sd)
{
   sd->obj = obj;
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);
   evas_object_smart_callbacks_descriptions_set(obj, _smart_callbacks);
   elm_interface_atspi_accessible_role_set(obj, ELM_ATSPI_ROLE_FILLER);

   return obj;
}

EOLIAN static void _elm_layout_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}



/* Legacy APIs */

EAPI Eina_Bool
elm_layout_file_set(Eo *obj, const char *file, const char *group)
{
   return efl_file_set((Eo *) obj, file, group);
}

EAPI void
elm_layout_file_get(Eo *obj, const char **file, const char **group)
{
   efl_file_get((Eo *) obj, file, group);
}

EAPI Eina_Bool
elm_layout_mmap_set(Eo *obj, const Eina_File *file, const char *group)
{
   return efl_file_mmap_set((Eo *) obj, file, group);
}

EAPI void
elm_layout_mmap_get(Eo *obj, const Eina_File **file, const char **group)
{
   efl_file_mmap_get((Eo *) obj, file, group);
}

EAPI Eina_Bool
elm_layout_box_append(Elm_Layout *obj, const char *part, Evas_Object *child)
{
   return efl_pack(efl_part(obj, part), child);
}

EAPI Eina_Bool
elm_layout_box_prepend(Elm_Layout *obj, const char *part, Evas_Object *child)
{
   return efl_pack_begin(efl_part(obj, part), child);
}

EAPI Eina_Bool
elm_layout_box_insert_before(Elm_Layout *obj, const char *part, Evas_Object *child, const Evas_Object *reference)
{
   return efl_pack_before(efl_part(obj, part), child, reference);
}

EAPI Eina_Bool
elm_layout_box_insert_at(Elm_Layout *obj, const char *part, Evas_Object *child, unsigned int pos)
{
   return efl_pack_at(efl_part(obj, part), child, pos);
}

EAPI Evas_Object *
elm_layout_box_remove(Elm_Layout *obj, const char *part, Evas_Object *child)
{
   if (!efl_pack_unpack(efl_part(obj, part), child))
     return NULL;
   return child;
}

EAPI Eina_Bool
elm_layout_box_remove_all(Elm_Layout *obj, const char *part, Eina_Bool clear)
{
   if (clear)
     return efl_pack_clear(efl_part(obj, part));
   else
     return efl_pack_unpack_all(efl_part(obj, part));
}

EAPI Eina_Bool
elm_layout_table_pack(Elm_Layout *obj, const char *part, Evas_Object *child, unsigned short col, unsigned short row, unsigned short colspan, unsigned short rowspan)
{
   return efl_pack_grid(efl_part(obj, part), child, col, row, colspan, rowspan);
}

EAPI Evas_Object *
elm_layout_table_unpack(Elm_Layout *obj, const char *part, Evas_Object *child)
{
   if (!efl_pack_unpack(efl_part(obj, part), child))
     return NULL;
   return child;
}

EAPI Eina_Bool
elm_layout_table_clear(Elm_Layout *obj, const char *part, Eina_Bool clear)
{
   if (clear)
     return efl_pack_clear(efl_part(obj, part));
   else
     return efl_pack_unpack_all(efl_part(obj, part));
}

EAPI Eina_Bool
elm_layout_text_set(Elm_Layout *obj, const char *part, const char *text)
{
   if (!part)
     {
        part = elm_widget_default_text_part_get(obj);
        if (!part) return EINA_FALSE;
     }
   efl_text_set(efl_part(obj, part), text);
   return EINA_TRUE;
}

EAPI const char *
elm_layout_text_get(const Elm_Layout *obj, const char *part)
{
   if (!part)
     {
        part = elm_widget_default_text_part_get(obj);
        if (!part) return NULL;
     }
   return efl_text_get(efl_part(obj, part));
}

EAPI Eina_Bool
elm_layout_part_cursor_engine_only_set(Elm_Layout *obj, const char *part, Eina_Bool engine_only)
{
   return efl_ui_cursor_theme_search_enabled_set(efl_part(obj, part), !engine_only);
}

EAPI Eina_Bool
elm_layout_part_cursor_engine_only_get(const Elm_Layout *obj, const char *part)
{
   return !efl_ui_cursor_theme_search_enabled_get(efl_part(obj, part));
}

EAPI Eina_Bool
elm_layout_part_cursor_set(Elm_Layout *obj, const char *part, const char *cursor)
{
   return efl_ui_cursor_set(efl_part(obj, part), cursor);
}

EAPI const char *
elm_layout_part_cursor_get(const Elm_Layout *obj, const char *part)
{
   return efl_ui_cursor_get(efl_part(obj, part));
}

EAPI Eina_Bool
elm_layout_part_cursor_style_set(Elm_Layout *obj, const char *part, const char *style)
{
   return efl_ui_cursor_style_set(efl_part(obj, part), style);
}

EAPI const char *
elm_layout_part_cursor_style_get(const Elm_Layout *obj, const char *part)
{
   return efl_ui_cursor_style_get(efl_part(obj, part));
}

EAPI Eina_Bool
elm_layout_part_cursor_unset(Elm_Layout *obj, const char *part)
{
   return efl_ui_cursor_set(efl_part(obj, part), NULL);
}

EAPI int
elm_layout_freeze(Evas_Object *obj)
{
   return efl_canvas_layout_calc_freeze(obj);
}

EAPI int
elm_layout_thaw(Evas_Object *obj)
{
   return efl_canvas_layout_calc_thaw(obj);
}

/* End of legacy only */


/* Efl.Part implementation */

static EOLIAN Efl_Object *
_elm_layout_efl_part_part(const Eo *obj, Elm_Layout_Smart_Data *sd EINA_UNUSED,
                          const char *part)
{
   Edje_Part_Type type;
   Elm_Part_Data *pd;
   Eo *proxy;

   EINA_SAFETY_ON_NULL_RETURN_VAL(part, NULL);
   ELM_WIDGET_DATA_GET_OR_RETURN((Eo *) obj, wd, NULL);

   // Check part type with edje_edit, as edje_object_part_object_get()
   // has side effects (it calls recalc, which may be really bad).
   type = edje_edit_part_type_get(wd->resize_obj, part);
   if ((type == EDJE_PART_TYPE_BOX) || (type == EDJE_PART_TYPE_TABLE))
     return _elm_layout_pack_proxy_get((Eo *) obj, type, part);

   // Generic parts (text, anything, ...)
   proxy = efl_add(ELM_LAYOUT_INTERNAL_PART_CLASS, (Eo *) obj);
   pd = efl_data_scope_get(proxy, ELM_LAYOUT_INTERNAL_PART_CLASS);
   if (pd)
     {
        pd->obj = (Eo *) obj;
        pd->sd = efl_data_xref(pd->obj, ELM_LAYOUT_CLASS, proxy);
        pd->part = strdup(part);
        pd->temp = 1;
     }

   return proxy;
}

static const char *
_elm_layout_default_content_part_get(const Eo *obj, Elm_Layout_Smart_Data *sd EINA_UNUSED)
{
   const char *part = NULL;
   if (!_elm_layout_part_aliasing_eval(obj, &part, EINA_FALSE))
     return NULL;
   return part;
}

static const char *
_elm_layout_default_text_part_get(const Eo *obj, Elm_Layout_Smart_Data *sd EINA_UNUSED)
{
   const char *part = NULL;
   if (!_elm_layout_part_aliasing_eval(obj, &part, EINA_TRUE))
     return NULL;
   return part;
}

EOLIAN static Eina_Bool
_elm_layout_internal_part_efl_ui_cursor_cursor_set(Eo *obj, Elm_Part_Data *pd, const char *cursor)
{
   ELM_PART_RETURN_VAL(_elm_layout_part_cursor_set(pd->sd, pd->part, cursor));
}

EOLIAN static const char *
_elm_layout_internal_part_efl_ui_cursor_cursor_get(Eo *obj, Elm_Part_Data *pd)
{
   ELM_PART_RETURN_VAL(_elm_layout_part_cursor_get(pd->sd, pd->part));
}

EOLIAN static Eina_Bool
_elm_layout_internal_part_efl_ui_cursor_cursor_style_set(Eo *obj, Elm_Part_Data *pd, const char *style)
{
   ELM_PART_RETURN_VAL(_elm_layout_part_cursor_style_set(pd->sd, pd->part, style));
}

EOLIAN static const char *
_elm_layout_internal_part_efl_ui_cursor_cursor_style_get(Eo *obj, Elm_Part_Data *pd)
{
   ELM_PART_RETURN_VAL(_elm_layout_part_cursor_style_get(pd->sd, pd->part));
}

EOLIAN static Eina_Bool
_elm_layout_internal_part_efl_ui_cursor_cursor_theme_search_enabled_set(Eo *obj, Elm_Part_Data *pd, Eina_Bool allow)
{
   ELM_PART_RETURN_VAL(_elm_layout_part_cursor_engine_only_set(pd->sd, pd->part, !allow));
}

EOLIAN static Eina_Bool
_elm_layout_internal_part_efl_ui_cursor_cursor_theme_search_enabled_get(Eo *obj, Elm_Part_Data *pd)
{
   ELM_PART_RETURN_VAL(!_elm_layout_part_cursor_engine_only_get(pd->sd, pd->part));
}

ELM_PART_IMPLEMENT_DESTRUCTOR(elm_layout, ELM_LAYOUT, Elm_Layout_Smart_Data, Elm_Part_Data)
ELM_PART_IMPLEMENT_CONTENT_SET(elm_layout, ELM_LAYOUT, Elm_Layout_Smart_Data, Elm_Part_Data)
ELM_PART_IMPLEMENT_CONTENT_GET(elm_layout, ELM_LAYOUT, Elm_Layout_Smart_Data, Elm_Part_Data)
ELM_PART_IMPLEMENT_CONTENT_UNSET(elm_layout, ELM_LAYOUT, Elm_Layout_Smart_Data, Elm_Part_Data)
ELM_PART_IMPLEMENT_TEXT_SET(elm_layout, ELM_LAYOUT, Elm_Layout_Smart_Data, Elm_Part_Data)
ELM_PART_IMPLEMENT_TEXT_GET(elm_layout, ELM_LAYOUT, Elm_Layout_Smart_Data, Elm_Part_Data)
#include "elm_layout_internal_part.eo.c"

/* Efl.Part end */


/* Internal EO APIs and hidden overrides */

EFL_FUNC_BODY_CONST(elm_layout_text_aliases_get, const Elm_Layout_Part_Alias_Description *, NULL)
EFL_FUNC_BODY_CONST(elm_layout_content_aliases_get, const Elm_Layout_Part_Alias_Description *, NULL)

ELM_LAYOUT_CONTENT_ALIASES_IMPLEMENT()
ELM_LAYOUT_TEXT_ALIASES_IMPLEMENT()

#define ELM_LAYOUT_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(elm_layout), \
   ELM_PART_CONTENT_DEFAULT_OPS(elm_layout), \
   ELM_PART_TEXT_DEFAULT_OPS(elm_layout), \
   ELM_LAYOUT_CONTENT_ALIASES_OPS(), \
   ELM_LAYOUT_TEXT_ALIASES_OPS(), \
   EFL_OBJECT_OP_FUNC(efl_dbg_info_get, _elm_layout_efl_object_dbg_info_get)

#include "elm_layout.eo.c"
