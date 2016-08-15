#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eo.h>

#include "a.eo.h"
#include "b.eo.h"

struct _B_Data
{
  int callbacks;
};
typedef struct _B_Data B_Data;

#define MY_CLASS B_CLASS

static Eo *_b_efl_object_constructor(Eo *obj EINA_UNUSED, B_Data *pd EINA_UNUSED)
{
   return efl_constructor(efl_super(obj, MY_CLASS));
}

#include "b.eo.c"
