#include "config.h"
#include "Efl.h"

#define MY_CLASS EFL_VPATH_FILE_CLASS

typedef struct _Efl_Vpath_File_Data Efl_Vpath_File_Data;

struct _Efl_Vpath_File_Data
{
   const char *path;
   const char *result;
   Eina_Bool called : 1;
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

EOLIAN static Eina_Bool
_efl_vpath_file_do(Eo *obj EINA_UNUSED, Efl_Vpath_File_Data *pd)
{
   if (pd->called) return EINA_FALSE;
   pd->called = EINA_TRUE;
   efl_event_callback_legacy_call(obj, EFL_VPATH_FILE_EVENT_FETCHED, NULL);
   return EINA_TRUE;
}

EOLIAN static void
_efl_vpath_file_wait(Eo *obj EINA_UNUSED, Efl_Vpath_File_Data *pd EINA_UNUSED)
{
   if (!pd->called)
     {
        pd->called = EINA_TRUE;
        if (pd->result)
          efl_event_callback_legacy_call(obj, EFL_VPATH_FILE_EVENT_FETCHED, NULL);
        else
          efl_event_callback_legacy_call(obj, EFL_VPATH_FILE_EVENT_FAILED, NULL);
     }
}

#include "interfaces/efl_vpath_file.eo.c"
