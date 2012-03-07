#include <locale.h>
#include <langinfo.h>
#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;
typedef struct _Datetime_Field Datetime_Field;
typedef struct _Datetime_Mod_Api Datetime_Mod_Api;
typedef struct _Format_Map Format_Map;

#define DATETIME_TYPE_COUNT         6
#define MAX_FORMAT_LEN              64
#define MAX_SEPARATOR_LEN           6
#define MAX_FIELD_FORMAT_LEN        3
#define MIN_DAYS_IN_MONTH           28
#define BUFFER_SIZE                 1024

// interface between EDC & C code. Field names & signal names.
// values 0 to DATETIME_TYPE_COUNT are valid range, can be substituted for %d.
#define EDC_DATETIME_ENABLE_SIG_STR          "elm,state,enabled"
#define EDC_DATETIME_DISABLE_SIG_STR         "elm,state,disabled"
#define EDC_DATETIME_FOCUSIN_SIG_STR         "elm,action,focus"
#define EDC_DATETIME_FOCUSOUT_SIG_STR        "elm,action,unfocus"
#define EDC_PART_FIELD_STR                   "field%d"
#define EDC_PART_SEPARATOR_STR               "separator%d"
#define EDC_PART_FIELD_ENABLE_SIG_STR        "field%d,enable"
#define EDC_PART_FIELD_DISABLE_SIG_STR       "field%d,disable"

// struct tm  does not define the fields in the order from year, month, date, hour, minute.
// values are reassigned to an array for easy handling.
#define DATETIME_TM_ARRAY(intptr, tmptr) int *intptr[] = {&(tmptr)->tm_year, \
           &(tmptr)->tm_mon, &(tmptr)->tm_mday, &(tmptr)->tm_hour, &(tmptr)->tm_min}

struct _Datetime_Field
{
   Evas_Object *item_obj;
   char fmt[MAX_FIELD_FORMAT_LEN];
   Elm_Datetime_Field_Type type;
   const char *separator;
   int location;          // location of the field as per the current format
   int min, max;
   Eina_Bool fmt_exist:1;  // whether field format is present or not
   Eina_Bool visible:1;    // whether field can be visible or not
};

struct _Datetime_Mod_Api
{
   Elm_Datetime_Module_Data *(*obj_hook) (Evas_Object *obj);
   void (*obj_unhook) (Elm_Datetime_Module_Data *module_data);
   Evas_Object *(*field_create) (Elm_Datetime_Module_Data *module_data,
                                     Elm_Datetime_Field_Type field_type);
   void (*field_value_display) (Elm_Datetime_Module_Data *module_data,
                                    Evas_Object *obj);
};

struct _Widget_Data
{
   Evas_Object *base;
   Datetime_Field field_list[DATETIME_TYPE_COUNT]; // fixed set of fields.
   struct tm curr_time, min_limit, max_limit;
   Elm_Datetime_Module_Data *mod_data;
   char format[MAX_FORMAT_LEN];
   Eina_Bool user_format:1; // whether user set format or default format.
};

struct _Format_Map
{
   char *fmt_char;
   int def_min;
   int def_max;
};

// default limits for individual fields
static const Format_Map mapping[DATETIME_TYPE_COUNT] = {
   [ELM_DATETIME_YEAR]   =  { "Yy",   0, 137 },
   [ELM_DATETIME_MONTH]  =  { "mbBh", 0,  11  },
   [ELM_DATETIME_DATE]   =  { "de",   1,  31  },
   [ELM_DATETIME_HOUR]   =  { "IHkl", 0,  23  },
   [ELM_DATETIME_MINUTE] =  { "M",    0,  59  },
   [ELM_DATETIME_AMPM]   =  { "pP",   0,  1   }
};

static const char *multifield_formats = "cxXrRTDF";

static Datetime_Mod_Api *dt_mod = NULL;
static const char *widtype = NULL;

