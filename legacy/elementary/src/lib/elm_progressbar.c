#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_progressbar.h"
#include "elm_widget_layout.h"

#define MY_CLASS ELM_PROGRESSBAR_CLASS

#define MY_CLASS_NAME "Elm_Progressbar"
#define MY_CLASS_NAME_LEGACY "elm_progressbar"

static const char SIG_CHANGED[] = "changed";

#define MIN_RATIO_LVL 0.0
#define MAX_RATIO_LVL 1.0

/* smart callbacks coming from elm progressbar objects (besides the
 * ones coming from elm layout): */
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_CHANGED, ""},
   {SIG_WIDGET_LANG_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_WIDGET_ACCESS_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_LAYOUT_FOCUSED, ""}, /**< handled by elm_layout */
   {SIG_LAYOUT_UNFOCUSED, ""}, /**< handled by elm_layout */
   {NULL, NULL}
};

static const Elm_Layout_Part_Alias_Description _content_aliases[] =
{
   {"icon", "elm.swallow.content"},
   {NULL, NULL}
};

static const Elm_Layout_Part_Alias_Description _text_aliases[] =
{
   {"default", "elm.text"},
   {NULL, NULL}
};

static Elm_Progress_Status *
_progress_status_new(const char *part_name, double val)
{
   Elm_Progress_Status *ps;
   ps = calloc(1, sizeof(Elm_Progress_Status));
   ps->part_name = eina_stringshare_add(part_name);
   ps->val = val;
   return ps;
}

static inline void
_progress_status_free(Elm_Progress_Status *ps)
{
   eina_stringshare_del(ps->part_name);
   free(ps);
}
static void
_units_set(Evas_Object *obj)
{
   ELM_PROGRESSBAR_DATA_GET(obj, sd);

   if (sd->unit_format_func)
     {
        char *buf;

        buf = sd->unit_format_func(sd->val);
        elm_layout_text_set(obj, "elm.text.status", buf);
        if (sd->unit_format_free) sd->unit_format_free(buf);
     }
   else if (sd->units)
     {
        char buf[1024];

        snprintf(buf, sizeof(buf), sd->units, 100 * sd->val);
        elm_layout_text_set(obj, "elm.text.status", buf);
     }
   else elm_layout_text_set(obj, "elm.text.status", NULL);
}

