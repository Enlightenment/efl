
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eo.h>
#include <Ecore.h>

#include <stdlib.h>

#include "eina_simple.eo.h"

#define MY_CLASS EINA_SIMPLE_CLASS

static Eo *_eina_simple_efl_object_constructor(Eo *obj, void *pd EINA_UNUSED)
{
   return efl_constructor(efl_super(obj, MY_CLASS));
}

#include "eina_simple.eo.c"

