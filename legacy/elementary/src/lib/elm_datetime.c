#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_datetime.h"

#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
#include "elm_interface_atspi_accessible.h"
#include "elm_interface_atspi_accessible.eo.h"

#define MY_CLASS ELM_DATETIME_CLASS

#define MY_CLASS_NAME "Elm_Datetime"
#define MY_CLASS_NAME_LEGACY "elm_datetime"

#ifdef HAVE_LOCALE_H
# include <locale.h>
#endif

#ifdef HAVE_LANGINFO_H
# include <langinfo.h>
#endif

#define MAX_SEPARATOR_LEN              6
#define MIN_DAYS_IN_MONTH              28
#define BUFFER_SIZE                    1024

/* interface between EDC & C code (field & signal names). values 0 to
 * ELM_DATETIME_TYPE_COUNT are in the valid range, and must get in the
 * place of "%d".
 */
#define EDC_DATETIME_FOCUSIN_SIG_STR   "elm,action,focus"
#define EDC_DATETIME_FOCUSOUT_SIG_STR  "elm,action,unfocus"
#define EDC_PART_FIELD_STR             "field%d"
#define EDC_PART_SEPARATOR_STR         "separator%d"
#define EDC_PART_FIELD_ENABLE_SIG_STR  "field%d,enable"
#define EDC_PART_FIELD_DISABLE_SIG_STR "field%d,disable"

/* struct tm does not define the fields in the order year, month,
 * date, hour, minute. values are reassigned to an array for easy
 * handling.
 */
#define DATETIME_TM_ARRAY(intptr, tmptr) \
  int *intptr[] = {                      \
     &(tmptr)->tm_year,                  \
     &(tmptr)->tm_mon,                   \
     &(tmptr)->tm_mday,                  \
     &(tmptr)->tm_hour,                  \
     &(tmptr)->tm_min}

// default limits for individual fields
static Format_Map mapping[ELM_DATETIME_TYPE_COUNT] = {
   [ELM_DATETIME_YEAR] = { "Yy", -1, -1, "" },
   [ELM_DATETIME_MONTH] = { "mbBh", 0, 11, "" },
   [ELM_DATETIME_DATE] = { "de", 1, 31, "" },
   [ELM_DATETIME_HOUR] = { "IHkl", 0, 23, "" },
   [ELM_DATETIME_MINUTE] = { "M", 0, 59, ":" },
   [ELM_DATETIME_AMPM] = { "pP", 0, 1, "" }
};

static const char *multifield_formats = "cxXrRTDF";
static const char *ignore_separators = "()";
static Datetime_Mod_Api *dt_mod = NULL;

static const char SIG_CHANGED[] = "changed";
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_CHANGED, ""},
   {SIG_WIDGET_LANG_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_WIDGET_ACCESS_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_LAYOUT_FOCUSED, ""}, /**< handled by elm_layout */
   {SIG_LAYOUT_UNFOCUSED, ""}, /**< handled by elm_layout */
   {NULL, NULL}
};

static Datetime_Mod_Api *
_dt_mod_init()
{
   Elm_Module *mod = NULL;

   if (!(mod = _elm_module_find_as("datetime/api"))) return NULL;

   mod->api = malloc(sizeof(Datetime_Mod_Api));
   if (!mod->api) return NULL;

   ((Datetime_Mod_Api *)(mod->api))->obj_hook =
     _elm_module_symbol_get(mod, "obj_hook");
   ((Datetime_Mod_Api *)(mod->api))->obj_unhook =
     _elm_module_symbol_get(mod, "obj_unhook");
   ((Datetime_Mod_Api *)(mod->api))->obj_hide =
     _elm_module_symbol_get(mod, "obj_hide");
   ((Datetime_Mod_Api *)(mod->api))->field_create =
     _elm_module_symbol_get(mod, "field_create");
   ((Datetime_Mod_Api *)(mod->api))->field_value_display =
     _elm_module_symbol_get(mod, "field_value_display");

   return mod->api;
}

static void
_field_list_display(Evas_Object *obj)
{
   Datetime_Field *field;
   unsigned int idx = 0;

   ELM_DATETIME_DATA_GET(obj, sd);

   if (!dt_mod || !dt_mod->field_value_display) return;

   for (idx = 0; idx < ELM_DATETIME_TYPE_COUNT; idx++)
     {
        field = sd->field_list + idx;
        if (field->fmt_exist && field->visible)
          dt_mod->field_value_display(sd->mod_data, field->item_obj);
     }
}

