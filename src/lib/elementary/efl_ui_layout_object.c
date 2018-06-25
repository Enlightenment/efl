#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_PROTECTED
#define ELM_LAYOUT_PROTECTED
#define EFL_UI_WIDGET_PART_BG_PROTECTED
#define EFL_PART_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_layout.h"
#include "elm_part_helper.h"
#include "elm_entry.eo.h"

#define MY_CLASS EFL_UI_LAYOUT_OBJECT_CLASS
#define MY_CLASS_PFX efl_ui_layout_object

#define MY_CLASS_NAME "Efl.Ui.Layout.Object"

Eo *_efl_ui_layout_pack_proxy_get(Efl_Ui_Layout_Object *obj, Edje_Part_Type type, const char *part);
static void _efl_model_properties_changed_cb(void *, const Efl_Event *);
static Eina_Bool _efl_ui_layout_part_cursor_unset(Efl_Ui_Layout_Object_Data *sd, const char *part_name);

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

static const char efl_ui_default_text[] = "efl.text";
static const char efl_ui_default_content[] = "efl.content";

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

static const char *_elm_legacy_layout_swallow_parts[] = {
   "elm.swallow.icon",
   "elm.swallow.end",
   "elm.swallow.background",
   NULL
};

static const char *_efl_ui_layout_swallow_parts[] = {
   "efl.content",
   "efl.icon",
   "efl.end",
   "efl.background",
   NULL
};


/* these are data operated by layout's class functions internally, and
 * should not be messed up by inhering classes */
typedef struct _Efl_Ui_Layout_Sub_Object_Data   Efl_Ui_Layout_Sub_Object_Data;
typedef struct _Efl_Ui_Layout_Sub_Object_Cursor Efl_Ui_Layout_Sub_Object_Cursor;
typedef struct _Efl_Ui_Layout_Sub_Iterator      Efl_Ui_Layout_Sub_Iterator;

struct _Efl_Ui_Layout_Sub_Iterator
{
   Eina_Iterator  iterator;
   Eina_Iterator *real_iterator;
   Efl_Ui_Layout_Object *object;
};

struct _Efl_Ui_Layout_Sub_Object_Data
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

struct _Efl_Ui_Layout_Sub_Object_Cursor
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
   elm_layout_sizing_eval(data);
}

static void
_part_cursor_free(Efl_Ui_Layout_Sub_Object_Cursor *pc)
{
   eina_stringshare_del(pc->part);
   eina_stringshare_del(pc->style);
   eina_stringshare_del(pc->cursor);

   free(pc);
}

static void
_sizing_eval(Evas_Object *obj, Efl_Ui_Layout_Object_Data *sd)
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
_signals_emit(Eo *obj,
              const char *type,
              Eina_Bool set)
{
   char buf[1024];

   if (elm_widget_is_legacy(obj))
     {
        snprintf(buf, sizeof(buf), "elm,state,%s,%s", type,
                 set ? "visible" : "hidden");
        efl_layout_signal_emit(obj, buf, "elm");
     }
   else
     {
        snprintf(buf, sizeof(buf), "efl,state,%s,%s", type,
                 set ? "set" : "unset");
        efl_layout_signal_emit(obj, buf, "efl");
     }
}

static inline void
_icon_signal_emit(Efl_Ui_Layout_Object_Data *sd,
                  Efl_Ui_Layout_Sub_Object_Data *sub_d,
                  Eina_Bool visible)
{
   const char *type;
   Eo *edje;
   int i;

   edje = elm_widget_resize_object_get(sd->obj);
   if (!edje) return;

   //FIXME: Don't limit to the icon and end here.
   // send signals for all contents after elm 2.0
   if (sub_d->type != SWALLOW) return;
   if (elm_widget_is_legacy(sd->obj))
     {
        for (i = 0;; i++)
          {
              if (!_elm_legacy_layout_swallow_parts[i]) return;
              if (!strcmp(sub_d->part, _elm_legacy_layout_swallow_parts[i])) break;
          }
     }
   else
     {
        for (i = 0;; i++)
          {
              if (!_efl_ui_layout_swallow_parts[i]) return;
              if (!strcmp(sub_d->part, _efl_ui_layout_swallow_parts[i])) break;
          }
     }

   if (elm_widget_is_legacy(sd->obj))
     {
        if (!strncmp(sub_d->part, "elm.swallow.", strlen("elm.swallow.")))
          type = sub_d->part + strlen("elm.swallow.");
        else
          type = sub_d->part;
     }
   else
     {
        if (!strncmp(sub_d->part, "efl.", strlen("efl.")))
          type = sub_d->part + strlen("efl.");
        else
          type = sub_d->part;
     }

   _signals_emit(sd->obj, type, visible);

   /* themes might need immediate action here */
   efl_layout_signal_process(sd->obj, EINA_FALSE);
}

static inline void
_text_signal_emit(Efl_Ui_Layout_Object_Data *sd,
                  Efl_Ui_Layout_Sub_Object_Data *sub_d,
                  Eina_Bool visible)
{
   char buf[1024];
   const char *type;

   //FIXME: Don't limit to "elm.text" prefix.
   //Send signals for all text parts after elm 2.0
   if (sub_d->type != TEXT) return;

   if (elm_widget_is_legacy(sd->obj))
     {
        if (!((!strcmp("elm.text", sub_d->part)) ||
              (!strncmp("elm.text.", sub_d->part, 9))))
          return;
     }
   else
     {
        if (!((!strcmp("efl.text", sub_d->part)) ||
              (!strncmp("efl.text.", sub_d->part, 9))))
          return;
     }

   ELM_WIDGET_DATA_GET_OR_RETURN(sd->obj, wd);

   if (elm_widget_is_legacy(sd->obj))
     {
        if (!strncmp(sub_d->part, "elm.text.", strlen("elm.text.")))
          type = sub_d->part + strlen("elm.text.");
        else
          type = sub_d->part;
     }
   else
     {
        if (!strncmp(sub_d->part, "efl.", strlen("efl.")))
          type = sub_d->part + strlen("efl.");
        else
          type = sub_d->part;
     }

   _signals_emit(sd->obj, type, visible);

   /* TODO: is this right? It was like that, but IMO it should be removed: */

   if (elm_widget_is_legacy(sd->obj))
     {
        snprintf(buf, sizeof(buf),
                 visible ? "elm,state,text,visible" : "elm,state,text,hidden");
        efl_layout_signal_emit(sd->obj, buf, "elm");
     }

   /* themes might need immediate action here */
   efl_layout_signal_process(sd->obj, EINA_FALSE);
}

static void
_parts_signals_emit(Efl_Ui_Layout_Object_Data *sd)
{
   const Eina_List *l;
   Efl_Ui_Layout_Sub_Object_Data *sub_d;

   EINA_LIST_FOREACH(sd->subs, l, sub_d)
     {
        _icon_signal_emit(sd, sub_d, EINA_TRUE);
        _text_signal_emit(sd, sub_d, EINA_TRUE);
     }
}

static void
_part_cursor_part_apply(const Efl_Ui_Layout_Sub_Object_Cursor *pc)
{
   elm_object_cursor_set(pc->obj, pc->cursor);
   elm_object_cursor_style_set(pc->obj, pc->style);
   elm_object_cursor_theme_search_enabled_set(pc->obj, !pc->engine_only);
}

static void
_parts_cursors_apply(Efl_Ui_Layout_Object_Data *sd)
{
   const Eina_List *l;
   const char *file, *group;
   Efl_Ui_Layout_Sub_Object_Cursor *pc;
   ELM_WIDGET_DATA_GET_OR_RETURN(sd->obj, wd);

   edje_object_file_get(wd->resize_obj, &file, &group);

   EINA_LIST_FOREACH(sd->parts_cursors, l, pc)
     {
        Evas_Object *obj;

        edje_object_freeze(wd->resize_obj);
        obj = (Evas_Object *)edje_object_part_object_get
          (wd->resize_obj, pc->part);
        edje_object_thaw(wd->resize_obj);

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
_efl_ui_layout_highlight_in_theme(Evas_Object *obj)
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
                 Efl_Ui_Layout_Object_Data *sd)
{
   Eina_Bool ret = EINA_FALSE;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   _parts_signals_emit(sd);
   _parts_cursors_apply(sd);

   edje_object_mirrored_set
     (wd->resize_obj, efl_ui_mirrored_get(obj));

   edje_object_scale_set
     (wd->resize_obj,
     efl_gfx_entity_scale_get(obj) * elm_config_scale_get());

   _efl_ui_layout_highlight_in_theme(obj);

   ret = efl_ui_widget_on_disabled_update(obj, elm_widget_disabled_get(obj));

   elm_layout_sizing_eval(obj);

   return ret;
}

EOLIAN static Eina_Bool
_efl_ui_layout_object_efl_ui_widget_on_disabled_update(Eo *obj, Efl_Ui_Layout_Object_Data *_pd EINA_UNUSED, Eina_Bool disabled)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   // Not calling efl_super here: Elm.Widget simply returns false.

   if (elm_widget_is_legacy(obj))
     {
        if (disabled)
          edje_object_signal_emit
             (wd->resize_obj, "elm,state,disabled", "elm");
        else
          edje_object_signal_emit
             (wd->resize_obj, "elm,state,enabled", "elm");
     }
   else
     {
        if (disabled)
          edje_object_signal_emit
             (wd->resize_obj, "efl,state,disabled", "efl");
        else
          edje_object_signal_emit
             (wd->resize_obj, "efl,state,enabled", "efl");
     }

   return EINA_TRUE;
}

