#include <Eina.h>
#include "eolian_database.h"

EAPI Eina_Stringshare *
eolian_constructor_full_name_get(const Eolian_Constructor *ctor)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(ctor, NULL);
   return ctor->full_name;
}

static Eina_Bool
_fill_class(Eolian_Constructor *ctor)
{
   const Eolian_Class *class = NULL;
   if (ctor->klass)
     return EINA_TRUE;
   if (!database_class_name_validate(ctor->full_name, &class) || !class)
     return EINA_FALSE;
   ctor->klass = class;
   return EINA_TRUE;
}

EAPI const Eolian_Class *
eolian_constructor_class_get(const Eolian_Constructor *ctor)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(ctor, NULL);
   if (!_fill_class((Eolian_Constructor*)ctor))
     return NULL;
   return ctor->klass;
}

EAPI const Eolian_Function *
eolian_constructor_function_get(const Eolian_Constructor *ctor)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(ctor, NULL);
   const Eolian_Class *klass = eolian_constructor_class_get(ctor);
   if (!klass)
     return NULL;
   return eolian_class_function_get_by_name(klass,
       ctor->full_name + strlen(klass->full_name) + 1, EOLIAN_UNRESOLVED);
}
