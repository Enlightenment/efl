
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eo.h>

#include "a.eo.h"
#include "b.eo.h"
#include "c.eo.h"
#include "d.eo.h"

struct _D_Data
{
  int callbacks;
};
typedef struct _D_Data D_Data;

#define MY_CLASS D_CLASS

static void _d_eo_base_constructor(Eo *obj EINA_UNUSED, D_Data *pd EINA_UNUSED)
{
  eo_do_super(obj, MY_CLASS, eo_constructor());
}

#include "d.eo.c"

