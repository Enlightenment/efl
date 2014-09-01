#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eo.h>
#include <Ecore.h>

#include "test_object.eo.h"

#include <check.h>

#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>

struct _Test_Object_Data
{
   int a;
};

typedef struct _Test_Object_Data Test_Object_Data;

#define MY_CLASS TEST_OBJECT_CLASS

EOLIAN static Eo_Base *
_test_object_eo_base_constructor(Eo* obj, Test_Object_Data *pd)
{
  fprintf(stdout, "_test_object_eo_base_constructor\n");

  pd->a = 0;
  return eo_do_super_ret(obj, MY_CLASS, obj, eo_constructor());
}

EOLIAN static Eo *
_test_object_eo_base_finalize(Eo *obj, Test_Object_Data *pd EINA_UNUSED)
{
  fprintf(stdout, "_test_object_eo_base_finalize\n");

  return eo_do_super_ret(obj, MY_CLASS, obj, eo_finalize());
}

EOLIAN static void
_test_object_eo_base_destructor(Eo* obj, Test_Object_Data *pd EINA_UNUSED)
{
  fprintf(stdout, "_test_object_eo_base_destructor\n");
  eo_do_super(obj, MY_CLASS, eo_destructor());
}


EOLIAN static void
_test_object_method_integral_in_a(Eo* obj EINA_UNUSED,
                                    Test_Object_Data *pd,
                                    int a)
{
  fprintf(stdout, "_test_object_method_integral_in_a(%d)\n", a);
  pd->a = a;
}

EOLIAN static void
_test_object_method_integral_out_a(Eo* obj EINA_UNUSED,
                                     Test_Object_Data *pd,
                                     int *a)
{
  fprintf(stdout, "_test_object_method_integral_out_a(%p)\n", a);
  *a = pd->a;
}

EOLIAN static void
_test_object_method_integral_inout(Eo* obj EINA_UNUSED,
                                     Test_Object_Data *pd EINA_UNUSED,
                                     int *a)
{
  fprintf(stdout, "_test_object_method_integral_inout(%d [%p])\n", *a, a);
  *a = -(*a);
}

EOLIAN static int
_test_object_method_integral_return_a(Eo* obj EINA_UNUSED,
                                        Test_Object_Data *pd)
{
  fprintf(stdout, "_test_object_method_integral_return_a()\n");
  return pd->a;
}

EOLIAN static Eina_Bool
_test_object_method_div_mod(Eo* obj EINA_UNUSED,
                              Test_Object_Data *pd EINA_UNUSED,
                              int a,
                              int b,
                              int *quotient,
                              int *remainder)
{
  fprintf(stdout, "_test_object_method_div_mod(%d, %d, %p, %p)\n", a, b, quotient, remainder);
  if (0 == b) return EINA_FALSE;
  *quotient = a / b;
  *remainder = a % b;
  return EINA_TRUE;
}

EOLIAN static Eina_Bool
_test_object_method_and(Eo* obj EINA_UNUSED,
                          Test_Object_Data *pd EINA_UNUSED,
                          Eina_Bool a,
                          Eina_Bool b)
{
  fprintf(stdout, "_test_object_method_and(%d, %d)\n", a, b);
  return a && b;
}

EOLIAN static double
_test_object_method_modf(Eo* obj EINA_UNUSED,
                           Test_Object_Data *pd EINA_UNUSED,
                           double x,
                           double *int_part)
{
  fprintf(stdout, "_test_object_method_modf(%f, %p)\n", x, int_part);
  return modf(x, int_part);
}

EOLIAN static void
_test_object_method_uppercase(Eo* obj EINA_UNUSED,
                                Test_Object_Data *pd EINA_UNUSED,
                                char **str)
{
  fprintf(stdout, "_test_object_method_uppercase('%s')\n", *str);
  if (!*str) return;

  char *p = *str;
  while ('\0' != *p)
    {
       *p = toupper(*p);
       p++;
    }
}

EOLIAN static Eina_Bool
_test_object_method_in_null(Eo* obj EINA_UNUSED,
                              Test_Object_Data *pd EINA_UNUSED,
                              char *a)
{
  fprintf(stdout, "_test_object_method_in_null(%p)\n", a);
  return NULL == a;
}

EOLIAN static Eina_Bool
_test_object_method_out_null(Eo* obj EINA_UNUSED,
                               Test_Object_Data *pd EINA_UNUSED,
                               char **a)
{
  fprintf(stdout, "_test_object_method_out_null(%p)\n", a);
  *a = NULL;
  return EINA_TRUE;
}

