#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_PROTECTED
#define ELM_LAYOUT_PROTECTED
#define EFL_ACCESS_VALUE_PROTECTED
#define EFL_PART_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "efl_ui_progressbar_private.h"
#include "elm_widget_layout.h"

#include "efl_ui_progressbar_part.eo.h"
#include "elm_part_helper.h"

#define MY_CLASS EFL_UI_PROGRESSBAR_CLASS
#define MY_CLASS_PFX efl_ui_progressbar

#define MY_CLASS_NAME "Efl.Ui.Progressbar"

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

static Efl_Ui_Progress_Status *
_progress_status_new(const char *part_name, double val)
{
   Efl_Ui_Progress_Status *ps;
   ps = calloc(1, sizeof(Efl_Ui_Progress_Status));
   ps->part_name = eina_stringshare_add(part_name);
   ps->val = val;
   return ps;
}

static inline void
_progress_status_free(Efl_Ui_Progress_Status *ps)
{
   eina_stringshare_del(ps->part_name);
   free(ps);
}

static inline Eina_Bool
_is_horizontal(Efl_Ui_Dir dir)
{
   return efl_ui_dir_is_horizontal(dir, EINA_TRUE);
}

static inline Eina_Bool
_is_inverted(Efl_Ui_Dir dir)
{
   if ((dir == EFL_UI_DIR_LEFT) || (dir == EFL_UI_DIR_UP))
     return EINA_TRUE;

   return EINA_FALSE;
}

static Efl_Ui_Dir
_direction_get(Eina_Bool horizontal, Eina_Bool inverted)
{
   if (horizontal)
     {
        if (inverted)
          return EFL_UI_DIR_LEFT;
        else
          return EFL_UI_DIR_RIGHT;
     }
   else
     {
        if (inverted)
          return EFL_UI_DIR_UP;
        else
          return EFL_UI_DIR_DOWN;
     }
}

static void
_units_set(Evas_Object *obj)
{
   EFL_UI_PROGRESSBAR_DATA_GET(obj, sd);

   if (sd->format_cb)
     {
        Eina_Value val;

        eina_value_setup(&val, EINA_VALUE_TYPE_DOUBLE);
        eina_value_set(&val, sd->val);

        // Keeping this bug since the legacy code was like that.
        if (sd->is_legacy_format_string && !sd->is_legacy_format_cb)
          eina_value_set(&val, 100 * sd->val);

        eina_strbuf_reset(sd->format_strbuf);
        sd->format_cb(sd->format_cb_data, sd->format_strbuf, val);

        if (elm_widget_is_legacy(obj))
          elm_layout_text_set(obj, "elm.text.status", eina_strbuf_string_get(sd->format_strbuf));
        else
          elm_layout_text_set(obj, "efl.text.status", eina_strbuf_string_get(sd->format_strbuf));

        eina_value_flush(&val);
     }
   else
     {
        if (elm_widget_is_legacy(obj))
          elm_layout_text_set(obj, "elm.text.status", NULL);
        else
          elm_layout_text_set(obj, "efl.text.status", NULL);
     }
}

