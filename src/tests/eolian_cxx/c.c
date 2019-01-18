#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eo.h>

#include "a.eo.h"
#include "b.eo.h"
#include "c.eo.h"

struct _C_Data
{
  int callbacks;
};
typedef struct _C_Data C_Data;

#define MY_CLASS C_CLASS

static Eo *_c_efl_object_constructor(Eo *obj EINA_UNUSED, C_Data *pd EINA_UNUSED)
{
   return efl_constructor(efl_super(obj, MY_CLASS));
}

#include "c.eo.c"
