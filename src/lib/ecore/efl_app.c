#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>

#include "ecore_private.h"

#ifndef _WIN32
# include <sys/resource.h>
#endif

#define MY_CLASS EFL_APP_CLASS

Efl_Version _app_efl_version = { 0, 0, 0, 0, NULL, NULL };

EOLIAN static Efl_App*
_efl_app_app_main_get(void)
{
   if (_mainloop_singleton) return _mainloop_singleton;
   _mainloop_singleton = efl_add_ref(EFL_APP_CLASS, NULL);
   _mainloop_singleton_data = efl_data_scope_get(_mainloop_singleton, EFL_LOOP_CLASS);
   return _mainloop_singleton;
}

EOLIAN static const Efl_Version *
_efl_app_build_efl_version_get(const Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   return &_app_efl_version;
}

EOLIAN static const Efl_Version *
_efl_app_efl_version_get(const Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
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

#ifdef _WIN32
#else
static const signed char primap[EFL_TASK_PRIORITY_ULTRA + 1] =
{
      10, // EFL_TASK_PRIORITY_NORMAL
      19, // EFL_TASK_PRIORITY_BACKGROUND
      15, // EFL_TASK_PRIORITY_LOW
      5, // EFL_TASK_PRIORITY_HIGH
      0  // EFL_TASK_PRIORITY_ULTRA
};
#endif

EOLIAN static void
_efl_app_efl_task_priority_set(Eo *obj, void *pd EINA_UNUSED, Efl_Task_Priority priority)
{
   efl_task_priority_set(efl_super(obj, MY_CLASS), priority);
#ifdef _WIN32
#else
   // -20 (high) -> 19 (low)
   int p = 0;

   if ((priority >= EFL_TASK_PRIORITY_NORMAL) &&
       (priority <= EFL_TASK_PRIORITY_ULTRA))
     p = primap[priority];
   setpriority(PRIO_PROCESS, 0, p);
#endif
}

EOLIAN static Efl_Task_Priority
_efl_app_efl_task_priority_get(const Eo *obj, void *pd EINA_UNUSED)
{
   Efl_Task_Priority pri = EFL_TASK_PRIORITY_NORMAL;
#ifdef _WIN32
#else
   int p, i, dist = 0x7fffffff, d;

   errno = 0;
   p = getpriority(PRIO_PROCESS, 0);
   if (errno != 0)
     return efl_task_priority_get(efl_super(obj, MY_CLASS));

   // find the closest matching priority in primap
   for (i = EFL_TASK_PRIORITY_NORMAL; i <= EFL_TASK_PRIORITY_ULTRA; i++)
     {
        d = primap[i] - p;
        if (d < 0) d = -d;
        if (d < dist)
          {
             pri = i;
             dist = d;
          }
     }

   Efl_Task_Data *td = efl_data_scope_get(obj, EFL_TASK_CLASS);
   if (td) td->priority = pri;
#endif
   return pri;
}

//////////////////////////////////////////////////////////////////////////

#include "efl_app.eo.c"
