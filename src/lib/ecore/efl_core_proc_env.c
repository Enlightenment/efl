#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>
#ifdef HAVE_CRT_EXTERNS_H
# include <crt_externs.h>
#endif
#include "ecore_private.h"

#define MY_CLASS EFL_CORE_PROC_ENV_CLASS

extern char **environ;

static Efl_Core_Env *env = NULL;

typedef struct {
   Eina_Bool in_sync;
} Efl_Core_Proc_Env_Data;

static void
_sync(Efl_Core_Env *obj, Efl_Core_Proc_Env_Data *pd)
{
   Eina_List *existing_keys = NULL, *n;
   Eina_Iterator *content;
   const char *key;

   pd->in_sync = EINA_TRUE;
   content = efl_core_env_content_get(obj);

   EINA_ITERATOR_FOREACH(content, key)
     {
        existing_keys = eina_list_append(existing_keys, key);
     }

   if (environ)
     {
        char **p;

        for (p = environ; *p; p++)
          {
             char **values;

             values = eina_str_split(*p, "=", 2);
             efl_core_env_set(obj, values[0], values[1]);

             EINA_LIST_FOREACH(existing_keys, n, key)
               {
                  if (!strcmp(key, values[0]))
                    {
                       existing_keys = eina_list_remove_list(existing_keys, n);
                       break;
                    }
               }
          }
     }
   EINA_LIST_FOREACH(existing_keys, n, key)
     {
        efl_core_env_unset(obj, key);
     }
   pd->in_sync = EINA_FALSE;
}

EOLIAN static const char*
_efl_core_proc_env_efl_core_env_env_get(const Eo *obj, Efl_Core_Proc_Env_Data *pd, const char *var)
{
   if (!pd->in_sync)
     _sync((Eo*)obj, pd);
   return efl_core_env_get(efl_super(obj, MY_CLASS), var);
}

EOLIAN static void
_efl_core_proc_env_efl_core_env_env_set(Eo *obj, Efl_Core_Proc_Env_Data *pd, const char *var, const char *value)
{
   efl_core_env_set(efl_super(obj, MY_CLASS), var, value);
   if (!pd->in_sync)
     {
        if (value)
          setenv(var, value, 1);
        else
          unsetenv(var);
     }
}

EOLIAN static void
_efl_core_proc_env_efl_core_env_unset(Eo *obj, Efl_Core_Proc_Env_Data *pd, const char *key)
{
   efl_core_env_unset(efl_super(obj, MY_CLASS), key);
   if (!pd->in_sync)
     {
        unsetenv(key);
     }
}

EOLIAN static void
_efl_core_proc_env_efl_core_env_clear(Eo *obj, Efl_Core_Proc_Env_Data *pd)
{
   efl_core_env_clear(efl_super(obj, MY_CLASS));
   if (!pd->in_sync)
     {
#ifdef HAVE_CLEARENV
        clearenv();
#else
        environ = NULL;
#endif
     }
}


EOLIAN static Efl_Duplicate*
_efl_core_proc_env_efl_duplicate_duplicate(const Eo *obj, Efl_Core_Proc_Env_Data *pd)
{
   if (!pd->in_sync)
     _sync((Eo*) obj, pd);
   return efl_duplicate(efl_super(obj, MY_CLASS));
}

EOLIAN static Eina_Iterator*
_efl_core_proc_env_efl_core_env_content_get(const Eo *obj, Efl_Core_Proc_Env_Data *pd)
{
   if (!pd->in_sync)
     _sync((Eo*) obj, pd);
   return efl_core_env_content_get(efl_super(obj, MY_CLASS));
}

EOLIAN static Efl_Object*
_efl_core_proc_env_efl_object_constructor(Eo *obj, Efl_Core_Proc_Env_Data *pd EINA_UNUSED)
{
   EINA_SAFETY_ON_TRUE_RETURN_VAL(!!env, NULL);

   obj = efl_constructor(efl_super(obj, MY_CLASS));
   return obj;
}

EOLIAN static Efl_Core_Env*
_efl_core_proc_env_self(void)
{
   if (!env)
     {
        efl_domain_current_push(EFL_ID_DOMAIN_SHARED);
        env = efl_add_ref(EFL_CORE_PROC_ENV_CLASS, NULL);
        efl_domain_current_pop();
        efl_wref_add(env, &env);
     }

   return env;
}

#include "efl_core_proc_env.eo.c"