static void
_val_set(Evas_Object *obj)
{
   Eina_Bool rtl;
   double pos;
   Efl_Ui_Progress_Status *ps;
   Eina_List *l;

   EFL_UI_PROGRESSBAR_DATA_GET(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   rtl = efl_ui_mirrored_get(obj);

   EINA_LIST_FOREACH(sd->progress_status, l, ps)
     {
        if (EINA_DBL_EQ(ps->val_max, ps->val_min))
          {
             WRN("progressbar min and max are equal.");
             continue;
          }
        pos = (ps->val - ps->val_min)/(ps->val_max - ps->val_min);

        if ((!rtl && _is_inverted(sd->dir)) ||
            (rtl && ((sd->dir == EFL_UI_DIR_UP) ||
                     (sd->dir == EFL_UI_DIR_RIGHT))))
          pos = MAX_RATIO_LVL - pos;

        edje_object_part_drag_value_set
              (wd->resize_obj, ps->part_name, pos, pos);
     }
}

EOLIAN static void
_efl_ui_progressbar_elm_layout_sizing_eval(Eo *obj, Efl_Ui_Progressbar_Data *_pd EINA_UNUSED)
{
   Evas_Coord minw = -1, minh = -1;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   edje_object_size_min_restricted_calc
     (wd->resize_obj, &minw, &minh, minw, minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, -1, -1);
}

//TODO: efl_ui_slider also use this.
static const char *
_theme_group_modify_pos_get(const char *cur_group, const char *search, size_t len, Eina_Bool is_legacy)
{
   const char *pos = NULL;
   const char *temp_str = NULL;

   if (is_legacy)
     return cur_group;

   temp_str = cur_group + len - strlen(search);
   if (temp_str >= cur_group)
     {
         if (!strcmp(temp_str, search))
           pos = temp_str;
     }

   return pos;
}

static char *
_efl_ui_progressbar_theme_group_get(Evas_Object *obj, Efl_Ui_Progressbar_Data *sd)
{
   const char *pos = NULL;
   const char *cur_group = elm_widget_theme_element_get(obj);
   Eina_Strbuf *new_group = eina_strbuf_new();
   Eina_Bool is_legacy = elm_widget_is_legacy(obj);
   size_t len = 0;

   if (cur_group)
     {
        len = strlen(cur_group);
        pos = _theme_group_modify_pos_get(cur_group, "horizontal", len, is_legacy);
        if (!pos)
          pos = _theme_group_modify_pos_get(cur_group, "vertical", len, is_legacy);


        // TODO: change separator when it is decided.
        //       can skip when prev_group == cur_group
        if (!pos)
          {
              eina_strbuf_append(new_group, cur_group);
              eina_strbuf_append(new_group, "/");
          }
        else
          {
              eina_strbuf_append_length(new_group, cur_group, pos - cur_group);
          }
     }

   if (_is_horizontal(sd->dir))
     eina_strbuf_append(new_group, "horizontal");
   else
     eina_strbuf_append(new_group, "vertical");

   return eina_strbuf_release(new_group);
}

EOLIAN static Efl_Ui_Theme_Apply
_efl_ui_progressbar_efl_ui_widget_theme_apply(Eo *obj, Efl_Ui_Progressbar_Data *sd)
{
   Efl_Ui_Theme_Apply int_ret = EFL_UI_THEME_APPLY_FAILED;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EFL_UI_THEME_APPLY_FAILED);
   char *group;

   group = _efl_ui_progressbar_theme_group_get(obj, sd);
   if (group)
     {
        elm_widget_theme_element_set(obj, group);
        free(group);
     }

   int_ret = efl_ui_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (!int_ret) return EFL_UI_THEME_APPLY_FAILED;

   if (elm_widget_is_legacy(obj))
     {
        if (sd->pulse)
          elm_layout_signal_emit(obj, "elm,state,pulse", "elm");
        else
          elm_layout_signal_emit(obj, "elm,state,fraction", "elm");

        if (sd->pulse_state)
          elm_layout_signal_emit(obj, "elm,state,pulse,start", "elm");

        if (sd->format_cb && (!sd->pulse))
          elm_layout_signal_emit(obj, "elm,state,units,visible", "elm");
     }
   else
     {
        if (sd->pulse)
          elm_layout_signal_emit(obj, "efl,state,pulse", "efl");
        else
          elm_layout_signal_emit(obj, "efl,state,fraction", "efl");

        if (sd->pulse_state)
          elm_layout_signal_emit(obj, "efl,state,pulse,start", "efl");

        if (sd->format_cb && (!sd->pulse))
          elm_layout_signal_emit(obj, "efl,state,units,visible", "efl");
     }

   if (_is_horizontal(sd->dir))
     evas_object_size_hint_min_set
       (sd->spacer, (double)sd->size * efl_gfx_entity_scale_get(obj) *
       elm_config_scale_get(), 1);
   else
     evas_object_size_hint_min_set
       (sd->spacer, 1, (double)sd->size * efl_gfx_entity_scale_get(obj) *
       elm_config_scale_get());

   if (elm_widget_is_legacy(obj))
     {
        if (_is_inverted(sd->dir))
          elm_layout_signal_emit(obj, "elm,state,inverted,on", "elm");
        else
          elm_layout_signal_emit(obj, "elm,state,inverted,off", "elm");
     }
   else
     {
        if (_is_inverted(sd->dir))
          elm_layout_signal_emit(obj, "efl,state,inverted,on", "efl");
        else
          elm_layout_signal_emit(obj, "efl,state,inverted,off", "efl");
     }

   _units_set(obj);
   _val_set(obj);

   edje_object_message_signal_process(wd->resize_obj);

   elm_layout_sizing_eval(obj);

   return int_ret;
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

   const char *txt;
   if (elm_widget_is_legacy(obj))
     txt = elm_layout_text_get(obj, "elm.text.status");
   else
     txt = elm_layout_text_get(obj, "efl.text.status");

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
_efl_ui_progressbar_efl_canvas_group_group_add(Eo *obj, Efl_Ui_Progressbar_Data *priv)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   char *group;

   if (!elm_widget_theme_klass_get(obj))
     elm_widget_theme_klass_set(obj, "progressbar");
   efl_canvas_group_add(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);

   priv->dir = EFL_UI_DIR_RIGHT;
   priv->val = MIN_RATIO_LVL;
   priv->val_max = 1.0;
   group = _efl_ui_progressbar_theme_group_get(obj, priv);

   if (!elm_widget_theme_object_set(obj, wd->resize_obj,
                                    elm_widget_theme_klass_get(obj),
                                    group,
                                    elm_widget_theme_style_get(obj)))
     CRI("Failed to set layout!");

   free(group);

   efl_ui_format_string_set(obj, "%.0f%%");

   priv->spacer = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_color_set(priv->spacer, 0, 0, 0, 0);
   evas_object_pass_events_set(priv->spacer, EINA_TRUE);

   if (elm_widget_is_legacy(obj))
     elm_layout_content_set(obj, "elm.swallow.bar", priv->spacer);
   else
     elm_layout_content_set(obj, "efl.bar", priv->spacer);

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
_efl_ui_progressbar_efl_canvas_group_group_del(Eo *obj, Efl_Ui_Progressbar_Data *sd)
{
   Efl_Ui_Progress_Status *progress_obj;

   if (sd->progress_status)
      {
         EINA_LIST_FREE(sd->progress_status, progress_obj)
           {
              _progress_status_free(progress_obj);
           }
      }

   efl_ui_format_cb_set(obj, NULL, NULL, NULL);
   eina_strbuf_free(sd->format_strbuf);

   efl_canvas_group_del(efl_super(obj, MY_CLASS));
}

EOLIAN static Eo *
_efl_ui_progressbar_efl_object_constructor(Eo *obj, Efl_Ui_Progressbar_Data *_pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   evas_object_smart_callbacks_descriptions_set(obj, _smart_callbacks);
   efl_access_object_role_set(obj, EFL_ACCESS_ROLE_PROGRESS_BAR);
   efl_ui_range_min_max_set(obj, 0.0, 1.0);
   return obj;
}

EOLIAN static void
_efl_ui_progressbar_pulse_mode_set(Eo *obj EINA_UNUSED, Efl_Ui_Progressbar_Data *sd, Eina_Bool pulse)
{
   pulse = !!pulse;
   if (sd->pulse == pulse) return;

   sd->pulse = pulse;

   efl_ui_widget_theme_apply(obj);
}

EOLIAN static Eina_Bool
_efl_ui_progressbar_pulse_mode_get(const Eo *obj EINA_UNUSED, Efl_Ui_Progressbar_Data *sd)
{
   return sd->pulse;
}

EOLIAN static void
_efl_ui_progressbar_efl_ui_direction_direction_set(Eo *obj, Efl_Ui_Progressbar_Data *sd, Efl_Ui_Dir dir)
{
   if (sd->dir == dir) return;

   sd->dir = dir;

   efl_ui_widget_theme_apply(obj);
}

EOLIAN static Efl_Ui_Dir
_efl_ui_progressbar_efl_ui_direction_direction_get(const Eo *obj EINA_UNUSED, Efl_Ui_Progressbar_Data *sd)
{
   return sd->dir;
}

static void
_progressbar_span_size_set(Eo *obj, Efl_Ui_Progressbar_Data *sd, Evas_Coord size)
{
   if (sd->size == size) return;

   sd->size = size;

   if (_is_horizontal(sd->dir))
     evas_object_size_hint_min_set
       (sd->spacer, (double)sd->size * efl_gfx_entity_scale_get(obj) *
       elm_config_scale_get(), 1);
   else
     evas_object_size_hint_min_set
       (sd->spacer, 1, (double)sd->size * efl_gfx_entity_scale_get(obj) *
       elm_config_scale_get());

   elm_layout_sizing_eval(obj);
}

static void
_progress_part_min_max_set(Eo *obj, Efl_Ui_Progressbar_Data *sd, const char *part_name, double min, double max)
{
   Efl_Ui_Progress_Status *ps;
   Eina_Bool  existing_ps = EINA_FALSE;
   Eina_List *l;

   if (EINA_DBL_EQ(min, max))
     {
        ERR("min & max provided are equal.");
        return;
     }

   if (min > max)
     {
        WRN("min is greater than max.");
     }

   if (elm_widget_is_legacy(obj))
     {
        if (!strcmp(part_name, "elm.cur.progressbar"))
          {
             sd->val_min = min;
             sd->val_max = max;
          }
     }
   else
     {
        if (!strcmp(part_name, "efl.cur.progressbar"))
          {
             sd->val_min = min;
             sd->val_max = max;
          }
     }

   EINA_LIST_FOREACH(sd->progress_status, l, ps)
     {
        if (!strcmp(ps->part_name, part_name))
          {
             existing_ps = EINA_TRUE;
             ps->val_min = min;
             ps->val_max = max;
             break;
          }
     }
    if (!existing_ps)
    {
      ps = _progress_status_new(part_name, min);
      ps->val_min = min;
      ps->val_max = max;
      sd->progress_status = eina_list_append(sd->progress_status, ps);
    }
    _val_set(obj);
}

static void
_progressbar_part_value_set(Eo *obj, Efl_Ui_Progressbar_Data *sd, const char *part_name, double val)
{
   Efl_Ui_Progress_Status *ps;
   Eina_Bool  existing_ps = EINA_FALSE;
   Eina_List *l;
   double min = 0.0, max = 0.0;

   efl_ui_range_min_max_get(efl_part(obj, part_name), &min, &max);

   if (val < min) val = min;
   if (val > max) val = max;

   if (elm_widget_is_legacy(obj))
     {
        if (!strcmp(part_name, "elm.cur.progressbar"))
          sd->val = val;
     }
   else
     {
        if (!strcmp(part_name, "efl.cur.progressbar"))
          sd->val = val;
     }

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
         ps->val_min = 0.0;
         ps->val_max = 1.0;
         ps->val = val;
         sd->progress_status = eina_list_append(sd->progress_status, ps);
      }
   else
      ps->val = val;

   _val_set(obj);
   _units_set(obj);
   efl_event_callback_legacy_call
     (obj, EFL_UI_PROGRESSBAR_EVENT_CHANGED, NULL);
}