EOLIAN static Eina_Bool
_test_object_method_inout_null(Eo* obj EINA_UNUSED,
                                 Test_Object_Data *pd EINA_UNUSED,
                                 char **a)
{
  fprintf(stdout, "_test_object_method_inout_null(%p)\n", a);
  return NULL == *a;
}

EOLIAN static char *
_test_object_method_return_null(Eo* obj EINA_UNUSED,
                                  Test_Object_Data *pd EINA_UNUSED)
{
  fprintf(stdout, "_test_object_method_return_null()\n");
  return NULL;
}

EOLIAN static void
_test_object_call_event(Eo* obj, Test_Object_Data *pd EINA_UNUSED)
{
   fprintf(stderr, "_test_object_event_call()\n"); fflush(stderr);
   static Test_Struct_Ex s = {42, TEST_ENUM_EX_THIRD};
   eo_do(obj, eo_event_callback_call(TEST_OBJECT_EVENT_TEST, NULL));
   eo_do(obj, eo_event_callback_call(TEST_OBJECT_EVENT_TEST_STRUCTARG, &s));
   eo_do(obj, eo_event_callback_call(TEST_OBJECT_EVENT_TEST_STRINGARG, "foo"));
}

EOLIAN static char *
_test_object_method_null(Eo* obj EINA_UNUSED,
                           Test_Object_Data *pd EINA_UNUSED,
                           char *in,
                           char **out,
                           char **inout)
{
  fprintf(stdout, "_test_object_method_null(%p, %p, %p)\n", in, out, inout);
  assert(!in);
  *out = NULL;
  *inout = NULL;
  return NULL;
}

// Arrays //

EOLIAN static int
_test_object_method_array_at(Eo* obj EINA_UNUSED,
                               Test_Object_Data *pd EINA_UNUSED,
                               Eina_Array *array,
                               int index)
{
  fprintf(stdout, "_test_object_method_array_at(%p, %d)\n", array, index);
  return *((int*)eina_array_data_get(array, index));
}

EOLIAN static Eina_Array *
_test_object_method_array_with_42(Eo* obj EINA_UNUSED,
                                    Test_Object_Data *pd EINA_UNUSED)
{
  fprintf(stdout, "_test_object_method_array_with_42()\n");
  Eina_Array *arr = eina_array_new(2);
  int* n = malloc(sizeof(int));
  *n = 42;
  eina_array_push(arr, n);
  return arr;
}

EOLIAN static void
_test_object_method_array_in_array_out(Eo* obj EINA_UNUSED,
                                         Test_Object_Data *pd EINA_UNUSED,
                                         Eina_Array *a_in,
                                         Eina_Array **a_out)
{
  fprintf(stdout, "_test_object_method_array_in_array_out(%p, %p)\n", a_in, a_out);
  *a_out = a_in;
}

EOLIAN static Eina_Array *
_test_object_method_array_of_objects(Eo* obj,
                                       Test_Object_Data *pd EINA_UNUSED,
                                       Eina_Array *a_in)
{
  fprintf(stdout, "_test_object_method_array_of_objects(%p)\n", a_in);
  if (a_in) return a_in;
  Eina_Array *arr = eina_array_new(2);
  eina_array_push(arr, obj);
  return arr;
}


EOLIAN static Eina_Array *
_test_object_method_array_of_strings(Eo* obj EINA_UNUSED,
                                       Test_Object_Data *pd EINA_UNUSED,
                                       Eina_Array *a_in)
{
  fprintf(stdout, "_test_object_method_array_of_strings(%p)\n", a_in);
  if (a_in) return a_in;
  Eina_Array *arr = eina_array_new(2);
  const char* v = "foo";
  fprintf(stdout, "v = %p\n", v);
  eina_array_push(arr, v);
  return arr;
}

EOLIAN static Eina_Array *
_test_object_method_array_of_ints(Eo* obj EINA_UNUSED,
                                    Test_Object_Data *pd EINA_UNUSED,
                                    Eina_Array *a_in)
{
  fprintf(stdout, "_test_object_method_array_of_ints(%p)\n", a_in);
  if (a_in) return a_in;
  int *v = malloc(sizeof(int));
  *v = 42;
  Eina_Array *arr = eina_array_new(2);
  eina_array_push(arr, v);
  return arr;
}