// FIXME: provide nl_langinfo on Windows if possible
// returns expanded format string for corresponding multi-field format character
static char *
_expanded_fmt_str_get(char ch)
{
   char *exp_fmt = "";
   switch (ch)
     {
      case 'c':
#ifdef HAVE_LANGINFO_H
        exp_fmt = nl_langinfo(D_T_FMT);
#else
        exp_fmt = "";
#endif
        break;

      case 'x':
#ifdef HAVE_LANGINFO_H
        exp_fmt = nl_langinfo(D_FMT);
#else
        exp_fmt = "";
#endif
        break;

      case 'X':
#ifdef HAVE_LANGINFO_H
        exp_fmt = nl_langinfo(T_FMT);
#else
        exp_fmt = "";
#endif
        break;

      case 'r':
#ifdef HAVE_LANGINFO_H
        exp_fmt = nl_langinfo(T_FMT_AMPM);
#else
        exp_fmt = "";
#endif
        break;

      case 'R':
        exp_fmt = "%H:%M";
        break;

      case 'T':
        exp_fmt = "%H:%M:%S";
        break;

      case 'D':
        exp_fmt = "%m/%d/%y";
        break;

      case 'F':
        exp_fmt = "%Y-%m-%d";
        break;

      default:
        exp_fmt = "";
        break;
     }

   return exp_fmt;
}

static void
_expand_format(char *dt_fmt)
{
   char *ptr, *expanded_fmt, ch;
   unsigned int idx = 0, len = 0;
   char buf[ELM_DATETIME_MAX_FORMAT_LEN] = {0, };
   Eina_Bool fmt_char = EINA_FALSE;

   ptr = dt_fmt;
   while ((ch = *ptr))
     {
        if ((fmt_char) && (strchr(multifield_formats, ch)))
          {
             /* replace the multi-field format characters with
              * corresponding expanded format */
             expanded_fmt = _expanded_fmt_str_get(ch);
             len = strlen(expanded_fmt);
             buf[--idx] = 0;
             strncat(buf, expanded_fmt, len);
             idx += len;
          }
        else buf[idx++] = ch;

        if (ch == '%') fmt_char = EINA_TRUE;
        else fmt_char = EINA_FALSE;

        ptr++;
     }

   buf[idx] = 0;
   strncpy(dt_fmt, buf, ELM_DATETIME_MAX_FORMAT_LEN);
}

static void
_field_list_arrange(Evas_Object *obj)
{
   Datetime_Field *field;
   char buf[BUFFER_SIZE];
   int idx;
   Eina_Bool freeze;

   ELM_DATETIME_DATA_GET(obj, sd);

   freeze = sd->freeze_sizing;
   sd->freeze_sizing = EINA_TRUE;
   for (idx = 0; idx < ELM_DATETIME_TYPE_COUNT; idx++)
     {
        field = sd->field_list + idx;
        snprintf(buf, sizeof(buf), EDC_PART_FIELD_STR, field->location);

        if (field->visible && field->fmt_exist)
          {
             elm_layout_content_unset(obj, buf);
             elm_layout_content_set(obj, buf, field->item_obj);
          }
        else
          evas_object_hide(elm_layout_content_unset(obj, buf));
     }
   sd->freeze_sizing = freeze;

   elm_layout_sizing_eval(obj);
   _field_list_display(obj);
}

static unsigned int
_parse_format(Evas_Object *obj,
              char *fmt_ptr)
{
   Eina_Bool fmt_parsing = EINA_FALSE, sep_parsing = EINA_FALSE,
             sep_lookup = EINA_FALSE;
   unsigned int len = 0, idx = 0, location = 0;
   char separator[MAX_SEPARATOR_LEN];
   Datetime_Field *field = NULL;
   char cur;

   ELM_DATETIME_DATA_GET(obj, sd);

   while ((cur = *fmt_ptr))
     {
        if (fmt_parsing)
          {
             fmt_parsing = EINA_FALSE;
             for (idx = 0; idx < ELM_DATETIME_TYPE_COUNT; idx++)
               {
                  if (strchr(mapping[idx].fmt_char, cur))
                    {
                       field = sd->field_list + idx;
                       /* ignore the fields already have or disabled
                        * valid formats, means already parsed &
                        * repeated, ignore. */
                       if (field->location != -1) break;
                       field->fmt[1] = cur;
                       field->fmt_exist = EINA_TRUE;
                       field->location = location++;
                       sep_lookup = EINA_TRUE;
                       len = 0;
                       break;
                    }
               }
          }
        if (cur == '%')
          {
             fmt_parsing = EINA_TRUE;
             sep_parsing = EINA_FALSE;
             // set the separator to previous field
             separator[len] = 0;
             if (field) eina_stringshare_replace(&field->separator, separator);
          }
        // ignore the set of chars (global, field specific) as field separators
        if (sep_parsing &&
            (len < MAX_SEPARATOR_LEN - 1) &&
            (field->type != ELM_DATETIME_AMPM) &&
            (!strchr(ignore_separators, cur)) &&
            (!strchr(mapping[idx].ignore_sep, cur)))
          separator[len++] = cur;
        if (sep_lookup) sep_parsing = EINA_TRUE;
        sep_lookup = EINA_FALSE;
        fmt_ptr++;
     }
   // return the number of valid fields parsed.
   return location;
}

