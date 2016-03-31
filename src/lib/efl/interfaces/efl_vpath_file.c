#define EFL_BETA_API_SUPPORT

#include "Efl.h"

typedef struct _Efl_Vpath_File_Data Efl_Vpath_File_Data;

struct _Efl_Vpath_File_Data
{
   const char *path;
   const char *result;
};

EOLIAN static void
_efl_vpath_file_path_set(Eo *obj EINA_UNUSED, Efl_Vpath_File_Data *pd, const char *path)
{
   eina_stringshare_replace(&(pd->path), path);
}

EOLIAN static const char *
_efl_vpath_file_path_get(Eo *obj EINA_UNUSED, Efl_Vpath_File_Data *pd)
{
   return pd->path;
}

EOLIAN static void
_efl_vpath_file_result_set(Eo *obj EINA_UNUSED, Efl_Vpath_File_Data *pd, const char *path)
{
   eina_stringshare_replace(&(pd->result), path);
}

EOLIAN static const char *
_efl_vpath_file_result_get(Eo *obj EINA_UNUSED, Efl_Vpath_File_Data *pd)
{
   return pd->result;
}

EOLIAN static void
_efl_vpath_file_do(Eo *obj EINA_UNUSED, Efl_Vpath_File_Data *pd EINA_UNUSED)
{
   // XXX: begin resolve or fetch for parent vpath object
}

#include "interfaces/efl_vpath_file.eo.c"