EOLIAN static Eina_Array *
_test_object_method_array_of_bools(Eo* obj EINA_UNUSED,
                                     Test_Object_Data *pd EINA_UNUSED,
                                     Eina_Array *a_in)
{
  fprintf(stdout, "_test_object_method_array_of_bools(%p)\n", a_in);
  if (a_in) return a_in;
  Eina_Bool *v = malloc(sizeof(Eina_Bool));
  *v = EINA_TRUE;
  Eina_Array *arr = eina_array_new(2);
  eina_array_push(arr, v);
  return arr;
}

EOLIAN static Eina_Array *
_test_object_method_array_of_doubles(Eo* obj EINA_UNUSED,
                                       Test_Object_Data *pd EINA_UNUSED,
                                       Eina_Array *a_in)
{
  fprintf(stdout, "_test_object_method_array_of_doubles(%p)\n", a_in);
  if (a_in) return a_in;
  double *v = malloc(sizeof(double));
  *v = 42;
  Eina_Array *arr = eina_array_new(2);
  eina_array_push(arr, v);
  return arr;
}

EOLIAN static Eina_Array *
_test_object_method_array_of_enums(Eo* obj EINA_UNUSED,
                                     Test_Object_Data *pd EINA_UNUSED,
                                     Eina_Array *a_in)
{
  fprintf(stdout, "_test_object_method_array_of_enums(%p)\n", a_in);
  if (a_in) return a_in;
  Test_Enum_Ex *v = malloc(sizeof(Test_Enum_Ex));
  *v = TEST_ENUM_EX_THIRD;
  Eina_Array *arr = eina_array_new(2);
  eina_array_push(arr, v);
  return arr;
}

EOLIAN static Eina_Array *
_test_object_method_array_of_structs(Eo* obj EINA_UNUSED,
                                       Test_Object_Data *pd EINA_UNUSED,
                                       Eina_Array *a_in)
{
  fprintf(stdout, "_test_object_method_array_of_structs(%p)\n", a_in);
  if (a_in) return a_in;
  Test_Struct_Ex *v = malloc(sizeof(Test_Struct_Ex));
  v->value_int = 42;
  v->value_enum = TEST_ENUM_EX_THIRD;
  Eina_Array *arr = eina_array_new(2);
  eina_array_push(arr, v);
  return arr;
}

EOLIAN static Eina_Array *
_test_object_method_array_of_arrays_of_ints(Eo* obj EINA_UNUSED,
                                              Test_Object_Data *pd EINA_UNUSED,
                                              Eina_Array *a_in)
{
  fprintf(stdout, "_test_object_method_array_of_arrays_of_ints(%p)\n", a_in);
  if (a_in) return a_in;
  int *v = malloc(sizeof(int));
  *v = 42;
  Eina_Array *a1 = eina_array_new(2);
  eina_array_push(a1, v);
  Eina_Array *a2 = eina_array_new(2);
  eina_array_push(a2, a1);
  return a2;
}

// Lists //

EOLIAN static Eina_List *
_test_object_method_list_with_42(Eo* obj EINA_UNUSED,
                                   Test_Object_Data *pd EINA_UNUSED)
{
  fprintf(stdout, "_test_object_method_list_with_42()\n");
  int* n = malloc(sizeof(int));
  *n = 42;
  return eina_list_append(NULL, n);
}

EOLIAN static void
_test_object_method_list_in_list_out(Eo* obj EINA_UNUSED,
                                       Test_Object_Data *pd EINA_UNUSED,
                                       Eina_List *l_in,
                                       Eina_List **l_out)
{
  fprintf(stdout, "_test_object_method_list_in_list_out(%p , %p)\n", l_in, l_out);
  *l_out = l_in;
}

EOLIAN static Eina_List *
_test_object_method_list_of_objects(Eo* obj,
                                      Test_Object_Data *pd EINA_UNUSED,
                                      Eina_List *l_in)
{
  fprintf(stdout, "_test_object_method_list_of_objects(%p)\n", l_in);
  if (l_in) return l_in;
  return eina_list_append(NULL, obj);
}


EOLIAN static Eina_List *
_test_object_method_list_of_strings(Eo* obj EINA_UNUSED,
                                      Test_Object_Data *pd EINA_UNUSED,
                                      Eina_List *l_in)
{
  fprintf(stdout, "_test_object_method_list_of_strings(%p)\n", l_in);
  if (l_in) return l_in;
  return eina_list_append(NULL, "foo");
}

EOLIAN static Eina_List *
_test_object_method_list_of_ints(Eo* obj EINA_UNUSED,
                                   Test_Object_Data *pd EINA_UNUSED,
                                   Eina_List *l_in)
{
  fprintf(stdout, "_test_object_method_list_of_ints(%p)\n", l_in);
  if (l_in) return l_in;
  int *v = malloc(sizeof(int));
  *v = 42;
  return eina_list_append(NULL, v);
}

