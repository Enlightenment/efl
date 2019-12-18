#define EFL_UI_FORMAT_PROTECTED 1

#include "config.h"
#include "Efl_Ui.h"
#include "elm_priv.h" /* To be able to use elm_widget_is_legacy() */

typedef enum _Format_Type
{
   /* When a format string is used, it is parsed to find out the expected data type */
   FORMAT_TYPE_INVALID, /* Format description not understood */
   FORMAT_TYPE_DOUBLE,  /* double */
   FORMAT_TYPE_INT,     /* int */
   FORMAT_TYPE_TM,      /* struct tm, for time and date values */
   FORMAT_TYPE_STRING,  /* const char* */
   FORMAT_TYPE_STATIC   /* No value is passed, the format string IS the formatted output */
} Format_Type;

typedef struct
{
   Efl_Ui_Format_Func  format_func;         /* User-supplied formatting function */
   void                *format_func_data;   /* User data for the above function */
   Eina_Free_Cb        format_func_free;    /* How to free the above data */

   Eina_Inarray        *format_values;      /* Array of formatting values, owned by us */

   const char          *format_string;      /* User-supplied formatting string, stringshare */
   Format_Type         format_string_type;  /* Type of data expected in the above string */
} Efl_Ui_Format_Data;

static Eina_Bool
_is_valid_digit(char x)
{
   return ((x >= '0' && x <= '9') || (x == '.')) ? EINA_TRUE : EINA_FALSE;
}

static Format_Type
_format_string_check(const char *fmt, Efl_Ui_Format_String_Type type)
{
   const char *itr;
   Eina_Bool found = EINA_FALSE;
   Format_Type ret_type = FORMAT_TYPE_STATIC;

   if (type == EFL_UI_FORMAT_STRING_TYPE_TIME) return FORMAT_TYPE_TM;

   for (itr = fmt; *itr; itr++)
     {
        if (itr[0] != '%') continue;
        if (itr[1] == '%')
          {
             itr++;
             if (ret_type == FORMAT_TYPE_STATIC)
               ret_type = FORMAT_TYPE_STRING;
             continue;
          }

        if (!found)
          {
             found = EINA_TRUE;
             for (itr++; *itr; itr++)
               {
                  // FIXME: This does not properly support int64 or unsigned.
                  if ((*itr == 'd') || (*itr == 'u') || (*itr == 'i') ||
                      (*itr == 'o') || (*itr == 'x') || (*itr == 'X'))
                    {
                       ret_type = FORMAT_TYPE_INT;
                       break;
                    }
                  else if ((*itr == 'f') || (*itr == 'F'))
                    {
                       ret_type = FORMAT_TYPE_DOUBLE;
                       break;
                    }
                  else if (*itr == 's')
                    {
                       ret_type = FORMAT_TYPE_STRING;
                       break;
                    }
                  else if (_is_valid_digit(*itr))
                    {
                       continue;
                    }
                  else
                    {
                       ERR("Format string '%s' has unknown format element '%c' in format. It must have one format element of type 's', 'f', 'F', 'd', 'u', 'i', 'o', 'x' or 'X'", fmt, *itr);
                       found = EINA_FALSE;
                       break;
                    }
               }
             if (!(*itr)) break;
          }
        else
          {
             ret_type = FORMAT_TYPE_INVALID;
             break;
          }
     }

   if (ret_type == FORMAT_TYPE_INVALID)
     {
        ERR("Format string '%s' is invalid. It must have one and only one format element of type 's', 'f', 'F', 'd', 'u', 'i', 'o', 'x' or 'X'", fmt);
     }
   return ret_type;
}

