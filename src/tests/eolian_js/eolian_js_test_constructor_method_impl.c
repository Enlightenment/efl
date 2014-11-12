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
  Eina_Bool one, two;
};
typedef struct _Constructor_Method_Class_Data Constructor_Method_Class_Data;

#define MY_CLASS CONSTRUCTOR_METHOD_CLASS_CLASS

static void _constructor_method_class_eo_base_constructor(Eo* obj EINA_UNUSED, Constructor_Method_Class_Data *pd)
{
  pd->one = pd->two = 0;
}

static void _constructor_method_class_constructor1(Eo* obj EINA_UNUSED, Constructor_Method_Class_Data *pd, int one)
{
  fprintf(stderr, "one == %d\n", one);
  fflush(stderr);
  ck_assert(one == 5);
  pd->one = 1;
}

static void _constructor_method_class_constructor2(Eo* obj EINA_UNUSED, Constructor_Method_Class_Data *pd, double two)
{
  fprintf(stderr, "two == %f\n", two);
  fflush(stderr);
  ck_assert(two == 10.0);
  pd->two = 1;
}

static Eo * _constructor_method_class_eo_base_finalize(Eo *obj, Constructor_Method_Class_Data *pd)
{
  if(!pd->one || !pd->two)
    return NULL;
  
  return obj;
}

void _constructor_method_class_method1(Eo* obj EINA_UNUSED, Constructor_Method_Class_Data *pd EINA_UNUSED, int one)
{
  fprintf(stderr, "method1 one == %d\n", one);
  fflush(stderr);
  ck_assert(one == 2);
}

int _constructor_method_class_method2(Eo* obj EINA_UNUSED, Constructor_Method_Class_Data *pd EINA_UNUSED, int one)
{
  fprintf(stderr, "method2 one == %d\n", one);
  fflush(stderr);
  ck_assert(one == 3);
  return 5;
}

void _constructor_method_class_method3(Eo* obj EINA_UNUSED, Constructor_Method_Class_Data *pd EINA_UNUSED, int one, double two)
{
  fprintf(stderr, "method3 one == %d two == %f\n", one, two);
  fflush(stderr);
  ck_assert(one == 3);
  ck_assert(two == 11.1);
}

void _constructor_method_class_outmethod1(Eo* obj EINA_UNUSED, Constructor_Method_Class_Data *pd EINA_UNUSED, int *one)
{
  *one = 5;
}

int _constructor_method_class_outmethod2(Eo* obj EINA_UNUSED, Constructor_Method_Class_Data *pd EINA_UNUSED, int *one)
{
  return *one = 10;
}

void _constructor_method_class_outmethod3(Eo* obj EINA_UNUSED, Constructor_Method_Class_Data *pd EINA_UNUSED, int *one, double *two)
{
  *one = 11;
  *two = 12.2;
}


#include <constructor_method_class.eo.c>