static double
_progressbar_part_value_get(Efl_Ui_Progressbar_Data *sd, const char* part)
{
   Efl_Ui_Progress_Status *ps;
   Eina_List *l;

   EINA_LIST_FOREACH(sd->progress_status, l, ps)
     {
        if (!strcmp(ps->part_name, part)) return ps->val;
     }

   return 0.0;
}

EOLIAN static void
_efl_ui_progressbar_efl_ui_range_range_value_set(Eo *obj, Efl_Ui_Progressbar_Data *sd, double val)
{
   if (EINA_DBL_EQ(sd->val, val)) return;

   if (elm_widget_is_legacy(obj))
     _progressbar_part_value_set(obj, sd, "elm.cur.progressbar", val);
   else
     _progressbar_part_value_set(obj, sd, "efl.cur.progressbar", val);
}

EOLIAN static double
_efl_ui_progressbar_efl_ui_range_range_value_get(const Eo *obj, Efl_Ui_Progressbar_Data *sd EINA_UNUSED)
{
   if (elm_widget_is_legacy(obj))
     return efl_ui_range_value_get(efl_part(obj, "elm.cur.progressbar"));
   else
     return efl_ui_range_value_get(efl_part(obj, "efl.cur.progressbar"));
}

EOLIAN static void
_efl_ui_progressbar_efl_ui_format_format_cb_set(Eo *obj, Efl_Ui_Progressbar_Data *sd, void *func_data, Efl_Ui_Format_Func_Cb func, Eina_Free_Cb func_free_cb)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (sd->format_cb_data == func_data && sd->format_cb == func)
     return;

   if (sd->format_cb_data && sd->format_free_cb)
     sd->format_free_cb(sd->format_cb_data);

   sd->format_cb = func;
   sd->format_cb_data = func_data;
   sd->format_free_cb = func_free_cb;
   if (!sd->format_strbuf) sd->format_strbuf = eina_strbuf_new();

   if (elm_widget_is_legacy(obj))
     elm_layout_signal_emit(obj, "elm,state,units,visible", "elm");
   else
     elm_layout_signal_emit(obj, "efl,state,units,visible", "efl");
   edje_object_message_signal_process(wd->resize_obj);

   _units_set(obj);
   elm_layout_sizing_eval(obj);
}

