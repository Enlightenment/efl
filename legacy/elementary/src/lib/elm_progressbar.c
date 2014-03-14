#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_progressbar.h"
#include "elm_widget_layout.h"

EAPI Eo_Op ELM_OBJ_PROGRESSBAR_BASE_ID = EO_NOOP;

#define MY_CLASS ELM_OBJ_PROGRESSBAR_CLASS

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

static void
_elm_progressbar_smart_sizing_eval(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
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
static void
_elm_progressbar_smart_sub_object_del(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Object *sobj = va_arg(*list, Evas_Object *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret = EINA_FALSE;

   eo_do_super(obj, MY_CLASS, elm_obj_widget_sub_object_del(sobj, &int_ret));
   if(!int_ret) return;

   _icon_signal_emit(obj);

   if (ret) *ret =  EINA_TRUE;
}

/* FIXME: replicated from elm_layout just because progressbar's icon
 * spot is elm.swallow.content, not elm.swallow.icon. Fix that
 * whenever we can changed the theme API */
static void
_elm_progressbar_smart_content_set(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Evas_Object *content = va_arg(*list, Evas_Object *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret = EINA_FALSE;
   eo_do_super(obj, MY_CLASS, elm_obj_container_content_set(part, content, &int_ret));
   if(!int_ret) return;

   _icon_signal_emit(obj);

   if (ret) *ret =  EINA_TRUE;
}

static void
_elm_progressbar_smart_theme(Eo *obj, void *_pd, va_list *list)
{
   Elm_Progressbar_Smart_Data *sd = _pd;
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret = EINA_FALSE;
   ELM_LAYOUT_DATA_GET(obj, ld);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (sd->horizontal)
     eina_stringshare_replace(&ld->group, "horizontal");
   else eina_stringshare_replace(&ld->group, "vertical");

   eo_do_super(obj, MY_CLASS, elm_obj_widget_theme_apply(&int_ret));
   if (!int_ret) return;

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

   if (ret) *ret = EINA_TRUE;
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

static void
_elm_progressbar_smart_add(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Progressbar_Smart_Data *priv = _pd;
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

static void
_elm_progressbar_smart_del(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Progressbar_Smart_Data *sd = _pd;
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

static void
_elm_progressbar_smart_text_aliases_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   const Elm_Layout_Part_Alias_Description **aliases = va_arg(*list, const Elm_Layout_Part_Alias_Description **);
   *aliases = _text_aliases;
}

static void
_elm_progressbar_smart_content_aliases_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   const Elm_Layout_Part_Alias_Description **aliases = va_arg(*list, const Elm_Layout_Part_Alias_Description **);
   *aliases = _content_aliases;
}

EAPI Evas_Object *
elm_progressbar_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = eo_add(MY_CLASS, parent);
   eo_unref(obj);
   return obj;
}

static void
_constructor(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME_LEGACY),
         evas_obj_smart_callbacks_descriptions_set(_smart_callbacks, NULL));
}

EAPI void
elm_progressbar_pulse_set(Evas_Object *obj,
                          Eina_Bool pulse)
{
   ELM_PROGRESSBAR_CHECK(obj);
   eo_do(obj, elm_obj_progressbar_pulse_set(pulse));
}

static void
_pulse_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool pulse = va_arg(*list, int);
   Elm_Progressbar_Smart_Data *sd = _pd;

   pulse = !!pulse;
   if (sd->pulse == pulse) return;

   sd->pulse = pulse;

   eo_do(obj, elm_obj_widget_theme_apply(NULL));
}

EAPI Eina_Bool
elm_progressbar_pulse_get(const Evas_Object *obj)
{
   ELM_PROGRESSBAR_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_progressbar_pulse_get(&ret));
   return ret;
}

static void
_pulse_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Progressbar_Smart_Data *sd = _pd;

   *ret =  sd->pulse;
}

EAPI void
elm_progressbar_pulse(Evas_Object *obj,
                      Eina_Bool state)
{
   ELM_PROGRESSBAR_CHECK(obj);
   eo_do(obj, elm_obj_progressbar_pulse(state));
}

static void
_pulse(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool state = va_arg(*list, int);
   Elm_Progressbar_Smart_Data *sd = _pd;

   state = !!state;
   if ((!sd->pulse) || (sd->pulse_state == state)) return;

   sd->pulse_state = state;

   if (sd->pulse_state)
     elm_layout_signal_emit(obj, "elm,state,pulse,start", "elm");
   else
     elm_layout_signal_emit(obj, "elm,state,pulse,stop", "elm");
}

EAPI void
elm_progressbar_part_value_set(Evas_Object *obj, const char *part, double val)
{
   ELM_PROGRESSBAR_CHECK(obj);
   eo_do(obj, elm_obj_progressbar_part_value_set(part, val));
}

