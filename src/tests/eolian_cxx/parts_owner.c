#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <string.h>

#define EFL_PART_PROTECTED
#include <Efl.h>

#define EOLIANCXXTEST_API
#define EOLIANCXXTEST_API_WEAK

#include "parts_inner.eo.h"
#include "parts_owner.eo.h"

struct _Parts_Owner_Data
{
  int unused;
};
typedef struct _Parts_Owner_Data Parts_Owner_Data;

#define MY_CLASS PARTS_OWNER_CLASS

static Eo *_parts_owner_efl_object_constructor(Eo *obj, Parts_Owner_Data *pd EINA_UNUSED)
{
   return efl_constructor(efl_super(obj, MY_CLASS));
}

static Eo *_parts_owner_efl_part_part_get(const Eo *obj, Parts_Owner_Data *pd EINA_UNUSED, const char *name)
{
   if (!name || strcmp(name, "inner")) return NULL;
   /* Not parented to the owner: the C++ accessor adopts what it gets, so the
    * wrapper must own the only reference, and frees it when it goes. */
   return efl_add_ref(PARTS_INNER_CLASS, NULL);
}

#include "parts_owner.eo.c"
