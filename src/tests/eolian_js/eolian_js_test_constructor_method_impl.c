#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Eo.h>
#include <Ecore.h>

#include <stdlib.h>

#include "constructor_method_class.eo.h"

#include <check.h>

struct _Constructor_Method_Class_Data
{
  Eina_Bool fail;
};
typedef struct _Constructor_Method_Class_Data Constructor_Method_Class_Data;

#define MY_CLASS CONSTRUCTOR_METHOD_CLASS_CLASS

EOLIAN static Efl_Object *
_constructor_method_class_efl_object_constructor(Eo* obj EINA_UNUSED, Constructor_Method_Class_Data *pd)
{
  pd->fail = EINA_FALSE;
  return efl_constructor(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_constructor_method_class_constructor1(Eo* obj EINA_UNUSED, Constructor_Method_Class_Data *pd, int one)
{
  fprintf(stderr, "one == %d\n", one);
  fflush(stderr);
  if (one != 5)
    pd->fail = EINA_TRUE;
}

EOLIAN static void
_constructor_method_class_constructor2(Eo* obj EINA_UNUSED, Constructor_Method_Class_Data *pd, double two)
{
  fprintf(stderr, "two == %f\n", two);
  fflush(stderr);
  if (two != 10.0)
    pd->fail = EINA_TRUE;
}

EOLIAN static Eo *
_constructor_method_class_efl_object_finalize(Eo *obj, Constructor_Method_Class_Data *pd)
{
  if (pd->fail)
    return NULL;

  return efl_finalize(efl_super(obj, MY_CLASS));
}

EOLIAN static Eina_Bool
_constructor_method_class_fail_get(Eo* obj EINA_UNUSED, Constructor_Method_Class_Data *pd)
{
  return pd->fail;
}

EOLIAN static void
_constructor_method_class_method1(Eo* obj EINA_UNUSED, Constructor_Method_Class_Data *pd, int one)
{
  fprintf(stderr, "method1 one == %d\n", one);
  fflush(stderr);
  if (one != 2)
    pd->fail = EINA_TRUE;
}

EOLIAN static int
_constructor_method_class_method2(Eo* obj EINA_UNUSED, Constructor_Method_Class_Data *pd, int one)
{
  fprintf(stderr, "method2 one == %d\n", one);
  fflush(stderr);
  if (one != 3)
    pd->fail = EINA_TRUE;
  return 5;
}

EOLIAN static void
_constructor_method_class_method3(Eo* obj EINA_UNUSED, Constructor_Method_Class_Data *pd, int one, double two)
{
  fprintf(stderr, "method3 one == %d two == %f\n", one, two);
  fflush(stderr);
  if (one != 3)
    pd->fail = EINA_TRUE;
  if (two != 11.1)
    pd->fail = EINA_TRUE;
}

EOLIAN static int
_constructor_method_class_method4(Eo * obj EINA_UNUSED, Constructor_Method_Class_Data *pd, int one, double two)
{
  fprintf(stderr, "method4 one == %d two == %f\n", one, two);
  fflush(stderr);
  if (one != 3)
    pd->fail = EINA_TRUE;
  if (two != 11.1)
    pd->fail = EINA_TRUE;
  return 5;
}

EOLIAN static void
_constructor_method_class_outmethod1(Eo* obj EINA_UNUSED, Constructor_Method_Class_Data *pd EINA_UNUSED, int *one)
{
  fprintf(stderr, "outmethod1\n");
  fflush(stderr);
  *one = 5;
}

EOLIAN static int
_constructor_method_class_outmethod2(Eo* obj EINA_UNUSED, Constructor_Method_Class_Data *pd EINA_UNUSED, int *one)
{
  fprintf(stderr, "outmethod2\n");
  fflush(stderr);
  *one = 10;
  return 11;
}

EOLIAN static void
_constructor_method_class_outmethod3(Eo* obj EINA_UNUSED, Constructor_Method_Class_Data *pd EINA_UNUSED, int *one, double *two)
{
  fprintf(stderr, "outmethod3\n");
  fflush(stderr);
  *one = 11;
  *two = 5.1;
}

EOLIAN static Constructor_Method_Class *
_constructor_method_class_classoutmethod1(Eo* obj EINA_UNUSED, Constructor_Method_Class_Data *pd EINA_UNUSED, int one, double two)
{
  fprintf(stderr, "classoutmethod1\n");
  fflush(stderr);
  return efl_add_ref(MY_CLASS, NULL, constructor_method_class_constructor1(efl_added, one), constructor_method_class_constructor2(efl_added, two));
}

EOLIAN static void
_constructor_method_class_classoutmethod2(Eo* obj EINA_UNUSED, Constructor_Method_Class_Data *pd EINA_UNUSED, int one, double two, Constructor_Method_Class **out_class)
{
  fprintf(stderr, "classoutmethod2\n");
  fflush(stderr);
  *out_class = efl_add_ref(MY_CLASS, NULL, constructor_method_class_constructor1(efl_added, one), constructor_method_class_constructor2(efl_added, two));
}


#include <constructor_method_class.eo.c>
