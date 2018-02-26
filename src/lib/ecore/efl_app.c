#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sys/time.h>
#ifndef _WIN32
# include <sys/resource.h>
#endif
#include <errno.h>

#include "Ecore.h"
#include "ecore_private.h"

#include "ecore_main_common.h"

Eo *_efl_app;
Efl_App_Data *_efl_app_data;

EOLIAN static Efl_Object *
_efl_app_efl_object_constructor(Eo *obj, Efl_App_Data *pd)
{
   if (_efl_app)
     {
        CRI("Efl.App object already exists!");
        return NULL;
     }
   obj = efl_constructor(efl_super(obj, EFL_APP_CLASS));
   if (!obj) return NULL;
   _efl_app = obj;
   _efl_app_data = pd;
   pd->main_loop = efl_add(EFL_LOOP_CLASS, NULL);
   pd->main_loop_data = efl_data_scope_get(pd->main_loop, EFL_LOOP_CLASS);

   return obj;
}

EOLIAN static void
_efl_app_efl_object_destructor(Eo *obj, Efl_App_Data *pd)
{
   Eo *loop;
   Eina_List *l, *ll;

   EINA_LIST_FOREACH_SAFE(pd->loops, l, ll, loop)
     efl_del(loop);
   efl_del(pd->main_loop);
   pd->main_loop = NULL;
   pd->main_loop_data = NULL;
   efl_destructor(efl_super(obj, EFL_APP_CLASS));
   if (obj != _efl_app) return;
   _efl_app = NULL;
   _efl_app_data = NULL;
}

Efl_Version _build_efl_version = { 0, 0, 0, 0, NULL, NULL };

EWAPI void
efl_build_version_set(int vmaj, int vmin, int vmic, int revision,
                      const char *flavor, const char *build_id)
{
   // note: EFL has not been initialized yet at this point (ie. no eina call)
   _build_efl_version.major = vmaj;
   _build_efl_version.minor = vmin;
   _build_efl_version.micro = vmic;
   _build_efl_version.revision = revision;
   free((char *)_build_efl_version.flavor);
   free((char *)_build_efl_version.build_id);
   _build_efl_version.flavor = eina_strdup(flavor);
   _build_efl_version.build_id = eina_strdup(build_id);
}

EOLIAN static const Efl_Version *
_efl_app_build_efl_version_get(Eo *obj EINA_UNUSED, Efl_App_Data *pd EINA_UNUSED)
{
   return &_build_efl_version;
}

EOLIAN static const Efl_Version *
_efl_app_efl_version_get(Eo *obj EINA_UNUSED, Efl_App_Data *pd EINA_UNUSED)
{
   /* vanilla EFL: flavor = NULL */
   static const Efl_Version version = {
      .major = VMAJ,
      .minor = VMIN,
      .micro = VMIC,
      .revision = VREV,
      .build_id = EFL_BUILD_ID,
      .flavor = NULL
   };
   return &version;
}

EOLIAN static Eo *
_efl_app_main_loop_get(Eo *obj EINA_UNUSED, Efl_App_Data *pd)
{
   return pd->main_loop;
}

EAPI Eo *
efl_app_get(void)
{
   if (_efl_app) return _efl_app;
   return efl_add(EFL_APP_CLASS, NULL);
}

#include "efl_app.eo.c"