static Efl_Ui_Theme_Apply
_efl_ui_layout_theme_internal(Eo *obj, Efl_Ui_Layout_Object_Data *sd)
{
   Efl_Ui_Theme_Apply ret = EFL_UI_THEME_APPLY_FAILED;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EFL_UI_THEME_APPLY_FAILED);

   /* function already prints error messages, if any */
   if (!sd->file_set)
     {
        ret = elm_widget_theme_object_set
                (obj, wd->resize_obj,
                elm_widget_theme_klass_get(obj), 
                elm_widget_theme_element_get(obj),
                elm_widget_theme_style_get(obj));
     }

   if (ret)
     efl_event_callback_legacy_call(obj, EFL_UI_LAYOUT_OBJECT_EVENT_THEME_CHANGED, NULL);

   if (!_visuals_refresh(obj, sd))
     ret = EFL_UI_THEME_APPLY_FAILED;

   return ret;
}

EOLIAN static Efl_Ui_Theme_Apply
_efl_ui_layout_object_efl_ui_widget_theme_apply(Eo *obj, Efl_Ui_Layout_Object_Data *sd)
{
   Efl_Ui_Theme_Apply theme_apply = EFL_UI_THEME_APPLY_FAILED;

   theme_apply = efl_ui_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (!theme_apply) return EFL_UI_THEME_APPLY_FAILED;

   theme_apply &= _efl_ui_layout_theme_internal(obj, sd);
   return theme_apply;
}

EOLIAN static Eina_Bool
_efl_ui_layout_object_efl_ui_focus_object_on_focus_update(Eo *obj, Efl_Ui_Layout_Object_Data *_pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   if (!elm_widget_can_focus_get(obj)) return EINA_FALSE;

   if (efl_ui_focus_object_focus_get(obj))
     {
        if (elm_widget_is_legacy(obj))
          elm_layout_signal_emit(obj, "elm,action,focus", "elm");
        else
          elm_layout_signal_emit(obj, "efl,action,focus", "efl");
        evas_object_focus_set(wd->resize_obj, EINA_TRUE);
     }
   else
     {
        if (elm_widget_is_legacy(obj))
          elm_layout_signal_emit(obj, "elm,action,unfocus", "elm");
        else
          elm_layout_signal_emit(obj, "efl,action,unfocus", "efl");
        evas_object_focus_set(wd->resize_obj, EINA_FALSE);
     }

   efl_ui_focus_object_on_focus_update(efl_super(obj, MY_CLASS));

   if (efl_isa(wd->resize_obj, EFL_CANVAS_LAYOUT_CLASS))
     edje_object_message_signal_process(wd->resize_obj);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_layout_object_efl_ui_widget_widget_sub_object_add(Eo *obj, Efl_Ui_Layout_Object_Data *_pd EINA_UNUSED, Evas_Object *sobj)
{
   Eina_Bool int_ret = EINA_FALSE;

   if (evas_object_data_get(sobj, "elm-parent") == obj) return EINA_TRUE;

   int_ret = elm_widget_sub_object_add(efl_super(obj, MY_CLASS), sobj);
   if (!int_ret) return EINA_FALSE;

   evas_object_event_callback_add
         (sobj, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
          _on_sub_object_size_hint_change, obj);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_layout_object_efl_ui_widget_widget_sub_object_del(Eo *obj, Efl_Ui_Layout_Object_Data *sd, Evas_Object *sobj)
{
   Eina_List *l;
   Efl_Ui_Layout_Sub_Object_Data *sub_d;

   Eina_Bool int_ret = EINA_FALSE;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   evas_object_event_callback_del_full
     (sobj, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
     _on_sub_object_size_hint_change, obj);

   int_ret = elm_widget_sub_object_del(efl_super(obj, MY_CLASS), sobj);
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

   // No need to resize object during destruction
   if (!efl_invalidated_get(obj))
     elm_layout_sizing_eval(obj);

   return EINA_TRUE;
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

   if (!elm_widget_is_legacy(obj))
     {
        if (!*part)
          {
             if (is_text)
               *part = efl_ui_default_text;
             else
               *part = efl_ui_default_content;
          }
        return EINA_TRUE;
     }

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
   Efl_Ui_Layout_Sub_Object_Data *sub_d = data;
   sub_d->p.box.reference = NULL;
}

static Evas_Object *
_sub_box_remove(Evas_Object *obj,
                Efl_Ui_Layout_Object_Data *sd,
                Efl_Ui_Layout_Sub_Object_Data *sub_d)
{
   Evas_Object *child = sub_d->obj; /* sub_d will die in
                                     * _efl_ui_layout_smart_widget_sub_object_del */

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
_sub_box_is(const Efl_Ui_Layout_Sub_Object_Data *sub_d)
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
                  Efl_Ui_Layout_Object_Data *sd,
                  Efl_Ui_Layout_Sub_Object_Data *sub_d)
{
   Evas_Object *child;
   ELM_WIDGET_DATA_GET_OR_RETURN(sd->obj, wd, NULL);

   child = sub_d->obj; /* sub_d will die in _efl_ui_layout_smart_widget_sub_object_del */

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
   elm_layout_sizing_eval(data);
}

EOLIAN static void
_efl_ui_layout_efl_canvas_group_group_add(Eo *obj, Efl_Ui_Layout_Object_Data *_pd EINA_UNUSED)
{
   Evas_Object *edje;

   elm_widget_sub_object_parent_add(obj);

   /* has to be there *before* parent's smart_add() */
   edje = edje_object_add(evas_object_evas_get(obj));
   elm_widget_resize_object_set(obj, edje);

   efl_canvas_group_add(efl_super(obj, MY_CLASS));

   elm_widget_can_focus_set(obj, EINA_FALSE);

   if (elm_widget_is_legacy(obj))
     edje_object_signal_callback_add
        (edje, "size,eval", "elm", _on_size_evaluate_signal, obj);
   else
     edje_object_signal_callback_add
        (edje, "size,eval", "efl", _on_size_evaluate_signal, obj);

   elm_layout_sizing_eval(obj);
}

EOLIAN static void
_efl_ui_layout_efl_canvas_group_group_del(Eo *obj, Efl_Ui_Layout_Object_Data *sd)
{
   Efl_Ui_Layout_Sub_Object_Data *sub_d;
   Efl_Ui_Layout_Sub_Object_Cursor *pc;
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

   if(sd->connect.model)
     {
         efl_event_callback_del(sd->connect.model, EFL_MODEL_EVENT_PROPERTIES_CHANGED,
                                _efl_model_properties_changed_cb, sd);
         efl_unref(sd->connect.model);
         sd->connect.model = NULL;
     }

   eina_hash_free(sd->connect.properties);
   sd->connect.properties = NULL;
   eina_hash_free(sd->connect.signals);
   sd->connect.signals = NULL;
   eina_hash_free(sd->connect.factories);
   sd->connect.factories = NULL;

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
_efl_ui_layout_object_efl_canvas_group_group_calculate(Eo *obj, Efl_Ui_Layout_Object_Data *sd)
{
   if (sd->needs_size_calc)
     {
        _sizing_eval(obj, sd);
        sd->needs_size_calc = EINA_FALSE;
     }
}

static Efl_Ui_Layout_Sub_Object_Cursor *
_parts_cursors_find(Efl_Ui_Layout_Object_Data *sd,
                    const char *part)
{
   const Eina_List *l;
   Efl_Ui_Layout_Sub_Object_Cursor *pc;

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
_efl_ui_layout_object_efl_file_file_set(Eo *obj, Efl_Ui_Layout_Object_Data *sd, const char *file, const char *group)
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
_efl_ui_layout_object_efl_file_file_get(const Eo *obj, Efl_Ui_Layout_Object_Data *sd EINA_UNUSED, const char **file, const char **group)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   edje_object_file_get(wd->resize_obj, file, group);
}


EOLIAN static Eina_Bool
_efl_ui_layout_object_efl_file_mmap_set(Eo *obj, Efl_Ui_Layout_Object_Data *sd, const Eina_File *file, const char *group)
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
_efl_ui_layout_object_efl_file_mmap_get(const Eo *obj, Efl_Ui_Layout_Object_Data *sd EINA_UNUSED, const Eina_File **file, const char **group)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   efl_file_mmap_get(wd->resize_obj, file, group);
}

EOLIAN static Efl_Ui_Theme_Apply
_efl_ui_layout_object_theme_set(Eo *obj, Efl_Ui_Layout_Object_Data *sd, const char *klass, const char *group, const char *style)
{
   Eina_Bool changed = EINA_FALSE;

   if (!elm_widget_is_legacy(obj) && efl_finalized_get(obj))
     {
        ERR("Efl.Ui.Layout.theme can only be set before finalize!");
        return EFL_UI_THEME_APPLY_FAILED;
     }

   if (sd->file_set) sd->file_set = EINA_FALSE;

   changed |= elm_widget_theme_klass_set(obj, klass);
   changed |= elm_widget_theme_element_set(obj, group);
   changed |= elm_widget_theme_style_set(obj, style);

   if (changed)
     return efl_ui_widget_theme_apply(obj);
   return EFL_UI_THEME_APPLY_SUCCESS;
}

EOLIAN static void
_efl_ui_layout_object_efl_layout_signal_signal_emit(Eo *obj, Efl_Ui_Layout_Object_Data *_pd EINA_UNUSED, const char *emission, const char *source)
{
   // Don't do anything else than call forward here
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   efl_layout_signal_emit(wd->resize_obj, emission, source);
}

EOLIAN static Eina_Bool
_efl_ui_layout_object_efl_layout_signal_signal_callback_add(Eo *obj, Efl_Ui_Layout_Object_Data *_pd EINA_UNUSED, const char *emission, const char *source, Efl_Signal_Cb func, void *data)
{
   // Don't do anything else than call forward here
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);
   return efl_layout_signal_callback_add(wd->resize_obj, emission, source, func, data);
}

EOLIAN static Eina_Bool
_efl_ui_layout_object_efl_layout_signal_signal_callback_del(Eo *obj, Efl_Ui_Layout_Object_Data *_pd EINA_UNUSED, const char *emission, const char *source, Edje_Signal_Cb func, void *data)
{
   // Don't do anything else than call forward here
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);
   return efl_layout_signal_callback_add(wd->resize_obj, emission, source, func, data);
}

