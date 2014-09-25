#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include "eolian_database.h"

EAPI Eina_Stringshare *
eolian_event_name_get(const Eolian_Event *event)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(event, NULL);
   return event->name;
}

EAPI const Eolian_Type *
eolian_event_type_get(const Eolian_Event *event)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(event, NULL);
   return event->type;
}

EAPI Eina_Stringshare *
eolian_event_description_get(const Eolian_Event *event)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(event, NULL);
   return event->comment;
}

EAPI Eolian_Object_Scope
eolian_event_scope_get(const Eolian_Event *event)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(event, EOLIAN_SCOPE_PUBLIC);
   return event->scope;
}

EAPI Eina_Stringshare *
eolian_event_c_name_get(const Eolian_Event *event)
{
    char  buf[512];
    char *tmp = buf;
    snprintf(buf, sizeof(buf), "%s_EVENT_%s", event->klass->full_name,
             event->name);
    eina_str_toupper(&tmp);
    while ((tmp = strchr(tmp, '.'))) *tmp = '_';
    return eina_stringshare_add(buf);
}
