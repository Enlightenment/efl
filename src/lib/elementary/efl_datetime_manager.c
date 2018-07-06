#include <config.h>
#include "Efl.h"

#ifdef HAVE_LANGINFO_H
# include <langinfo.h>
#endif

#include <Elementary.h>
#include "elm_priv.h"

#define MY_CLASS EFL_DATETIME_MANAGER_CLASS

#define MAX_FORMAT_LEN 32

static const char *multifield_formats = "cxXrRTDF";

typedef struct
{
   Efl_Time time;
   char format[MAX_FORMAT_LEN];
} Efl_Datetime_Manager_Data;

Eina_Bool init = EINA_FALSE;

static void
_time_init(Efl_Time *curr_time)
{
   time_t t;

   t = time(NULL);
   localtime_r(&t, curr_time);

   init = EINA_TRUE;
}

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
   char buf[MAX_FORMAT_LEN] = {0, };
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
        strncpy(dt_fmt, buf, MAX_FORMAT_LEN);
   } while (fmt_expanded);
}

EOLIAN static Eo*
_efl_datetime_manager_efl_object_constructor(Eo *obj,  Efl_Datetime_Manager_Data *pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   return obj;
}


EOLIAN static void
_efl_datetime_manager_efl_object_destructor(Eo *obj,  Efl_Datetime_Manager_Data *pd EINA_UNUSED)
{
   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_datetime_manager_value_set(Eo *obj EINA_UNUSED, Efl_Datetime_Manager_Data *pd, Efl_Time newtime)
{
   pd->time = newtime;
}

EOLIAN static Efl_Time
_efl_datetime_manager_value_get(const Eo *obj EINA_UNUSED, Efl_Datetime_Manager_Data *pd)
{
   if (!init) _time_init(&pd->time);

   return pd->time;
}

EOLIAN static void
_efl_datetime_manager_format_set(Eo *obj EINA_UNUSED, Efl_Datetime_Manager_Data *pd EINA_UNUSED, const char *fmt EINA_UNUSED)
{
   //Is this needed?
}

EOLIAN static const char *
_efl_datetime_manager_format_get(const Eo *obj EINA_UNUSED, Efl_Datetime_Manager_Data *pd)
{
#if defined(HAVE_LANGINFO_H) || defined (_WIN32)
   strncpy(pd->format, nl_langinfo(D_T_FMT), MAX_FORMAT_LEN);
#else
   strncpy(pd->format, "", MAX_FORMAT_LEN);
#endif
   pd->format[MAX_FORMAT_LEN - 1] = '\0';

   _expand_format(pd->format);

   return pd->format;
}
EOLIAN static const char *
_efl_datetime_manager_string_get(const Eo *obj EINA_UNUSED, Efl_Datetime_Manager_Data *pd EINA_UNUSED, const char *fmt EINA_UNUSED)
{
    //TODO: strftime on upsteam, icu module connect here on tizen.
   return NULL;
}
#include "efl_datetime_manager.eo.c"

