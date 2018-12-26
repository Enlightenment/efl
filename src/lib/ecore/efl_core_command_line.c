#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define EFL_CORE_COMMAND_LINE_PROTECTED

#include <Efl_Core.h>

#define MY_CLASS EFL_CORE_COMMAND_LINE_MIXIN

typedef struct {
   Eina_Bool filled;
   char *string_command;
   Eina_Array *command;
} Efl_Core_Command_Line_Data;

static Eina_Array *
_unescape(const char *s)
{
   Eina_Array *args;
   const char *p;
   char *tmp = NULL, *d = NULL;
   if (!s) return NULL;

   Eina_Bool in_quote_dbl = EINA_FALSE;
   Eina_Bool in_quote = EINA_FALSE;

   args = eina_array_new(16);
   if (!args) return NULL;
   for (p = s; *p; p++)
     {
        if (!tmp) tmp = d = strdup(p);
        if (tmp)
          {
             if (in_quote_dbl)
               {
                  switch (*p)
                    {
                     case '\"':
                       in_quote_dbl = EINA_FALSE;
                       *d = 0;
                       eina_array_push(args, eina_stringshare_add(tmp));
                       free(tmp);
                       tmp = d = NULL;
                       break;
                     case '\\':
                       p++;
                       EINA_FALLTHROUGH
                     default:
                       *d = *p;
                       d++;
                       break;
                    }
               }
             else if (in_quote)
               {
                  switch (*p)
                    {
                     case '\'':
                       in_quote = EINA_FALSE;
                       *d = 0;
                       eina_array_push(args, eina_stringshare_add(tmp));
                       free(tmp);
                       tmp = d = NULL;
                       break;
                     case '\\':
                       p++;
                       EINA_FALLTHROUGH
                     default:
                       *d = *p;
                       d++;
                       break;
                    }
               }
             else
               {
                  switch (*p)
                    {
                     case ' ':
                     case '\t':
                     case '\r':
                     case '\n':
                       *d = 0;
                       eina_array_push(args, eina_stringshare_add(tmp));
                       free(tmp);
                       tmp = d = NULL;
                       break;
                     case '\"':
                       in_quote_dbl = EINA_TRUE;
                       break;
                     case '\'':
                       in_quote = EINA_TRUE;
                       break;
                     case '\\':
                       p++;
                       EINA_FALLTHROUGH
                     default:
                       *d = *p;
                       d++;
                       break;
                    }
               }
          }
     }
   if (tmp)
     {
        *d = 0;
        eina_array_push(args, eina_stringshare_add(tmp));
        free(tmp);
     }
   return args;
}

static char *
_escape(const char *s)
{
   Eina_Bool need_quote = EINA_FALSE;
   const char *p;
   char *s2 = malloc((strlen(s) * 2) + 1 + 2), *d;

   if (!s2) return NULL;

   for (p = s; *p; p++)
     {
        switch (*p)
          {
           case '\'':
           case '\"':
           case '$':
           case '#':
           case ';':
           case '&':
           case '`':
           case '|':
           case '(':
           case ')':
           case '[':
           case ']':
           case '{':
           case '}':
           case '>':
           case '<':
           case '\n':
           case '\r':
           case '\t':
           case ' ':
             need_quote = EINA_TRUE;
           default:
             break;
          }
     }

   d = s2;
   if (need_quote)
     {
        *d = '\"';
        d++;
     }
   for (p = s; *p; p++, d++)
     {
        switch (*p)
          {
           case '\\':
           case '\'':
           case '\"':
             *d = '\\';
             d++;
             EINA_FALLTHROUGH
           default:
             *d = *p;
             break;
          }
     }
   if (need_quote)
     {
        *d = '\"';
        d++;
     }
   *d = 0;
   return s2;
}

EOLIAN static const char*
_efl_core_command_line_command_get(const Eo *obj EINA_UNUSED, Efl_Core_Command_Line_Data *pd)
{
   return eina_strdup(pd->string_command);
}

EOLIAN static Eina_Accessor*
_efl_core_command_line_command_access(Eo *obj EINA_UNUSED, Efl_Core_Command_Line_Data *pd)
{
   return pd->command ? eina_array_accessor_new(pd->command) : NULL;
}

static void
_remove_invalid_chars(char *command)
{
   for (unsigned int i = 0; i < strlen(command); ++i)
     {
        char c = command[i];
        if (c < 0x20 || c == 0x7f)
          command[i] = '\x12';
     }
}

EOLIAN static Eina_Bool
_efl_core_command_line_command_array_set(Eo *obj EINA_UNUSED, Efl_Core_Command_Line_Data *pd, Eina_Array *array)
{
   EINA_SAFETY_ON_TRUE_RETURN_VAL(pd->filled, EINA_FALSE);
   Eina_Strbuf *command = eina_strbuf_new();
   unsigned int i = 0;

   pd->command = eina_array_new(eina_array_count(array));
   for (i = 0; i < (array ? eina_array_count(array) : 0); ++i)
     {
        char *content = eina_array_data_get(array, i);
        char *param = calloc(1, strlen(content));

        if (!param)
          {
             free(param);
             while (eina_array_count(pd->command) > 0)
              eina_stringshare_del(eina_array_pop(pd->command));
             eina_array_free(pd->command);
             pd->command = NULL;
             eina_array_free(array);
             return EINA_FALSE;
          }

        //build the command
        if (i != 0)
          eina_strbuf_append(command, " ");
        eina_strbuf_append(command, _escape(content));
        //convert string to stringshare
        strcpy(param, content);
        _remove_invalid_chars(param);
        eina_array_push(pd->command, eina_stringshare_add(param));
        free(param);
     }
   pd->string_command = eina_strbuf_release(command);
   pd->filled = EINA_TRUE;
   eina_array_free(array);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_core_command_line_command_string_set(Eo *obj EINA_UNUSED, Efl_Core_Command_Line_Data *pd, const char *str)
{
   EINA_SAFETY_ON_TRUE_RETURN_VAL(pd->filled, EINA_FALSE);

   pd->string_command = eina_strdup(str);
   _remove_invalid_chars(pd->string_command);
   pd->command = _unescape(str);
   if (!pd->command)
     {
        if (pd->string_command)
          free(pd->string_command);
        pd->string_command = NULL;
        return EINA_FALSE;
     }
   pd->filled = EINA_TRUE;

   return EINA_TRUE;
}

#include "efl_core_command_line.eo.c"