// TODO:
// - message_send
// - message_signal_process
// and also message handler (not implemented yet as an EO interface!)

EAPI Eina_Bool
elm_layout_content_set(Evas_Object *obj, const char *swallow, Evas_Object *content)
{
   EFL_UI_LAYOUT_CHECK(obj) EINA_FALSE;
   if (!swallow)
     {
        swallow = efl_ui_widget_default_content_part_get(obj);
        if (!swallow) return EINA_FALSE;
     }
   else if (!_elm_layout_part_aliasing_eval(obj, &swallow, EINA_FALSE))
     return EINA_FALSE;

   return efl_content_set(efl_part(obj, swallow), content);
}

static Eina_Bool
_efl_ui_layout_content_set(Eo *obj, Efl_Ui_Layout_Object_Data *sd, const char *part, Evas_Object *content)
{
   Efl_Ui_Layout_Sub_Object_Data *sub_d;
   Eina_List *l;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

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
                  sd->subs = eina_list_remove_list(sd->subs, l);
                  _icon_signal_emit(sd, sub_d, EINA_FALSE);
                  eina_stringshare_del(sub_d->part);
                  free(sub_d);

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
        sub_d = ELM_NEW(Efl_Ui_Layout_Sub_Object_Data);
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

   elm_layout_sizing_eval(obj);

end:
   return EINA_TRUE;
}

EAPI Evas_Object *
elm_layout_content_get(const Evas_Object *obj, const char *swallow)
{
   EFL_UI_LAYOUT_CHECK(obj) NULL;
   // If the object is already dead, their shouldn't be any part in it
   if (efl_invalidated_get(obj)) return NULL;
   if (!swallow)
     {
        swallow = efl_ui_widget_default_content_part_get(obj);
        if (!swallow) return NULL;
     }
   else if (!_elm_layout_part_aliasing_eval(obj, &swallow, EINA_FALSE))
     return NULL;

   return efl_content_get(efl_part(obj, swallow));
}

static Evas_Object*
_efl_ui_layout_content_get(Eo *obj, Efl_Ui_Layout_Object_Data *sd, const char *part)
{
   const Eina_List *l;
   Efl_Ui_Layout_Sub_Object_Data *sub_d;

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
elm_layout_content_unset(Evas_Object *obj, const char *swallow)
{
   EFL_UI_LAYOUT_CHECK(obj) NULL;
   if (!swallow)
     {
        swallow = efl_ui_widget_default_content_part_get(obj);
        if (!swallow) return NULL;
     }
   else if (!_elm_layout_part_aliasing_eval(obj, &swallow, EINA_FALSE))
     return NULL;

   return efl_content_unset(efl_part(obj, swallow));
}

static Evas_Object*
_efl_ui_layout_content_unset(Eo *obj, Efl_Ui_Layout_Object_Data *sd, const char *part)
{
   Efl_Ui_Layout_Sub_Object_Data *sub_d;
   Eina_List *l;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   EINA_LIST_FOREACH(sd->subs, l, sub_d)
     {
        if ((sub_d->type == SWALLOW) && (!strcmp(part, sub_d->part)))
          {
             Evas_Object *content;

             if (!sub_d->obj) return NULL;

             content = sub_d->obj; /* sub_d will die in
                                    * _efl_ui_layout_smart_widget_sub_object_del */

             if (!_elm_widget_sub_object_redirect_to_top(obj, content))
               {
                  ERR("could not remove sub object %p from %p", content, obj);
                  return NULL;
               }

             edje_object_part_unswallow
               (wd->resize_obj, content);
             EINA_LIST_FOREACH(sd->subs, l, sub_d)
               {
                  if (sub_d->obj == content)
                    {
                       sd->subs = eina_list_remove_list(sd->subs, l);
                       _icon_signal_emit(sd, sub_d, EINA_FALSE);
                       eina_stringshare_del(sub_d->part);
                       free(sub_d);
                       break;
                    }
               }
             _eo_unparent_helper(content, obj);
             return content;
          }
     }

   return NULL;
}

EOLIAN static Eina_Bool
_efl_ui_layout_object_efl_container_content_remove(Eo *obj, Efl_Ui_Layout_Object_Data *sd EINA_UNUSED,
                                         Efl_Gfx_Entity *content)
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
   EFL_UI_LAYOUT_CHECK(obj) NULL;
   Eina_List *ret = NULL;
   Efl_Ui_Layout_Sub_Object_Data *sub_d = NULL;
   Efl_Ui_Layout_Object_Data *sd = efl_data_scope_get(obj, MY_CLASS);
   Eina_List *l = NULL;

   EINA_LIST_FOREACH(sd->subs, l, sub_d)
     {
        if (sub_d->type == SWALLOW)
          ret = eina_list_append(ret, sub_d->obj);
     }

   return ret;
}

static Eina_Bool
_sub_iterator_next(Efl_Ui_Layout_Sub_Iterator *it, void **data)
{
   Efl_Ui_Layout_Sub_Object_Data *sub;

   if (!eina_iterator_next(it->real_iterator, (void **)&sub))
     return EINA_FALSE;

   if (data) *data = sub->obj;
   return EINA_TRUE;
}

static Efl_Ui_Layout_Object *
_sub_iterator_get_container(Efl_Ui_Layout_Sub_Iterator *it)
{
   return it->object;
}

static void
_sub_iterator_free(Efl_Ui_Layout_Sub_Iterator *it)
{
   eina_iterator_free(it->real_iterator);
   free(it);
}

static Eina_Iterator *
_sub_iterator_create(Eo *eo_obj, Efl_Ui_Layout_Object_Data *sd)
{
   Efl_Ui_Layout_Sub_Iterator *it;

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
_efl_ui_layout_object_efl_container_content_iterate(Eo *eo_obj EINA_UNUSED, Efl_Ui_Layout_Object_Data *sd)
{
   return _sub_iterator_create(eo_obj, sd);
}

EOLIAN static int
_efl_ui_layout_object_efl_container_content_count(Eo *eo_obj EINA_UNUSED, Efl_Ui_Layout_Object_Data *sd)
{
   return eina_list_count(sd->subs);
}

static Eina_Bool
_efl_ui_layout_text_generic_set(Eo *obj, Efl_Ui_Layout_Object_Data *sd, const char *part, const char *text, Eina_Bool is_markup)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   Efl_Ui_Layout_Sub_Object_Data *sub_d = NULL;
   Eina_List *l;

   EINA_LIST_FOREACH(sd->subs, l, sub_d)
     {
        if ((sub_d->type == TEXT) && (!strcmp(part, sub_d->part)))
          {
             if (!text)
               {
                  _text_signal_emit(sd, sub_d, EINA_FALSE);
                  eina_stringshare_del(sub_d->part);
                  free(sub_d);
                  edje_object_part_text_escaped_set
                    (wd->resize_obj, part, NULL);
                  sd->subs = eina_list_remove_list(sd->subs, l);
                  elm_layout_sizing_eval(obj);
                  return EINA_TRUE;
               }
             else
               break;
          }
     }

   if (!text) return EINA_TRUE;

   if (elm_widget_is_legacy(obj))
     {
        if (!edje_object_part_text_escaped_set
         (wd->resize_obj, part, text))
           return EINA_FALSE;
     }
   else if (is_markup)
     {
        efl_text_markup_set(efl_part(wd->resize_obj, part), text);
     }
   else
     {
        efl_text_set(efl_part(wd->resize_obj, part), text);
     }

   if (!sub_d)
     {
        sub_d = ELM_NEW(Efl_Ui_Layout_Sub_Object_Data);
        if (!sub_d) return EINA_FALSE;
        sub_d->type = TEXT;
        sub_d->part = eina_stringshare_add(part);
        sd->subs = eina_list_append(sd->subs, sub_d);
     }

   _text_signal_emit(sd, sub_d, EINA_TRUE);

   elm_layout_sizing_eval(obj);

   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON &&
       sd->can_access && !(sub_d->obj))
     sub_d->obj = _elm_access_edje_object_part_object_register
         (obj, elm_layout_edje_get(obj), part);

   if (sd->connect.model && !sd->connect.updating)
     {
        char *property = eina_hash_find(sd->connect.properties, sub_d->part);

        if (property)
          {
             Eina_Value v = EINA_VALUE_EMPTY;

             eina_value_setup(&v, EINA_VALUE_TYPE_STRING);
             eina_value_set(&v, text);

             efl_model_property_set(sd->connect.model, property, &v);
          }
     }

   return EINA_TRUE;
}

