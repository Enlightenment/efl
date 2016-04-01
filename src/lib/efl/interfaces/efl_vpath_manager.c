#define EFL_BETA_API_SUPPORT

#include "Efl.h"

typedef struct _Efl_Vpath_Manager_Data Efl_Vpath_Manager_Data;

struct _Efl_Vpath_Manager_Data
{
   Eina_List *list;
};

EOLIAN static Efl_Vpath_File *
_efl_vpath_manager_fetch(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, const char *path EINA_UNUSED)
{
   return NULL;
}

EOLIAN static void
_efl_vpath_manager_register(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, int priority EINA_UNUSED, Efl_Vpath *vpath EINA_UNUSED)
{
}

EOLIAN static void
_efl_vpath_manager_unregister(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, Efl_Vpath *vpath EINA_UNUSED)
{
}

#include "interfaces/efl_vpath_manager.eo.c"
