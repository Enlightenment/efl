#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecordova_filesystem_private.h"

#define MY_CLASS ECORDOVA_FILESYSTEM_CLASS
#define MY_CLASS_NAME "Ecordova_FileSystem"

static Eo_Base *
_ecordova_filesystem_eo_base_constructor(Eo *obj, Ecordova_FileSystem_Data *pd)
{
   DBG("(%p)", obj);

   pd->obj = obj;

   return eo_do_super_ret(obj, MY_CLASS, obj, eo_constructor());
}

static void
_ecordova_filesystem_constructor(Eo *obj EINA_UNUSED,
                                 Ecordova_FileSystem_Data *pd EINA_UNUSED,
                                 const char *name EINA_UNUSED,
                                 Ecordova_DirectoryEntry *root EINA_UNUSED)
{
   DBG("(%p)", obj);
}

static void
_ecordova_filesystem_eo_base_destructor(Eo *obj,
                                        Ecordova_FileSystem_Data *pd EINA_UNUSED)
{
   DBG("(%p)", obj);

   eo_do_super(obj, MY_CLASS, eo_destructor());
}

#include "ecordova_filesystem.eo.c"