static Eina_Bool
_efl_ui_layout_text_set(Eo *obj, Efl_Ui_Layout_Object_Data *sd, const char *part, const char *text)
{
   return _efl_ui_layout_text_generic_set(obj, sd, part, text, EINA_FALSE);
}

static const char*
_efl_ui_layout_text_get(Eo *obj, Efl_Ui_Layout_Object_Data *sd EINA_UNUSED, const char *part)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   if (elm_widget_is_legacy(obj))
     {
        return edje_object_part_text_get(wd->resize_obj, part);
     }
   return efl_text_get(efl_part(wd->resize_obj, part));
}

static const char*
_efl_ui_layout_text_markup_get(Eo *obj, Efl_Ui_Layout_Object_Data *sd EINA_UNUSED, const char *part)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   return efl_text_markup_get(efl_part(wd->resize_obj, part));
}

static Eina_Bool
_efl_ui_layout_text_markup_set(Eo *obj, Efl_Ui_Layout_Object_Data *sd, const char *part, const char *text)
{
   return _efl_ui_layout_text_generic_set(obj, sd, part, text, EINA_TRUE);
}

static void
_layout_box_subobj_init(Efl_Ui_Layout_Object_Data *sd, Efl_Ui_Layout_Sub_Object_Data *sub_d, const char *part, Evas_Object *child)
{
   sub_d->part = eina_stringshare_add(part);
   sub_d->obj = child;
   sd->subs = eina_list_append(sd->subs, sub_d);
   efl_parent_set(child, sd->obj);
}

Eina_Bool
_efl_ui_layout_box_append(Eo *obj, Efl_Ui_Layout_Object_Data *sd, const char *part, Evas_Object *child)
{
   Efl_Ui_Layout_Sub_Object_Data *sub_d;

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

   sub_d = ELM_NEW(Efl_Ui_Layout_Sub_Object_Data);
   if (!sub_d)
     {
        ERR("failed to allocate memory!");
        _elm_widget_sub_object_redirect_to_top(obj, child);
        edje_object_part_box_remove(wd->resize_obj, part, child);
        return EINA_FALSE;
     }
   sub_d->type = BOX_APPEND;
   _layout_box_subobj_init(sd, sub_d, part, child);

   elm_layout_sizing_eval(obj);

   return EINA_TRUE;
}

Eina_Bool
_efl_ui_layout_box_prepend(Eo *obj, Efl_Ui_Layout_Object_Data *sd, const char *part, Evas_Object *child)
{
   Efl_Ui_Layout_Sub_Object_Data *sub_d;

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

   sub_d = ELM_NEW(Efl_Ui_Layout_Sub_Object_Data);
   if (!sub_d)
     {
        ERR("failed to allocate memory!");
        _elm_widget_sub_object_redirect_to_top(obj, child);
        edje_object_part_box_remove(wd->resize_obj, part, child);
        return EINA_FALSE;
     }
   sub_d->type = BOX_PREPEND;
   _layout_box_subobj_init(sd, sub_d, part, child);

   elm_layout_sizing_eval(obj);

   return EINA_TRUE;
}

Eina_Bool
_efl_ui_layout_box_insert_before(Eo *obj, Efl_Ui_Layout_Object_Data *sd, const char *part, Evas_Object *child, const Evas_Object *reference)
{
   Efl_Ui_Layout_Sub_Object_Data *sub_d;

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

   sub_d = ELM_NEW(Efl_Ui_Layout_Sub_Object_Data);
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

   elm_layout_sizing_eval(obj);

   return EINA_TRUE;
}

Eina_Bool
_efl_ui_layout_box_insert_at(Eo *obj, Efl_Ui_Layout_Object_Data *sd, const char *part, Evas_Object *child, unsigned int pos)
{
   Efl_Ui_Layout_Sub_Object_Data *sub_d;
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

   sub_d = ELM_NEW(Efl_Ui_Layout_Sub_Object_Data);
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

   elm_layout_sizing_eval(obj);

   return EINA_TRUE;
}

Evas_Object *
_efl_ui_layout_box_remove(Eo *obj, Efl_Ui_Layout_Object_Data *sd, const char *part, Evas_Object *child)
{

   EINA_SAFETY_ON_NULL_RETURN_VAL(part, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(child, NULL);


   const Eina_List *l;
   Efl_Ui_Layout_Sub_Object_Data *sub_d;

   EINA_LIST_FOREACH(sd->subs, l, sub_d)
     {
        if (!_sub_box_is(sub_d)) continue;
        if ((sub_d->obj == child) && (!strcmp(sub_d->part, part)))
           return _sub_box_remove(obj, sd, sub_d);
     }

   return NULL;
}

Eina_Bool
_efl_ui_layout_box_remove_all(Eo *obj, Efl_Ui_Layout_Object_Data *sd, const char *part, Eina_Bool clear)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(part, EINA_FALSE);

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   Efl_Ui_Layout_Sub_Object_Data *sub_d;
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
_efl_ui_layout_table_pack(Eo *obj, Efl_Ui_Layout_Object_Data *sd, const char *part, Evas_Object *child, unsigned short col, unsigned short row, unsigned short colspan, unsigned short rowspan)
{
   Efl_Ui_Layout_Sub_Object_Data *sub_d;

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

   sub_d = ELM_NEW(Efl_Ui_Layout_Sub_Object_Data);
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

   elm_layout_sizing_eval(obj);

   return EINA_TRUE;
}

Evas_Object *
_efl_ui_layout_table_unpack(Eo *obj, Efl_Ui_Layout_Object_Data *sd, const char *part, Evas_Object *child)
{

   EINA_SAFETY_ON_NULL_RETURN_VAL(part, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(child, NULL);

   const Eina_List *l;
   Efl_Ui_Layout_Sub_Object_Data *sub_d;

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
_efl_ui_layout_table_clear(Eo *obj, Efl_Ui_Layout_Object_Data *sd, const char *part, Eina_Bool clear)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(part, EINA_FALSE);

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   Efl_Ui_Layout_Sub_Object_Data *sub_d;
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

EOLIAN static const char *
_efl_ui_layout_object_efl_layout_group_group_data_get(const Eo *obj, Efl_Ui_Layout_Object_Data *_pd EINA_UNUSED, const char *key)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   return efl_layout_group_data_get(wd->resize_obj, key);
}

EOLIAN static Eina_Size2D
_efl_ui_layout_object_efl_layout_group_group_size_min_get(const Eo *obj, Efl_Ui_Layout_Object_Data *_pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_SIZE2D(0, 0));

   return efl_layout_group_size_min_get(wd->resize_obj);
}

EOLIAN static Eina_Size2D
_efl_ui_layout_object_efl_layout_group_group_size_max_get(const Eo *obj, Efl_Ui_Layout_Object_Data *_pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_SIZE2D(0, 0));

   return efl_layout_group_size_max_get(wd->resize_obj);
}

EOLIAN static Eina_Bool
_efl_ui_layout_object_efl_layout_group_part_exist_get(const Eo *obj, Efl_Ui_Layout_Object_Data *_pd EINA_UNUSED, const char *part)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   return efl_layout_group_part_exist_get(wd->resize_obj, part);
}

/* layout's sizing evaluation is deferred. evaluation requests are
 * queued up and only flag the object as 'changed'. when it comes to
 * Evas's rendering phase, it will be addressed, finally (see
 * _efl_ui_layout_smart_calculate()). */
static void
_elm_layout_sizing_eval(Eo *obj, Efl_Ui_Layout_Object_Data *sd)
{
   if (sd->frozen) return;
   if (sd->needs_size_calc) return;
   sd->needs_size_calc = EINA_TRUE;

   evas_object_smart_changed(obj);
}

EAPI void
elm_layout_sizing_restricted_eval(Eo *obj, Eina_Bool w, Eina_Bool h)
{
   Efl_Ui_Layout_Object_Data *sd = efl_data_scope_safe_get(obj, MY_CLASS);

   if (!sd) return;
   sd->restricted_calc_w = !!w;
   sd->restricted_calc_h = !!h;

   evas_object_smart_changed(obj);
}

EOLIAN static int
_efl_ui_layout_object_efl_layout_calc_calc_freeze(Eo *obj, Efl_Ui_Layout_Object_Data *sd)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, 0);

   if ((sd->frozen)++ != 0) return sd->frozen;

   edje_object_freeze(wd->resize_obj);

   return 1;
}