static void
_part_value_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part_name = va_arg(*list, const char *);
   double val = va_arg(*list, double);
   Elm_Progressbar_Smart_Data *sd = _pd;
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

EAPI double
elm_progressbar_part_value_get(const Evas_Object *obj, const char * part)
{
   ELM_PROGRESSBAR_CHECK(obj) 0.0;
   double ret;
   eo_do((Eo *) obj, elm_obj_progressbar_part_value_get(part,&ret));
   return ret;
}

static void
_part_value_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char* part = va_arg(*list, const char *);
   double *ret = va_arg(*list, double *);
   Elm_Progressbar_Smart_Data *sd = _pd;
   Elm_Progress_Status *ps;
   Eina_List *l;

   EINA_LIST_FOREACH(sd->progress_status, l, ps)
     {
        if (!strcmp(ps->part_name, part))
           {
              *ret = ps->val;
              return;
           }
     }
}

EAPI void
elm_progressbar_value_set(Evas_Object *obj,
                          double val)
{
   ELM_PROGRESSBAR_CHECK(obj);
   eo_do(obj, elm_obj_progressbar_value_set(val));
}

static void
_value_set(Eo *obj, void *_pd, va_list *list)
{
   double val = va_arg(*list, double);
   Elm_Progressbar_Smart_Data *sd = _pd;
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

EAPI double
elm_progressbar_value_get(const Evas_Object *obj)
{
   ELM_PROGRESSBAR_CHECK(obj) 0.0;
   double ret;
   eo_do((Eo *) obj, elm_obj_progressbar_value_get(&ret));
   return ret;
}

static void
_value_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   double *ret = va_arg(*list, double *);
   Elm_Progressbar_Smart_Data *sd = _pd;

   *ret = sd->val;
}

EAPI void
elm_progressbar_span_size_set(Evas_Object *obj,
                              Evas_Coord size)
{
   ELM_PROGRESSBAR_CHECK(obj);
   eo_do(obj, elm_obj_progressbar_span_size_set(size));
}

static void
_span_size_set(Eo *obj, void *_pd, va_list *list)
{
   Evas_Coord size = va_arg(*list, Evas_Coord);
   Elm_Progressbar_Smart_Data *sd = _pd;

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

EAPI Evas_Coord
elm_progressbar_span_size_get(const Evas_Object *obj)
{
   ELM_PROGRESSBAR_CHECK(obj) 0;
   Evas_Coord ret;
   eo_do((Eo *) obj, elm_obj_progressbar_span_size_get(&ret));
   return ret;
}

static void
_span_size_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Coord *ret = va_arg(*list, Evas_Coord *);
   Elm_Progressbar_Smart_Data *sd = _pd;

   *ret = sd->size;
}

EAPI void
elm_progressbar_unit_format_set(Evas_Object *obj,
                                const char *units)
{
   ELM_PROGRESSBAR_CHECK(obj);
   eo_do(obj, elm_obj_progressbar_unit_format_set(units));
}

static void
_unit_format_set(Eo *obj, void *_pd, va_list *list)
{
   const char *units = va_arg(*list, const char *);
   Elm_Progressbar_Smart_Data *sd = _pd;
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

EAPI const char *
elm_progressbar_unit_format_get(const Evas_Object *obj)
{
   ELM_PROGRESSBAR_CHECK(obj) NULL;
   const char *ret = NULL;
   eo_do((Eo *) obj, elm_obj_progressbar_unit_format_get(&ret));
   return ret;
}

static void
_unit_format_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char **ret = va_arg(*list, const char **);
   Elm_Progressbar_Smart_Data *sd = _pd;

   *ret = sd->units;
}

EAPI void
elm_progressbar_unit_format_function_set(Evas_Object *obj, char *(func)(double), void (*free_func) (char *))
{
   ELM_PROGRESSBAR_CHECK(obj);
   eo_do(obj, elm_obj_progressbar_unit_format_function_set(func, free_func));
}

EAPI void
_unit_format_function_set(Eo *obj, void *_pd, va_list *list)
{
   progressbar_func_type func = va_arg(*list, progressbar_func_type);
   progressbar_freefunc_type free_func = va_arg(*list, progressbar_freefunc_type);
   Elm_Progressbar_Smart_Data *sd = _pd;

   sd->unit_format_func = func;
   sd->unit_format_free = free_func;

   _units_set(obj);
   elm_layout_sizing_eval(obj);
}

EAPI void
elm_progressbar_horizontal_set(Evas_Object *obj,
                               Eina_Bool horizontal)
{
   ELM_PROGRESSBAR_CHECK(obj);
   eo_do(obj, elm_obj_progressbar_horizontal_set(horizontal));
}

