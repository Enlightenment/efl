#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>
#include <ctype.h>

#include "ecore_private.h"

#define MY_CLASS EFL_CORE_ENV_CLASS

extern char **environ;

typedef struct {
   Eina_Hash *env;
} Efl_Core_Env_Data;

static inline Eina_Bool
str_valid(const char *var)
{
   return var && var[0] != '\0';
}

static inline Eina_Bool
key_valid(const char *key)
{
   if (!key || key[0] == '\0') return EINA_FALSE;

   if isdigit(key[0]) return EINA_FALSE;

   for (int i = 0; key[i] != '\0'; ++i) {
     if (!isalnum(key[i]) && key[i] != '_') return EINA_FALSE;
   }

   return EINA_TRUE;
}

EOLIAN static void
_efl_core_env_env_set(Eo *obj EINA_UNUSED, Efl_Core_Env_Data *pd, const char *var, const char *value)
{
   EINA_SAFETY_ON_FALSE_RETURN(key_valid(var));
   if (str_valid(value))
     {
        Eina_Stringshare *share;
        share = eina_hash_set(pd->env, var, eina_stringshare_add(value));
        if (share) eina_stringshare_del(share);
     }
   else
     eina_hash_del(pd->env, var, NULL);
}

EOLIAN static const char*
_efl_core_env_env_get(const Eo *obj EINA_UNUSED, Efl_Core_Env_Data *pd, const char *var)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL(key_valid(var), NULL);

   return eina_hash_find(pd->env, var);
}

EOLIAN static void
_efl_core_env_unset(Eo *obj EINA_UNUSED, Efl_Core_Env_Data *pd, const char *var)
{
   EINA_SAFETY_ON_FALSE_RETURN(key_valid(var));
   eina_hash_del_by_key(pd->env, var);
}

EOLIAN static void
_efl_core_env_clear(Eo *obj EINA_UNUSED, Efl_Core_Env_Data *pd)
{
   eina_hash_free_buckets(pd->env);
}

EOLIAN static Efl_Core_Env*
_efl_core_env_efl_duplicate_duplicate(const Eo *obj EINA_UNUSED, Efl_Core_Env_Data *pd)
{
   Efl_Core_Env *fork = efl_add_ref(MY_CLASS, NULL);
   Eina_Iterator *iter;
   Eina_Hash_Tuple *tuple;

   iter = eina_hash_iterator_tuple_new(pd->env);

   EINA_ITERATOR_FOREACH(iter, tuple)
     {
        efl_core_env_set(fork, tuple->key, tuple->data);
     }

   eina_iterator_free(iter);
   return fork;
}

EOLIAN static Efl_Object*
_efl_core_env_efl_object_constructor(Eo *obj, Efl_Core_Env_Data *pd)
{
   pd->env = eina_hash_string_superfast_new((Eina_Free_Cb)eina_stringshare_del);

   return efl_constructor(efl_super(obj, MY_CLASS));
}

EOLIAN static Eina_Iterator*
_efl_core_env_content_get(const Eo *obj EINA_UNUSED, Efl_Core_Env_Data *pd)
{
   Eina_Iterator *iter = eina_hash_iterator_key_new(pd->env);
   return iter;
}


#include "efl_core_env.eo.c"