EOLIAN static int
_efl_ui_layout_object_efl_layout_calc_calc_thaw(Eo *obj, Efl_Ui_Layout_Object_Data *sd)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, 0);

   if (--(sd->frozen) != 0) return sd->frozen;

   edje_object_thaw(wd->resize_obj);

   elm_layout_sizing_eval(obj);

   return 0;
}

static Eina_Bool
_efl_ui_layout_part_cursor_set(Efl_Ui_Layout_Object_Data *sd, const char *part_name, const char *cursor)
{
   Evas_Object *part_obj;
   Efl_Ui_Layout_Sub_Object_Cursor *pc;
   Eo *obj = sd->obj;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(part_name, EINA_FALSE);

   if (!cursor) return _efl_ui_layout_part_cursor_unset(sd, part_name);

   edje_object_freeze(wd->resize_obj);
   part_obj = (Evas_Object *)edje_object_part_object_get
       (wd->resize_obj, part_name);
   edje_object_thaw(wd->resize_obj);

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
_efl_ui_layout_part_cursor_get(Efl_Ui_Layout_Object_Data *sd, const char *part_name)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(part_name, NULL);

   Efl_Ui_Layout_Sub_Object_Cursor *pc = _parts_cursors_find(sd, part_name);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pc, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pc->obj, NULL);

   return elm_object_cursor_get(pc->obj);
}

static Eina_Bool
_efl_ui_layout_part_cursor_unset(Efl_Ui_Layout_Object_Data *sd, const char *part_name)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(part_name, EINA_FALSE);

   Eina_List *l;
   Efl_Ui_Layout_Sub_Object_Cursor *pc;

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
_efl_ui_layout_part_cursor_style_set(Efl_Ui_Layout_Object_Data *sd, const char *part_name, const char *style)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(part_name, EINA_FALSE);

   Efl_Ui_Layout_Sub_Object_Cursor *pc = _parts_cursors_find(sd, part_name);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pc, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pc->obj, EINA_FALSE);

   eina_stringshare_replace(&pc->style, style);
   elm_object_cursor_style_set(pc->obj, pc->style);

   return EINA_TRUE;
}

static const char*
_efl_ui_layout_part_cursor_style_get(Efl_Ui_Layout_Object_Data *sd, const char *part_name)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(part_name, NULL);

   Efl_Ui_Layout_Sub_Object_Cursor *pc = _parts_cursors_find(sd, part_name);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pc, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pc->obj, NULL);

   return elm_object_cursor_style_get(pc->obj);
}

static Eina_Bool
_efl_ui_layout_part_cursor_engine_only_set(Efl_Ui_Layout_Object_Data *sd, const char *part_name, Eina_Bool engine_only)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(part_name, EINA_FALSE);

   Efl_Ui_Layout_Sub_Object_Cursor *pc = _parts_cursors_find(sd, part_name);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pc, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pc->obj, EINA_FALSE);

   pc->engine_only = !!engine_only;
   elm_object_cursor_theme_search_enabled_set(pc->obj, !pc->engine_only);

   return EINA_TRUE;
}

static Eina_Bool
_efl_ui_layout_part_cursor_engine_only_get(Efl_Ui_Layout_Object_Data *sd, const char *part_name)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(part_name, EINA_FALSE);

   Efl_Ui_Layout_Sub_Object_Cursor *pc = _parts_cursors_find(sd, part_name);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pc, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pc->obj, EINA_FALSE);

   return !elm_object_cursor_theme_search_enabled_get(pc->obj);
}

EAPI Eina_Bool
elm_layout_edje_object_can_access_set(Eo *obj, Eina_Bool can_access)
{
   Efl_Ui_Layout_Object_Data *sd = efl_data_scope_safe_get(obj, MY_CLASS);
   EINA_SAFETY_ON_NULL_RETURN_VAL(sd, EINA_FALSE);
   sd->can_access = !!can_access;
   return EINA_TRUE;
}

EAPI Eina_Bool
elm_layout_edje_object_can_access_get(const Eo *obj)
{
   Efl_Ui_Layout_Object_Data *sd = efl_data_scope_safe_get(obj, MY_CLASS);
   EINA_SAFETY_ON_NULL_RETURN_VAL(sd, EINA_FALSE);
   return sd->can_access;
}