EOLIAN static void
_efl_ui_progressbar_pulse_set(Eo *obj, Efl_Ui_Progressbar_Data *sd, Eina_Bool state)
{
   state = !!state;
   if ((!sd->pulse) || (sd->pulse_state == state)) return;

   sd->pulse_state = state;

   if (elm_widget_is_legacy(obj))
     {
        if (sd->pulse_state)
          elm_layout_signal_emit(obj, "elm,state,pulse,start", "elm");
        else
          elm_layout_signal_emit(obj, "elm,state,pulse,stop", "elm");
     }
   else
     {
        if (sd->pulse_state)
          elm_layout_signal_emit(obj, "efl,state,pulse,start", "efl");
        else
          elm_layout_signal_emit(obj, "efl,state,pulse,stop", "efl");
     }
}

EOLIAN static Eina_Bool
_efl_ui_progressbar_pulse_get(const Eo *obj EINA_UNUSED, Efl_Ui_Progressbar_Data *sd)
{
   return (sd->pulse_state && sd->pulse);
}

EOLIAN static void
_efl_ui_progressbar_efl_ui_range_range_min_max_set(Eo *obj, Efl_Ui_Progressbar_Data *sd, double min, double max)
{
  if (elm_widget_is_legacy(obj))
    _progress_part_min_max_set(obj, sd, "elm.cur.progressbar", min, max);
  else
    _progress_part_min_max_set(obj, sd, "efl.cur.progressbar", min, max);
}