EOLIAN static Eina_List *
_test_object_method_list_of_bools(Eo* obj EINA_UNUSED,
                                    Test_Object_Data *pd EINA_UNUSED,
                                    Eina_List *l_in)
{
  fprintf(stdout, "_test_object_method_list_of_bools(%p)\n", l_in);
  if (l_in) return l_in;
  Eina_Bool *v = malloc(sizeof(Eina_Bool));
  *v = EINA_TRUE;
  return eina_list_append(NULL, v);
}

EOLIAN static Eina_List *
_test_object_method_list_of_doubles(Eo* obj EINA_UNUSED,
                                      Test_Object_Data *pd EINA_UNUSED,
                                      Eina_List *l_in)
{
  fprintf(stdout, "_test_object_method_list_of_doubles(%p)\n", l_in);
  if (l_in) return l_in;
  double *v = malloc(sizeof(double));
  *v = 42;
  return eina_list_append(NULL, v);
}

EOLIAN static Eina_List *
_test_object_method_list_of_enums(Eo* obj EINA_UNUSED,
                                    Test_Object_Data *pd EINA_UNUSED,
                                    Eina_List *l_in)
{
  fprintf(stdout, "_test_object_method_list_of_enums(%p)\n", l_in);
  if (l_in) return l_in;
  Test_Enum_Ex *v = malloc(sizeof(Test_Enum_Ex));
  *v = TEST_ENUM_EX_THIRD;
  return eina_list_append(NULL, v);
}

EOLIAN static Eina_List *
_test_object_method_list_of_structs(Eo* obj EINA_UNUSED,
                                      Test_Object_Data *pd EINA_UNUSED,
                                      Eina_List *l_in)
{
  fprintf(stdout, "_test_object_method_list_of_structs(%p)\n", l_in);
  if (l_in) return l_in;
  Test_Struct_Ex *v = malloc(sizeof(Test_Struct_Ex));
  v->value_int = 42;
  v->value_enum = TEST_ENUM_EX_THIRD;
  return eina_list_append(NULL, v);
}

// Accessors //

EOLIAN static Eina_Accessor *
_test_object_method_accessor_of_objects(Eo* obj,
                                          Test_Object_Data *pd EINA_UNUSED,
                                          Eina_Accessor *a_in)
{
  fprintf(stdout, "_test_object_method_accessor_of_objects(%p)\n", a_in);
  if (a_in) return a_in;
  Eina_Array *arr = eina_array_new(2);
  eina_array_push(arr, obj);
  return eina_array_accessor_new(arr);
}

EOLIAN static Eina_Accessor *
_test_object_method_accessor_of_strings(Eo* obj EINA_UNUSED,
                                          Test_Object_Data *pd EINA_UNUSED,
                                          Eina_Accessor *a_in)
{
  fprintf(stdout, "_test_object_method_accessor_of_strings(%p)\n", a_in);
  if (a_in) return a_in;
  Eina_Array *arr = eina_array_new(2);
  eina_array_push(arr, "foo");
  return eina_array_accessor_new(arr);
}

EOLIAN static Eina_Accessor *
_test_object_method_accessor_of_ints(Eo* obj EINA_UNUSED,
                                       Test_Object_Data *pd EINA_UNUSED,
                                       Eina_Accessor *a_in)
{
  fprintf(stdout, "_test_object_method_accessor_of_ints(%p)\n", a_in);
  if (a_in) return a_in;
  int *v = malloc(sizeof(int));
  *v = 42;
  Eina_Array *arr = eina_array_new(2);
  eina_array_push(arr, v);
  return eina_array_accessor_new(arr);
}

EOLIAN static Eina_Accessor *
_test_object_method_accessor_of_bools(Eo* obj EINA_UNUSED,
                                        Test_Object_Data *pd EINA_UNUSED,
                                        Eina_Accessor *a_in)
{
  fprintf(stdout, "_test_object_method_accessor_of_bools(%p)\n", a_in);
  if (a_in) return a_in;
  Eina_Bool *v = malloc(sizeof(Eina_Bool));
  *v = EINA_TRUE;
  Eina_Array *arr = eina_array_new(2);
  eina_array_push(arr, v);
  return eina_array_accessor_new(arr);
}