static void
_val_set(Evas_Object *obj)
{
   Eina_Bool rtl;
   double pos;

   ELM_PROGRESSBAR_DATA_GET(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   rtl = elm_widget_mirrored_get(obj);
   Elm_Progress_Status *ps;
   Eina_List *l;

   EINA_LIST_FOREACH(sd->progress_status, l, ps)
      {
         pos = ps->val;
         if ((!rtl && sd->inverted) ||
              (rtl && ((!sd->horizontal && sd->inverted) ||
                (sd->horizontal && !sd->inverted))))
            pos = MAX_RATIO_LVL - pos;

         edje_object_part_drag_value_set
           (wd->resize_obj, ps->part_name, pos, pos);
      }
}

EOLIAN static void
_elm_progressbar_elm_layout_sizing_eval(Eo *obj, Elm_Progressbar_Data *_pd EINA_UNUSED)
{
   Evas_Coord minw = -1, minh = -1;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   edje_object_size_min_restricted_calc
     (wd->resize_obj, &minw, &minh, minw, minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, -1, -1);
}

/* FIXME: replicated from elm_layout just because progressbar's icon
 * spot is elm.swallow.content, not elm.swallow.icon. Fix that
 * whenever we can changed the theme API */
static void
_icon_signal_emit(Evas_Object *obj)
{
   char buf[64];

   snprintf(buf, sizeof(buf), "elm,state,icon,%s",
            elm_layout_content_get(obj, "icon") ? "visible" : "hidden");

   elm_layout_signal_emit(obj, buf, "elm");
}

/* FIXME: replicated from elm_layout just because progressbar's icon
 * spot is elm.swallow.content, not elm.swallow.icon. Fix that
 * whenever we can changed the theme API */
EOLIAN static Eina_Bool
_elm_progressbar_elm_widget_sub_object_del(Eo *obj, Elm_Progressbar_Data *_pd EINA_UNUSED, Evas_Object *sobj)
{
   Eina_Bool int_ret = EINA_FALSE;
   eo_do_super(obj, MY_CLASS, int_ret = elm_obj_widget_sub_object_del(sobj));
   if (!int_ret) return EINA_FALSE;

   _icon_signal_emit(obj);

   return EINA_TRUE;
}

/* FIXME: replicated from elm_layout just because progressbar's icon
 * spot is elm.swallow.content, not elm.swallow.icon. Fix that
 * whenever we can changed the theme API */
EOLIAN static Eina_Bool
_elm_progressbar_elm_container_content_set(Eo *obj, Elm_Progressbar_Data *_pd EINA_UNUSED, const char *part, Evas_Object *content)
{
   Eina_Bool int_ret = EINA_FALSE;
   eo_do_super(obj, MY_CLASS, int_ret = elm_obj_container_content_set(part, content));
   if (!int_ret) return EINA_FALSE;

   _icon_signal_emit(obj);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_progressbar_elm_widget_theme_apply(Eo *obj, Elm_Progressbar_Data *sd)
{
   Eina_Bool int_ret = EINA_FALSE;
   ELM_LAYOUT_DATA_GET(obj, ld);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   if (sd->horizontal)
     eina_stringshare_replace(&ld->group, "horizontal");
   else eina_stringshare_replace(&ld->group, "vertical");

   eo_do_super(obj, MY_CLASS, int_ret = elm_obj_widget_theme_apply());
   if (!int_ret) return EINA_FALSE;

   if (sd->pulse)
     elm_layout_signal_emit(obj, "elm,state,pulse", "elm");
   else
     elm_layout_signal_emit(obj, "elm,state,fraction", "elm");

   if (sd->pulse_state)
     elm_layout_signal_emit(obj, "elm,state,pulse,start", "elm");

   if ((sd->units) && (!sd->pulse))
     elm_layout_signal_emit(obj, "elm,state,units,visible", "elm");

   if (sd->horizontal)
     evas_object_size_hint_min_set
       (sd->spacer, (double)sd->size * elm_widget_scale_get(obj) *
       elm_config_scale_get(), 1);
   else
     evas_object_size_hint_min_set
       (sd->spacer, 1, (double)sd->size * elm_widget_scale_get(obj) *
       elm_config_scale_get());

   if (sd->inverted)
     elm_layout_signal_emit(obj, "elm,state,inverted,on", "elm");

   _units_set(obj);
   _val_set(obj);

   /* FIXME: replicated from elm_layout just because progressbar's
    * icon spot is elm.swallow.content, not elm.swallow.icon. Fix that
    * whenever we can changed the theme API */
   _icon_signal_emit(obj);

   edje_object_message_signal_process(wd->resize_obj);

   elm_layout_sizing_eval(obj);

   return EINA_TRUE;
}

static char *
_access_info_cb(void *data EINA_UNUSED, Evas_Object *obj)
{
   const char *txt = elm_widget_access_info_get(obj);

   if (!txt) txt = elm_layout_text_get(obj, NULL);
   if (txt) return strdup(txt);

   return NULL;
}

static char *
_access_state_cb(void *data EINA_UNUSED, Evas_Object *obj)
{
   char *ret;
   Eina_Strbuf *buf;
   buf = eina_strbuf_new();

   const char *txt = elm_layout_text_get(obj, "elm.text.status");
   if (txt) eina_strbuf_append(buf, txt);

   if (elm_widget_disabled_get(obj))
     eina_strbuf_append(buf, " state: disabled");

   if (eina_strbuf_length_get(buf))
     {
        ret = eina_strbuf_string_steal(buf);
        eina_strbuf_free(buf);
        return ret;
     }

   eina_strbuf_free(buf);
   return NULL;
}

EOLIAN static void
_elm_progressbar_evas_object_smart_add(Eo *obj, Elm_Progressbar_Data *priv)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());
   elm_widget_sub_object_parent_add(obj);

   priv->horizontal = EINA_TRUE;
   priv->units = eina_stringshare_add("%.0f %%");
   priv->val = MIN_RATIO_LVL;

   if (!elm_layout_theme_set
       (obj, "progressbar", "horizontal", elm_widget_style_get(obj)))
     CRI("Failed to set layout!");

   priv->spacer = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_color_set(priv->spacer, 0, 0, 0, 0);
   evas_object_pass_events_set(priv->spacer, EINA_TRUE);

   elm_layout_content_set(obj, "elm.swallow.bar", priv->spacer);

   _units_set(obj);
   _val_set(obj);

   elm_layout_sizing_eval(obj);

   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
     elm_widget_can_focus_set(obj, EINA_TRUE);

   _elm_access_object_register(obj, wd->resize_obj);
   _elm_access_text_set
     (_elm_access_info_get(obj), ELM_ACCESS_TYPE, E_("progressbar"));
   _elm_access_callback_set
     (_elm_access_info_get(obj), ELM_ACCESS_INFO, _access_info_cb, NULL);
   _elm_access_callback_set
     (_elm_access_info_get(obj), ELM_ACCESS_STATE, _access_state_cb, NULL);
}