EOLIAN static void
_efl_ui_progressbar_efl_ui_range_range_min_max_get(const Eo *obj EINA_UNUSED, Efl_Ui_Progressbar_Data *sd, double *min, double *max)
{
   if (min) *min = sd->val_min;
   if (max) *max = sd->val_max;
}

/* Efl.Part begin */

EOLIAN static Eo *
_efl_ui_progressbar_efl_part_part_get(const Eo *obj, Efl_Ui_Progressbar_Data *sd EINA_UNUSED, const char *part)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(part, NULL);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   // Progress bars are dragable types
   if (edje_object_part_drag_dir_get(wd->resize_obj, part) != EFL_UI_DRAG_DIR_NONE)
     return ELM_PART_IMPLEMENT(EFL_UI_PROGRESSBAR_PART_CLASS, obj, part);

   return efl_part_get(efl_super(obj, MY_CLASS), part);
}

EOLIAN static void
_efl_ui_progressbar_part_efl_ui_range_range_value_set(Eo *obj, void *_pd EINA_UNUSED, double val)
{
  Elm_Part_Data *pd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
  Efl_Ui_Progressbar_Data *sd = efl_data_scope_get(pd->obj, EFL_UI_PROGRESSBAR_CLASS);

  _progressbar_part_value_set(pd->obj, sd, pd->part, val);
}