EOLIAN static void
_efl_ui_layout_efl_object_dbg_info_get(Eo *eo_obj, Efl_Ui_Layout_Object_Data *_pd EINA_UNUSED, Efl_Dbg_Info *root)
{
   efl_dbg_info_get(efl_super(eo_obj, MY_CLASS), root);
   ELM_WIDGET_DATA_GET_OR_RETURN(eo_obj, wd);

   if (wd->resize_obj && efl_isa(wd->resize_obj, EFL_CANVAS_LAYOUT_CLASS))
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
_efl_ui_layout_view_model_property_update(Efl_Ui_Layout_Object_Data *pd, const char *part, const char *fetch)
{
   Eina_Value *v = NULL;
   char *value = NULL;

   v = efl_model_property_get(pd->connect.model, fetch);
   if (!v) return;

   if (eina_value_type_get(v) != EINA_VALUE_TYPE_ERROR)
       value = eina_value_to_string(v);

   pd->connect.updating = EINA_TRUE; // Prevent recursive call to property_set while updating text
   efl_text_set(efl_part(pd->obj, part), value);
   pd->connect.updating = EINA_FALSE;

   eina_value_free(v);
   free(value);
}

static void
_efl_ui_layout_view_model_signal_update(Efl_Ui_Layout_Object_Data *pd, const char *signal, const char *fetch)
{
   Eina_Value *v = NULL;
   Eina_Strbuf *buf;
   char *value;

   v = efl_model_property_get(pd->connect.model, fetch);
   if (!v) return;

   if (eina_value_type_get(v) == EINA_VALUE_TYPE_ERROR)
     {
        Eina_Error error;

        eina_value_get(v, &error);
        ERR("Failed to fetch signal value. Error: %s", eina_error_msg_get(error));
        return;
     }

   // FIXME: previous implementation would just do that for signal/part == "selected"
   if (eina_value_type_get(v) == EINA_VALUE_TYPE_UCHAR)
     {
        Eina_Bool bl;

        eina_value_bool_get(v, &bl);
        if (bl) value = strdup("selected");
        else value = strdup("unselected");
     }
   else
     {
        value = eina_value_to_string(v);
     }

   buf = eina_strbuf_new();
   // FIXME: is it really the form of signal we want to send ?
   eina_strbuf_append_printf(buf, "%s%s", signal, value);
   elm_layout_signal_emit(pd->obj, eina_strbuf_string_get(buf),
                          elm_widget_is_legacy(pd->obj) ? "elm" : "efl");

   eina_strbuf_free(buf);
   eina_value_free(v);
   free(value);
}

static void
_efl_ui_layout_view_model_content_update(Efl_Ui_Layout_Object_Data *pd, Efl_Ui_Factory *factory, const char *name)
{
   Efl_Gfx_Entity *content;

   content = efl_ui_factory_create(factory, pd->connect.model, pd->obj);
   elm_layout_content_set(pd->obj, name, content);
}

static void
_efl_ui_layout_view_model_update(Efl_Ui_Layout_Object_Data *pd)
{
   Eina_Hash_Tuple *tuple;
   Eina_Iterator *it;

   if (!pd->connect.model) return ;

   it = eina_hash_iterator_tuple_new(pd->connect.properties);
   EINA_ITERATOR_FOREACH(it, tuple)
     _efl_ui_layout_view_model_property_update(pd, tuple->data, tuple->key);
   eina_iterator_free(it);

   it = eina_hash_iterator_tuple_new(pd->connect.signals);
   EINA_ITERATOR_FOREACH(it, tuple)
     _efl_ui_layout_view_model_signal_update(pd, tuple->data, tuple->key);
   eina_iterator_free(it);

   it = eina_hash_iterator_tuple_new(pd->connect.factories);
   EINA_ITERATOR_FOREACH(it, tuple)
     _efl_ui_layout_view_model_content_update(pd, tuple->data, tuple->key);
   eina_iterator_free(it);
}

static void
_efl_model_properties_changed_cb(void *data, const Efl_Event *event)
{
   Efl_Model_Property_Event *evt = event->info;
   Efl_Ui_Layout_Object_Data *pd = data;
   const char *prop;
   Eina_Array_Iterator it;
   unsigned int i;

   if (!evt->changed_properties) return ;

   EINA_ARRAY_ITER_NEXT(evt->changed_properties, i, prop, it)
     {
        Eina_Stringshare *sprop = eina_stringshare_add(prop);
        const char *part;
        const char *signal;
        Efl_Ui_Factory *factory;

        part = eina_hash_find(pd->connect.properties, sprop);
        if (part) _efl_ui_layout_view_model_property_update(pd, part, sprop);

        signal = eina_hash_find(pd->connect.signals, sprop);
        if (signal) _efl_ui_layout_view_model_signal_update(pd, signal, sprop);

        factory = eina_hash_find(pd->connect.factories, sprop);
        if (factory) _efl_ui_layout_view_model_content_update(pd, factory, sprop);

        eina_stringshare_del(sprop);
     }
}

static void
_efl_ui_layout_connect_hash(Efl_Ui_Layout_Object_Data *pd)
{
   if (pd->connect.properties) return ;

   // FIXME: fix destruction function definition
   pd->connect.properties = eina_hash_stringshared_new(NULL); // Hash of property targeting a part
   pd->connect.signals = eina_hash_stringshared_new(NULL); // Hash of property triggering a signal
   pd->connect.factories = eina_hash_stringshared_new(EINA_FREE_CB(efl_unref)); // Hash of property triggering a content creation
}

EOLIAN static void
_efl_ui_layout_object_efl_ui_view_model_set(Eo *obj, Efl_Ui_Layout_Object_Data *pd, Efl_Model *model)
{
   Eina_Stringshare *name;
   Eina_Hash_Tuple *tuple;
   Eina_Iterator *it;

   efl_replace(&pd->connect.model, model);

   if (model)
     {
        efl_event_callback_add(pd->connect.model, EFL_MODEL_EVENT_PROPERTIES_CHANGED,
                               _efl_model_properties_changed_cb, pd);
     }

   _efl_ui_layout_connect_hash(pd);

   // Reset to empty state
   it = eina_hash_iterator_key_new(pd->connect.properties);
   EINA_ITERATOR_FOREACH(it, name)
     {
        efl_text_set(efl_part(obj, name), NULL);
     }
   eina_iterator_free(it);

   it = eina_hash_iterator_tuple_new(pd->connect.factories);
   EINA_ITERATOR_FOREACH(it, tuple)
     {
        Efl_Ui_Factory *factory;
        Efl_Gfx_Entity *content;

        name = tuple->key;
        factory = tuple->data;
        content = elm_layout_content_get(obj, name);

        elm_layout_content_set(obj, name, NULL);
        efl_ui_factory_release(factory, content);
     }
   eina_iterator_free(it);

   // Refresh content if necessary
   _efl_ui_layout_view_model_update(pd);
}

EOLIAN static Efl_Model *
_efl_ui_layout_object_efl_ui_view_model_get(const Eo *obj EINA_UNUSED, Efl_Ui_Layout_Object_Data *pd)
{
   return pd->connect.model;
}

EOLIAN static void
_efl_ui_layout_object_efl_ui_model_connect_connect(Eo *obj EINA_UNUSED, Efl_Ui_Layout_Object_Data *pd, const char *name, const char *property)
{
   EINA_SAFETY_ON_NULL_RETURN(name);
   Eina_Stringshare *sprop;
   Eina_Hash *hash = NULL;
   char *data = NULL;

   if (!_elm_layout_part_aliasing_eval(obj, &name, EINA_TRUE))
     return;

   _efl_ui_layout_connect_hash(pd);

   sprop = eina_stringshare_add(property);

   // FIXME: prevent double connect of name to multiple property ?
   if (strncmp(SIGNAL_PREFIX, name, sizeof(SIGNAL_PREFIX) - 1) == 0)
     {
        hash = pd->connect.signals;
        data = strdup(name + sizeof(SIGNAL_PREFIX) - 1);
     }
   else
     {
        hash = pd->connect.properties;
        data = strdup(name);
     }

   if (!sprop)
     {
        // FIXME: remove the entry from the hash ?
     }
   else
     {
        eina_hash_add(hash, sprop, data);
     }

   // Update display right away if possible
   if (!pd->connect.model) return ;

   if (hash == pd->connect.signals)
     _efl_ui_layout_view_model_signal_update(pd, data, sprop);
   else
     _efl_ui_layout_view_model_property_update(pd, data, sprop);
}

EOLIAN static void
_efl_ui_layout_object_efl_ui_factory_model_connect(Eo *obj EINA_UNUSED, Efl_Ui_Layout_Object_Data *pd,
                const char *name, Efl_Ui_Factory *factory)
{
   EINA_SAFETY_ON_NULL_RETURN(name);
   Eina_Stringshare *ss_name;
   Efl_Ui_Factory *old_factory;
   Evas_Object *new_ev, *old_ev;

   if (!_elm_layout_part_aliasing_eval(obj, &name, EINA_TRUE))
     return;

   ss_name = eina_stringshare_add(name);

   if (!pd->connect.factories)
     pd->connect.factories = eina_hash_stringshared_new(EINA_FREE_CB(efl_unref));

   new_ev = efl_ui_factory_create(factory, pd->connect.model, obj);
   EINA_SAFETY_ON_NULL_RETURN(new_ev);

   old_factory = eina_hash_set(pd->connect.factories, ss_name, efl_ref(factory));
   if (old_factory)
     {
         old_ev = elm_layout_content_get(obj, name);
         if (old_ev)
           efl_ui_factory_release(old_factory, old_ev);
         efl_unref(old_factory);
     }

   elm_layout_content_set(obj, name, new_ev);
}

EOLIAN static Eo *
_efl_ui_layout_object_efl_object_constructor(Eo *obj, Efl_Ui_Layout_Object_Data *sd)
{
   sd->obj = obj;
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   evas_object_smart_callbacks_descriptions_set(obj, _smart_callbacks);
   efl_access_object_role_set(obj, EFL_ACCESS_ROLE_FILLER);

   return obj;
}

EOLIAN static Efl_Object*
_efl_ui_layout_object_efl_object_finalize(Eo *obj, Efl_Ui_Layout_Object_Data *pd EINA_UNUSED)
{
   Eo *eo;

   eo = efl_finalize(efl_super(obj, MY_CLASS));
   efl_ui_widget_theme_apply(eo);

   return eo;
}

EOLIAN static void
_efl_ui_layout_object_efl_layout_signal_message_send(Eo *obj, Efl_Ui_Layout_Object_Data *pd EINA_UNUSED, int id, const Eina_Value msg)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   efl_layout_signal_message_send(wd->resize_obj, id, msg);
}

EOLIAN static void
_efl_ui_layout_object_efl_layout_signal_signal_process(Eo *obj, Efl_Ui_Layout_Object_Data *pd EINA_UNUSED, Eina_Bool recurse)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   efl_layout_signal_process(wd->resize_obj, recurse);
}

/* Efl.Part implementation */

EOLIAN static Eo *
_efl_ui_layout_object_efl_part_part_get(const Eo *obj, Efl_Ui_Layout_Object_Data *sd EINA_UNUSED, const char *part)
{
   Efl_Canvas_Layout_Part_Type type = EFL_CANVAS_LAYOUT_PART_TYPE_NONE;

   EINA_SAFETY_ON_NULL_RETURN_VAL(part, NULL);
   ELM_WIDGET_DATA_GET_OR_RETURN((Eo *) obj, wd, NULL);

   // Check part type without using edje_object_part_object_get(), as this
   // can cause recalc, which has side effects... and could be slow.

   if (eina_streq(part, "background"))
     {
        if (efl_layout_group_part_exist_get(wd->resize_obj, "efl.background"))
          type = efl_canvas_layout_part_type_get(efl_part(wd->resize_obj, "efl.background"));
        if (type != EFL_CANVAS_LAYOUT_PART_TYPE_SWALLOW)
          {
             if (type < EFL_CANVAS_LAYOUT_PART_TYPE_LAST &&
                 type > EFL_CANVAS_LAYOUT_PART_TYPE_NONE)
               {
                  const char *file = NULL, *key = NULL;
                  efl_file_get(wd->resize_obj, &file, &key);
                  WRN("Layout has a background but it's not a swallow: '%s'",
                      elm_widget_theme_element_get(obj));
               }
             return efl_part_get(efl_super(obj, MY_CLASS), part);
          }

        return ELM_PART_IMPLEMENT(EFL_UI_LAYOUT_PART_BG_CLASS, obj, part);
     }
   else if (eina_streq(part, "shadow"))
     return efl_part_get(efl_super(obj, MY_CLASS), part);

   if (!efl_layout_group_part_exist_get(wd->resize_obj, part))
     {
        // edje part will handle the error message
        return efl_part_get(wd->resize_obj, part);
     }

   type = efl_canvas_layout_part_type_get(efl_part(wd->resize_obj, part));
   if (type >= EFL_CANVAS_LAYOUT_PART_TYPE_LAST)
     {
        ERR("Invalid type found for part '%s' in group '%s'",
            part, elm_widget_theme_element_get(obj));
        return NULL;
     }

   switch (type)
     {
      case EFL_CANVAS_LAYOUT_PART_TYPE_BOX:
      case EFL_CANVAS_LAYOUT_PART_TYPE_TABLE:
        return _efl_ui_layout_pack_proxy_get((Eo *) obj, type, part);
      case EFL_CANVAS_LAYOUT_PART_TYPE_TEXT:
      case EFL_CANVAS_LAYOUT_PART_TYPE_TEXTBLOCK:
        return ELM_PART_IMPLEMENT(EFL_UI_LAYOUT_PART_TEXT_CLASS, obj, part);
      case EFL_CANVAS_LAYOUT_PART_TYPE_SWALLOW:
        return ELM_PART_IMPLEMENT(EFL_UI_LAYOUT_PART_CONTENT_CLASS, obj, part);
      default:
        return ELM_PART_IMPLEMENT(EFL_UI_LAYOUT_PART_CLASS, obj, part);
     }
}

