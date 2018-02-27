#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>

#include <Eo.h>

#include "Ecore.h"
#include "ecore_private.h"

EFL_CALLBACKS_ARRAY_DEFINE(ecore_idle_enterer_callbacks,
                          { EFL_LOOP_EVENT_IDLE_ENTER, _ecore_factorized_idle_process },
                          { EFL_EVENT_DEL, _ecore_factorized_idle_event_del });


EAPI Ecore_Idle_Enterer *
ecore_idle_enterer_add(Ecore_Task_Cb func,
                       const void   *data)
{
   return _ecore_factorized_idle_add(ecore_idle_enterer_callbacks(), func, data);
}

EAPI Ecore_Idle_Enterer *
ecore_idle_enterer_before_add(Ecore_Task_Cb func,
                              const void   *data)
{
   Ecore_Idle_Enterer *ie = NULL;
   ie = _ecore_factorized_idle_add(ecore_idle_enterer_callbacks(), func, data);

   // This avoid us duplicating code and should only be slightly slower
   // due to a useless cycle of callback registration
   efl_event_callback_array_del(_mainloop_singleton, ecore_idle_enterer_callbacks(), ie);
   efl_event_callback_array_priority_add(_mainloop_singleton, ecore_idle_enterer_callbacks(), EFL_CALLBACK_PRIORITY_BEFORE, ie);

   return ie;
}

EAPI void *
ecore_idle_enterer_del(Ecore_Idle_Enterer *idle_enterer)
{
   return _ecore_factorized_idle_del(idle_enterer);
}
