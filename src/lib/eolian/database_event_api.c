#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include "eolian_database.h"

EAPI const Eolian_Type *
eolian_event_type_get(const Eolian_Event *event)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(event, NULL);
   if (event->type && (event->type->type == EOLIAN_TYPE_VOID))
     return NULL;
   return event->type;
}

EAPI const Eolian_Documentation *
eolian_event_documentation_get(const Eolian_Event *event)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(event, NULL);
   return event->doc;
}

EAPI Eolian_Object_Scope
eolian_event_scope_get(const Eolian_Event *event)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(event, EOLIAN_SCOPE_UNKNOWN);
   return event->scope;
}

EAPI Eina_Bool
eolian_event_is_beta(const Eolian_Event *event)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(event, EINA_FALSE);
   return event->is_beta;
}

EAPI Eina_Bool
eolian_event_is_hot(const Eolian_Event *event)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(event, EINA_FALSE);
   return event->is_hot;
}

EAPI Eina_Bool
eolian_event_is_restart(const Eolian_Event *event)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(event, EINA_FALSE);
   return event->is_restart;
}

EAPI Eina_Stringshare *
eolian_event_c_name_get(const Eolian_Event *event)
{
    char  buf[512];
    char *tmp = buf;
    const char *pfx = event->klass->ev_prefix;
    if (!pfx) pfx = event->klass->eo_prefix;
    if (!pfx) pfx = event->klass->base.name;
    snprintf(buf, sizeof(buf), "%s_EVENT_%s", pfx, event->base.name);
    eina_str_toupper(&tmp);
    while ((tmp = strpbrk(tmp, ".,"))) *tmp = '_';
    return eina_stringshare_add(buf);
}

EAPI const Eolian_Event *
eolian_class_event_by_name_get(const Eolian_Class *klass, const char *event_name)
{
   Eina_List *itr;
   Eolian_Event *event = NULL;
   if (!klass) return NULL;
   Eina_Stringshare *shr_ev = eina_stringshare_add(event_name);

   EINA_LIST_FOREACH(klass->events, itr, event)
        {
           if (event->base.name == shr_ev)
              goto end;
        }

   event = NULL;
end:
   eina_stringshare_del(shr_ev);
   return event;
}