static void _del_hook(Evas_Object *obj);
static void _disable_hook(Evas_Object *obj);
static void _translate_hook(Evas_Object *obj);
static void _on_focus_hook(void *data __UNUSED__, Evas_Object *obj);
static void _mirrored_set(Evas_Object *obj, Eina_Bool rtl);
static void _sizing_eval(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _validate_datetime_limits(struct tm *time1, struct tm *time2, Eina_Bool swap);
static void _apply_field_limits(Evas_Object *obj);
static void _apply_range_restrictions(Evas_Object *obj, struct tm *time);
static const char *_field_format_get(Evas_Object * obj, Elm_Datetime_Field_Type field_type);
static void _field_limit_get(Evas_Object * obj, Elm_Datetime_Field_Type field_type,
                             int *range_min, int *range_max);
static void _reload_format(Evas_Object *obj);
static void _field_list_display(Evas_Object *obj);
static void _field_list_arrange(Evas_Object *obj);
static void _field_list_init(Evas_Object *obj);

static const char SIG_CHANGED[] = "changed";
static const char SIG_LANGUAGE_CHANGED[] = "language,changed";
static const Evas_Smart_Cb_Description _signals[] = {
       {SIG_CHANGED, ""},
       {SIG_LANGUAGE_CHANGED, ""},
       {NULL, NULL}
};

static Datetime_Mod_Api *
_dt_mod_init()
{
   Elm_Module *mod = NULL;
   if (!(mod = _elm_module_find_as("datetime/api"))) return NULL;

   mod->api = malloc(sizeof(Datetime_Mod_Api));
   if (!mod->api) return NULL;

   ((Datetime_Mod_Api *)(mod->api))->obj_hook            =   _elm_module_symbol_get(mod, "obj_hook");
   ((Datetime_Mod_Api *)(mod->api))->obj_unhook          =   _elm_module_symbol_get(mod, "obj_unhook");
   ((Datetime_Mod_Api *)(mod->api))->field_create        =   _elm_module_symbol_get(mod, "field_create");
   ((Datetime_Mod_Api *)(mod->api))->field_value_display =   _elm_module_symbol_get(mod, "field_value_display");

   return mod->api;
}

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd;
   Datetime_Field *tmp;
   unsigned int idx;

   wd = elm_widget_data_get(obj);
   if (!wd) return;

   for (idx = 0; idx < DATETIME_TYPE_COUNT; idx++)
     {
        tmp = wd->field_list + idx;
        evas_object_del(tmp->item_obj);
        eina_stringshare_del(tmp->separator);
     }

   if ((dt_mod) && (dt_mod->obj_unhook))
     dt_mod->obj_unhook(wd->mod_data); // module - unhook

   free(wd);
}

static void
_disable_hook(Evas_Object *obj)
{
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd || !wd->base) return;
   if (elm_widget_disabled_get(obj))
     edje_object_signal_emit(wd->base, EDC_DATETIME_DISABLE_SIG_STR, "elm");
   else
     edje_object_signal_emit(wd->base, EDC_DATETIME_ENABLE_SIG_STR, "elm");
}

static void
_translate_hook(Evas_Object *obj)
{
   Widget_Data *wd;
   wd = elm_widget_data_get(obj);
   if (!wd) return;

   if (!wd->user_format) _reload_format(obj);
   else _field_list_display(obj);
   evas_object_smart_callback_call(obj, SIG_LANGUAGE_CHANGED, NULL);
}

static void
_on_focus_hook(void *data __UNUSED__, Evas_Object *obj)
{
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd) return;

   if (elm_widget_focus_get(obj))
     edje_object_signal_emit(wd->base, EDC_DATETIME_FOCUSIN_SIG_STR, "elm");
   else
     edje_object_signal_emit(wd->base, EDC_DATETIME_FOCUSOUT_SIG_STR, "elm");
}

