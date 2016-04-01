#define EFL_BETA_API_SUPPORT

#include "Efl.h"

typedef struct _Efl_Vpath_Data Efl_Vpath_Data;

struct _Efl_Vpath_Data
{
   int dummy;
};

EOLIAN static Efl_Vpath_File *
_efl_vpath_fetch(Eo *obj EINA_UNUSED, Efl_Vpath_Data *pd EINA_UNUSED, const char *path EINA_UNUSED)
{
   return NULL;
}

#include "interfaces/efl_vpath.eo.c"