EOLIAN static Eina_Accessor *
_test_object_method_accessor_of_doubles(Eo* obj EINA_UNUSED,
                                          Test_Object_Data *pd EINA_UNUSED,
                                          Eina_Accessor *a_in)
{
  fprintf(stdout, "_test_object_method_accessor_of_doubles(%p)\n", a_in);
  if (a_in) return a_in;
  double *v = malloc(sizeof(double));
  *v = 42.0;
  Eina_Array *arr = eina_array_new(2);
  eina_array_push(arr, v);
  return eina_array_accessor_new(arr);
}

EOLIAN static Eina_Accessor *
_test_object_method_accessor_of_enums(Eo* obj EINA_UNUSED,
                                        Test_Object_Data *pd EINA_UNUSED,
                                        Eina_Accessor *a_in)
{
  fprintf(stdout, "_test_object_method_accessor_of_enums(%p)\n", a_in);
  if (a_in) return a_in;
  Test_Enum_Ex *v = malloc(sizeof(Test_Enum_Ex));
  *v = TEST_ENUM_EX_THIRD;
  Eina_Array *arr = eina_array_new(2);
  eina_array_push(arr, v);
  return eina_array_accessor_new(arr);
}

EOLIAN static Eina_Accessor *
_test_object_method_accessor_of_structs(Eo* obj EINA_UNUSED,
                                          Test_Object_Data *pd EINA_UNUSED,
                                          Eina_Accessor *a_in)
{
  fprintf(stdout, "_test_object_method_accessor_of_structs(%p)\n", a_in);
  if (a_in) return a_in;
  Test_Struct_Ex *v = malloc(sizeof(Test_Struct_Ex));
  v->value_int = 42;
  v->value_enum = TEST_ENUM_EX_THIRD;
  Eina_Array *arr = eina_array_new(2);
  eina_array_push(arr, v);
  return eina_array_accessor_new(arr);
}

// Combinations of complex types

EOLIAN static Eina_List *
_test_object_method_list_of_lists_of_ints(Eo* obj EINA_UNUSED,
                                            Test_Object_Data *pd EINA_UNUSED,
                                            Eina_List *l_in)
{
  fprintf(stdout, "_test_object_method_list_of_lists_of_ints(%p)\n", l_in);
  if (l_in) return l_in;
  int *v = malloc(sizeof(int));
  *v = 42;
  return eina_list_append(NULL, eina_list_append(NULL, v));
}

EOLIAN static Eina_Array *
_test_object_method_array_of_lists_of_ints(Eo* obj EINA_UNUSED,
                                             Test_Object_Data *pd EINA_UNUSED,
                                             Eina_Array *a_in)
{
  fprintf(stdout, "_test_object_method_array_of_lists_of_ints(%p)\n", a_in);
  if (a_in) return a_in;
  int *v = malloc(sizeof(int));
  *v = 42;
  Eina_Array *arr = eina_array_new(2);
  eina_array_push(arr, eina_list_append(NULL, v));
  return arr;
}

EOLIAN static Eina_List *
_test_object_method_list_of_arrays_of_ints(Eo* obj EINA_UNUSED,
                                             Test_Object_Data *pd EINA_UNUSED,
                                             Eina_List *l_in)
{
  fprintf(stdout, "_test_object_method_list_of_arrays_of_ints(%p)\n", l_in);
  if (l_in) return l_in;
  int *v = malloc(sizeof(int));
  *v = 42;
  Eina_Array *arr = eina_array_new(2);
  eina_array_push(arr, v);
  return eina_list_append(NULL, arr);
}

EOLIAN static const Eina_List *
_test_object_method_list_with_opaque_elements(Eo* obj EINA_UNUSED,
                                                Test_Object_Data *pd EINA_UNUSED)
{
  fprintf(stdout, "_test_object_method_list_with_opaque_elements()\n");
  return NULL;
}

EOLIAN static Test_Enum_Ex
_test_object_method_in_enum_return_enum(Eo* obj EINA_UNUSED,
                                          Test_Object_Data *pd EINA_UNUSED,
                                          Test_Enum_Ex e)
{
  fprintf(stdout, "_test_object_method_in_enum_return_enum(%d)\n", e);
  return e;
}

EOLIAN static Test_Struct_Ex *
_test_object_method_in_struct_return_struct(Eo* obj EINA_UNUSED,
                                     Test_Object_Data *pd EINA_UNUSED,
                                     Test_Struct_Ex *s)
{
  fprintf(stdout, "_test_object_method_in_struct_return_struct()\n");
  return s;
}

EOLIAN static void
_test_object_event_repeated_event_name(Eo* obj EINA_UNUSED,
                                         Test_Object_Data *pd EINA_UNUSED)
{
}

#include <test_object.eo.c>
