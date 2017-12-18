#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>

#include "ecore_private.h"

typedef struct _Efl_Loop_Consumer_Data Efl_Loop_Consumer_Data;
struct _Efl_Loop_Consumer_Data
{
};

static Efl_Loop *
_efl_loop_consumer_loop_get(Eo *obj, Efl_Loop_Consumer_Data *pd EINA_UNUSED)
{
   if (eina_main_loop_is())
     return ecore_main_loop_get();
   return efl_provider_find(obj, EFL_LOOP_CLASS);
}

static void
_efl_loop_consumer_efl_object_parent_set(Eo *obj, Efl_Loop_Consumer_Data *pd EINA_UNUSED, Efl_Object *parent)
{
   if (parent != NULL && efl_provider_find(parent, EFL_LOOP_CLASS) == NULL)
     {
        ERR("parent=%p is not a provider of EFL_LOOP_CLASS!", parent);
        return;
     }

   efl_parent_set(efl_super(obj, EFL_LOOP_CONSUMER_CLASS), parent);
}

#include "efl_loop_consumer.eo.c"