static void
_reload_format(Evas_Object *obj)
{
   unsigned int idx, field_count;
   Datetime_Field *field;
   char buf[BUFFER_SIZE];
   char *dt_fmt;

   ELM_DATETIME_DATA_GET(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   // FIXME: provide nl_langinfo on Windows if possible
   // fetch the default format from Libc.
   if (!sd->user_format)
#ifdef HAVE_LANGINFO_H
     strncpy(sd->format, nl_langinfo(D_T_FMT), ELM_DATETIME_MAX_FORMAT_LEN);
#else
     strncpy(sd->format, "", ELM_DATETIME_MAX_FORMAT_LEN);
#endif
   sd->format[ELM_DATETIME_MAX_FORMAT_LEN - 1] = '\0';

   dt_fmt = (char *)malloc(ELM_DATETIME_MAX_FORMAT_LEN);
   if (!dt_fmt) return;

   strncpy(dt_fmt, sd->format, ELM_DATETIME_MAX_FORMAT_LEN);

   _expand_format(dt_fmt);

   // reset all the fields to disable state
   sd->enabled_field_count = 0;
   for (idx = 0; idx < ELM_DATETIME_TYPE_COUNT; idx++)
     {
        field = sd->field_list + idx;
        field->fmt_exist = EINA_FALSE;
        field->location = -1;
     }

   field_count = _parse_format(obj, dt_fmt);
   free(dt_fmt);

   for (idx = 0; idx < ELM_DATETIME_TYPE_COUNT; idx++)
     {
        field = sd->field_list + idx;
        if (field->fmt_exist && field->visible)
          sd->enabled_field_count++;
     }

   // assign locations to disabled fields for uniform usage
   for (idx = 0; idx < ELM_DATETIME_TYPE_COUNT; idx++)
     {
        field = sd->field_list + idx;
        if (field->location == -1) field->location = field_count++;

        if (field->fmt_exist && field->visible)
          {
             snprintf(buf, sizeof(buf), EDC_PART_FIELD_ENABLE_SIG_STR,
                      field->location);
             elm_layout_signal_emit(obj, buf, "elm");
          }
        else
          {
             snprintf(buf, sizeof(buf), EDC_PART_FIELD_DISABLE_SIG_STR,
                      field->location);
             elm_layout_signal_emit(obj, buf, "elm");
          }
        snprintf
          (buf, sizeof(buf), EDC_PART_SEPARATOR_STR, (field->location + 1));
        elm_layout_text_set(obj, buf, field->separator);
     }

   edje_object_message_signal_process(wd->resize_obj);
   _field_list_arrange(obj);
}

EOLIAN static Eina_Bool
_elm_datetime_elm_widget_translate(Eo *obj, Elm_Datetime_Data *sd)
{
   if (!sd->user_format) _reload_format(obj);
   else _field_list_display(obj);

   eo_do_super(obj, MY_CLASS, elm_obj_widget_translate());

   return EINA_TRUE;
}

static Eina_List *
_datetime_items_get(const Evas_Object *obj)
{
   Eina_List *items = NULL;
   Datetime_Field *field;
   unsigned int idx;
   Datetime_Field *sorted_fields[ELM_DATETIME_TYPE_COUNT];

   ELM_DATETIME_DATA_GET(obj, sd);

   for (idx = 0; idx < ELM_DATETIME_TYPE_COUNT; idx++)
     {
        field = sd->field_list + idx;
        sorted_fields[field->location] = field;
     }

   for (idx = 0; idx < ELM_DATETIME_TYPE_COUNT; idx++)
     {
        field = sorted_fields[idx];
        if (field->fmt_exist && field->visible)
          items = eina_list_append(items, field->item_obj);
     }

   // ACCESS
   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
     items = eina_list_append(items, sd->access_obj);

   return items;
}

EOLIAN static Eina_Bool
_elm_datetime_elm_widget_focus_next_manager_is(Eo *obj EINA_UNUSED, Elm_Datetime_Data *_pd EINA_UNUSED)
{
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_datetime_elm_widget_focus_next(Eo *obj, Elm_Datetime_Data *_pd EINA_UNUSED, Elm_Focus_Direction dir, Evas_Object **next)
{
   const Eina_List *items;
   Eina_List *(*list_free)(Eina_List *list);
   void *(*list_data_get)(const Eina_List *list);

   Eina_Bool int_ret;

   if ((items = elm_widget_focus_custom_chain_get(obj)))
     {
        list_data_get = eina_list_data_get;
        list_free = NULL;
     }
   else
     {
        items = _datetime_items_get(obj);
        list_data_get = eina_list_data_get;
        list_free = eina_list_free;
        if (!items) return EINA_FALSE;
     }

   int_ret = elm_widget_focus_list_next_get(obj, items, list_data_get, dir, next);
   if (list_free) list_free((Eina_List *)items);

   return int_ret;
}

EOLIAN static Eina_Bool
_elm_datetime_elm_widget_on_focus(Eo *obj, Elm_Datetime_Data *sd)
{
   Eina_Bool int_ret = EINA_FALSE;

   eo_do_super(obj, MY_CLASS, int_ret = elm_obj_widget_on_focus());
   if (!int_ret) return EINA_FALSE;

   if (!elm_widget_focus_get(obj))
     {
        if ((dt_mod) && (dt_mod->obj_hide))
          dt_mod->obj_hide(sd->mod_data);
     }

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_datetime_elm_widget_disable(Eo *obj, Elm_Datetime_Data *sd)
{
   Datetime_Field *field;
   unsigned int idx = 0;
   Eina_Bool int_ret = EINA_FALSE;

   eo_do_super(obj, MY_CLASS, int_ret = elm_obj_widget_disable());
   if (!int_ret) return EINA_FALSE;

   for (idx = 0; idx < ELM_DATETIME_TYPE_COUNT; idx++)
     {
        field = sd->field_list + idx;
        elm_object_disabled_set(field->item_obj, elm_object_disabled_get(obj));
     }
   return EINA_TRUE;
}

EOLIAN static void
_elm_datetime_elm_layout_sizing_eval(Eo *obj, Elm_Datetime_Data *sd)
{
   Evas_Coord minw = -1, minh = -1;

   if (sd->freeze_sizing) return;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (sd->enabled_field_count)
     elm_coords_finger_size_adjust(sd->enabled_field_count, &minw, 1, &minh);

   edje_object_size_min_restricted_calc
     (wd->resize_obj, &minw, &minh, minw, minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, -1, -1);
}

EOLIAN static Eina_Bool
_elm_datetime_elm_widget_theme_apply(Eo *obj, Elm_Datetime_Data *sd)
{
   Eina_Bool int_ret = EINA_FALSE;

   Datetime_Field *field;
   char buf[BUFFER_SIZE];
   unsigned int idx;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   eo_do_super(obj, MY_CLASS, int_ret = elm_obj_widget_theme_apply());
   if (!int_ret) return EINA_FALSE;

   if ((!dt_mod) || (!dt_mod->field_value_display)) return EINA_TRUE;

   for (idx = 0; idx < ELM_DATETIME_TYPE_COUNT; idx++)
     {
        field = sd->field_list + idx;
        if (field->fmt_exist && field->visible)
          {
             snprintf(buf, sizeof(buf), EDC_PART_FIELD_ENABLE_SIG_STR,
                      field->location);
             elm_layout_signal_emit(obj, buf, "elm");

             snprintf
               (buf, sizeof(buf), EDC_PART_SEPARATOR_STR, field->location);
             elm_layout_text_set(obj, buf, field->separator);

             dt_mod->field_value_display(sd->mod_data, field->item_obj);
          }
        else
          {
             snprintf(buf, sizeof(buf), EDC_PART_FIELD_DISABLE_SIG_STR,
                      field->location);
             elm_layout_signal_emit(obj, buf, "elm");
          }
     }

   edje_object_message_signal_process(wd->resize_obj);
   elm_layout_sizing_eval(obj);

   return EINA_TRUE;
}

static int
_max_days_get(int year,
              int month)
{
   struct tm time1;
   time_t t;
   int day;

   t = time(NULL);
   localtime_r(&t, &time1);
   time1.tm_year = year;
   time1.tm_mon = month;
   for (day = MIN_DAYS_IN_MONTH; day <= mapping[ELM_DATETIME_DATE].def_max;
        day++)
     {
        time1.tm_mday = day;
        mktime(&time1);
        /* To restrict month wrapping because of summer time in some locales,
        * ignore day light saving mode in mktime(). */
        time1.tm_isdst = -1;
        if (time1.tm_mday == 1) break;
     }
   day--;

   return day;
}

static Eina_Bool
_date_cmp(struct tm *time1,
          struct tm *time2)
{
   unsigned int idx;

   DATETIME_TM_ARRAY(timearr1, time1);
   DATETIME_TM_ARRAY(timearr2, time2);

   for (idx = 0; idx < ELM_DATETIME_TYPE_COUNT - 1; idx++)
     {
        if (*timearr1[idx] != *timearr2[idx])
          return EINA_FALSE;
     }

   return EINA_TRUE;
}

// validates curr_time/min_limt/max_limit according to the newly set value
static void
_validate_datetime_limits(struct tm *time1,
                          struct tm *time2,
                          Eina_Bool swap)
{
   struct tm *t1, *t2;
   unsigned int idx;

   if (!time1 || !time2) return;

   t1 = (swap) ? time2 : time1;
   t2 = (swap) ? time1 : time2;

   DATETIME_TM_ARRAY(timearr1, time1);
   DATETIME_TM_ARRAY(timearr2, time2);
   for (idx = 0; idx < ELM_DATETIME_TYPE_COUNT - 1; idx++)
     {
        if (*timearr1[idx] < *timearr2[idx])
          {
             memcpy(t1, t2, sizeof(struct tm));
             break;
          }
        else if (*timearr1[idx] > *timearr2[idx])
          break;
     }
}

static void
_apply_field_limits(Evas_Object *obj)
{
   Datetime_Field *field;
   unsigned int idx = 0;
   int val;

   ELM_DATETIME_DATA_GET(obj, sd);

   DATETIME_TM_ARRAY(timearr, &sd->curr_time);
   for (idx = 0; idx < ELM_DATETIME_TYPE_COUNT - 1; idx++)
     {
        field = sd->field_list + idx;
        val = *timearr[idx];
        if (val < field->min)
          *timearr[idx] = field->min;
        else if (val > field->max)
          *timearr[idx] = field->max;
     }

   _field_list_display(obj);
}

static void
_apply_range_restrictions(struct tm *tim)
{
   unsigned int idx;
   int val, min, max;

   if (!tim) return;

   DATETIME_TM_ARRAY(timearr, tim);
   for (idx = ELM_DATETIME_MONTH; idx < ELM_DATETIME_TYPE_COUNT - 1; idx++)
     {
        val = *timearr[idx];
        min = mapping[idx].def_min;
        max = mapping[idx].def_max;
        if (idx == ELM_DATETIME_DATE)
          max = _max_days_get(tim->tm_year, tim->tm_mon);
        if (val < min)
          *timearr[idx] = min;
        else if (val > max)
          *timearr[idx] = max;
     }
}

static const char *
_field_format_get(Evas_Object *obj,
                  Elm_Datetime_Field_Type field_type)
{
   Datetime_Field *field;

   if (field_type > ELM_DATETIME_AMPM) return NULL;

   ELM_DATETIME_DATA_GET(obj, sd);

   field = sd->field_list + field_type;

   return field->fmt;
}

static void
_field_limit_get(Evas_Object *obj,
                 Elm_Datetime_Field_Type field_type,
                 int *range_min,
                 int *range_max)
{
   int min, max, max_days;
   Datetime_Field *field;
   unsigned int idx;

   if (field_type > ELM_DATETIME_MINUTE) return;

   ELM_DATETIME_DATA_GET(obj, sd);

   field = sd->field_list + field_type;

   min = field->min;
   max = field->max;

   DATETIME_TM_ARRAY(curr_timearr, &sd->curr_time);
   DATETIME_TM_ARRAY(min_timearr, &sd->min_limit);
   DATETIME_TM_ARRAY(max_timearr, &sd->max_limit);

   for (idx = 0; idx < field->type; idx++)
     if (*curr_timearr[idx] > *min_timearr[idx]) break;
   if ((idx == field_type) && (min < *min_timearr[field_type]))
     min = *min_timearr[field_type];
   if (field_type == ELM_DATETIME_DATE)
     {
        max_days = _max_days_get(sd->curr_time.tm_year, sd->curr_time.tm_mon);
        if (max > max_days) max = max_days;
     }
   for (idx = 0; idx < field->type; idx++)
     if (*curr_timearr[idx] < *max_timearr[idx]) break;
   if ((idx == field_type) && (max > *max_timearr[field_type]))
     max = *max_timearr[field_type];

   *range_min = min;
   *range_max = max;
}

static void
_field_list_init(Evas_Object *obj)
{
   Datetime_Field *field;
   unsigned int idx;
   time_t t;

   ELM_DATETIME_DATA_GET(obj, sd);

   t = time(NULL);
   localtime_r(&t, &sd->curr_time);

   mapping[ELM_DATETIME_YEAR].def_min = _elm_config->year_min;
   mapping[ELM_DATETIME_YEAR].def_max = _elm_config->year_max;
   for (idx = 0; idx < ELM_DATETIME_TYPE_COUNT; idx++)
     {
        field = sd->field_list + idx;
        field->type = ELM_DATETIME_YEAR + idx;
        field->fmt[0] = '%';
        field->fmt_exist = EINA_FALSE;
        field->visible = EINA_TRUE;
        field->min = mapping[idx].def_min;
        field->max = mapping[idx].def_max;
     }
   DATETIME_TM_ARRAY(min_timearr, &sd->min_limit);
   DATETIME_TM_ARRAY(max_timearr, &sd->max_limit);
   for (idx = 0; idx < ELM_DATETIME_TYPE_COUNT - 1; idx++)
     {
        *min_timearr[idx] = mapping[idx].def_min;
        *max_timearr[idx] = mapping[idx].def_max;
     }
}

static char *
_access_info_cb(void *data, Evas_Object *obj EINA_UNUSED)
{
   char *ret;
   Eina_Strbuf *buf;
   buf = eina_strbuf_new();

   ELM_DATETIME_DATA_GET(data, sd);
   eina_strbuf_append_printf(buf,
                             "%d year, %d month, %d date, %d hour, %d minute",
                             sd->curr_time.tm_year, sd->curr_time.tm_mon + 1,
                             sd->curr_time.tm_mday, sd->curr_time.tm_hour,
                             sd->curr_time.tm_min);

   ret = eina_strbuf_string_steal(buf);
   eina_strbuf_free(buf);
   return ret;
}

EOLIAN static void
_elm_datetime_evas_object_smart_add(Eo *obj, Elm_Datetime_Data *priv)
{
   Datetime_Field *field;
   int idx;

   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());
   elm_widget_sub_object_parent_add(obj);

   // module - initialise module for datetime
   if (!dt_mod) dt_mod = _dt_mod_init();
   if (dt_mod)
     {
        if (dt_mod->obj_hook)
          {
             priv->mod_data = dt_mod->obj_hook(obj);

             // update module data
             if (priv->mod_data)
               {
                  priv->mod_data->base = obj;
                  priv->mod_data->field_limit_get = _field_limit_get;
                  priv->mod_data->field_format_get = _field_format_get;
               }
          }

        if (dt_mod->field_create)
          {
             for (idx = 0; idx < ELM_DATETIME_TYPE_COUNT; idx++)
               {
                  field = priv->field_list + idx;
                  field->item_obj = dt_mod->field_create(priv->mod_data, idx);
               }
          }
     }

   priv->freeze_sizing = EINA_TRUE;
   if (!elm_layout_theme_set(obj, "datetime", "base",
                             elm_widget_style_get(obj)))
     CRI("Failed to set layout!");

   _field_list_init(obj);
   _reload_format(obj);

   elm_widget_can_focus_set(obj, EINA_TRUE);

   priv->freeze_sizing = EINA_FALSE;
   elm_layout_sizing_eval(obj);

   // ACCESS
   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
     {
        Elm_Access_Info *ai;

        priv->access_obj = _elm_access_edje_object_part_object_register
          (obj, elm_layout_edje_get(obj), "elm.access");
        if (!priv->access_obj)
          priv->access_obj = _elm_access_edje_object_part_object_register
          (obj, elm_layout_edje_get(obj), "access");

        ai = _elm_access_info_get(priv->access_obj);
        _elm_access_text_set(ai, ELM_ACCESS_TYPE, "date time");
        _elm_access_callback_set(ai, ELM_ACCESS_INFO, _access_info_cb, obj);
     }
}

EOLIAN static void
_elm_datetime_evas_object_smart_del(Eo *obj, Elm_Datetime_Data *sd)
{
   Datetime_Field *tmp;
   unsigned int idx;

   for (idx = 0; idx < ELM_DATETIME_TYPE_COUNT; idx++)
     {
        tmp = sd->field_list + idx;
        evas_object_del(tmp->item_obj);
        eina_stringshare_del(tmp->separator);
     }

   if ((dt_mod) && (dt_mod->obj_unhook))
     dt_mod->obj_unhook(sd->mod_data);  // module - unhook

   eo_do_super(obj, MY_CLASS, evas_obj_smart_del());
}

EAPI Evas_Object *
elm_datetime_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = eo_add(MY_CLASS, parent);
   return obj;
}

EOLIAN static void
_elm_datetime_eo_base_constructor(Eo *obj, Elm_Datetime_Data *_pd EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME_LEGACY),
         evas_obj_smart_callbacks_descriptions_set(_smart_callbacks),
         elm_interface_atspi_accessible_role_set(ELM_ATSPI_ROLE_DATE_EDITOR));
}

