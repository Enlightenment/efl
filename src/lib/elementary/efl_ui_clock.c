#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_PROTECTED
#define EFL_UI_L10N_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"
#include <efl_ui_clock.h>
#include <efl_ui_clock_private.h>

#define MY_CLASS EFL_UI_CLOCK_CLASS

#define MY_CLASS_NAME "Efl.Ui.Clock"

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
 * EFL_UI_CLOCK_TYPE_COUNT are in the valid range, and must get in the
 * place of "%d".
 */
#define EDC_PART_FIELD_STR             "efl.field%d"
#define EDC_PART_SEPARATOR_STR         "efl.separator%d"
#define EDC_PART_FIELD_ENABLE_SIG_STR  "field%d,enable"
#define EDC_PART_FIELD_DISABLE_SIG_STR "field%d,disable"

/* struct tm does not define the fields in the order year, month,
 * date, hour, minute. values are reassigned to an array for easy
 * handling.
 */
#define CLOCK_TM_ARRAY(intptr, tmptr) \
  int *intptr[] = {                      \
     &(tmptr)->tm_year,                  \
     &(tmptr)->tm_mon,                   \
     &(tmptr)->tm_mday,                  \
     &(tmptr)->tm_hour,                  \
     &(tmptr)->tm_min,                  \
     &(tmptr)->tm_sec,                  \
     &(tmptr)->tm_wday,                  \
     &(tmptr)->tm_hour}

// default limits for individual fields
static Format_Map mapping[EFL_UI_CLOCK_TYPE_COUNT] = {
   [EFL_UI_CLOCK_TYPE_YEAR] = { "Yy", -1, -1, "" },
   [EFL_UI_CLOCK_TYPE_MONTH] = { "mbBh", 0, 11, "" },
   [EFL_UI_CLOCK_TYPE_DATE] = { "de", 1, 31, "" },
   [EFL_UI_CLOCK_TYPE_HOUR] = { "IHkl", 0, 23, "" },
   [EFL_UI_CLOCK_TYPE_MINUTE] = { "M", 0, 59, "" },
   [EFL_UI_CLOCK_TYPE_SECOND] = { "S", 0, 59, "" },
   [EFL_UI_CLOCK_TYPE_DAY] = { "Aa", 0, 6, "" },
   [EFL_UI_CLOCK_TYPE_AMPM] = { "pP", 0, 1, "" }
};

static const char *multifield_formats = "cxXrRTDF";
static const char *ignore_separators = "()";
static const char *ignore_extensions = "E0_-O^#";

static const char SIG_CHANGED[] = "changed";
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_CHANGED, ""},
   {SIG_WIDGET_LANG_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_WIDGET_ACCESS_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_LAYOUT_FOCUSED, ""}, /**< handled by elm_layout */
   {SIG_LAYOUT_UNFOCUSED, ""}, /**< handled by elm_layout */
   {NULL, NULL}
};

static void _part_name_snprintf(char *buffer, int buffer_size,
   const Evas_Object *obj, const char *template, int n)
{
   snprintf(buffer, buffer_size, template, n);
   if (edje_object_part_exists (obj, buffer)) return;
   // Try 'elm' prefix instead of 'efl'
   buffer[0] = 'e';
   buffer[1] = 'l';
   buffer[2] = 'm';
   if (edje_object_part_exists (obj, buffer)) return;
   // Skip the namespace prefix "elm." which was not present
   // in previous versions
   snprintf(buffer, buffer_size, template + 4, n);
}

static Elm_Module *
_dt_mod_find(void)
{
   static int tried_fallback = 0;
   Elm_Module *mod = _elm_module_find_as("clock/api");

   if (mod) return mod;
   if (!tried_fallback &&
       (!_elm_config->modules || !strstr(_elm_config->modules, "clock/api")))
     {
        // See also _config_update(): we hardcode here the default module
        ERR("Elementary config does not contain the required module "
            "name for the clock widget! Verify your installation.");
        _elm_module_add("clock_input_ctxpopup", "clock/api");
        mod = _elm_module_find_as("clock/api");
        tried_fallback = EINA_TRUE;
     }
   return mod;
}

