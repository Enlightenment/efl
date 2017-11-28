#include "config.h"
#include "Efl.h"

#define ERR(...) EINA_LOG_DOM_ERR(EINA_LOG_DOMAIN_DEFAULT, __VA_ARGS__)

typedef struct
{
   const char *template;
} Efl_Ui_Format_Data;

typedef enum _Format_Type
{
   FORMAT_TYPE_INVALID,
   FORMAT_TYPE_DOUBLE,
   FORMAT_TYPE_INT
} Format_Type;

static Eina_Bool
_is_valid_digit(char x)
{
   return ((x >= '0' && x <= '9') || (x == '.')) ? EINA_TRUE : EINA_FALSE;
}

static Format_Type
_format_string_check(const char *fmt)
{
   const char *itr = NULL;
   const char *start = NULL;
   Eina_Bool found = EINA_FALSE;
   Format_Type ret_type = FORMAT_TYPE_INVALID;

   start = strchr(fmt, '%');
   if (!start) return 0;

   while (start)
     {
        if (found && start[1] != '%')
          {
             ret_type = FORMAT_TYPE_INVALID;
             break;
          }

        if (start[1] != '%' && !found)
          {
             found = EINA_TRUE;
             for (itr = start + 1; *itr != '\0'; itr++)
               {
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
          }
        start = strchr(start + 2, '%');
     }

   return ret_type;
}

static void
_default_format_cb(void *data, Eina_Strbuf *str, const Eina_Value value)
{
   const Eina_Value_Type *type = eina_value_type_get(&value);
   Efl_Ui_Format_Data *sd = data;
   int format_check_result;

   if (type == EINA_VALUE_TYPE_TM)
     {
        struct tm v;
        eina_value_get(&value, &v);
        eina_strbuf_append_strftime(str, sd->template, &v);

        return;
     }

   format_check_result = _format_string_check(sd->template);

   if (format_check_result == FORMAT_TYPE_INVALID)
     {
        ERR("Wrong String Format: %s", sd->template);
        return;
     }

   if ((format_check_result == FORMAT_TYPE_DOUBLE)
       && (type == EINA_VALUE_TYPE_DOUBLE))
     {
        double v;
        eina_value_get(&value, &v);
        eina_strbuf_append_printf(str, sd->template, v);
     }
   else if ((format_check_result == FORMAT_TYPE_INT)
            && (type == EINA_VALUE_TYPE_INT))
     {
        int v;
        eina_value_get(&value, &v);
        eina_strbuf_append_printf(str, sd->template, v);
     }
   else if ((format_check_result == FORMAT_TYPE_DOUBLE)
            && (type == EINA_VALUE_TYPE_INT))
     {
        int v;
        double d_v;

        eina_value_get(&value, &v);

        d_v = v;
        eina_strbuf_append_printf(str, sd->template, d_v);
     }
   else if ((format_check_result == FORMAT_TYPE_INT)
            && (type == EINA_VALUE_TYPE_DOUBLE))
     {
        double v;
        int i_v;

        eina_value_get(&value, &v);

        i_v = v;
        eina_strbuf_append_printf(str, sd->template, i_v);
     }
   else
     {
        char *v = eina_value_to_string(&value);
        eina_strbuf_append_printf(str, "%s", v);
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
   efl_ui_format_cb_set(obj, sd, _default_format_cb, _default_format_free_cb);
}

EOLIAN static const char *
_efl_ui_format_format_string_get(Eo *obj EINA_UNUSED, Efl_Ui_Format_Data *sd)
{
   return sd->template;
}

#include "interfaces/efl_ui_format.eo.c"

