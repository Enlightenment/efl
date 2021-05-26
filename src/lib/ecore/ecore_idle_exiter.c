#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>

#include <Eo.h>

#include "Ecore.h"
#include "ecore_private.h"

EFL_CALLBACKS_ARRAY_DEFINE(ecore_idle_exiter_callbacks,
                          { EFL_LOOP_EVENT_IDLE_EXIT, _ecore_factorized_idle_process },
                          { EFL_EVENT_DEL, _ecore_factorized_idle_event_del });

EAPI Ecore_Idle_Exiter *
ecore_idle_exiter_add(Ecore_Task_Cb func,
                      const void   *data)
{
   return  _ecore_factorized_idle_add(ecore_idle_exiter_callbacks(), func, data);
}

EAPI void *
ecore_idle_exiter_del(Ecore_Idle_Exiter *idle_exiter)
{
   return _ecore_factorized_idle_del(idle_exiter);
}
