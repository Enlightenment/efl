
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

static void _c_eo_base_constructor(Eo *obj EINA_UNUSED, C_Data *pd EINA_UNUSED)
{
  eo_do_super(obj, MY_CLASS, eo_constructor());
}

#include "c.eo.c"