static const char *
_efl_ui_layout_default_content_part_get(const Eo *obj, Efl_Ui_Layout_Object_Data *sd EINA_UNUSED)
{
   const char *part = NULL;
   if (!_elm_layout_part_aliasing_eval(obj, &part, EINA_FALSE))
     return NULL;
   return part;
}

static const char *
_efl_ui_layout_default_text_part_get(const Eo *obj, Efl_Ui_Layout_Object_Data *sd EINA_UNUSED)
{
   const char *part = NULL;
   if (!_elm_layout_part_aliasing_eval(obj, &part, EINA_TRUE))
     return NULL;
   return part;
}

/* Efl.Ui.Layout.Part (common) */

EOLIAN static Eina_Bool
_efl_ui_layout_part_efl_ui_cursor_cursor_set(Eo *obj, void *_pd EINA_UNUSED, const char *cursor)
{
   Elm_Part_Data *pd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   Efl_Ui_Layout_Object_Data *sd = efl_data_scope_get(pd->obj, MY_CLASS);
   return _efl_ui_layout_part_cursor_set(sd, pd->part, cursor);
}

EOLIAN static const char *
_efl_ui_layout_part_efl_ui_cursor_cursor_get(const Eo *obj, void *_pd EINA_UNUSED)
{
   Elm_Part_Data *pd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   Efl_Ui_Layout_Object_Data *sd = efl_data_scope_get(pd->obj, MY_CLASS);
   return _efl_ui_layout_part_cursor_get(sd, pd->part);
}

EOLIAN static Eina_Bool
_efl_ui_layout_part_efl_ui_cursor_cursor_style_set(Eo *obj, void *_pd EINA_UNUSED, const char *style)
{
   Elm_Part_Data *pd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   Efl_Ui_Layout_Object_Data *sd = efl_data_scope_get(pd->obj, MY_CLASS);
   return _efl_ui_layout_part_cursor_style_set(sd, pd->part, style);
}

EOLIAN static const char *
_efl_ui_layout_part_efl_ui_cursor_cursor_style_get(const Eo *obj, void *_pd EINA_UNUSED)
{
   Elm_Part_Data *pd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   Efl_Ui_Layout_Object_Data *sd = efl_data_scope_get(pd->obj, MY_CLASS);
   return _efl_ui_layout_part_cursor_style_get(sd, pd->part);
}

EOLIAN static Eina_Bool
_efl_ui_layout_part_efl_ui_cursor_cursor_theme_search_enabled_set(Eo *obj, void *_pd EINA_UNUSED, Eina_Bool allow)
{
   Elm_Part_Data *pd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   Efl_Ui_Layout_Object_Data *sd = efl_data_scope_get(pd->obj, MY_CLASS);
   return _efl_ui_layout_part_cursor_engine_only_set(sd, pd->part, !allow);
}

EOLIAN static Eina_Bool
_efl_ui_layout_part_efl_ui_cursor_cursor_theme_search_enabled_get(const Eo *obj, void *_pd EINA_UNUSED)
{
   Elm_Part_Data *pd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   Efl_Ui_Layout_Object_Data *sd = efl_data_scope_get(pd->obj, MY_CLASS);
   return !_efl_ui_layout_part_cursor_engine_only_get(sd, pd->part);
}

#define CONTENT_FULL(part_typename, typename, CLASS, TYPENAME) \
  ELM_PART_OVERRIDE_CONTENT_GET_FULL(part_typename, typename, ELM_PART_OVERRIDE_INTERNALS_FETCH(CLASS, TYPENAME)) \
  ELM_PART_OVERRIDE_CONTENT_SET_FULL(part_typename, typename, ELM_PART_OVERRIDE_INTERNALS_FETCH(CLASS, TYPENAME)) \
  ELM_PART_OVERRIDE_CONTENT_UNSET_FULL(part_typename, typename, ELM_PART_OVERRIDE_INTERNALS_FETCH(CLASS, TYPENAME))


#define TEXT_FULL(part_typename, typename, CLASS, TYPENAME) \
  ELM_PART_OVERRIDE_TEXT_TEXT_GET_FULL(part_typename, typename, ELM_PART_OVERRIDE_INTERNALS_FETCH(CLASS, TYPENAME)) \
  ELM_PART_OVERRIDE_TEXT_TEXT_SET_FULL(part_typename, typename, ELM_PART_OVERRIDE_INTERNALS_FETCH(CLASS, TYPENAME)) \


#define MARKUP_FULL(part_typename, typename, CLASS, TYPENAME) \
  ELM_PART_OVERRIDE_TEXT_MARKUP_GET_FULL(part_typename, typename, ELM_PART_OVERRIDE_INTERNALS_FETCH(CLASS, TYPENAME)) \
  ELM_PART_OVERRIDE_TEXT_MARKUP_SET_FULL(part_typename, typename, ELM_PART_OVERRIDE_INTERNALS_FETCH(CLASS, TYPENAME)) \

/* Efl.Ui.Layout.Part_Content */
CONTENT_FULL(efl_ui_layout_part_content, efl_ui_layout, EFL_UI_LAYOUT_OBJECT, Efl_Ui_Layout_Object_Data)

/* Efl.Ui.Layout.Part_Text */
TEXT_FULL(efl_ui_layout_part_text, efl_ui_layout, EFL_UI_LAYOUT_OBJECT, Efl_Ui_Layout_Object_Data)
MARKUP_FULL(efl_ui_layout_part_text, efl_ui_layout, EFL_UI_LAYOUT_OBJECT, Efl_Ui_Layout_Object_Data)

EOLIAN static const char *
_efl_ui_layout_part_text_efl_ui_translatable_translatable_text_get(const Eo *obj, void *_pd EINA_UNUSED, const char **domain)
{
   Elm_Part_Data *pd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   return elm_widget_part_translatable_text_get(pd->obj, pd->part, domain);
}

EOLIAN static void
_efl_ui_layout_part_text_efl_ui_translatable_translatable_text_set(Eo *obj, void *_pd EINA_UNUSED, const char *label, const char *domain)
{
   Elm_Part_Data *pd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   elm_widget_part_translatable_text_set(pd->obj, pd->part, label, domain);
}

/* Efl.Ui.Layout.Part_Legacy */
CONTENT_FULL(efl_ui_layout_part_legacy, efl_ui_layout, EFL_UI_LAYOUT_OBJECT, Efl_Ui_Layout_Object_Data)
TEXT_FULL(efl_ui_layout_part_legacy, efl_ui_layout, EFL_UI_LAYOUT_OBJECT, Efl_Ui_Layout_Object_Data)
MARKUP_FULL(efl_ui_layout_part_legacy, efl_ui_layout, EFL_UI_LAYOUT_OBJECT, Efl_Ui_Layout_Object_Data)

EOLIAN static const char *
_efl_ui_layout_part_legacy_efl_ui_translatable_translatable_text_get(const Eo *obj, void *_pd EINA_UNUSED, const char **domain)
{
   Elm_Part_Data *pd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   return elm_widget_part_translatable_text_get(pd->obj, pd->part, domain);
}

EOLIAN static void
_efl_ui_layout_part_legacy_efl_ui_translatable_translatable_text_set(Eo *obj, void *_pd EINA_UNUSED, const char *label, const char *domain)
{
   Elm_Part_Data *pd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   elm_widget_part_translatable_text_set(pd->obj, pd->part, label, domain);
}

/* Efl.Ui.Layout.Part_Bg (common) */

EOLIAN static Efl_Object *
_efl_ui_layout_part_bg_efl_object_finalize(Eo *obj, void *_pd EINA_UNUSED)
{
   Efl_Ui_Layout_Object_Data *sd;
   Elm_Part_Data *pd;
   Eo *bg;

   obj = efl_finalize(efl_super(obj, EFL_UI_LAYOUT_PART_BG_CLASS));
   if (!obj) return NULL;

   pd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   sd = efl_data_scope_get(pd->obj, MY_CLASS);
   bg = _efl_ui_widget_bg_get(pd->obj);
   if (!_efl_ui_layout_content_set(pd->obj, sd, "efl.background", bg))
     {
        ERR("Failed to swallow new background object!");
        // Shouldn't happen. What now? del bg? call super? return null?
     }

   return obj;
}

/* Efl.Ui.Layout.Part_Xxx includes */
#include "efl_ui_layout_part.eo.c"
#include "efl_ui_layout_part_content.eo.c"
#include "efl_ui_layout_part_bg.eo.c"
#include "efl_ui_layout_part_text.eo.c"
#include "efl_ui_layout_part_legacy.eo.c"

/* Efl.Part end */


/* Internal EO APIs and hidden overrides */

EAPI EFL_VOID_FUNC_BODY(elm_layout_sizing_eval)
EFL_FUNC_BODY_CONST(elm_layout_text_aliases_get, const Elm_Layout_Part_Alias_Description *, NULL)
EFL_FUNC_BODY_CONST(elm_layout_content_aliases_get, const Elm_Layout_Part_Alias_Description *, NULL)

ELM_LAYOUT_CONTENT_ALIASES_IMPLEMENT(MY_CLASS_PFX)
ELM_LAYOUT_TEXT_ALIASES_IMPLEMENT(MY_CLASS_PFX)