static Eina_List *
_datetime_items_get(const Evas_Object *obj)
{
   Widget_Data *wd;
   Eina_List *items = NULL;
   Datetime_Field *field;
   int loc, count = 0;
   unsigned int idx;

   wd = elm_widget_data_get(obj);
   if (!wd) return NULL;

   for (idx = 0; idx < DATETIME_TYPE_COUNT; idx++)
     {
        field = wd->field_list + idx;
        if (field->fmt_exist && field->visible) count++;
     }
   for (loc = 0; loc < count; loc++)
     {
        for (idx = 0; idx < DATETIME_TYPE_COUNT; idx++)
          {
             field = wd->field_list + idx;
             if (field->location == loc)
               items = eina_list_append(items, field->item_obj);
          }
     }

   return items;
}

static Eina_Bool
_elm_datetime_focus_next_hook(const Evas_Object *obj, Elm_Focus_Direction dir, Evas_Object **next)
{
   Widget_Data *wd;
   const Eina_List *items;
   void *(*list_data_get) (const Eina_List *list);
   Eina_List *(*list_free) (Eina_List *list);
   Eina_Bool ret;

   wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;

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

   ret = elm_widget_focus_list_next_get(obj, items, list_data_get, dir, next);
   if (list_free) list_free((Eina_List *)items);

   return ret;
}

static void
_mirrored_set(Evas_Object *obj, Eina_Bool rtl)
{
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd) return;

   edje_object_mirrored_set(wd->base, rtl);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd;
   Datetime_Field *field;
   Evas_Coord minw = -1, minh = -1;
   unsigned int idx, field_count = 0;

   wd = elm_widget_data_get(obj);
   if (!wd || !wd->base) return;
   for (idx = 0; idx < DATETIME_TYPE_COUNT; idx++)
     {
        field = wd->field_list + idx;
        if ((field->visible) && (field->fmt_exist)) field_count ++;
     }
   if (field_count)
     elm_coords_finger_size_adjust(field_count, &minw, 1, &minh);
   edje_object_size_min_restricted_calc(wd->base, &minw, &minh, minw, minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, -1, -1);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd;
   Datetime_Field *field;
   char buf[BUFFER_SIZE];
   unsigned int idx;

   wd = elm_widget_data_get(obj);
   if (!wd || !wd->base) return;

   _elm_theme_object_set(obj, wd->base, "datetime", "base",
                         elm_widget_style_get(obj));
   _elm_widget_mirrored_reload(obj);
   _mirrored_set(obj, elm_widget_mirrored_get(obj));

   edje_object_scale_set(wd->base, elm_widget_scale_get(obj) * _elm_config->scale);

   if (elm_widget_disabled_get(obj))
     edje_object_signal_emit(wd->base, EDC_DATETIME_DISABLE_SIG_STR,"elm");
   else
     edje_object_signal_emit(wd->base, EDC_DATETIME_ENABLE_SIG_STR, "elm");

   if ((!dt_mod) || (!dt_mod->field_value_display)) return;

   for (idx = 0; idx < DATETIME_TYPE_COUNT; idx++)
     {
        field = wd->field_list + idx;
        if (field->fmt_exist && field->visible)
          {
             snprintf(buf, sizeof(buf), EDC_PART_FIELD_ENABLE_SIG_STR, field->location);
             edje_object_signal_emit(wd->base, buf, "elm");
             snprintf(buf, sizeof(buf), EDC_PART_SEPARATOR_STR, field->location);
             edje_object_part_text_set(wd->base, buf, field->separator);
             dt_mod->field_value_display(wd->mod_data, field->item_obj);
          }
        else
          {
             snprintf(buf, sizeof(buf),EDC_PART_FIELD_DISABLE_SIG_STR, field->location);
             edje_object_signal_emit(wd->base, buf, "elm");
          }
     }
   edje_object_message_signal_process(wd->base);
   _sizing_eval(obj);
}