EOLIAN static void
_elm_progressbar_evas_object_smart_del(Eo *obj, Elm_Progressbar_Data *sd)
{
   Elm_Progress_Status *progress_obj;

   eina_stringshare_del(sd->units);
   if (sd->progress_status)
      {
         EINA_LIST_FREE(sd->progress_status, progress_obj)
           {
              _progress_status_free(progress_obj);
           }
      }

   eo_do_super(obj, MY_CLASS, evas_obj_smart_del());
}

EOLIAN static const Elm_Layout_Part_Alias_Description*
_elm_progressbar_elm_layout_text_aliases_get(Eo *obj EINA_UNUSED, Elm_Progressbar_Data *_pd EINA_UNUSED)
{
   return _text_aliases;
}

EOLIAN static const Elm_Layout_Part_Alias_Description*
_elm_progressbar_elm_layout_content_aliases_get(Eo *obj EINA_UNUSED, Elm_Progressbar_Data *_pd EINA_UNUSED)
{
   return _content_aliases;
}

EAPI Evas_Object *
elm_progressbar_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = eo_add(MY_CLASS, parent);
   eo_unref(obj);
   return obj;
}

EOLIAN static void
_elm_progressbar_eo_base_constructor(Eo *obj, Elm_Progressbar_Data *_pd EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME_LEGACY),
         evas_obj_smart_callbacks_descriptions_set(_smart_callbacks));
}

EOLIAN static void
_elm_progressbar_pulse_set(Eo *obj EINA_UNUSED, Elm_Progressbar_Data *sd, Eina_Bool pulse)
{
   pulse = !!pulse;
   if (sd->pulse == pulse) return;

   sd->pulse = pulse;

   eo_do(obj, elm_obj_widget_theme_apply());
}

EOLIAN static Eina_Bool
_elm_progressbar_pulse_get(Eo *obj EINA_UNUSED, Elm_Progressbar_Data *sd)
{
   return sd->pulse;
}

EOLIAN static void
_elm_progressbar_pulse(Eo *obj, Elm_Progressbar_Data *sd, Eina_Bool state)
{
   state = !!state;
   if ((!sd->pulse) || (sd->pulse_state == state)) return;

   sd->pulse_state = state;

   if (sd->pulse_state)
     elm_layout_signal_emit(obj, "elm,state,pulse,start", "elm");
   else
     elm_layout_signal_emit(obj, "elm,state,pulse,stop", "elm");
}

EOLIAN static void
_elm_progressbar_part_value_set(Eo *obj EINA_UNUSED, Elm_Progressbar_Data *sd, const char *part_name, double val)
{
   Elm_Progress_Status *ps;
   Eina_Bool  existing_ps = EINA_FALSE;
   Eina_List *l;

   if (val < MIN_RATIO_LVL) val = MIN_RATIO_LVL;
   if (val > MAX_RATIO_LVL) val = MAX_RATIO_LVL;

   if (!strcmp(part_name, "elm.cur.progressbar"))
     sd->val = val;

   EINA_LIST_FOREACH(sd->progress_status, l, ps)
     {
        if (!strcmp(ps->part_name, part_name))
          {
             existing_ps = EINA_TRUE;
             break;
          }
     }

   if (!existing_ps)
      {
         ps = _progress_status_new(part_name, val);
         sd->progress_status = eina_list_append(sd->progress_status, ps);
      }
   else
      ps->val = val;

   _val_set(obj);
   _units_set(obj);
   evas_object_smart_callback_call(obj, SIG_CHANGED, NULL);
}

EOLIAN static double
_elm_progressbar_part_value_get(Eo *obj EINA_UNUSED, Elm_Progressbar_Data *sd, const char* part)
{
   Elm_Progress_Status *ps;
   Eina_List *l;

   EINA_LIST_FOREACH(sd->progress_status, l, ps)
     {
        if (!strcmp(ps->part_name, part)) return ps->val;
     }

   return 0.0;
}