EOLIAN static double
_efl_ui_progressbar_part_efl_ui_range_range_value_get(const Eo *obj, void *_pd EINA_UNUSED)
{
   Elm_Part_Data *pd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   Efl_Ui_Progressbar_Data *sd = efl_data_scope_get(pd->obj, EFL_UI_PROGRESSBAR_CLASS);

   return _progressbar_part_value_get(sd, pd->part);
}

EOLIAN static void
_efl_ui_progressbar_efl_access_value_value_and_text_get(const Eo *obj EINA_UNUSED, Efl_Ui_Progressbar_Data *_pd, double *value, const char **text EINA_UNUSED)
{
   if (value) *value = _pd->val;
}

EOLIAN static void
_efl_ui_progressbar_part_efl_ui_range_range_min_max_set(Eo *obj, void *_pd EINA_UNUSED, double min, double max)
{
  Elm_Part_Data *pd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
  Efl_Ui_Progressbar_Data *sd = efl_data_scope_get(pd->obj, EFL_UI_PROGRESSBAR_CLASS);

  _progress_part_min_max_set(pd->obj, sd, pd->part, min, max);
}

EOLIAN static void
_efl_ui_progressbar_part_efl_ui_range_range_min_max_get(const Eo *obj, void *_pd EINA_UNUSED, double *min, double *max)
{
   Efl_Ui_Progress_Status *ps;
   Eina_List *l;

   Elm_Part_Data *pd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   Efl_Ui_Progressbar_Data *sd = efl_data_scope_get(pd->obj, EFL_UI_PROGRESSBAR_CLASS);

   EINA_LIST_FOREACH(sd->progress_status, l, ps)
     {
        if (!strcmp(ps->part_name, pd->part))
          {
             if (min) *min = ps->val_min;
             if (max) *max = ps->val_max;
             break;
          }
     }
}

#include "efl_ui_progressbar_part.eo.c"

/* Efl.Part end */

/* Internal EO APIs and hidden overrides */
ELM_PART_TEXT_DEFAULT_IMPLEMENT(efl_ui_progressbar, Efl_Ui_Progressbar_Data)
ELM_PART_MARKUP_DEFAULT_IMPLEMENT(efl_ui_progressbar, Efl_Ui_Progressbar_Data)
ELM_PART_CONTENT_DEFAULT_IMPLEMENT(efl_ui_progressbar, Efl_Ui_Progressbar_Data)

ELM_LAYOUT_CONTENT_ALIASES_IMPLEMENT(efl_ui_progressbar)

#define EFL_UI_PROGRESSBAR_EXTRA_OPS \
   ELM_LAYOUT_CONTENT_ALIASES_OPS(efl_ui_progressbar), \
   ELM_LAYOUT_SIZING_EVAL_OPS(efl_ui_progressbar), \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(efl_ui_progressbar)

#include "efl_ui_progressbar.eo.c"

#include "efl_ui_progressbar_legacy.eo.h"
#include "efl_ui_progressbar_legacy_part.eo.h"

#define MY_CLASS_NAME_LEGACY "elm_progressbar"

static void
_efl_ui_progressbar_legacy_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

EOLIAN static Eo *
_efl_ui_progressbar_legacy_efl_object_constructor(Eo *obj, void *pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, EFL_UI_PROGRESSBAR_LEGACY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);
   return obj;
}

/* FIXME: replicated from elm_layout just because progressbar's icon spot
 * is elm.swallow.content, not elm.swallow.icon. Fix that whenever we
 * can changed the theme API */
static void
_icon_signal_emit(Evas_Object *obj)
{
   char buf[64];

   if (!elm_widget_resize_object_get(obj)) return;
   snprintf(buf, sizeof(buf), "elm,state,icon,%s",
            elm_layout_content_get(obj, "icon") ? "visible" : "hidden");

   elm_layout_signal_emit(obj, buf, "elm");
   edje_object_message_signal_process(elm_layout_edje_get(obj));
   elm_layout_sizing_eval(obj);
}

/* FIXME: replicated from elm_layout just because progressbar's icon spot
 * is elm.swallow.content, not elm.swallow.icon. Fix that whenever we
 * can changed the theme API */
