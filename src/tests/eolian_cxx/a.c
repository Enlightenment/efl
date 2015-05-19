
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eo.h>

#include "a.eo.h"

struct _A_Data
{
  int callbacks;
};
typedef struct _A_Data A_Data;

#define MY_CLASS A_CLASS

static Eo *_a_eo_base_constructor(Eo *obj EINA_UNUSED, A_Data *pd EINA_UNUSED)
{
   return eo_do_super_ret(obj, MY_CLASS, obj, eo_constructor());
}

#include "a.eo.c"

