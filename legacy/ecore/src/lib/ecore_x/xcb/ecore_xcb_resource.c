/* NB: Reference GetDflt.c, Xresource.h, Xrm.c, Quarks.c */

#include "ecore_xcb_private.h"
#include <ctype.h>
#ifdef NEED_SYS_PARAM_H
# include <sys/param.h>
#endif
#ifdef NEED_NETDB_H
# include <netdb.h>
#endif

/* local structs */
typedef struct _Ecore_Xcb_Resource_Value 
{
   unsigned int size;
   void *addr;
} Ecore_Xcb_Resource_Value;

/* local function prototypes */
static char *_ecore_xcb_resource_get(const char *prog, const char *name);
static int _ecore_xcb_resource_string_to_name(const char *str);
static int _ecore_xcb_resource_string_to_class(const char *str);
static Eina_Bool _ecore_xcb_resource_fetch(int *names, int *klasses, int *type, Ecore_Xcb_Resource_Value value);

char *
_ecore_xcb_resource_get_string(const char *prog, const char *name) 
{
   char *ret = NULL;

   ret = _ecore_xcb_resource_get(prog, name);
   return NULL;
//   return ret;
}

int 
_ecore_xcb_resource_get_int(const char *prog, const char *name) 
{
   char *ret = NULL;

   ret = _ecore_xcb_resource_get(prog, name);
   return 0;
//   return atoi(ret);
}

/* local functions */
static char *
_ecore_xcb_resource_get(const char *prog, const char *name) 
{
   char *prog_name = NULL;
   int names[3], klasses[3];
   int type;
   Ecore_Xcb_Resource_Value value;

   prog_name = strrchr(prog, '/');
   if (prog_name) 
     prog_name++;
   else
     prog_name = (char *)prog;

   names[0] = _ecore_xcb_resource_string_to_name(prog_name);
   names[1] = _ecore_xcb_resource_string_to_name(name);
   names[2] = 0;
   klasses[0] = _ecore_xcb_resource_string_to_class("Program");
   klasses[1] = _ecore_xcb_resource_string_to_class("Name");
   klasses[2] = 0;

   _ecore_xcb_resource_fetch(names, klasses, &type, value);
   return (value.addr);
}

static int 
_ecore_xcb_resource_string_to_name(const char *str) 
{
   return 0;
}

static int 
_ecore_xcb_resource_string_to_class(const char *str) 
{
   return 0;
}

static Eina_Bool 
_ecore_xcb_resource_fetch(int *names, int *klasses, int *type, Ecore_Xcb_Resource_Value value) 
{
   return EINA_FALSE;
}