EOLIAN static Efl_Ui_Theme_Apply
_efl_ui_progressbar_legacy_efl_ui_widget_theme_apply(Eo *obj, void *_pd EINA_UNUSED)
{
   Efl_Ui_Theme_Apply int_ret = EFL_UI_THEME_APPLY_FAILED;

   int_ret = efl_ui_widget_theme_apply(efl_super(obj, EFL_UI_PROGRESSBAR_LEGACY_CLASS));
   if (!int_ret) return EFL_UI_THEME_APPLY_FAILED;
   _icon_signal_emit(obj);

   return int_ret;
}

/* FIXME: replicated from elm_layout just because progressbar's icon spot
 * is elm.swallow.content, not elm.swallow.icon. Fix that whenever we
 * can changed the theme API */
EOLIAN static Eina_Bool
_efl_ui_progressbar_legacy_efl_ui_widget_widget_sub_object_del(Eo *obj, void *_pd EINA_UNUSED, Evas_Object *sobj)
{
   Eina_Bool int_ret = EINA_FALSE;

   int_ret = elm_widget_sub_object_del(efl_super(obj, EFL_UI_PROGRESSBAR_LEGACY_CLASS), sobj);
   if (!int_ret) return EINA_FALSE;

   _icon_signal_emit(obj);

   return EINA_TRUE;
}

/* FIXME: replicated from elm_layout just because progressbar's icon spot
 * is elm.swallow.content, not elm.swallow.icon. Fix that whenever we
 * can changed the theme API */
static Eina_Bool
_efl_ui_progressbar_legacy_content_set(Eo *obj, void *_pd EINA_UNUSED, const char *part, Evas_Object *content)
{
   Eina_Bool int_ret = EINA_FALSE;

   int_ret = efl_content_set(efl_part(efl_super(obj, EFL_UI_PROGRESSBAR_LEGACY_CLASS), part), content);
   if (!int_ret) return EINA_FALSE;

   _icon_signal_emit(obj);

   return EINA_TRUE;
}

/* Efl.Part for legacy begin */

static Eina_Bool
_part_is_efl_ui_progressbar_legacy_part(const Eo *obj EINA_UNUSED, const char *part)
{
   return eina_streq(part, "elm.swallow.content");
}

ELM_PART_OVERRIDE_PARTIAL(efl_ui_progressbar_legacy, EFL_UI_PROGRESSBAR_LEGACY, void, _part_is_efl_ui_progressbar_legacy_part)
ELM_PART_OVERRIDE_CONTENT_SET_NO_SD(efl_ui_progressbar_legacy)
#include "efl_ui_progressbar_legacy_part.eo.c"

/* Efl.Part for legacy end */

EAPI Evas_Object *
elm_progressbar_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Eo *obj = elm_legacy_add(EFL_UI_PROGRESSBAR_LEGACY_CLASS, parent);
   elm_progressbar_unit_format_set(obj, "%.0f%%");

   return obj;
}

EAPI void
elm_progressbar_pulse_set(Evas_Object *obj, Eina_Bool pulse)
{
   efl_ui_progressbar_pulse_mode_set(obj, pulse);
}

EAPI Eina_Bool
elm_progressbar_pulse_get(const Evas_Object *obj)
{
   return efl_ui_progressbar_pulse_mode_get(obj);
}

EAPI void
elm_progressbar_pulse(Evas_Object *obj, Eina_Bool state)
{
   efl_ui_progressbar_pulse_set(obj, state);
}

EAPI Eina_Bool
elm_progressbar_is_pulsing_get(const Evas_Object *obj)
{
   return efl_ui_progressbar_pulse_get(obj);
}

EAPI void
elm_progressbar_part_value_set(Evas_Object *obj, const char *part, double val)
{
   if (EINA_DBL_EQ(efl_ui_range_value_get(efl_part(obj, part)), val)) return;
   efl_ui_range_value_set(efl_part(obj, part), val);
}

EAPI double
elm_progressbar_part_value_get(const Evas_Object *obj, const char *part)
{
   return efl_ui_range_value_get(efl_part(obj, part));
}

