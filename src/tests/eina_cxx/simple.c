
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eo.h>
#include <Ecore.h>

#include <stdlib.h>

#include "simple.eo.h"

#define MY_CLASS SIMPLE_CLASS

static Eo *_simple_eo_base_constructor(Eo *obj, void *pd EINA_UNUSED)
{
   return eo_constructor(eo_super(obj, MY_CLASS));
}

#include "simple.eo.c"

