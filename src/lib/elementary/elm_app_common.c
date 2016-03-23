#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"

char *
_dbus_package_to_path(const char *pkg)
{
   Eina_Strbuf *buffer;
   char *ret;

   buffer = eina_strbuf_new();
   eina_strbuf_append_char(buffer, '/');
   for (; *pkg != '\0'; pkg++)
     {
        if (*pkg == '.')
          eina_strbuf_append_char(buffer, '/');
        else if (isalnum(*pkg))
          eina_strbuf_append_char(buffer, *pkg);
        else
          eina_strbuf_append_printf(buffer, "_%02x", *pkg);
     }

   ret = eina_strbuf_string_steal(buffer);
   eina_strbuf_free(buffer);

   return ret;
}

static const char *_string_states[] = {
   "unknown",
   "live",
   "paused",
   "closed",
   "shallow",
   NULL
};

Elm_App_View_State _string_state_to_id(const char *state)
{
   int i;

   for (i = 0; _string_states[i]; i++)
     {
        if (!strcmp(state, _string_states[i]))
          return i;
     }
   return 0;
}

const char *_state_id_to_string(Elm_App_View_State state)
{
   return _string_states[state];
}
