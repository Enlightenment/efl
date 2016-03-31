#define EFL_BETA_API_SUPPORT

#include "Efl.h"

typedef struct _Efl_Vpath_Core_Data Efl_Vpath_Core_Data;

struct _Efl_Vpath_Core_Data
{
   Eina_Hash *meta;
};

static Efl_Vpath_Core *vpath_core = NULL;

EOLIAN static Efl_Vpath_Core *
_efl_vpath_core_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   return vpath_core;
}

EOLIAN static const char *
_efl_vpath_core_meta_get(Eo *obj EINA_UNUSED, Efl_Vpath_Core_Data *pd, const char *key)
{
   return eina_hash_find(pd->meta, key);
}

EOLIAN static void
_efl_vpath_core_meta_set(Eo *obj EINA_UNUSED, Efl_Vpath_Core_Data *pd, const char *key, const char *path)
{
   eina_hash_add(pd->meta, key, eina_stringshare_add(path));
}

EOLIAN static void
_efl_vpath_core_eo_base_destructor(Eo *obj, Efl_Vpath_Core_Data *pd)
{
   eina_hash_free(pd->meta);
   pd->meta = NULL;
   if (vpath_core == obj) vpath_core = NULL;
}

EOLIAN static Eo_Base *
_efl_vpath_core_eo_base_constructor(Eo *obj, Efl_Vpath_Core_Data *pd)
{
   if (vpath_core) return NULL;
   pd->meta = eina_hash_string_superfast_new
     ((Eina_Free_Cb)eina_stringshare_del);
   vpath_core = obj;
   return obj;
}

EOLIAN static Efl_Vpath_File *
_efl_vpath_core_efl_vpath_fetch(Eo *obj EINA_UNUSED, Efl_Vpath_Core_Data *pd EINA_UNUSED, const char *path EINA_UNUSED)
{
   // XXX: implement parsse of path then look up in hash if not just create
   // object where path and result are the same and return that
   return NULL;
}

#include "interfaces/efl_vpath_core.eo.c"
