#include "config.h"
#include "Efl.h"

#define ERR(...) EINA_LOG_DOM_ERR(EINA_LOG_DOMAIN_DEFAULT, __VA_ARGS__)
#define DBG(...) EINA_LOG_DOM_DBG(EINA_LOG_DOMAIN_DEFAULT, __VA_ARGS__)

typedef enum _Format_Type
{
   FORMAT_TYPE_INVALID,
   FORMAT_TYPE_DOUBLE,
   FORMAT_TYPE_INT,
   FORMAT_TYPE_STRING,
   FORMAT_TYPE_STATIC
} Format_Type;

typedef struct
{
   const char *template;
   Format_Type format_type;
} Efl_Ui_Format_Data;

static Eina_Bool
_is_valid_digit(char x)
{
   return ((x >= '0' && x <= '9') || (x == '.')) ? EINA_TRUE : EINA_FALSE;
}

static Format_Type
_format_string_check(const char *fmt)
{
   const char *itr;
   Eina_Bool found = EINA_FALSE;
   Format_Type ret_type = FORMAT_TYPE_STATIC;

   for (itr = fmt; *itr; itr++)
     {
        if (itr[0] != '%') continue;
        if (itr[1] == '%')
          {
             itr++;
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
                       ret_type = FORMAT_TYPE_INVALID;
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

static void
_default_format_cb(void *data, Eina_Strbuf *str, const Eina_Value value)
{
   Efl_Ui_Format_Data *sd = data;
   Eina_Value copy;

   if (sd->format_type == FORMAT_TYPE_DOUBLE)
     {
        double v = 0.0;
        eina_value_setup(&copy, EINA_VALUE_TYPE_DOUBLE);
        eina_value_convert(&value, &copy);
        eina_value_get(&copy, &v);
        eina_strbuf_append_printf(str, sd->template, v);
        eina_value_flush(&copy);
     }
   else if (sd->format_type == FORMAT_TYPE_INT)
     {
        int v = 0;
        eina_value_setup(&copy, EINA_VALUE_TYPE_INT);
        eina_value_convert(&value, &copy);
        eina_value_get(&copy, &v);
        eina_strbuf_append_printf(str, sd->template, v);
        eina_value_flush(&copy);
     }
   else if (sd->format_type == FORMAT_TYPE_STRING)
     {
        char *v = eina_value_to_string(&value);
        eina_strbuf_append_printf(str, sd->template, v);
        free(v);
     }
   else if (sd->format_type == FORMAT_TYPE_STATIC)
     {
        eina_strbuf_append_printf(str, "%s", sd->template);
     }
   else
     {
        // Error: Discard format string and just print value.
        DBG("Could not guess value type in format string: '%s'", sd->template);
        char *v = eina_value_to_string(&value);
        eina_strbuf_append(str, v);
        free(v);
     }
}

static void
_default_format_free_cb(void *data)
{
   Efl_Ui_Format_Data *sd = data;

   if (sd && sd->template)
     {
        eina_stringshare_del(sd->template);
        sd->template = NULL;
     }
}

EOLIAN static void
_efl_ui_format_format_string_set(Eo *obj, Efl_Ui_Format_Data *sd, const char *template)
{
   if (!template) return;

   eina_stringshare_replace(&sd->template, template);
   sd->format_type = _format_string_check(sd->template);

   efl_ui_format_cb_set(obj, sd, _default_format_cb, _default_format_free_cb);
}

EOLIAN static const char *
_efl_ui_format_format_string_get(const Eo *obj EINA_UNUSED, Efl_Ui_Format_Data *sd)
{
   return sd->template;
}

#include "interfaces/efl_ui_format.eo.c"