EOLIAN static const char*
_elm_datetime_format_get(Eo *obj EINA_UNUSED, Elm_Datetime_Data *sd)
{
   return sd->format;
}

EOLIAN static void
_elm_datetime_format_set(Eo *obj, Elm_Datetime_Data *sd, const char *fmt)
{
   if (fmt)
     {
        strncpy(sd->format, fmt, ELM_DATETIME_MAX_FORMAT_LEN);
        sd->format[ELM_DATETIME_MAX_FORMAT_LEN - 1] = '\0';
        sd->user_format = EINA_TRUE;
     }
   else sd->user_format = EINA_FALSE;

   _reload_format(obj);
}

EOLIAN static Eina_Bool
_elm_datetime_field_visible_get(Eo *obj EINA_UNUSED, Elm_Datetime_Data *sd, Elm_Datetime_Field_Type fieldtype)
{
   Datetime_Field *field;

   if (fieldtype > ELM_DATETIME_AMPM) return EINA_FALSE;

   field = sd->field_list + fieldtype;

   return field->visible;
}

EOLIAN static void
_elm_datetime_field_visible_set(Eo *obj, Elm_Datetime_Data *sd, Elm_Datetime_Field_Type fieldtype, Eina_Bool visible)
{
   char buf[BUFFER_SIZE];
   Datetime_Field *field;

   if (fieldtype > ELM_DATETIME_AMPM) return;

   field = sd->field_list + fieldtype;
   visible = !!visible;
   if (field->visible == visible) return;

   field->visible = visible;

   sd->freeze_sizing = EINA_TRUE;
   if (visible)
     {
        sd->enabled_field_count++;

        if (!field->fmt_exist) return;

        snprintf(buf, sizeof(buf), EDC_PART_FIELD_ENABLE_SIG_STR,
                 field->location);
        elm_layout_signal_emit(obj, buf, "elm");

        ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
        edje_object_message_signal_process(wd->resize_obj);

        snprintf(buf, sizeof(buf), EDC_PART_FIELD_STR, field->location);
        elm_layout_content_unset(obj, buf);
        elm_layout_content_set(obj, buf, field->item_obj);
     }
   else
     {
        sd->enabled_field_count--;

        if (!field->fmt_exist) return;

        snprintf(buf, sizeof(buf), EDC_PART_FIELD_DISABLE_SIG_STR,
                 field->location);
        elm_layout_signal_emit(obj, buf, "elm");

        ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
        edje_object_message_signal_process(wd->resize_obj);

        snprintf(buf, sizeof(buf), EDC_PART_FIELD_STR, field->location);
        evas_object_hide(elm_layout_content_unset(obj, buf));
     }
   sd->freeze_sizing = EINA_FALSE;

   elm_layout_sizing_eval(obj);

   if (!visible) return;
   if (!dt_mod || !dt_mod->field_value_display) return;

   dt_mod->field_value_display(sd->mod_data, field->item_obj);
}

