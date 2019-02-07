#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Efl.h>

typedef struct _Efl_File_Data Efl_File_Data;
struct _Efl_File_Data
{
   Efl_Gfx_Image_Load_Error error;
};

static Eina_Bool
_efl_file_file_set(Eo *obj, Efl_File_Data *pd, const char *file, const char *key)
{
   char *tmp = NULL;
   Eina_File *f = NULL;
   Eina_Bool r = EINA_FALSE;

   pd->error = EFL_GFX_IMAGE_LOAD_ERROR_DOES_NOT_EXIST;

   tmp = (char*)(file);
   if (tmp)
     {
        tmp = eina_vpath_resolve(tmp);
     }

   if (tmp)
     {
        f = eina_file_open(tmp, EINA_FALSE);
        if (!f) goto on_error;
     }

   pd->error = EFL_GFX_IMAGE_LOAD_ERROR_NONE;

   r = efl_file_mmap_set(obj, f, key);
   if (f) eina_file_close(f);

 on_error:

   free(tmp);
   return r;
}

static void
_efl_file_file_get(const Eo *obj, Efl_File_Data *pd EINA_UNUSED, const char **file, const char **key)
{
   const Eina_File *f = NULL;

   efl_file_mmap_get(obj, &f, key);

   if (f && file) *file = eina_file_filename_get(f);
   else if (file) *file = NULL;
}

static Efl_Gfx_Image_Load_Error
_efl_file_load_error_get(const Eo *obj EINA_UNUSED, Efl_File_Data *pd)
{
   return pd->error;
}

#include "interfaces/efl_file.eo.c"
#include "interfaces/efl_file_save.eo.c"
