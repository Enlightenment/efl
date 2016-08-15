#include "config.h"
#include "Efl.h"

#define MY_CLASS EFL_VPATH_MANAGER_CLASS

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
   file = efl_add(EFL_VPATH_FILE_CLASS, NULL);
   if (file)
     {
        efl_vpath_file_path_set(file, path);
        efl_vpath_file_result_set(file, path);
     }
   return file;
}

static int
_register_sort_cb(Efl_Vpath_Manager_Entry *e1, Efl_Vpath_Manager_Entry *e2)
{
   // sort higher numbers first in  list
   return (e2->priority - e1->priority);
}

static void
_cb_vpath_del(void *data, const Eo_Event *event)
{
   efl_vpath_manager_unregister(EFL_VPATH_MANAGER_CLASS, event->object);
   efl_event_callback_del(event->object, EFL_EVENT_DEL, _cb_vpath_del, data);
}

EOLIAN static void
_efl_vpath_manager_register(Eo *obj, void *pd EINA_UNUSED, int priority, Efl_Vpath *vpath)
{
   Efl_Vpath_Manager_Entry *entry = malloc(sizeof(Efl_Vpath_Manager_Entry));
   entry->vpath = vpath;
   entry->priority = priority;
   efl_event_callback_add(vpath, EFL_EVENT_DEL, _cb_vpath_del, obj);
   vpath_manager.list = eina_list_sorted_insert
     (vpath_manager.list, EINA_COMPARE_CB(_register_sort_cb), entry);
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