EOLIAN static void
_elm_progressbar_value_set(Eo *obj, Elm_Progressbar_Data *sd, double val)
{
   Elm_Progress_Status *ps;

   if (sd->val == val) return;

   sd->val = val;
   if (sd->val < MIN_RATIO_LVL) sd->val = MIN_RATIO_LVL;
   if (sd->val > MAX_RATIO_LVL) sd->val = MAX_RATIO_LVL;

   ps = _progress_status_new("elm.cur.progressbar", sd->val);
   sd->progress_status = eina_list_append(sd->progress_status, ps);
   _val_set(obj);
   _units_set(obj);
   evas_object_smart_callback_call(obj, SIG_CHANGED, NULL);
}

EOLIAN static double
_elm_progressbar_value_get(Eo *obj EINA_UNUSED, Elm_Progressbar_Data *sd)
{
   return sd->val;
}

EOLIAN static void
_elm_progressbar_span_size_set(Eo *obj, Elm_Progressbar_Data *sd, Evas_Coord size)
{
   if (sd->size == size) return;

   sd->size = size;

   if (sd->horizontal)
     evas_object_size_hint_min_set
       (sd->spacer, (double)sd->size * elm_widget_scale_get(obj) *
       elm_config_scale_get(), 1);
   else
     evas_object_size_hint_min_set
       (sd->spacer, 1, (double)sd->size * elm_widget_scale_get(obj) *
       elm_config_scale_get());

   elm_layout_sizing_eval(obj);
}

EOLIAN static Evas_Coord
_elm_progressbar_span_size_get(Eo *obj EINA_UNUSED, Elm_Progressbar_Data *sd)
{
   return sd->size;
}

EOLIAN static void
_elm_progressbar_unit_format_set(Eo *obj, Elm_Progressbar_Data *sd, const char *units)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   eina_stringshare_replace(&sd->units, units);
   if (units)
     {
        elm_layout_signal_emit(obj, "elm,state,units,visible", "elm");
        edje_object_message_signal_process(wd->resize_obj);
     }
   else
     {
        elm_layout_signal_emit(obj, "elm,state,units,hidden", "elm");
        edje_object_message_signal_process(wd->resize_obj);
     }

   _units_set(obj);
   elm_layout_sizing_eval(obj);
}

EOLIAN static const char*
_elm_progressbar_unit_format_get(Eo *obj EINA_UNUSED, Elm_Progressbar_Data *sd)
{
   return sd->units;
}

EOLIAN static void
_elm_progressbar_unit_format_function_set(Eo *obj, Elm_Progressbar_Data *sd, progressbar_func_type func, progressbar_freefunc_type free_func)
{
   sd->unit_format_func = func;
   sd->unit_format_free = free_func;

   _units_set(obj);
   elm_layout_sizing_eval(obj);
}

EOLIAN static void
_elm_progressbar_horizontal_set(Eo *obj, Elm_Progressbar_Data *sd, Eina_Bool horizontal)
{
   horizontal = !!horizontal;
   if (sd->horizontal == horizontal) return;

   sd->horizontal = horizontal;
   eo_do(obj, elm_obj_widget_theme_apply());
}

EOLIAN static Eina_Bool
_elm_progressbar_horizontal_get(Eo *obj EINA_UNUSED, Elm_Progressbar_Data *sd)
{
   return sd->horizontal;
}

EOLIAN static void
_elm_progressbar_inverted_set(Eo *obj, Elm_Progressbar_Data *sd, Eina_Bool inverted)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   inverted = !!inverted;
   if (sd->inverted == inverted) return;

   sd->inverted = inverted;
   if (sd->inverted)
     elm_layout_signal_emit(obj, "elm,state,inverted,on", "elm");
   else
     elm_layout_signal_emit(obj, "elm,state,inverted,off", "elm");

   edje_object_message_signal_process(wd->resize_obj);

   _val_set(obj);
   _units_set(obj);
}

EOLIAN static Eina_Bool
_elm_progressbar_inverted_get(Eo *obj EINA_UNUSED, Elm_Progressbar_Data *sd)
{
   return sd->inverted;
}

EOLIAN static Eina_Bool
_elm_progressbar_elm_widget_focus_next_manager_is(Eo *obj EINA_UNUSED, Elm_Progressbar_Data *_pd EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_elm_progressbar_elm_widget_focus_direction_manager_is(Eo *obj EINA_UNUSED, Elm_Progressbar_Data *_pd EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static void
_elm_progressbar_class_constructor(Eo_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

#include "elm_progressbar.eo.c"