static int
_max_days_get(int year, int month)
{
   struct tm time1;
   time_t t;
   int day;

   t = time(NULL);
   localtime_r(&t, &time1);
   time1.tm_year = year;
   time1.tm_mon = month;
   for(day = MIN_DAYS_IN_MONTH; day <= mapping[ELM_DATETIME_DATE].def_max; day++)
     {
        time1.tm_mday = day;
        mktime(&time1);
        if (time1.tm_mday == 1) break;
     }
   day --;
   return day;
}

static Eina_Bool
_date_cmp(struct tm *time1, struct tm *time2)
{
   unsigned int idx;
   DATETIME_TM_ARRAY(timearr1, time1);
   DATETIME_TM_ARRAY(timearr2, time2);

   for (idx = 0; idx < DATETIME_TYPE_COUNT - 1; idx++)
     {
        if (*timearr1[idx] != *timearr2[idx])
          return EINA_FALSE;
     }
   return EINA_TRUE;
}

// validates curr_time/min_limt/max_limit according to the newly set value
static void
_validate_datetime_limits(struct tm *time1, struct tm *time2, Eina_Bool swap)
{
   struct tm *t1, *t2;
   unsigned int idx;
   if (!time1 || !time2) return;

   t1 = (swap) ? time2 : time1;
   t2 = (swap) ? time1 : time2;

   DATETIME_TM_ARRAY(timearr1, time1);
   DATETIME_TM_ARRAY(timearr2, time2);
   for (idx = 0; idx < DATETIME_TYPE_COUNT - 1; idx++)
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
   Widget_Data *wd;
   Datetime_Field *field;
   int val;
   unsigned int idx = 0;

   wd = elm_widget_data_get(obj);
   if (!wd) return;

   DATETIME_TM_ARRAY(timearr, &wd->curr_time);
   for (idx = 0; idx < DATETIME_TYPE_COUNT - 1; idx++)
     {
        field = wd->field_list + idx;
        val = *timearr[idx];
        if (val < field->min)
          *timearr[idx] = field->min;
        else if (val > field->max)
          *timearr[idx] = field->max;
     }
   _field_list_display(obj);
}

