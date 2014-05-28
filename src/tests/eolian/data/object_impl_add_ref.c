#define EFL_BETA_API_SUPPORT
#include <Eo.h>
#include "object.eo.h"

typedef struct
{

} Object_Data;

EOLIAN static Eina_Bool
_object_a_set(Eo *obj, Object_Data *pd, const char *part, const Eina_List *value)
{

}

EOLIAN static Eina_List *
_object_a_get(Eo *obj, Object_Data *pd, const char *part)
{

}

EOLIAN static void
_object_b_set(Eo *obj, Object_Data *pd, Eina_List *value)
{

}

EOLIAN static char *
_object_foo1(Eo *obj, Object_Data *pd, int a, char *b, double *c)
{

}

EOLIAN static void
_object_foo2(const Eo *obj, Object_Data *pd, int a, const char *b)
{

}

EOLIAN static void
_object_constructor_1(Eo *obj, Object_Data *pd, int a, char b)
{

}

EOLIAN static void
_object_constructor_2(Eo *obj, Object_Data *pd)
{

}

EOLIAN static void
_object_base_constructor(Eo *obj, Object_Data *pd)
{
   eo_do_super(obj, OBJECT_CLASS, base_constructor();

}

EOLIAN static void
_object_base_destructor(Eo *obj, Object_Data *pd)
{

}

EOLIAN static void
_object_class_constructor(Eo_Class *klass)
{

}

EOLIAN static void
_object_class_destructor(Eo_Class *klass)
{

}

EOLIAN static void
_object_c_set(Eo *obj, Object_Data *pd, Eina_List *value)
{

}

EOLIAN static Eina_List *
_object_c_get(Eo *obj, Object_Data *pd)
{

}

