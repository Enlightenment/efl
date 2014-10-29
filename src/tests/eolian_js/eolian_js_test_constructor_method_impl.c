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

static void _constructor_method_class_eo_base_constructor(Eo *obj, Constructor_Method_Class_Data *pd)
{
  pd->one = pd->two = 0;
}

static void _constructor_method_class_constructor1(Eo *obj, Constructor_Method_Class_Data *pd, int one)
{
  fprintf(stderr, "one == %d\n", one);
  fflush(stderr);
  ck_assert(one == 5);
  pd->one = 1;
}

static void _constructor_method_class_constructor2(Eo *obj, Constructor_Method_Class_Data *pd, double two)
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

#include <constructor_method_class.eo.c>
