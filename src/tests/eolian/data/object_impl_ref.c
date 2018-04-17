#define EFL_BETA_API_SUPPORT
#include <Eo.h>
#include "object_impl.eo.h"

typedef struct
{

} Object_Impl_Data;

EOLIAN static Eina_Bool
_object_impl_a_set(Eo *obj, Object_Impl_Data *pd, const char *part, const Eina_List *value)
{

}

EOLIAN static Eina_List *
_object_impl_a_get(const Eo *obj, Object_Impl_Data *pd, const char *part)
{

}

EOLIAN static void
_object_impl_b_set(Eo *obj, Object_Impl_Data *pd, Eina_List *value)
{

}

EOLIAN static void
_object_impl_constructor_1(Eo *obj, Object_Impl_Data *pd, int a, char b)
{

}

EOLIAN static void
_object_impl_constructor_2(Eo *obj, Object_Impl_Data *pd)
{

}

EOLIAN static char *
_object_impl_foo1(Eo *obj, Object_Impl_Data *pd, int a, char *b, double *c)
{

}

EOLIAN static void
_object_impl_foo2(const Eo *obj, Object_Impl_Data *pd, int a, const char *b)
{

}

EOLIAN static void
_object_impl_base_constructor(Eo *obj, Object_Impl_Data *pd)
{

}

EOLIAN static void
_object_impl_base_destructor(Eo *obj, Object_Impl_Data *pd)
{
   base_destructor(efl_super(obj, OBJECT_IMPL_CLASS));

}

EOLIAN static void
_object_impl_class_constructor(Efl_Class *klass)
{

}

EOLIAN static void
_object_impl_class_destructor(Efl_Class *klass)
{

}

#include "object_impl.eo.c"