#define EFL_UI_LAYOUT_OBJECT_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(efl_ui_layout), \
   ELM_PART_CONTENT_DEFAULT_OPS(efl_ui_layout), \
   ELM_PART_TEXT_DEFAULT_OPS(efl_ui_layout), \
   ELM_LAYOUT_CONTENT_ALIASES_OPS(MY_CLASS_PFX), \
   ELM_LAYOUT_TEXT_ALIASES_OPS(MY_CLASS_PFX), \
   EFL_OBJECT_OP_FUNC(elm_layout_sizing_eval, _elm_layout_sizing_eval), \
   EFL_OBJECT_OP_FUNC(efl_dbg_info_get, _efl_ui_layout_efl_object_dbg_info_get)

#include "efl_ui_layout_object.eo.c"

#include "efl_ui_layout_legacy.eo.h"

#define MY_CLASS_NAME_LEGACY "elm_layout"

static void
_efl_ui_layout_legacy_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

EOLIAN static Eo *
_efl_ui_layout_legacy_efl_object_constructor(Eo *obj, void *pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, EFL_UI_LAYOUT_LEGACY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);
   return obj;
}

EAPI Evas_Object *
elm_layout_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   return elm_legacy_add(EFL_UI_LAYOUT_LEGACY_CLASS, parent);
}

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
elm_layout_box_append(Eo *obj, const char *part, Evas_Object *child)
{
   return efl_pack(efl_part(obj, part), child);
}

EAPI Eina_Bool
elm_layout_box_prepend(Eo *obj, const char *part, Evas_Object *child)
{
   return efl_pack_begin(efl_part(obj, part), child);
}

EAPI Eina_Bool
elm_layout_box_insert_before(Eo *obj, const char *part, Evas_Object *child, const Evas_Object *reference)
{
   return efl_pack_before(efl_part(obj, part), child, reference);
}

EAPI Eina_Bool
elm_layout_box_insert_at(Eo *obj, const char *part, Evas_Object *child, unsigned int pos)
{
   return efl_pack_at(efl_part(obj, part), child, pos);
}

EAPI Evas_Object *
elm_layout_box_remove(Eo *obj, const char *part, Evas_Object *child)
{
   if (!efl_pack_unpack(efl_part(obj, part), child))
     return NULL;
   return child;
}

EAPI Eina_Bool
elm_layout_box_remove_all(Eo *obj, const char *part, Eina_Bool clear)
{
   if (clear)
     return efl_pack_clear(efl_part(obj, part));
   else
     return efl_pack_unpack_all(efl_part(obj, part));
}

EAPI Eina_Bool
elm_layout_table_pack(Eo *obj, const char *part, Evas_Object *child, unsigned short col, unsigned short row, unsigned short colspan, unsigned short rowspan)
{
   return efl_pack_table(efl_part(obj, part), child, col, row, colspan, rowspan);
}

EAPI Evas_Object *
elm_layout_table_unpack(Eo *obj, const char *part, Evas_Object *child)
{
   if (!efl_pack_unpack(efl_part(obj, part), child))
     return NULL;
   return child;
}

EAPI Eina_Bool
elm_layout_table_clear(Eo *obj, const char *part, Eina_Bool clear)
{
   if (clear)
     return efl_pack_clear(efl_part(obj, part));
   else
     return efl_pack_unpack_all(efl_part(obj, part));
}

EAPI Eina_Bool
elm_layout_text_set(Eo *obj, const char *part, const char *text)
{
   if (!part)
     {
        part = efl_ui_widget_default_text_part_get(obj);
        if (!part) return EINA_FALSE;
     }
   else if (!_elm_layout_part_aliasing_eval(obj, &part, EINA_TRUE))
     return EINA_FALSE;

   efl_text_set(efl_part(obj, part), text);
   return EINA_TRUE;
}

EAPI const char *
elm_layout_text_get(const Eo *obj, const char *part)
{
   if (!part)
     {
        part = efl_ui_widget_default_text_part_get(obj);
        if (!part) return NULL;
     }
   else if (!_elm_layout_part_aliasing_eval(obj, &part, EINA_TRUE))
     return NULL;

   return efl_text_get(efl_part(obj, part));
}

EAPI Eina_Bool
elm_layout_part_cursor_engine_only_set(Eo *obj, const char *part, Eina_Bool engine_only)
{
   return efl_ui_cursor_theme_search_enabled_set(efl_part(obj, part), !engine_only);
}

EAPI Eina_Bool
elm_layout_part_cursor_engine_only_get(const Eo *obj, const char *part)
{
   return !efl_ui_cursor_theme_search_enabled_get(efl_part(obj, part));
}

EAPI Eina_Bool
elm_layout_part_cursor_set(Eo *obj, const char *part, const char *cursor)
{
   return efl_ui_cursor_set(efl_part(obj, part), cursor);
}

EAPI const char *
elm_layout_part_cursor_get(const Eo *obj, const char *part)
{
   return efl_ui_cursor_get(efl_part(obj, part));
}

EAPI Eina_Bool
elm_layout_part_cursor_style_set(Eo *obj, const char *part, const char *style)
{
   return efl_ui_cursor_style_set(efl_part(obj, part), style);
}

EAPI const char *
elm_layout_part_cursor_style_get(const Eo *obj, const char *part)
{
   return efl_ui_cursor_style_get(efl_part(obj, part));
}

EAPI Eina_Bool
elm_layout_part_cursor_unset(Eo *obj, const char *part)
{
   return efl_ui_cursor_set(efl_part(obj, part), NULL);
}

EAPI int
elm_layout_freeze(Evas_Object *obj)
{
   return efl_layout_calc_freeze(obj);
}

EAPI int
elm_layout_thaw(Evas_Object *obj)
{
   return efl_layout_calc_thaw(obj);
}

void
_elm_layout_signal_callback_add_legacy(Eo *obj, Eo *edje, Eina_List **p_edje_signals,
                                       const char *emission, const char *source,
                                       Edje_Signal_Cb func, void *data)
{
   Edje_Signal_Data *esd;

   esd = ELM_NEW(Edje_Signal_Data);
   if (!esd) return;

   esd->obj = obj;
   esd->func = func;
   esd->emission = eina_stringshare_add(emission);
   esd->source = eina_stringshare_add(source);
   esd->data = data;
   *p_edje_signals = eina_list_append(*p_edje_signals, esd);

   efl_layout_signal_callback_add(edje, emission, source,
                                         _edje_signal_callback, esd);
}

EAPI void
elm_layout_signal_callback_add(Eo *obj, const char *emission, const char *source, Edje_Signal_Cb func, void *data)
{
   Efl_Ui_Layout_Object_Data *sd;

   if (!emission || !source) return;

   if (efl_isa(obj, ELM_ENTRY_CLASS))
     {
        _elm_entry_signal_callback_add_legacy(obj, emission, source, func, data);
        return;
     }

   sd = efl_data_scope_safe_get(obj, MY_CLASS);
   if (!sd) return;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   _elm_layout_signal_callback_add_legacy(obj, wd->resize_obj, &sd->edje_signals,
                                          emission, source, func, data);
}

void *
_elm_layout_signal_callback_del_legacy(Eo *obj EINA_UNUSED, Eo *edje, Eina_List **p_edje_signals,
                                       const char *emission, const char *source,
                                       Edje_Signal_Cb func)
{
   Edje_Signal_Data *esd = NULL;
   void *data = NULL;
   Eina_List *l;

   if (!emission || !source) return NULL;

   EINA_LIST_FOREACH(*p_edje_signals, l, esd)
     {
        if ((esd->func == func) && (!strcmp(esd->emission, emission)) &&
            (!strcmp(esd->source, source)))
          {
             *p_edje_signals = eina_list_remove_list(*p_edje_signals, l);

             efl_layout_signal_callback_del(edje, emission, source,
                                                   _edje_signal_callback, esd);

             eina_stringshare_del(esd->emission);
             eina_stringshare_del(esd->source);
             data = esd->data;
             free(esd);

             return data; /* stop at 1st match */
          }
     }

   return NULL;
}

EAPI void *
elm_layout_signal_callback_del(Eo *obj, const char *emission, const char *source, Edje_Signal_Cb func)
{
   Efl_Ui_Layout_Object_Data *sd;

   if (!emission || !source) return NULL;

   if (efl_isa(obj, ELM_ENTRY_CLASS))
     return _elm_entry_signal_callback_del_legacy(obj, emission, source, func);

   sd = efl_data_scope_safe_get(obj, MY_CLASS);
   if (!sd) return NULL;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   return _elm_layout_signal_callback_del_legacy(obj, wd->resize_obj, &sd->edje_signals,
                                                 emission, source, func);
}

EAPI void
elm_layout_signal_emit(Eo *obj, const char *emission, const char *source)
{
   efl_layout_signal_emit(obj, emission, source);
}

EAPI const char *
elm_layout_data_get(const Evas_Object *obj, const char *key)
{
   return efl_layout_group_data_get(obj, key);
}

EAPI Eina_Bool
elm_layout_theme_set(Evas_Object *obj, const char *klass, const char *group, const char *style)
{
   Efl_Ui_Theme_Apply ta;

   ta = efl_ui_layout_object_theme_set(obj, klass, group, style);
   return (ta != EFL_UI_THEME_APPLY_FAILED);
}

#include "efl_ui_layout_legacy.eo.c"