EAPI Eina_Bool
elm_progressbar_horizontal_get(const Evas_Object *obj)
{
   EFL_UI_PROGRESSBAR_DATA_GET_OR_RETURN(obj, sd, EINA_FALSE);

   return _is_horizontal(sd->dir);
}

EAPI void
elm_progressbar_inverted_set(Evas_Object *obj, Eina_Bool inverted)
{
   Efl_Ui_Dir dir;
   EFL_UI_PROGRESSBAR_DATA_GET_OR_RETURN(obj, sd);

   dir = _direction_get(_is_horizontal(sd->dir), inverted);

   efl_ui_direction_set(obj, dir);
}

EAPI Eina_Bool
elm_progressbar_inverted_get(const Evas_Object *obj)
{
   EFL_UI_PROGRESSBAR_DATA_GET_OR_RETURN(obj, sd, EINA_FALSE);

   return _is_inverted(sd->dir);
}

EAPI void
elm_progressbar_horizontal_set(Evas_Object *obj, Eina_Bool horizontal)
{
   Efl_Ui_Dir dir;
   EFL_UI_PROGRESSBAR_DATA_GET_OR_RETURN(obj, sd);

   dir = _direction_get(horizontal, _is_inverted(sd->dir));

   efl_ui_direction_set(obj, dir);
}

typedef struct
{
   progressbar_func_type format_cb;
   progressbar_freefunc_type format_free_cb;
} Pb_Format_Wrapper_Data;

static void
_format_legacy_to_format_eo_cb(void *data, Eina_Strbuf *str, const Eina_Value value)
{
   Pb_Format_Wrapper_Data *pfwd = data;
   char *buf = NULL;
   double val = 0;
   const Eina_Value_Type *type = eina_value_type_get(&value);

   if (type == EINA_VALUE_TYPE_DOUBLE)
     eina_value_get(&value, &val);

   if (pfwd->format_cb)
     buf = pfwd->format_cb(val);
   if (buf)
     eina_strbuf_append(str, buf);
   if (pfwd->format_free_cb) pfwd->format_free_cb(buf);
}

static void
_format_legacy_to_format_eo_free_cb(void *data)
{
   Pb_Format_Wrapper_Data *pfwd = data;
   free(pfwd);
}

EAPI void
elm_progressbar_unit_format_function_set(Evas_Object *obj, progressbar_func_type func, progressbar_freefunc_type free_func)
{
   EFL_UI_PROGRESSBAR_DATA_GET_OR_RETURN(obj, sd);
   Pb_Format_Wrapper_Data *pfwd = malloc(sizeof(Pb_Format_Wrapper_Data));

   pfwd->format_cb = func;
   pfwd->format_free_cb = free_func;
   sd->is_legacy_format_cb = EINA_TRUE;

   efl_ui_format_cb_set(obj, pfwd, _format_legacy_to_format_eo_cb,
                        _format_legacy_to_format_eo_free_cb);
}

EAPI void
elm_progressbar_span_size_set(Evas_Object *obj, Evas_Coord size)
{
   EFL_UI_PROGRESSBAR_DATA_GET_OR_RETURN(obj, sd);
   _progressbar_span_size_set(obj, sd, size);
}

EAPI Evas_Coord
elm_progressbar_span_size_get(const Evas_Object *obj)
{
   EFL_UI_PROGRESSBAR_DATA_GET_OR_RETURN(obj, sd, 0);
   return sd->size;
}

EAPI void
elm_progressbar_unit_format_set(Evas_Object *obj, const char *units)
{
   EFL_UI_PROGRESSBAR_DATA_GET_OR_RETURN(obj, sd);

   sd->is_legacy_format_string = EINA_TRUE;
   efl_ui_format_string_set(obj, units);
}

EAPI const char *
elm_progressbar_unit_format_get(const Evas_Object *obj)
{
   return efl_ui_format_string_get(obj);
}

EAPI void
elm_progressbar_value_set(Evas_Object *obj, double val)
{
   efl_ui_range_value_set(obj, val);
}

EAPI double
elm_progressbar_value_get(const Evas_Object *obj)
{
   return efl_ui_range_value_get(obj);
}

#include "efl_ui_progressbar_legacy.eo.c"
