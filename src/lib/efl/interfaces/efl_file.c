#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Efl.h>

typedef struct _Efl_File_Data Efl_File_Data;
struct _Efl_File_Data
{
   Eo *vo;

   Efl_Image_Load_Error error;
};

static Eina_Bool
_efl_file_file_set(Eo *obj, Efl_File_Data *pd, const char *file, const char *key)
{
   Eina_File *f;
   Eina_Bool r = EINA_FALSE;

   pd->error = EFL_IMAGE_LOAD_ERROR_DOES_NOT_EXIST;

   if (file)
     {
        pd->vo = efl_vpath_manager_fetch(EFL_VPATH_MANAGER_CLASS, file);
        efl_vpath_file_do(pd->vo);
        // XXX:FIXME: allow this to be async
        efl_vpath_file_wait(pd->vo);
        file = efl_vpath_file_result_get(pd->vo);
     }

   if (file)
     {
        f = eina_file_open(file, EINA_FALSE);
        if (!f) goto on_error;
     }

   pd->error = EFL_IMAGE_LOAD_ERROR_NONE;

   r = efl_file_mmap_set(obj, f, key);
   if (f) eina_file_close(f);

 on_error:
   if (pd->vo && (!efl_vpath_file_keep_get(pd->vo)))
     {
        efl_del(pd->vo);
        pd->vo = NULL;
     }

   return r;
}

static void
_efl_file_file_get(Eo *obj, Efl_File_Data *pd EINA_UNUSED, const char **file, const char **key)
{
   const Eina_File *f = NULL;

   efl_file_mmap_get(obj, &f, key);

   if (f && file) *file = eina_file_filename_get(f);
}

static Efl_Image_Load_Error
_efl_file_load_error_get(Eo *obj EINA_UNUSED, Efl_File_Data *pd)
{
   return pd->error;
}

#include "interfaces/efl_file.eo.c"
