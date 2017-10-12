#ifndef EFL_SELECTION_MANAGER_PRIVATE_H
#define EFL_SELECTION_MANAGER_PRIVATE_H

#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Efl_Selection_Manager_Atom Efl_Selection_Manager_Atom;
typedef struct _Efl_Selection_Manager_Data Efl_Selection_Manager_Data;

struct _Efl_Selection_Manager_Atom
{
    const char *name;
    Ecore_X_Atom x_atom;
};

struct _Efl_Selection_Manager_Data
{
    Eo *obj;
    Ecore_Event_Handler *notify_handler;
    Ecore_Event_Handler *clear_handler;
    Efl_Promise *promise;
    Efl_Selection_Type loss_type;
#ifdef HAVE_ELEMENTARY_X
    Ecore_X_Window *xwin;
#endif

    void *data_func_data;
    Efl_Selection_Data_Ready data_func;
    Eina_Free_Cb data_func_free_cb;

    Efl_Selection_Type type;
    Efl_Selection_Format format;
    Efl_Selection_Manager_Atom atom;
    void *buf;
    int len;

    Eo *sel_owner;
};

#endif
