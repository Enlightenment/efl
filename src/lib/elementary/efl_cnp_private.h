#ifndef EFL_CNP_PRIVATE_H
#define EFL_CNP_PRIVATE_H

#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_CNP_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Efl_Cnp_Data Efl_Cnp_Data;
typedef struct _Efl_Cnp_Atom Efl_Cnp_Atom;

struct _Efl_Cnp_Atom
{
   const char    *name;
   Ecore_X_Atom  x_atom;
};

struct _Efl_Cnp_Data
{
   Eo *obj;
   Ecore_Event_Handler *notify_handler;
   Efl_Promise *promise;

   void *data_func_data;
   Efl_Selection_Data_Ready data_func;
   Eina_Free_Cb data_func_free_cb;

   Efl_Selection_Type type;
   Efl_Selection_Format format;
   Efl_Cnp_Atom atom;
   Eina_Bool has_sel;
   void *buf;
   int len;
};

#endif