EOLIAN static void
_elm_datetime_field_limit_get(Eo *obj EINA_UNUSED, Elm_Datetime_Data *sd, Elm_Datetime_Field_Type fieldtype, int *min, int *max)
{
   Datetime_Field *field;

   if (fieldtype >= ELM_DATETIME_AMPM) return;

   field = sd->field_list + fieldtype;
   if (min) *min = field->min;
   if (max) *max = field->max;
}

EOLIAN static void
_elm_datetime_field_limit_set(Eo *obj, Elm_Datetime_Data *sd, Elm_Datetime_Field_Type fieldtype, int min, int max)
{
   Datetime_Field *field;

   if (fieldtype >= ELM_DATETIME_AMPM) return;

   if (min > max) return;

   field = sd->field_list + fieldtype;
   if (((min >= mapping[fieldtype].def_min) &&
        (min <= mapping[fieldtype].def_max)) ||
       (field->type == ELM_DATETIME_YEAR))
     field->min = min;
   if (((max >= mapping[fieldtype].def_min) &&
        (max <= mapping[fieldtype].def_max)) ||
       (field->type == ELM_DATETIME_YEAR))
     field->max = max;

   _apply_field_limits(obj);
}

EOLIAN static Eina_Bool
_elm_datetime_value_get(Eo *obj EINA_UNUSED, Elm_Datetime_Data *sd, struct tm *currtime)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(currtime, EINA_FALSE);

   *currtime = sd->curr_time;
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_datetime_value_set(Eo *obj, Elm_Datetime_Data *sd, const struct tm *newtime)
{
   struct tm old_time;

   EINA_SAFETY_ON_NULL_RETURN_VAL(newtime, EINA_FALSE);

   old_time = sd->curr_time;
   sd->curr_time = *newtime;
   // apply default field restrictions for curr_time
   _apply_range_restrictions(&sd->curr_time);
   // validate the curr_time according to the min_limt and max_limt
   _validate_datetime_limits(&sd->curr_time, &sd->min_limit, EINA_FALSE);
   _validate_datetime_limits(&sd->max_limit, &sd->curr_time, EINA_TRUE);
   _apply_field_limits(obj);

   if (!_date_cmp(&old_time, &sd->curr_time))
     evas_object_smart_callback_call(obj, SIG_CHANGED, NULL);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_datetime_value_min_get(Eo *obj EINA_UNUSED, Elm_Datetime_Data *sd, struct tm *mintime)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(mintime, EINA_FALSE);

   *mintime = sd->min_limit;
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_datetime_value_min_set(Eo *obj, Elm_Datetime_Data *sd, const struct tm *mintime)
{
   struct tm old_time;

   EINA_SAFETY_ON_NULL_RETURN_VAL(mintime, EINA_FALSE);

   sd->min_limit = *mintime;
   old_time = sd->curr_time;
   // apply default field restrictions for min_limit
   _apply_range_restrictions(&sd->min_limit);
   // validate curr_time and max_limt according to the min_limit
   _validate_datetime_limits(&sd->max_limit, &sd->min_limit, EINA_FALSE);
   _validate_datetime_limits(&sd->curr_time, &sd->min_limit, EINA_FALSE);
   _apply_field_limits(obj);

   if (!_date_cmp(&old_time, &sd->curr_time))
     evas_object_smart_callback_call(obj, SIG_CHANGED, NULL);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_datetime_value_max_get(Eo *obj EINA_UNUSED, Elm_Datetime_Data *sd, struct tm *maxtime)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(maxtime, EINA_FALSE);

   *maxtime = sd->max_limit;
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_datetime_value_max_set(Eo *obj, Elm_Datetime_Data *sd, const struct tm *maxtime)
{
   struct tm old_time;

   EINA_SAFETY_ON_NULL_RETURN_VAL(maxtime, EINA_FALSE);

   sd->max_limit = *maxtime;
   old_time = sd->curr_time;
   // apply default field restrictions for max_limit
   _apply_range_restrictions(&sd->max_limit);
   // validate curr_time and min_limt according to the max_limit
   _validate_datetime_limits(&sd->max_limit, &sd->min_limit, EINA_TRUE);
   _validate_datetime_limits(&sd->max_limit, &sd->curr_time, EINA_TRUE);
   _apply_field_limits(obj);

   if (!_date_cmp(&old_time, &sd->curr_time))
     evas_object_smart_callback_call(obj, SIG_CHANGED, NULL);

   return EINA_TRUE;
}

EOLIAN static void
_elm_datetime_class_constructor(Eo_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

#include "elm_datetime.eo.c"
