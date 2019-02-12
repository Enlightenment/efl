#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>

#include "ecore_private.h"

#define MY_CLASS EFL_TASK_CLASS

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

EOLIAN static void
_efl_task_priority_set(Eo *obj EINA_UNUSED, Efl_Task_Data *pd, Efl_Task_Priority priority)
{
   pd->priority = priority;
}

EOLIAN static Efl_Task_Priority
_efl_task_priority_get(const Eo *obj EINA_UNUSED, Efl_Task_Data *pd)
{
   return pd->priority;
}

EOLIAN static int
_efl_task_exit_code_get(const Eo *obj EINA_UNUSED, Efl_Task_Data *pd)
{
   return pd->exit_code;
}

EOLIAN static void
_efl_task_flags_set(Eo *obj EINA_UNUSED, Efl_Task_Data *pd, Efl_Task_Flags flags)
{
   pd->flags = flags;
}

EOLIAN static Efl_Task_Flags
_efl_task_flags_get(const Eo *obj EINA_UNUSED, Efl_Task_Data *pd)
{
   return pd->flags;
}

EOLIAN static void
_efl_task_efl_object_destructor(Eo *obj EINA_UNUSED, Efl_Task_Data *pd)
{
   eina_stringshare_del(pd->command);
   pd->command = NULL;
   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_task_efl_object_parent_set(Eo *obj, Efl_Task_Data *pd EINA_UNUSED, Efl_Object *parent)
{
   efl_parent_set(efl_super(obj, MY_CLASS), parent);
}

//////////////////////////////////////////////////////////////////////////

#include "efl_task.eo.c"