static Clock_Mod_Api *
_dt_mod_init()
{
   Elm_Module *mod;

   mod = _dt_mod_find();
   if (!mod) return NULL;
   if (mod->api) return mod->api;
   mod->api = malloc(sizeof(Clock_Mod_Api));
   if (!mod->api) return NULL;

   ((Clock_Mod_Api *)(mod->api))->obj_hook =
     _elm_module_symbol_get(mod, "obj_hook");
   ((Clock_Mod_Api *)(mod->api))->obj_unhook =
     _elm_module_symbol_get(mod, "obj_unhook");
   ((Clock_Mod_Api *)(mod->api))->obj_hide =
     _elm_module_symbol_get(mod, "obj_hide");
   ((Clock_Mod_Api *)(mod->api))->field_create =
     _elm_module_symbol_get(mod, "field_create");
   ((Clock_Mod_Api *)(mod->api))->field_value_display =
     _elm_module_symbol_get(mod, "field_value_display");

   return mod->api;
}

static void
_field_list_display(Evas_Object *obj)
{
   Clock_Field *field;
   unsigned int idx = 0;
   Clock_Mod_Api *dt_mod;

   EFL_UI_CLOCK_DATA_GET(obj, sd);

   dt_mod = _dt_mod_init();
   if (!dt_mod || !dt_mod->field_value_display) return;

   for (idx = 0; idx < EFL_UI_CLOCK_TYPE_COUNT; idx++)
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
#if defined(HAVE_LANGINFO_H) || defined (_WIN32)
        exp_fmt = nl_langinfo(D_T_FMT);
#else
        exp_fmt = "";
#endif
        break;

      case 'x':
#if defined(HAVE_LANGINFO_H) || defined (_WIN32)
        exp_fmt = nl_langinfo(D_FMT);
#else
        exp_fmt = "";
#endif
        break;

      case 'X':
#if defined(HAVE_LANGINFO_H) || defined (_WIN32)
        exp_fmt = nl_langinfo(T_FMT);
#else
        exp_fmt = "";
#endif
        break;

      case 'r':
#if defined(HAVE_LANGINFO_H) || defined (_WIN32)
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
   unsigned int idx, len = 0;
   char buf[EFL_UI_CLOCK_MAX_FORMAT_LEN] = {0, };
   Eina_Bool fmt_char, fmt_expanded;

   do {
     idx = 0;
     fmt_char = EINA_FALSE;
     fmt_expanded = EINA_FALSE;
     ptr = dt_fmt;
     while ((ch = *ptr))
       {
          if ((fmt_char) && (strchr(multifield_formats, ch)))
            {
               /* replace the multi-field format characters with
                * corresponding expanded format */
               expanded_fmt = _expanded_fmt_str_get(ch);
               len = strlen(expanded_fmt);
               if (len > 0) fmt_expanded = EINA_TRUE;
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
     strncpy(dt_fmt, buf, EFL_UI_CLOCK_MAX_FORMAT_LEN);
   } while (fmt_expanded);
}

static void
_field_list_arrange(Evas_Object *obj)
{
   Clock_Field *field;
   char buf[BUFFER_SIZE];
   int idx;
   Eina_Bool freeze;

   EFL_UI_CLOCK_DATA_GET(obj, sd);

   freeze = sd->freeze_sizing;
   sd->freeze_sizing = EINA_TRUE;
   for (idx = 0; idx < EFL_UI_CLOCK_TYPE_COUNT; idx++)
     {
        field = sd->field_list + idx;
        _part_name_snprintf(buf, sizeof(buf), obj, EDC_PART_FIELD_STR,
                            field->location);

        if (field->visible && field->fmt_exist)
          {
             evas_object_hide(elm_layout_content_unset(obj, buf));
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
   Clock_Field *field = NULL;
   char cur;

   EFL_UI_CLOCK_DATA_GET(obj, sd);

   while ((cur = *fmt_ptr))
     {
        if (fmt_parsing)
          {
             if (strchr(ignore_extensions, cur))
               {
                  fmt_ptr++;
                  continue;
               }
             fmt_parsing = EINA_FALSE;
             for (idx = 0; idx < EFL_UI_CLOCK_TYPE_COUNT; idx++)
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
            (field->type != EFL_UI_CLOCK_TYPE_AMPM) &&
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
   Clock_Field *field;
   char buf[BUFFER_SIZE];
   char *dt_fmt;

   EFL_UI_CLOCK_DATA_GET(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   // FIXME: provide nl_langinfo on Windows if possible
   // fetch the default format from Libc.
   if (!sd->user_format)
#if defined(HAVE_LANGINFO_H) || defined (_WIN32)
     strncpy(sd->format, nl_langinfo(D_T_FMT), EFL_UI_CLOCK_MAX_FORMAT_LEN);
#else
     strncpy(sd->format, "", EFL_UI_CLOCK_MAX_FORMAT_LEN);
#endif
   sd->format[EFL_UI_CLOCK_MAX_FORMAT_LEN - 1] = '\0';

   dt_fmt = (char *)malloc(EFL_UI_CLOCK_MAX_FORMAT_LEN);
   if (!dt_fmt) return;

   strncpy(dt_fmt, sd->format, EFL_UI_CLOCK_MAX_FORMAT_LEN);

   _expand_format(dt_fmt);

   // reset all the fields to disable state
   sd->enabled_field_count = 0;
   for (idx = 0; idx < EFL_UI_CLOCK_TYPE_COUNT; idx++)
     {
        field = sd->field_list + idx;
        field->fmt_exist = EINA_FALSE;
        field->location = -1;
     }

   field_count = _parse_format(obj, dt_fmt);
   free(dt_fmt);

   for (idx = 0; idx < EFL_UI_CLOCK_TYPE_COUNT; idx++)
     {
        field = sd->field_list + idx;
        if (field->fmt_exist && field->visible)
          sd->enabled_field_count++;
     }

   // assign locations to disabled fields for uniform usage
   for (idx = 0; idx < EFL_UI_CLOCK_TYPE_COUNT; idx++)
     {
        field = sd->field_list + idx;
        if (field->location == -1) field->location = field_count++;

        if (field->fmt_exist && field->visible)
          {
             snprintf(buf, sizeof(buf), EDC_PART_FIELD_ENABLE_SIG_STR,
                      field->location);
             if (elm_widget_is_legacy(obj))
               elm_layout_signal_emit(obj, buf, "elm");
             else
               elm_layout_signal_emit(obj, buf, "efl");
          }
        else
          {
             snprintf(buf, sizeof(buf), EDC_PART_FIELD_DISABLE_SIG_STR,
                      field->location);
             if (elm_widget_is_legacy(obj))
               elm_layout_signal_emit(obj, buf, "elm");
             else
               elm_layout_signal_emit(obj, buf, "efl");
          }
        if (field->location + 1)
          {
             _part_name_snprintf(buf, sizeof(buf), obj, EDC_PART_SEPARATOR_STR,
                                 field->location + 1);
             elm_layout_text_set(obj, buf, field->separator);
          }
     }

   edje_object_message_signal_process(wd->resize_obj);
   _field_list_arrange(obj);
}

EOLIAN static void
_efl_ui_clock_efl_ui_l10n_translation_update(Eo *obj, Efl_Ui_Clock_Data *sd)
{
   if (!sd->user_format) _reload_format(obj);
   else _field_list_display(obj);

   efl_ui_l10n_translation_update(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_ui_clock_pause_set(Eo *obj EINA_UNUSED, Efl_Ui_Clock_Data *sd, Eina_Bool paused)
{
   paused = !!paused;
   if (sd->paused == paused)
     return;
   sd->paused = paused;
   if (paused)
     ecore_timer_freeze(sd->ticker);
   else
     ecore_timer_thaw(sd->ticker);
}

EOLIAN static Eina_Bool
_efl_ui_clock_pause_get(const Eo *obj EINA_UNUSED, Efl_Ui_Clock_Data *sd)
{
   return sd->paused;
}

EOLIAN static void
_efl_ui_clock_edit_mode_set(Eo *obj EINA_UNUSED, Efl_Ui_Clock_Data *sd, Eina_Bool edit_mode)
{
   sd->edit_mode = edit_mode;
}

EOLIAN static Eina_Bool
_efl_ui_clock_edit_mode_get(const Eo *obj EINA_UNUSED, Efl_Ui_Clock_Data *sd)
{
   return sd->edit_mode;
}

EOLIAN static Eina_Bool
_efl_ui_clock_efl_ui_focus_object_on_focus_update(Eo *obj, Efl_Ui_Clock_Data *sd)
{
   Eina_Bool int_ret = EINA_FALSE;

   int_ret = efl_ui_focus_object_on_focus_update(efl_super(obj, MY_CLASS));
   if (!int_ret) return EINA_FALSE;

   if (!efl_ui_focus_object_focus_get(obj))
     {
        Clock_Mod_Api *dt_mod = _dt_mod_init();
        if ((dt_mod) && (dt_mod->obj_hide))
          dt_mod->obj_hide(sd->mod_data);
     }

   return EINA_TRUE;
}

EOLIAN static void
_efl_ui_clock_elm_layout_sizing_eval(Eo *obj, Efl_Ui_Clock_Data *sd)
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

EOLIAN static Eina_Error
_efl_ui_clock_efl_ui_widget_theme_apply(Eo *obj, Efl_Ui_Clock_Data *sd)
{
   Eina_Error int_ret = EFL_UI_THEME_APPLY_ERROR_GENERIC;

   Clock_Field *field;
   char buf[BUFFER_SIZE];
   unsigned int idx;
   Clock_Mod_Api *dt_mod;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   int_ret = efl_ui_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (int_ret == EFL_UI_THEME_APPLY_ERROR_GENERIC) return int_ret;

   dt_mod = _dt_mod_init();
   if ((!dt_mod) || (!dt_mod->field_value_display)) return EINA_TRUE;

   for (idx = 0; idx < EFL_UI_CLOCK_TYPE_COUNT; idx++)
     {
        field = sd->field_list + idx;
        // TODO: Different group name for each field_obj may be needed.
        elm_widget_element_update(obj, field->item_obj, PART_NAME_ARRAY[idx]);
        if (field->fmt_exist && field->visible)
          {
             snprintf(buf, sizeof(buf), EDC_PART_FIELD_ENABLE_SIG_STR,
                      field->location);
             if (elm_widget_is_legacy(obj))
               elm_layout_signal_emit(obj, buf, "elm");
             else
               elm_layout_signal_emit(obj, buf, "efl");

             if (field->location)
               {
                  _part_name_snprintf(buf, sizeof(buf), obj, EDC_PART_SEPARATOR_STR,
                                      field->location);
                  elm_layout_text_set(obj, buf, field->separator);
               }

             dt_mod->field_value_display(sd->mod_data, field->item_obj);
          }
        else
          {
             snprintf(buf, sizeof(buf), EDC_PART_FIELD_DISABLE_SIG_STR,
                      field->location);
             if (elm_widget_is_legacy(obj))
               elm_layout_signal_emit(obj, buf, "elm");
             else
               elm_layout_signal_emit(obj, buf, "efl");
          }
     }

   edje_object_message_signal_process(wd->resize_obj);
   elm_layout_sizing_eval(obj);

   return int_ret;
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
   for (day = MIN_DAYS_IN_MONTH; day <= mapping[EFL_UI_CLOCK_TYPE_DATE].def_max;
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
_date_cmp(const struct tm *time1,
          const struct tm *time2)
{
   unsigned int idx;

   const CLOCK_TM_ARRAY(timearr1, time1);
   const CLOCK_TM_ARRAY(timearr2, time2);

   for (idx = 0; idx < EFL_UI_CLOCK_TYPE_COUNT; idx++)
     {
        if (*timearr1[idx] != *timearr2[idx])
          return EINA_FALSE;
     }

   return EINA_TRUE;
}

static Eina_Bool
_field_cmp(Efl_Ui_Clock_Type field_type,
          struct tm *time1,
          struct tm *time2)
{
   CLOCK_TM_ARRAY(timearr1, time1);
   CLOCK_TM_ARRAY(timearr2, time2);

   if (*timearr1[field_type] != *timearr2[field_type])
     return EINA_FALSE;
   else
     return EINA_TRUE;
}

// validates curr_time/min_limt/max_limit according to the newly set value
static void
_validate_clock_limits(struct tm *time1,
                          struct tm *time2,
                          Eina_Bool swap)
{
   struct tm *t1, *t2;
   unsigned int idx;

   if (!time1 || !time2) return;

   t1 = (swap) ? time2 : time1;
   t2 = (swap) ? time1 : time2;

   CLOCK_TM_ARRAY(timearr1, time1);
   CLOCK_TM_ARRAY(timearr2, time2);
   for (idx = 0; idx < EFL_UI_CLOCK_TYPE_COUNT - 1; idx++)
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
   Clock_Field *field;
   unsigned int idx = 0;
   int val;

   EFL_UI_CLOCK_DATA_GET(obj, sd);

   CLOCK_TM_ARRAY(timearr, &sd->curr_time);
   for (idx = 0; idx < EFL_UI_CLOCK_TYPE_COUNT - 1; idx++)
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

   CLOCK_TM_ARRAY(timearr, tim);
   for (idx = EFL_UI_CLOCK_TYPE_MONTH; idx < EFL_UI_CLOCK_TYPE_COUNT - 1; idx++)
     {
        val = *timearr[idx];
        min = mapping[idx].def_min;
        max = mapping[idx].def_max;
        if (idx == EFL_UI_CLOCK_TYPE_DATE)
          max = _max_days_get(tim->tm_year, tim->tm_mon);
        if (val < min)
          *timearr[idx] = min;
        else if (val > max)
          *timearr[idx] = max;
     }
}

static const char *
_field_format_get(Evas_Object *obj,
                  Efl_Ui_Clock_Type field_type)
{
   Clock_Field *field;

   if (field_type > EFL_UI_CLOCK_TYPE_AMPM) return NULL;

   EFL_UI_CLOCK_DATA_GET(obj, sd);

   field = sd->field_list + field_type;

   return field->fmt;
}

static void
_field_limit_get(Evas_Object *obj,
                 Efl_Ui_Clock_Type field_type,
                 int *range_min,
                 int *range_max)
{
   int min, max, max_days;
   Clock_Field *field;
   unsigned int idx;

   if (field_type > EFL_UI_CLOCK_TYPE_DAY) return;

   EFL_UI_CLOCK_DATA_GET(obj, sd);

   field = sd->field_list + field_type;

   min = field->min;
   max = field->max;

   CLOCK_TM_ARRAY(curr_timearr, &sd->curr_time);
   CLOCK_TM_ARRAY(min_timearr, &sd->min_limit);
   CLOCK_TM_ARRAY(max_timearr, &sd->max_limit);

   for (idx = 0; idx < field->type; idx++)
     if (*curr_timearr[idx] > *min_timearr[idx]) break;
   if ((idx == field_type) && (min < *min_timearr[field_type]))
     min = *min_timearr[field_type];
   if (field_type == EFL_UI_CLOCK_TYPE_DATE)
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
   Clock_Field *field;
   unsigned int idx;
   time_t t;

   EFL_UI_CLOCK_DATA_GET(obj, sd);

   t = time(NULL);
   localtime_r(&t, &sd->curr_time);

   mapping[EFL_UI_CLOCK_TYPE_YEAR].def_min = _elm_config->year_min;
   mapping[EFL_UI_CLOCK_TYPE_YEAR].def_max = _elm_config->year_max;
   for (idx = 0; idx < EFL_UI_CLOCK_TYPE_COUNT; idx++)
     {
        field = sd->field_list + idx;
        field->type = EFL_UI_CLOCK_TYPE_YEAR + idx;
        field->fmt[0] = '%';
        field->fmt_exist = EINA_FALSE;
        field->visible = EINA_TRUE;
        field->min = mapping[idx].def_min;
        field->max = mapping[idx].def_max;
     }
   CLOCK_TM_ARRAY(min_timearr, &sd->min_limit);
   CLOCK_TM_ARRAY(max_timearr, &sd->max_limit);
   for (idx = 0; idx < EFL_UI_CLOCK_TYPE_COUNT - 1; idx++)
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

   EFL_UI_CLOCK_DATA_GET(data, sd);
   eina_strbuf_append_printf(buf,
                             "%d year, %d month, %d date, %d hour, %d minute",
                             sd->curr_time.tm_year, sd->curr_time.tm_mon + 1,
                             sd->curr_time.tm_mday, sd->curr_time.tm_hour,
                             sd->curr_time.tm_min);

   ret = eina_strbuf_string_steal(buf);
   eina_strbuf_free(buf);
   return ret;
}

static Eina_Bool
_ticker(void *data)
{
   double t;
   time_t tt;
   struct timeval timev;
   Clock_Field *field;

   EFL_UI_CLOCK_DATA_GET(data, sd);

   tt = time(NULL);
   localtime_r(&tt, &sd->curr_time);

   if (sd->curr_time.tm_sec > 0)
     {
        Clock_Mod_Api *dt_mod = _dt_mod_init();
        field = sd->field_list + EFL_UI_CLOCK_TYPE_SECOND;
        if (field->fmt_exist && field->visible &&
            dt_mod && dt_mod->field_value_display)
          dt_mod->field_value_display(sd->mod_data, field->item_obj);
     }
   else
     _field_list_display(data);

   gettimeofday(&timev, NULL);
   t = ((double)(1000000 - timev.tv_usec)) / 1000000.0;
   sd->ticker = ecore_timer_add(t, _ticker, data);

   return ECORE_CALLBACK_CANCEL;
}

EOLIAN static void
_efl_ui_clock_efl_canvas_group_group_add(Eo *obj, Efl_Ui_Clock_Data *priv)
{
   Clock_Field *field;
   Clock_Mod_Api *dt_mod;
   int idx;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (!elm_widget_theme_klass_get(obj))
     elm_widget_theme_klass_set(obj, "uiclock");
   efl_canvas_group_add(efl_super(obj, MY_CLASS));

   if (elm_widget_theme_object_set(obj, wd->resize_obj,
                                       elm_widget_theme_klass_get(obj),
                                       elm_widget_theme_element_get(obj),
                                       elm_widget_theme_style_get(obj)) == EFL_UI_THEME_APPLY_ERROR_GENERIC)
     CRI("Failed to set layout!");

   // module - initialise module for clock
   dt_mod = _dt_mod_init();
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
             for (idx = 0; idx < EFL_UI_CLOCK_TYPE_COUNT; idx++)
               {
                  field = priv->field_list + idx;
                  field->item_obj = dt_mod->field_create(priv->mod_data, idx);
               }
          }
     }
   else WRN("Failed to load clock module, clock widget may not show properly!");

   priv->freeze_sizing = EINA_TRUE;

   _field_list_init(obj);
   _reload_format(obj);
   _ticker(obj);

   elm_widget_can_focus_set(obj, EINA_TRUE);

   priv->freeze_sizing = EINA_FALSE;
   elm_layout_sizing_eval(obj);

   // ACCESS
   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
     {
        Elm_Access_Info *ai;

        priv->access_obj = _elm_access_edje_object_part_object_register
          (obj, elm_layout_edje_get(obj), "efl.access");
        if (!priv->access_obj)
          priv->access_obj = _elm_access_edje_object_part_object_register
          (obj, elm_layout_edje_get(obj), "access");

        ai = _elm_access_info_get(priv->access_obj);
        _elm_access_text_set(ai, ELM_ACCESS_TYPE, "date time");
        _elm_access_callback_set(ai, ELM_ACCESS_INFO, _access_info_cb, obj);
     }
}

EOLIAN static void
_efl_ui_clock_efl_canvas_group_group_del(Eo *obj, Efl_Ui_Clock_Data *sd)
{
   Clock_Field *tmp;
   unsigned int idx;
   Clock_Mod_Api *dt_mod;

   ecore_timer_del(sd->ticker);
   for (idx = 0; idx < EFL_UI_CLOCK_TYPE_COUNT; idx++)
     {
        tmp = sd->field_list + idx;
        evas_object_del(tmp->item_obj);
        eina_stringshare_del(tmp->separator);
     }

   dt_mod = _dt_mod_init();
   if ((dt_mod) && (dt_mod->obj_unhook))
     dt_mod->obj_unhook(sd->mod_data);  // module - unhook

   efl_canvas_group_del(efl_super(obj, MY_CLASS));
}

EOLIAN static Eo *
_efl_ui_clock_efl_object_constructor(Eo *obj, Efl_Ui_Clock_Data *_pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   evas_object_smart_callbacks_descriptions_set(obj, _smart_callbacks);
   efl_access_object_role_set(obj, EFL_ACCESS_ROLE_DATE_EDITOR);

   return obj;
}

EOLIAN static const char*
_efl_ui_clock_format_get(const Eo *obj EINA_UNUSED, Efl_Ui_Clock_Data *sd)
{
   return sd->format;
}

EOLIAN static void
_efl_ui_clock_format_set(Eo *obj, Efl_Ui_Clock_Data *sd, const char *fmt)
{
   if (fmt)
     {
        strncpy(sd->format, fmt, EFL_UI_CLOCK_MAX_FORMAT_LEN);
        sd->format[EFL_UI_CLOCK_MAX_FORMAT_LEN - 1] = '\0';
        sd->user_format = EINA_TRUE;
     }
   else sd->user_format = EINA_FALSE;

   _reload_format(obj);
}

EOLIAN static Eina_Bool
_efl_ui_clock_field_visible_get(const Eo *obj EINA_UNUSED, Efl_Ui_Clock_Data *sd, Efl_Ui_Clock_Type fieldtype)
{
   Clock_Field *field;

   if (fieldtype > EFL_UI_CLOCK_TYPE_AMPM) return EINA_FALSE;

   field = sd->field_list + fieldtype;

   return field->visible;
}

EOLIAN static void
_efl_ui_clock_field_visible_set(Eo *obj, Efl_Ui_Clock_Data *sd, Efl_Ui_Clock_Type fieldtype, Eina_Bool visible)
{
   char buf[BUFFER_SIZE];
   Clock_Field *field;

   if (fieldtype > EFL_UI_CLOCK_TYPE_AMPM) return;

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
        if (elm_widget_is_legacy(obj))
          elm_layout_signal_emit(obj, buf, "elm");
        else
          elm_layout_signal_emit(obj, buf, "efl");

        ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
        edje_object_message_signal_process(wd->resize_obj);

        _part_name_snprintf(buf, sizeof(buf), obj, EDC_PART_FIELD_STR,
                            field->location);
        elm_layout_content_unset(obj, buf);
        elm_layout_content_set(obj, buf, field->item_obj);
     }
   else
     {
        sd->enabled_field_count--;

        if (!field->fmt_exist) return;

        snprintf(buf, sizeof(buf), EDC_PART_FIELD_DISABLE_SIG_STR,
                 field->location);
        if (elm_widget_is_legacy(obj))
          elm_layout_signal_emit(obj, buf, "elm");
        else
          elm_layout_signal_emit(obj, buf, "efl");

        ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
        edje_object_message_signal_process(wd->resize_obj);

        _part_name_snprintf(buf, sizeof(buf), obj, EDC_PART_FIELD_STR,
                            field->location);
        evas_object_hide(elm_layout_content_unset(obj, buf));
     }
   sd->freeze_sizing = EINA_FALSE;

   elm_layout_sizing_eval(obj);

   if (!visible) return;
   {
      Clock_Mod_Api *dt_mod = _dt_mod_init();
      if (!dt_mod || !dt_mod->field_value_display) return;
      dt_mod->field_value_display(sd->mod_data, field->item_obj);
   }
}

EOLIAN static void
_efl_ui_clock_field_limit_get(const Eo *obj EINA_UNUSED, Efl_Ui_Clock_Data *sd, Efl_Ui_Clock_Type fieldtype, int *min, int *max)
{
   Clock_Field *field;

   if (fieldtype >= EFL_UI_CLOCK_TYPE_AMPM) return;

   field = sd->field_list + fieldtype;
   if (min) *min = field->min;
   if (max) *max = field->max;
}

EOLIAN static void
_efl_ui_clock_field_limit_set(Eo *obj, Efl_Ui_Clock_Data *sd, Efl_Ui_Clock_Type fieldtype, int min, int max)
{
   Clock_Field *field;
   struct tm old_time;

   if (fieldtype >= EFL_UI_CLOCK_TYPE_AMPM) return;

   if (min > max) return;

   old_time = sd->curr_time;
   field = sd->field_list + fieldtype;
   if (((min >= mapping[fieldtype].def_min) &&
        (min <= mapping[fieldtype].def_max)) ||
       (field->type == EFL_UI_CLOCK_TYPE_YEAR))
     field->min = min;
   if (((max >= mapping[fieldtype].def_min) &&
        (max <= mapping[fieldtype].def_max)) ||
       (field->type == EFL_UI_CLOCK_TYPE_YEAR))
     field->max = max;

   _apply_field_limits(obj);

   if (!_field_cmp(fieldtype, &old_time, &sd->curr_time))
     efl_event_callback_legacy_call(obj, EFL_UI_CLOCK_EVENT_CHANGED, NULL);
}

EOLIAN static Efl_Time
_efl_ui_clock_time_get(const Eo *obj EINA_UNUSED, Efl_Ui_Clock_Data *sd)
{
   return sd->curr_time;
}

EOLIAN static void
_efl_ui_clock_time_set(Eo *obj, Efl_Ui_Clock_Data *sd, Efl_Time newtime)
{
   if (_date_cmp(&sd->curr_time, &newtime)) return;
   sd->curr_time = newtime;
   // apply default field restrictions for curr_time
   _apply_range_restrictions(&sd->curr_time);
   // validate the curr_time according to the min_limt and max_limt
   _validate_clock_limits(&sd->curr_time, &sd->min_limit, EINA_FALSE);
   _validate_clock_limits(&sd->max_limit, &sd->curr_time, EINA_TRUE);
   _apply_field_limits(obj);

   efl_event_callback_legacy_call(obj, EFL_UI_CLOCK_EVENT_CHANGED, NULL);
}

EOLIAN static Efl_Time
_efl_ui_clock_time_min_get(const Eo *obj EINA_UNUSED, Efl_Ui_Clock_Data *sd)
{
   return sd->min_limit;
}

EOLIAN static void
_efl_ui_clock_time_min_set(Eo *obj, Efl_Ui_Clock_Data *sd, Efl_Time mintime)
{
   struct tm old_time;

   if (_date_cmp(&sd->min_limit, &mintime)) return;
   sd->min_limit = mintime;
   old_time = sd->curr_time;
   // apply default field restrictions for min_limit
   _apply_range_restrictions(&sd->min_limit);
   // validate curr_time and max_limt according to the min_limit
   _validate_clock_limits(&sd->max_limit, &sd->min_limit, EINA_FALSE);
   _validate_clock_limits(&sd->curr_time, &sd->min_limit, EINA_FALSE);
   _apply_field_limits(obj);

   if (!_date_cmp(&old_time, &sd->curr_time))
     efl_event_callback_legacy_call(obj, EFL_UI_CLOCK_EVENT_CHANGED, NULL);
}

EOLIAN static Efl_Time
_efl_ui_clock_time_max_get(const Eo *obj EINA_UNUSED, Efl_Ui_Clock_Data *sd)
{
   return sd->max_limit;
}

EOLIAN static void
_efl_ui_clock_time_max_set(Eo *obj, Efl_Ui_Clock_Data *sd, Efl_Time maxtime)
{
   struct tm old_time;

   if (_date_cmp(&sd->max_limit, &maxtime)) return;
   sd->max_limit = maxtime;
   old_time = sd->curr_time;
   // apply default field restrictions for max_limit
   _apply_range_restrictions(&sd->max_limit);
   // validate curr_time and min_limt according to the max_limit
   _validate_clock_limits(&sd->max_limit, &sd->min_limit, EINA_TRUE);
   _validate_clock_limits(&sd->max_limit, &sd->curr_time, EINA_TRUE);
   _apply_field_limits(obj);

   if (!_date_cmp(&old_time, &sd->curr_time))
     efl_event_callback_legacy_call(obj, EFL_UI_CLOCK_EVENT_CHANGED, NULL);
}

/* Internal EO APIs and hidden overrides */

#define EFL_UI_CLOCK_EXTRA_OPS \
   ELM_LAYOUT_SIZING_EVAL_OPS(efl_ui_clock), \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(efl_ui_clock)

#include "efl_ui_clock.eo.c"