static void
_apply_range_restrictions(Evas_Object *obj, struct tm *tim)
{
   Widget_Data *wd;
   unsigned int idx;
   int val, min, max;

   wd = elm_widget_data_get(obj);
   if (!wd || !tim) return;

   DATETIME_TM_ARRAY(timearr, tim);
   for (idx = ELM_DATETIME_MONTH; idx < DATETIME_TYPE_COUNT - 1; idx++)
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
_field_format_get(Evas_Object * obj, Elm_Datetime_Field_Type field_type)
{
   Widget_Data *wd;
   Datetime_Field *field;

   wd = elm_widget_data_get(obj);
   if (!wd) return NULL;

   field = wd->field_list + field_type;
   if (!field) return NULL;

   return field->fmt;
}

static void
_field_limit_get(Evas_Object * obj, Elm_Datetime_Field_Type field_type, int *range_min, int *range_max)
{
   Widget_Data *wd;
   Datetime_Field *field;
   int min, max, max_days;
   unsigned int idx;

   wd = elm_widget_data_get(obj);
   if (!wd) return;

   field = wd->field_list + field_type;
   if (!field) return;

   min = field->min;
   max = field->max;

   DATETIME_TM_ARRAY(curr_timearr, &wd->curr_time);
   DATETIME_TM_ARRAY(min_timearr, &wd->min_limit);
   DATETIME_TM_ARRAY(max_timearr, &wd->max_limit);

   for (idx = 0; idx < field->type; idx++)
     if (*curr_timearr[idx] > *min_timearr[idx]) break;
   if ((idx == field_type) && (min < *min_timearr[field_type]))
     min = *min_timearr[field_type];
   if (field_type == ELM_DATETIME_DATE)
     {
        max_days = _max_days_get(wd->curr_time.tm_year, wd->curr_time.tm_mon);
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
_field_list_display(Evas_Object *obj)
{
   Widget_Data *wd;
   Datetime_Field *field;
   unsigned int idx= 0;

   wd = elm_widget_data_get(obj);
   if (!wd) return;

   for (idx = 0; idx < DATETIME_TYPE_COUNT; idx++)
     {
        field = wd->field_list + idx;
        if (field->fmt_exist && field->visible)
          {
             if ((dt_mod) && (dt_mod->field_value_display))
               dt_mod->field_value_display(wd->mod_data, field->item_obj);
          }
     }
}

static void
_field_list_arrange(Evas_Object *obj)
{
   Widget_Data *wd;
   Datetime_Field *field;
   char buf[BUFFER_SIZE];
   int idx;

   wd = elm_widget_data_get(obj);
   if (!wd) return;

   for (idx = 0; idx < DATETIME_TYPE_COUNT; idx++)
     {
        field = wd->field_list + idx;
        edje_object_part_unswallow(wd->base, field->item_obj);
     }
   for (idx = 0; idx < DATETIME_TYPE_COUNT; idx++)
     {
        field = wd->field_list + idx;
        if (field->visible && field->fmt_exist)
          {
             snprintf(buf, sizeof(buf), EDC_PART_FIELD_STR, field->location);
             edje_object_part_swallow(wd->base, buf, field->item_obj);
          }
        else evas_object_hide(field->item_obj);
     }
   _sizing_eval(obj);
   _field_list_display(obj);
}

// returns expanded format string for corresponding multi-field format character
static char *
_expanded_fmt_str_get(char ch)
{
   char *exp_fmt = "";
   switch (ch)
     {
      case 'c':
         exp_fmt = nl_langinfo(D_T_FMT);
         break;
      case 'x':
         exp_fmt = nl_langinfo(D_FMT);
         break;
      case 'X':
         exp_fmt = nl_langinfo(T_FMT);
         break;
      case 'r':
         exp_fmt = nl_langinfo(T_FMT_AMPM);
         break;
      case 'R':
         exp_fmt =  "%H:%M";
         break;
      case 'T':
         exp_fmt =  "%H:%M:%S";
         break;
      case 'D':
         exp_fmt =  "%m/%d/%y";
         break;
      case 'F':
         exp_fmt =  "%Y-%m-%d";
         break;
      default:
         exp_fmt =  "";
         break;
     }
   return exp_fmt;
}

static void
_expand_format(char * dt_fmt)
{
   char *ptr, *expanded_fmt, ch;
   char buf[MAX_FORMAT_LEN] = {0,};
   unsigned int idx = 0, len = 0;
   Eina_Bool fmt_char = EINA_FALSE;

   ptr = dt_fmt;
   while ((ch = *ptr))
     {
        if ((fmt_char) && (strchr(multifield_formats, ch)))
          {
             // replace the multi-field format characters with corresponding expanded format
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
   strncpy(dt_fmt, buf, MAX_FORMAT_LEN);
}

static unsigned int
_parse_format(Evas_Object *obj, char *fmt_ptr)
{
   Widget_Data *wd;
   Datetime_Field *field = NULL;
   unsigned int len = 0, idx, location = 0;
   char separator[MAX_SEPARATOR_LEN];
   char cur;
   Eina_Bool fmt_parsing = EINA_FALSE, sep_parsing = EINA_FALSE,
             sep_lookup = EINA_FALSE;

   wd = elm_widget_data_get(obj);

   while ((cur = *fmt_ptr))
     {
        if (fmt_parsing)
          {
             for (idx = 0; idx < DATETIME_TYPE_COUNT; idx++)
               {
                  if (strchr(mapping[idx].fmt_char, cur))
                    {
                       field = wd->field_list + idx;
                       // ignore the fields already have or disabled
                       // valid formats, means already parsed & repeated, ignore.
                       if (!field->visible || field->location != -1) break;
                       field->fmt[1] = cur;
                       field->fmt_exist = EINA_TRUE;
                       field->location = location++;
                       fmt_parsing = EINA_FALSE;
                       sep_lookup = EINA_TRUE;
                       len = 0;
                       break;
                    }
               }
          }
        if (cur == ' ') separator[len++] = cur;
        else if (cur == '%') fmt_parsing = EINA_TRUE;
        if ((cur == ' ') || (cur == '%'))
          {
             sep_parsing = EINA_FALSE;
             // set the separator to previous field
             separator[len] = 0;
             if (field) eina_stringshare_replace(&field->separator, separator);
          }
        if (sep_parsing && (len < MAX_SEPARATOR_LEN-1) &&
            (field->type != ELM_DATETIME_AMPM) &&
            (!((field->type == ELM_DATETIME_MINUTE) && (cur ==':'))))
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
   Widget_Data *wd;
   Datetime_Field *field;
   char buf[BUFFER_SIZE];
   unsigned int idx, field_count;
   char *dt_fmt;

   wd = elm_widget_data_get(obj);
   if (!wd) return;

   // fetch the default format from Libc.
   if (!wd->user_format)
     strncpy(wd->format, nl_langinfo(D_T_FMT), MAX_FORMAT_LEN);

   dt_fmt = (char *)malloc(MAX_FORMAT_LEN);
   strncpy(dt_fmt, wd->format, MAX_FORMAT_LEN);

   _expand_format(dt_fmt);

   // reset all the fields to disable state
   for (idx = 0; idx < DATETIME_TYPE_COUNT; idx++)
     {
        field = wd->field_list + idx;
        field->fmt_exist = EINA_FALSE;
        field->location = -1;
     }

   field_count = _parse_format(obj, dt_fmt);
   free(dt_fmt);

   // assign locations to disabled fields for uniform usage
   for (idx = 0; idx < DATETIME_TYPE_COUNT; idx++)
     {
        field = wd->field_list + idx;
        if (field->location == -1) field->location = field_count++;

        if (field->fmt_exist && field->visible)
          {
             snprintf(buf, sizeof(buf), EDC_PART_FIELD_ENABLE_SIG_STR,
                      field->location);
             edje_object_signal_emit(wd->base, buf, "elm");
          }
        else
          {
             snprintf(buf, sizeof(buf),EDC_PART_FIELD_DISABLE_SIG_STR,
                      field->location);
             edje_object_signal_emit(wd->base, buf, "elm");
          }
        snprintf(buf, sizeof(buf), EDC_PART_SEPARATOR_STR, (field->location + 1));
        edje_object_part_text_set(wd->base, buf, field->separator);
     }
   edje_object_message_signal_process(wd->base);
   _field_list_arrange(obj);
}

static void
_field_list_init(Evas_Object *obj)
{
   Widget_Data *wd;
   Datetime_Field *field;
   unsigned int idx;
   time_t t;

   wd = elm_widget_data_get(obj);
   if (!wd) return;

   t = time(NULL);
   localtime_r(&t, &wd->curr_time);

   for (idx = 0; idx < DATETIME_TYPE_COUNT; idx++)
     {
        field = wd->field_list + idx;
        field->type = ELM_DATETIME_YEAR + idx;
        field->fmt[0] = '%';
        field->fmt_exist = EINA_FALSE;
        field->visible  = EINA_TRUE;
        field->min = mapping[idx].def_min;
        field->max = mapping[idx].def_max;
     }
   DATETIME_TM_ARRAY(min_timearr, &wd->min_limit);
   DATETIME_TM_ARRAY(max_timearr, &wd->max_limit);
   for (idx = 0; idx < DATETIME_TYPE_COUNT-1; idx++)
     {
        *min_timearr[idx] = mapping[idx].def_min;
        *max_timearr[idx] = mapping[idx].def_max;
     }
}

EAPI Evas_Object *
elm_datetime_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;
   Datetime_Field *field;
   int idx;

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);

   ELM_SET_WIDTYPE(widtype, "datetime");
   elm_widget_type_set(obj, widtype);
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_translate_hook_set(obj, _translate_hook);
   elm_widget_on_focus_hook_set(obj, _on_focus_hook, NULL);
   elm_widget_disable_hook_set(obj, _disable_hook);
   elm_widget_can_focus_set(obj, EINA_TRUE);
   elm_widget_focus_next_hook_set(obj, _elm_datetime_focus_next_hook);

   wd->base = edje_object_add(e);
   elm_widget_resize_object_set(obj, wd->base);
   _elm_theme_object_set(obj, wd->base, "datetime", "base", "default");
   evas_object_smart_callbacks_descriptions_set(obj, _signals);

   // module - initialise module for datetime
   if (!dt_mod) dt_mod = _dt_mod_init();
   if ((dt_mod) && (dt_mod->obj_hook))
     wd->mod_data = dt_mod->obj_hook(obj);
   // update module data
   if (wd->mod_data)
     {
        wd->mod_data->base = obj;
        wd->mod_data->field_limit_get = _field_limit_get;
        wd->mod_data->field_format_get = _field_format_get;
     }

   _field_list_init(obj);
   _reload_format(obj);

   if ((dt_mod)&&(dt_mod->field_create))
     {
        for (idx = 0; idx < DATETIME_TYPE_COUNT; idx++)
          {
             field = wd->field_list + idx;
             field->item_obj = dt_mod->field_create(wd->mod_data, idx);
          }
     }
   _field_list_arrange(obj);
   _mirrored_set(obj, elm_widget_mirrored_get(obj));

   return obj;
}

EAPI const char *
elm_datetime_format_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd) return NULL;

   return wd->format;
}

EAPI void
elm_datetime_format_set(Evas_Object *obj, const char *fmt)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd) return;

   if (fmt)
     {
        strncpy(wd->format, fmt, MAX_FORMAT_LEN);
        wd->user_format = EINA_TRUE;
     }
   else
     wd->user_format = EINA_FALSE;

   _reload_format(obj);
}