static void
_horizontal_set(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool horizontal = va_arg(*list, int);
   Elm_Progressbar_Smart_Data*sd = _pd;

   horizontal = !!horizontal;
   if (sd->horizontal == horizontal) return;

   sd->horizontal = horizontal;
   eo_do(obj, elm_obj_widget_theme_apply(NULL));
}

EAPI Eina_Bool
elm_progressbar_horizontal_get(const Evas_Object *obj)
{
   ELM_PROGRESSBAR_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_progressbar_horizontal_get(&ret));
   return ret;
}

static void
_horizontal_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Progressbar_Smart_Data*sd = _pd;
   *ret = sd->horizontal;
}

EAPI void
elm_progressbar_inverted_set(Evas_Object *obj,
                             Eina_Bool inverted)
{
   ELM_PROGRESSBAR_CHECK(obj);
   eo_do(obj, elm_obj_progressbar_inverted_set(inverted));
}

static void
_inverted_set(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool inverted = va_arg(*list, int);
   Elm_Progressbar_Smart_Data*sd = _pd;
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

EAPI Eina_Bool
elm_progressbar_inverted_get(const Evas_Object *obj)
{
   ELM_PROGRESSBAR_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_progressbar_inverted_get(&ret));
   return ret;
}

static void
_inverted_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Progressbar_Smart_Data*sd = _pd;
   *ret = sd->inverted;
}

static void
_elm_progressbar_smart_focus_next_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_FALSE;
}

static void
_elm_progressbar_smart_focus_direction_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_FALSE;
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),

        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_ADD), _elm_progressbar_smart_add),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_DEL), _elm_progressbar_smart_del),

        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_THEME_APPLY), _elm_progressbar_smart_theme),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_SUB_OBJECT_DEL), _elm_progressbar_smart_sub_object_del),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_NEXT_MANAGER_IS), _elm_progressbar_smart_focus_next_manager_is),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_DIRECTION_MANAGER_IS), _elm_progressbar_smart_focus_direction_manager_is),

        EO_OP_FUNC(ELM_OBJ_CONTAINER_ID(ELM_OBJ_CONTAINER_SUB_ID_CONTENT_SET), _elm_progressbar_smart_content_set),

        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_SIZING_EVAL), _elm_progressbar_smart_sizing_eval),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_TEXT_ALIASES_GET), _elm_progressbar_smart_text_aliases_get),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_CONTENT_ALIASES_GET), _elm_progressbar_smart_content_aliases_get),

        EO_OP_FUNC(ELM_OBJ_PROGRESSBAR_ID(ELM_OBJ_PROGRESSBAR_SUB_ID_PULSE_SET), _pulse_set),
        EO_OP_FUNC(ELM_OBJ_PROGRESSBAR_ID(ELM_OBJ_PROGRESSBAR_SUB_ID_PULSE_GET), _pulse_get),
        EO_OP_FUNC(ELM_OBJ_PROGRESSBAR_ID(ELM_OBJ_PROGRESSBAR_SUB_ID_PULSE), _pulse),
        EO_OP_FUNC(ELM_OBJ_PROGRESSBAR_ID(ELM_OBJ_PROGRESSBAR_SUB_ID_VALUE_SET), _value_set),
        EO_OP_FUNC(ELM_OBJ_PROGRESSBAR_ID(ELM_OBJ_PROGRESSBAR_SUB_ID_VALUE_GET), _value_get),
        EO_OP_FUNC(ELM_OBJ_PROGRESSBAR_ID(ELM_OBJ_PROGRESSBAR_SUB_ID_SPAN_SIZE_SET), _span_size_set),
        EO_OP_FUNC(ELM_OBJ_PROGRESSBAR_ID(ELM_OBJ_PROGRESSBAR_SUB_ID_SPAN_SIZE_GET), _span_size_get),
        EO_OP_FUNC(ELM_OBJ_PROGRESSBAR_ID(ELM_OBJ_PROGRESSBAR_SUB_ID_UNIT_FORMAT_SET), _unit_format_set),
        EO_OP_FUNC(ELM_OBJ_PROGRESSBAR_ID(ELM_OBJ_PROGRESSBAR_SUB_ID_UNIT_FORMAT_GET), _unit_format_get),
        EO_OP_FUNC(ELM_OBJ_PROGRESSBAR_ID(ELM_OBJ_PROGRESSBAR_SUB_ID_UNIT_FORMAT_FUNCTION_SET), _unit_format_function_set),
        EO_OP_FUNC(ELM_OBJ_PROGRESSBAR_ID(ELM_OBJ_PROGRESSBAR_SUB_ID_HORIZONTAL_SET), _horizontal_set),
        EO_OP_FUNC(ELM_OBJ_PROGRESSBAR_ID(ELM_OBJ_PROGRESSBAR_SUB_ID_HORIZONTAL_GET), _horizontal_get),
        EO_OP_FUNC(ELM_OBJ_PROGRESSBAR_ID(ELM_OBJ_PROGRESSBAR_SUB_ID_INVERTED_SET), _inverted_set),
        EO_OP_FUNC(ELM_OBJ_PROGRESSBAR_ID(ELM_OBJ_PROGRESSBAR_SUB_ID_INVERTED_GET), _inverted_get),
        EO_OP_FUNC(ELM_OBJ_PROGRESSBAR_ID(ELM_OBJ_PROGRESSBAR_SUB_ID_PART_VALUE_SET), _part_value_set),
        EO_OP_FUNC(ELM_OBJ_PROGRESSBAR_ID(ELM_OBJ_PROGRESSBAR_SUB_ID_PART_VALUE_GET), _part_value_get),
        EO_OP_FUNC_SENTINEL
   };
   eo_class_funcs_set(klass, func_desc);

   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}
