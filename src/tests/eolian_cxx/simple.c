#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>

#include <Eo.h>
#include <Ecore.h>

#include "simple.eo.h"

#define MY_CLASS SIMPLE_CLASS

static Eo *_simple_efl_object_constructor(Eo *obj, void *pd EINA_UNUSED)
{
   return efl_constructor(efl_super(obj, MY_CLASS));
}

static Eina_Bool _simple_simple_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   printf("calling %s\n", __func__);

   return EINA_TRUE;
}

static Eina_Bool _simple_name_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, const char** name)
{
   static const char* _name= "simple_class";

   printf("calling %s= simples class\n", __func__);
   *name = _name;

   return EINA_TRUE;
}

#include "simple.eo.c"