EAPI Eina_Bool
elm_datetime_field_visible_get(const Evas_Object *obj, Elm_Datetime_Field_Type
                               fieldtype)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd;
   Datetime_Field *field;

   wd = elm_widget_data_get(obj);
   if (!wd || (fieldtype > ELM_DATETIME_AMPM)) return EINA_FALSE;

   field = wd->field_list + fieldtype;
   return field->visible;
}

EAPI void
elm_datetime_field_visible_set(Evas_Object *obj, Elm_Datetime_Field_Type fieldtype,
                              Eina_Bool visible)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd;
   Datetime_Field *field;

   wd = elm_widget_data_get(obj);
   if (!wd || (fieldtype > ELM_DATETIME_AMPM)) return;

   field = wd->field_list + fieldtype;
   if (field->visible == visible) return;

   field->visible = visible;
   _reload_format(obj);
}

EAPI void
elm_datetime_field_limit_get(const Evas_Object *obj, Elm_Datetime_Field_Type fieldtype,
                             int *min, int *max)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd;
   Datetime_Field *field;

   wd = elm_widget_data_get(obj);
   if (!wd || (fieldtype >= ELM_DATETIME_AMPM)) return;

   field = wd->field_list + fieldtype;
   if (min) *min = field->min;
   if (max) *max = field->max;
}

