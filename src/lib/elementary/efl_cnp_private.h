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
   int type;
   Efl_Cnp_Atom atom;
   Ecore_Event_Handler *notify_handler;
   Efl_Promise *promise;
};

#endif
