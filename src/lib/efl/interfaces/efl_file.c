#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Efl.h>

typedef struct _Efl_File_Data Efl_File_Data;
struct _Efl_File_Data
{
   Eina_Stringshare *vpath; /* efl_file_set */
   Eina_Stringshare *key; /* efl_file_key_set */
   Eina_File *file; /* efl_file_mmap_set */
   Eina_Bool file_opened : 1; /* if `file` was opened implicitly during load */
   Eina_Bool setting : 1; /* set when this file is internally calling methods to avoid infinite recursion */
   Eina_Bool loaded : 1; /* whether the currently set file properties have been loaded */
};

EOLIAN static void
_efl_file_unload(Eo *obj, Efl_File_Data *pd)
{
   if (!pd->loaded) return;
   if (!pd->file) return;
   if (!pd->file_opened) return;
   pd->setting = 1;
   eina_file_close(pd->file);
   efl_file_mmap_set(obj, NULL);
   pd->setting = 0;
   pd->loaded = pd->file_opened = EINA_FALSE;
}

EOLIAN static Eina_Error
_efl_file_load(Eo *obj, Efl_File_Data *pd)
{
   Eina_Error ret = 0;

   if (pd->loaded) return 0;
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->vpath, ENOENT);
   errno = 0;
   if (!pd->file)
     {
        Eina_File *f;
        f = eina_file_open(pd->vpath, EINA_FALSE);
        if (!f) return errno;
        pd->file_opened = EINA_TRUE;
        pd->setting = 1;
        ret = efl_file_mmap_set(obj, f);
        pd->setting = 0;
        if (ret) pd->file_opened = EINA_FALSE;
        eina_file_close(f);
     }
   pd->loaded = !ret;
   return ret;
}

EOLIAN static Eina_Error
_efl_file_mmap_set(Eo *obj, Efl_File_Data *pd, const Eina_File *f)
{
   Eina_Error err = 0;
   Eina_File *file = NULL;

   if (f == pd->file) return 0;
   if (f)
     {
        file = eina_file_dup(f);
        if (!file) return errno;
     }
   if (pd->file) eina_file_close(pd->file);
   pd->file = file;
   pd->loaded = EINA_FALSE;
   
   if (!pd->setting)
     {
        /* avoid infinite recursion */
        pd->setting = 1;
        err = efl_file_set(obj, eina_file_filename_get(pd->file));
        pd->setting = 0;
     }
   return err;
}

EOLIAN static const Eina_File *
_efl_file_mmap_get(const Eo *obj EINA_UNUSED, Efl_File_Data *pd)
{
   return pd->file;
}

EOLIAN static Eina_Error
_efl_file_file_set(Eo *obj, Efl_File_Data *pd, const char *file)
{
   char *tmp;
   Eina_Error err = 0;
   Eina_Bool same;

   tmp = (char*)(file);
   if (tmp)
     tmp = eina_vpath_resolve(tmp);

   same = !eina_stringshare_replace(&pd->vpath, tmp ?: file);
   free(tmp);
   if (same) return err;
   pd->loaded = EINA_FALSE;
   if (!pd->setting)
     {
        pd->setting = 1;
        err = efl_file_mmap_set(obj, NULL);
        pd->setting = 0;
     }
   return err;
}

EOLIAN static Eina_Stringshare *
_efl_file_file_get(const Eo *obj EINA_UNUSED, Efl_File_Data *pd)
{
   return pd->vpath;
}

EOLIAN static void
_efl_file_key_set(Eo *obj EINA_UNUSED, Efl_File_Data *pd, const char *key)
{
   if (eina_stringshare_replace(&pd->key, key))
     pd->loaded = 0;
}

EOLIAN static Eina_Stringshare *
_efl_file_key_get(const Eo *obj EINA_UNUSED, Efl_File_Data *pd)
{
   return pd->key;
}

EOLIAN static Eina_Bool 
_efl_file_loaded_get(const Eo *obj EINA_UNUSED, Efl_File_Data *pd)
{
   return pd->loaded;
}

EOLIAN static void
_efl_file_efl_object_destructor(Eo *obj, Efl_File_Data *pd)
{
   eina_stringshare_del(pd->vpath);
   eina_stringshare_del(pd->key);
   eina_file_close(pd->file);
   efl_destructor(efl_super(obj, EFL_FILE_MIXIN));
}

EOLIAN static Eo *
_efl_file_efl_object_finalize(Eo *obj, Efl_File_Data *pd)
{
   obj = efl_finalize(efl_super(obj, EFL_FILE_MIXIN));
   if (!obj) return NULL;
   if (pd->file || pd->vpath) efl_file_load(obj);
   return obj;
}

////////////////////////////////////////////////////////////////////////////

EAPI Eina_Bool
efl_file_simple_load(Eo *obj, const char *file, const char *key)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);
   efl_ref(obj);
   EINA_SAFETY_ON_TRUE_GOTO(efl_file_set(obj, file), fail);
   efl_file_key_set(obj, key);
   if (file)
     {
        if (efl_file_load(obj)) goto fail;
        efl_unref(obj);
        return EINA_TRUE;
     }
   efl_file_unload(obj);
   efl_unref(obj);
   return EINA_TRUE;
fail:
   efl_unref(obj);
   return EINA_FALSE;
}

EAPI Eina_Bool
efl_file_simple_mmap_load(Eo *obj, const Eina_File *file, const char *key)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);
   efl_ref(obj);
   EINA_SAFETY_ON_TRUE_GOTO(efl_file_mmap_set(obj, file), fail);
   efl_file_key_set(obj, key);
   if (file)
     {
        if (efl_file_load(obj)) goto fail;
        efl_unref(obj);
        return EINA_TRUE;
     }
   efl_file_unload(obj);
   efl_unref(obj);
   return EINA_TRUE;
fail:
   efl_unref(obj);
   return EINA_FALSE;
}

EAPI void
efl_file_simple_get(const Eo *obj, const char **file, const char **key)
{
   efl_ref((Eo*)obj);
   if (file) *file = efl_file_get(obj);
   if (key) *key = efl_file_key_get(obj);
   efl_unref((Eo*)obj);
}

EAPI void
efl_file_simple_mmap_get(const Eo *obj, const Eina_File **file, const char **key)
{
   efl_ref((Eo*)obj);
   if (file) *file = efl_file_mmap_get(obj);
   if (key) *key = efl_file_key_get(obj);
   efl_unref((Eo*)obj);
}

#include "interfaces/efl_file.eo.c"
#include "interfaces/efl_file_save.eo.c"
