#define EFL_BETA_API_SUPPORT

#include "Efl.h"

typedef struct _Efl_Vpath_Manager_Data Efl_Vpath_Manager_Data;
typedef struct _Efl_Vpath_Manager_Entry Efl_Vpath_Manager_Entry;

struct _Efl_Vpath_Manager_Data
{
   Eina_List *list;
};

struct _Efl_Vpath_Manager_Entry
{
   Efl_Vpath *vpath;
   int priority;
};

static Efl_Vpath_Manager_Data vpath_manager =
{
   NULL
};

EOLIAN static Efl_Vpath_File *
_efl_vpath_manager_fetch(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, const char *path)
{
   Efl_Vpath_Manager_Entry *entry;
   Eina_List *l;
   Efl_Vpath_File *file;

   EINA_LIST_FOREACH(vpath_manager.list, l, entry)
     {
        file = efl_vpath_fetch(entry->vpath, path);
        if (file) return file;
     }
   return NULL;
}

EOLIAN static void
_efl_vpath_manager_register(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, int priority, Efl_Vpath *vpath)
{
   Efl_Vpath_Manager_Entry *entry = malloc(sizeof(Efl_Vpath_Manager_Entry));
   entry->vpath = vpath;
   entry->priority = priority;
   // XXX: inset sorted
   vpath_manager.list = eina_lisst_prepend(vpath_manager.list, entry);
}

EOLIAN static void
_efl_vpath_manager_unregister(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, Efl_Vpath *vpath)
{
   Efl_Vpath_Manager_Entry *entry;
   Eina_List *l;

   EINA_LIST_FOREACH(vpath_manager.list, l, entry)
     {
        if (entry->vpath == vpath)
          {
             vpath_manager.list = eina_list_remove_list(vpath_manager.list, l);
             free(entry);
             return;
          }
     }
}

#include "interfaces/efl_vpath_manager.eo.c"
