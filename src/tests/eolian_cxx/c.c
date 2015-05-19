
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eo.h>

#include "a.eo.h"
#include "c.eo.h"

struct _C_Data
{
  int callbacks;
};
typedef struct _C_Data C_Data;

#define MY_CLASS C_CLASS

static Eo *_c_eo_base_constructor(Eo *obj EINA_UNUSED, C_Data *pd EINA_UNUSED)
{
   return eo_do_super_ret(obj, MY_CLASS, obj, eo_constructor());
}

#include "c.eo.c"

