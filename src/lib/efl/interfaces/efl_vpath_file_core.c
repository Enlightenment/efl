#include "config.h"
#include "Efl.h"

#define MY_CLASS EFL_VPATH_FILE_CORE_CLASS

typedef struct _Efl_Vpath_File_Core_Data Efl_Vpath_File_Core_Data;

struct _Efl_Vpath_File_Core_Data
{
   int dummy;
};

EOLIAN static Efl_Object *
_efl_vpath_file_core_efl_object_constructor(Eo *obj, Efl_Vpath_File_Core_Data *pd)
{
   obj = efl_constructor(eo_super(obj, MY_CLASS));
   pd->dummy = 0;
   return obj;
}

EOLIAN static void
_efl_vpath_file_core_efl_object_destructor(Eo *obj, Efl_Vpath_File_Core_Data *pd)
{
   pd->dummy = 0;
   efl_destructor(eo_super(obj, MY_CLASS));
}

EOLIAN static Eina_Bool
_efl_vpath_file_core_efl_vpath_file_do(Eo *obj, Efl_Vpath_File_Core_Data *pd)
{
   const char *path;

   if (efl_vpath_file_result_get(obj))
     return efl_vpath_file_do(eo_super(obj, MY_CLASS));
   // vpath core didn't find a match, so it'ss likely a protocol like
   // http:// etc. etc. so deal with that here
   path = efl_vpath_file_path_get(obj);
   if (path)
     {
        if ((!strncasecmp(path, "http://", 7)) ||
            (!strncasecmp(path, "https://", 8)))
          {
             // XXX: handle urls --- need a loop object
          }
     }
   // ...
   pd->dummy = 0;

   // not a magic path - just set result to path
   efl_vpath_file_result_set(obj, efl_vpath_file_path_get(obj));
   return efl_vpath_file_do(eo_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_vpath_file_core_efl_vpath_file_wait(Eo *obj, Efl_Vpath_File_Core_Data *pd)
{
   if (efl_vpath_file_result_get(obj))
     {
        efl_vpath_file_do(eo_super(obj, MY_CLASS));
        return;
     }
   pd->dummy = 0;
   // XXX: not found yet, so do what is necessary 
}

#include "interfaces/efl_vpath_file_core.eo.c"