static Eina_Bool
_do_format_string(Efl_Ui_Format_Data *pd, Eina_Strbuf *str, const Eina_Value value)
{
   switch (pd->format_string_type)
     {
      case FORMAT_TYPE_DOUBLE:
      {
        double v = 0.0;
        if (!eina_value_double_convert(&value, &v))
           ERR("Format conversion failed");
        eina_strbuf_append_printf(str, pd->format_string, v);
        break;
      }
      case FORMAT_TYPE_INT:
      {
        int v = 0;
        if (!eina_value_int_convert(&value, &v))
           ERR("Format conversion failed");
        eina_strbuf_append_printf(str, pd->format_string, v);
        break;
      }
      case FORMAT_TYPE_STRING:
      {
        char *v = eina_value_to_string(&value);
        eina_strbuf_append_printf(str, pd->format_string, v);
        free(v);
        break;
      }
      case FORMAT_TYPE_STATIC:
      {
        eina_strbuf_append(str, pd->format_string);
        break;
      }
      case FORMAT_TYPE_TM:
      {
        struct tm v;
        char *buf = NULL;
        eina_value_get(&value, &v);
        buf = eina_strftime(pd->format_string, &v);
        if (buf)
          {
             eina_strbuf_append(str, buf);
             free(buf);
          }
        break;
      }
      default:
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

static Eina_Bool
_legacy_default_format_func(void *data, Eina_Strbuf *str, const Eina_Value value)
{
   if (!_do_format_string(data, str, value))
      {
        /* Fallback to just printing the value if format string fails (legacy behavior) */
        char *v = eina_value_to_string(&value);
        eina_strbuf_append(str, v);
        free(v);
      }
   return EINA_TRUE;
}

EOLIAN static void
_efl_ui_format_format_func_set(Eo *obj, Efl_Ui_Format_Data *pd, void *func_data, Efl_Ui_Format_Func func, Eina_Free_Cb func_free_cb)
{
   if (pd->format_func_free)
     pd->format_func_free(pd->format_func_data);
   pd->format_func = func;
   pd->format_func_data = func_data;
   pd->format_func_free = func_free_cb;

   if (efl_alive_get(obj))
     efl_ui_format_apply_formatted_value(obj);
}

EOLIAN static Efl_Ui_Format_Func
_efl_ui_format_format_func_get(const Eo *obj EINA_UNUSED, Efl_Ui_Format_Data *pd)
{
   return pd->format_func;
}

static int
_value_compare(const Efl_Ui_Format_Value *val1, const Efl_Ui_Format_Value *val2)
{
   return val1->value - val2->value;
}

EOLIAN static void
_efl_ui_format_format_values_set(Eo *obj, Efl_Ui_Format_Data *pd, Eina_Accessor *values)
{
   Efl_Ui_Format_Value *v;
   int i;
   if (pd->format_values)
     {
        Efl_Ui_Format_Value *vptr;
        /* Delete previous values array */
        EINA_INARRAY_FOREACH(pd->format_values, vptr)
          {
             eina_stringshare_del(vptr->text);
          }
        eina_inarray_free(pd->format_values);
        pd->format_values = NULL;
     }
   if (values == NULL)
     {
        if (efl_alive_get(obj))
          efl_ui_format_apply_formatted_value(obj);
        return;
     }

   /* Copy the values to our internal array */
   pd->format_values = eina_inarray_new(sizeof(Efl_Ui_Format_Value), 4);
   EINA_ACCESSOR_FOREACH(values, i, v)
     {
        Efl_Ui_Format_Value vcopy = { v->value, eina_stringshare_add(v->text) };
        eina_inarray_insert_sorted(pd->format_values, &vcopy, (Eina_Compare_Cb)_value_compare);
     }
   eina_accessor_free(values);

   if (efl_alive_get(obj))
     efl_ui_format_apply_formatted_value(obj);
}

EOLIAN static Eina_Accessor *
_efl_ui_format_format_values_get(const Eo *obj EINA_UNUSED, Efl_Ui_Format_Data *pd)
{
   if (!pd->format_values) return NULL;
   return eina_inarray_accessor_new(pd->format_values);
}

EOLIAN static void
_efl_ui_format_format_string_set(Eo *obj EINA_UNUSED, Efl_Ui_Format_Data *sd, const char *string, Efl_Ui_Format_String_Type type)
{
   eina_stringshare_replace(&sd->format_string, string);
   if (string)
     sd->format_string_type = _format_string_check(sd->format_string, type);
   else
     sd->format_string_type = FORMAT_TYPE_INVALID;

   /* In legacy, setting the format string installs a default format func.
      Some widgets then override the format_func_set method so we keep that behavior. */
   if (elm_widget_is_legacy(obj))
     efl_ui_format_func_set(obj, sd, _legacy_default_format_func, NULL);

   if (efl_alive_get(obj))
     efl_ui_format_apply_formatted_value(obj);
}

EOLIAN static void
_efl_ui_format_format_string_get(const Eo *obj EINA_UNUSED, Efl_Ui_Format_Data *sd, const char **string, Efl_Ui_Format_String_Type *type)
{
   if (string) *string = sd->format_string;
   if (type) *type = sd->format_string_type == FORMAT_TYPE_TM ?
     EFL_UI_FORMAT_STRING_TYPE_TIME : EFL_UI_FORMAT_STRING_TYPE_SIMPLE;
}

EOLIAN static void
_efl_ui_format_formatted_value_get(Eo *obj EINA_UNUSED, Efl_Ui_Format_Data *pd, Eina_Strbuf *str, const Eina_Value value)
{
   char *v;
   eina_strbuf_reset(str);
   if (pd->format_values)
     {
        /* Search in the format_values array if we have one */
        Efl_Ui_Format_Value val = { 0 };
        int ndx;
        if (!eina_value_int_convert(&value, &val.value))
           ERR("Format conversion failed");
        ndx = eina_inarray_search_sorted(pd->format_values, &val, (Eina_Compare_Cb)_value_compare);
        if (ndx > -1) {
          Efl_Ui_Format_Value *entry = eina_inarray_nth(pd->format_values, ndx);
          eina_strbuf_append(str, entry->text);
          return;
        }
     }
   if (pd->format_func)
     {
        /* If we have a formatting function, try to use it */
        if (pd->format_func(pd->format_func_data, str, value))
          return;
     }
   if (pd->format_string)
     {
        /* If we have a formatting string, use it */
        if (_do_format_string(pd, str, value))
          return;
     }

   /* Fallback to just printing the value if everything else fails */
   v = eina_value_to_string(&value);
   eina_strbuf_append(str, v);
   free(v);
}

EOLIAN static int
_efl_ui_format_decimal_places_get(Eo *obj EINA_UNUSED, Efl_Ui_Format_Data *pd)
{
   char result[16] = "0";
   const char *start;

   /* This method can only be called if a format_string has been supplied */
   if (!pd->format_string) return 0;

   start = strchr(pd->format_string, '%');
   while (start)
     {
        if (start[1] != '%')
          {
             start = strchr(start, '.');
             if (start)
                start++;
             break;
          }
        else
          start = strchr(start + 2, '%');
     }

   if (start)
     {
        const char *p = strchr(start, 'f');

        if ((p) && ((p - start) < 15))
          sscanf(start, "%[^f]", result);
     }

   return atoi(result);
}

EOLIAN static void
_efl_ui_format_efl_object_destructor(Eo *obj, Efl_Ui_Format_Data *pd EINA_UNUSED)
{
  /* Legacy widgets keep their own formatting data and have their own destructors */
  if (!elm_widget_is_legacy(obj))
    {
       /* Otherwise, free formatting data */
       efl_ui_format_func_set(obj, NULL, NULL, NULL);
       efl_ui_format_values_set(obj, NULL);
       efl_ui_format_string_set(obj, NULL, 0);
    }
  efl_destructor(efl_super(obj, EFL_UI_FORMAT_MIXIN));
}

#include "efl_ui_format.eo.c"