static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(ELM_OBJ_PROGRESSBAR_SUB_ID_PULSE_SET, "Set whether a given progress bar widget is at 'pulsing mode' or not."),
     EO_OP_DESCRIPTION(ELM_OBJ_PROGRESSBAR_SUB_ID_PULSE_GET, "Get whether a given progress bar widget is at 'pulsing mode' or not."),
     EO_OP_DESCRIPTION(ELM_OBJ_PROGRESSBAR_SUB_ID_PULSE, "Start/stop a given progress bar 'pulsing' animation, if its under that mode."),
     EO_OP_DESCRIPTION(ELM_OBJ_PROGRESSBAR_SUB_ID_VALUE_SET, "Set the progress value (in percentage) on a given progress bar widget."),
     EO_OP_DESCRIPTION(ELM_OBJ_PROGRESSBAR_SUB_ID_VALUE_GET, "Get the progress value (in percentage) on a given progress bar widget."),
     EO_OP_DESCRIPTION(ELM_OBJ_PROGRESSBAR_SUB_ID_SPAN_SIZE_SET, "Set the (exact) length of the bar region of a given progress bar widget."),
     EO_OP_DESCRIPTION(ELM_OBJ_PROGRESSBAR_SUB_ID_SPAN_SIZE_GET, "Get the length set for the bar region of a given progress bar widget."),
     EO_OP_DESCRIPTION(ELM_OBJ_PROGRESSBAR_SUB_ID_UNIT_FORMAT_SET, "Set the format string for a given progress bar widget's units label."),
     EO_OP_DESCRIPTION(ELM_OBJ_PROGRESSBAR_SUB_ID_UNIT_FORMAT_GET, "Retrieve the format string set for a given progress bar widget's units label."),
     EO_OP_DESCRIPTION(ELM_OBJ_PROGRESSBAR_SUB_ID_UNIT_FORMAT_FUNCTION_SET, "Set the format function pointer for the units label."),
     EO_OP_DESCRIPTION(ELM_OBJ_PROGRESSBAR_SUB_ID_HORIZONTAL_SET, "Set the orientation of a given progress bar widget."),
     EO_OP_DESCRIPTION(ELM_OBJ_PROGRESSBAR_SUB_ID_HORIZONTAL_GET, "Retrieve the orientation of a given progress bar widget."),
     EO_OP_DESCRIPTION(ELM_OBJ_PROGRESSBAR_SUB_ID_INVERTED_SET, "Invert a given progress bar widget's displaying values order."),
     EO_OP_DESCRIPTION(ELM_OBJ_PROGRESSBAR_SUB_ID_INVERTED_GET, "Get whether a given progress bar widget's displaying values are inverted or not."),
     EO_OP_DESCRIPTION(ELM_OBJ_PROGRESSBAR_SUB_ID_PART_VALUE_SET, "Set the progress value (in percentage) on a given progress bar widget for a part."),
     EO_OP_DESCRIPTION(ELM_OBJ_PROGRESSBAR_SUB_ID_PART_VALUE_GET, "Get the progress value (in percentage) on a given progress bar widget for a part."),
     EO_OP_DESCRIPTION_SENTINEL
};
static const Eo_Class_Description class_desc = {
     EO_VERSION,
     MY_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&ELM_OBJ_PROGRESSBAR_BASE_ID, op_desc, ELM_OBJ_PROGRESSBAR_SUB_ID_LAST),
     NULL,
     sizeof(Elm_Progressbar_Smart_Data),
     _class_constructor,
     NULL
};
EO_DEFINE_CLASS(elm_obj_progressbar_class_get, &class_desc, ELM_OBJ_LAYOUT_CLASS, NULL);
