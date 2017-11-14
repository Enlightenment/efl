#include "config.h"
#include "Efl.h"

typedef struct
{
   const char *template;
} Efl_Ui_Format_Data;

static void
_default_format_cb(void *data, Eina_Strbuf *str, const Eina_Value value)
{
   const Eina_Value_Type *type = eina_value_type_get(&value);
   Efl_Ui_Format_Data *sd = data;

   if (type == EINA_VALUE_TYPE_DOUBLE)
     {
        double v;
        eina_value_get(&value, &v);
        eina_strbuf_append_printf(str, sd->template, v);
     }
   else if (type == EINA_VALUE_TYPE_INT)
     {
        int v;
        eina_value_get(&value, &v);
        eina_strbuf_append_printf(str, sd->template, v);
     }
   else if (type == EINA_VALUE_TYPE_TM)
     {
        struct tm v;
        eina_value_get(&value, &v);
        eina_strbuf_append_strftime(str, sd->template, &v);
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

