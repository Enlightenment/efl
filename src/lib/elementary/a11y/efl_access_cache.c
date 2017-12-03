#ifdef HAVE_CONFIG_H
  #include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"
#include "efl_access_cache.eo.h"

typedef struct _Efl_Access_Cache_Data
{
    Eina_Hash *objects;
    Efl_Access *root;
} Efl_Access_Cache_Data;

EOLIAN static Eo*
_efl_access_cache_efl_object_constructor(Eo *obj, Efl_Access_Cache_Data *pd)
{
   obj = efl_constructor(efl_super(obj, EFL_ACCESS_CACHE_CLASS));
   pd->objects = eina_hash_pointer_new(NULL);
   return obj;
}

EOLIAN static void
_efl_access_cache_efl_object_destructor(Eo *obj, Efl_Access_Cache_Data *pd)
{
   eina_hash_free(pd->objects);
   efl_destructor(efl_super(obj, EFL_ACCESS_CACHE_CLASS));
}

EOLIAN static void
_efl_access_cache_root_set(Efl_Access_Cache *obj, Efl_Access_Cache_Data *pd, Efl_Access *root)
{
   if (root == pd->root)
     return;

   pd->root = root;
   efl_event_callback_call(obj, EFL_ACCESS_CACHE_EVENT_ROOT_CHANGED, root);
}

EOLIAN static Efl_Access*
_efl_access_cache_root_get(Efl_Access_Cache *obj EINA_UNUSED, Efl_Access_Cache_Data *pd)
{
   return pd->root;
}

EOLIAN static Eina_Bool
_efl_access_cache_contains(Efl_Access_Cache *obj EINA_UNUSED, Efl_Access_Cache_Data *pd, Efl_Access *access)
{
   return eina_hash_find(pd->objects, &access) != NULL;
}

EOLIAN static Eina_Bool
_efl_access_cache_add(Efl_Access_Cache *obj EINA_UNUSED, Efl_Access_Cache_Data *pd, Efl_Access *access)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(access, EINA_FALSE);

   if (!eina_hash_add(pd->objects, &access, access))
     return EINA_FALSE;

   efl_event_callback_call(obj, EFL_ACCESS_CACHE_EVENT_ADDED, access);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_access_cache_remove(Efl_Access_Cache *obj EINA_UNUSED, Efl_Access_Cache_Data *pd, Efl_Access *access)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(access, EINA_FALSE);

   if (!eina_hash_del_by_key(pd->objects, &access))
     return EINA_FALSE;

   efl_event_callback_call(obj, EFL_ACCESS_CACHE_EVENT_REMOVED, access);
   return EINA_TRUE;
}

EOLIAN static Eina_Iterator*
_efl_access_cache_objects_iterator_new(Efl_Access_Cache *obj EINA_UNUSED, Efl_Access_Cache_Data *pd)
{
   return eina_hash_iterator_key_new(pd->objects);
}

#include "efl_access_cache.eo.c"