EAPI void
elm_datetime_field_limit_set(Evas_Object *obj, Elm_Datetime_Field_Type fieldtype,
                             int min, int max)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd;
   Datetime_Field *field;

   wd = elm_widget_data_get(obj);
   if (!wd || (fieldtype >= ELM_DATETIME_AMPM)) return;

   if (min > max) return;

   field = wd->field_list + fieldtype;
   if ((min > mapping[fieldtype].def_min && min < mapping[fieldtype].def_max)
        || (field->type == ELM_DATETIME_YEAR))
     field->min = min;
   if ((max > mapping[fieldtype].def_min && max < mapping[fieldtype].def_max)
        || (field->type == ELM_DATETIME_YEAR))
     field->max = max;

   _apply_field_limits(obj);
}

EAPI Eina_Bool
elm_datetime_value_get(const Evas_Object *obj, struct tm *currtime)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   EINA_SAFETY_ON_NULL_RETURN_VAL(currtime, EINA_FALSE);
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;

   *currtime = wd->curr_time;
   return EINA_TRUE;
}

EAPI Eina_Bool
elm_datetime_value_set(Evas_Object *obj, const struct tm *newtime)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   EINA_SAFETY_ON_NULL_RETURN_VAL(newtime, EINA_FALSE);
   Widget_Data *wd;
   struct tm old_time;

   wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;

   old_time = wd->curr_time;
   wd->curr_time = *newtime;
   // apply default field restrictions for curr_time
   _apply_range_restrictions(obj, &wd->curr_time);
   // validate the curr_time according to the min_limt and max_limt
   _validate_datetime_limits(&wd->curr_time, &wd->min_limit, EINA_FALSE);
   _validate_datetime_limits(&wd->max_limit, &wd->curr_time, EINA_TRUE);
   _apply_field_limits(obj);

   if (!_date_cmp(&old_time, &wd->curr_time))
     evas_object_smart_callback_call(obj, SIG_CHANGED, NULL);

   return EINA_TRUE;
}

EAPI Eina_Bool
elm_datetime_value_min_get(const Evas_Object *obj, struct tm *mintime)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   EINA_SAFETY_ON_NULL_RETURN_VAL(mintime, EINA_FALSE);
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;

   *mintime = wd->min_limit;
   return EINA_TRUE;
}

EAPI Eina_Bool
elm_datetime_value_min_set(Evas_Object *obj, const struct tm *mintime)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   EINA_SAFETY_ON_NULL_RETURN_VAL(mintime, EINA_FALSE);
   Widget_Data *wd;
   struct tm old_time;

   wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;

   wd->min_limit = *mintime;
   old_time = wd->curr_time;
   // apply default field restrictions for min_limit
   _apply_range_restrictions(obj, &wd->min_limit);
   // validate curr_time and max_limt according to the min_limit
   _validate_datetime_limits(&wd->max_limit, &wd->min_limit, EINA_FALSE);
   _validate_datetime_limits(&wd->curr_time, &wd->min_limit, EINA_FALSE);
   _apply_field_limits(obj);

   if (!_date_cmp(&old_time, &wd->curr_time))
     evas_object_smart_callback_call(obj, SIG_CHANGED, NULL);

   return EINA_TRUE;
}

EAPI Eina_Bool
elm_datetime_value_max_get(const Evas_Object *obj, struct tm *maxtime)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   EINA_SAFETY_ON_NULL_RETURN_VAL(maxtime, EINA_FALSE);
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;

   *maxtime = wd->max_limit;
   return EINA_TRUE;
}

EAPI Eina_Bool
elm_datetime_value_max_set(Evas_Object *obj, const struct tm *maxtime)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   EINA_SAFETY_ON_NULL_RETURN_VAL(maxtime, EINA_FALSE);
   Widget_Data *wd;
   struct tm old_time;

   wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;

   wd->max_limit = *maxtime;
   old_time = wd->curr_time;
   // apply default field restrictions for max_limit
   _apply_range_restrictions(obj, &wd->max_limit);
   // validate curr_time and min_limt according to the max_limit
   _validate_datetime_limits(&wd->max_limit, &wd->min_limit, EINA_TRUE);
   _validate_datetime_limits(&wd->max_limit, &wd->curr_time, EINA_TRUE);
   _apply_field_limits(obj);

   if (!_date_cmp(&old_time, &wd->curr_time))
     evas_object_smart_callback_call(obj, SIG_CHANGED, NULL);

   return EINA_TRUE;
}
