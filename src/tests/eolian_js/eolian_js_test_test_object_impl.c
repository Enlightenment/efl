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

EOLIAN static Efl_Object *
_test_object_efl_object_constructor(Eo* obj, Test_Object_Data *pd)
{
  fprintf(stdout, "_test_object_efl_object_constructor\n");
  fflush(stdout);

  pd->a = 0;
  return efl_constructor(efl_super(obj, MY_CLASS));
}

EOLIAN static Eo *
_test_object_efl_object_finalize(Eo *obj, Test_Object_Data *pd EINA_UNUSED)
{
  fprintf(stdout, "_test_object_efl_object_finalize\n");
  fflush(stdout);

  return efl_finalize(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_test_object_efl_object_destructor(Eo* obj, Test_Object_Data *pd EINA_UNUSED)
{
  fprintf(stdout, "_test_object_efl_object_destructor\n");
  fflush(stdout);
  efl_destructor(efl_super(obj, MY_CLASS));
}


EOLIAN static void
_test_object_method_integral_in_a_check(Eo* obj EINA_UNUSED,
  Test_Object_Data *pd,
  int a)
{
  fprintf(stdout, "_test_object_method_integral_in_a_check(%d)\n", a);
  fflush(stdout);
  pd->a = a;
}

EOLIAN static void
_test_object_method_integral_out_a_check(Eo* obj EINA_UNUSED,
  Test_Object_Data *pd,
  int *a)
{
  fprintf(stdout, "_test_object_method_integral_out_a_check(%p)\n", a);
  fflush(stdout);
  *a = pd->a;
}

EOLIAN static void
_test_object_method_integral_inout_check(Eo* obj EINA_UNUSED,
  Test_Object_Data *pd EINA_UNUSED,
  int *a)
{
  fprintf(stdout, "_test_object_method_integral_inout_check(%d [%p])\n", *a, a);
  fflush(stdout);
  *a = -(*a);
}

EOLIAN static int
_test_object_method_integral_return_a_check(Eo* obj EINA_UNUSED,
  Test_Object_Data *pd)
{
  fprintf(stdout, "_test_object_method_integral_return_a_check()\n");
  fflush(stdout);
  return pd->a;
}

EOLIAN static Eina_Bool
_test_object_method_div_mod_check(Eo* obj EINA_UNUSED,
  Test_Object_Data *pd EINA_UNUSED,
  int a,
  int b,
  int *quotient,
  int *remainder)
{
  fprintf(stdout, "_test_object_method_div_mod_check(%d, %d, %p, %p)\n", a, b, quotient, remainder);
  fflush(stdout);
  if (0 == b) return EINA_FALSE;
  *quotient = a / b;
  *remainder = a % b;
  return EINA_TRUE;
}

EOLIAN static Eina_Bool
_test_object_method_and_check(Eo* obj EINA_UNUSED,
  Test_Object_Data *pd EINA_UNUSED,
  Eina_Bool a,
  Eina_Bool b)
{
  fprintf(stdout, "_test_object_method_and_check(%d, %d)\n", a, b);
  fflush(stdout);
  return a && b;
}

EOLIAN static double
_test_object_method_modf_check(Eo* obj EINA_UNUSED,
  Test_Object_Data *pd EINA_UNUSED,
  double x,
  double *int_part)
{
  fprintf(stdout, "_test_object_method_modf_check(%f, %p)\n", x, int_part);
  fflush(stdout);
  return modf(x, int_part);
}

EOLIAN static void
_test_object_method_uppercase_check(Eo* obj EINA_UNUSED,
  Test_Object_Data *pd EINA_UNUSED,
  char **str)
{
  fprintf(stdout, "_test_object_method_uppercase_check('%s')\n", *str);
  fflush(stdout);
  if (!*str) return;

  char *p = *str;
  while ('\0' != *p)
    {
       *p = toupper(*p);
       p++;
    }
}

EOLIAN static Eina_Bool
_test_object_method_in_null_check(Eo* obj EINA_UNUSED,
  Test_Object_Data *pd EINA_UNUSED,
  char *a)
{
  fprintf(stdout, "_test_object_method_in_null_check(%p)\n", a);
  fflush(stdout);
  return NULL == a;
}

EOLIAN static Eina_Bool
_test_object_method_out_null_check(Eo* obj EINA_UNUSED,
  Test_Object_Data *pd EINA_UNUSED,
  char **a)
{
  fprintf(stdout, "_test_object_method_out_null_check(%p)\n", a);
  fflush(stdout);
  *a = NULL;
  return EINA_TRUE;
}

EOLIAN static Eina_Bool
_test_object_method_inout_null_check(Eo* obj EINA_UNUSED,
  Test_Object_Data *pd EINA_UNUSED,
  char **a)
{
  fprintf(stdout, "_test_object_method_inout_null_check(%p)\n", a);
  fflush(stdout);
  return NULL == *a;
}

EOLIAN static char *
_test_object_method_return_null_check(Eo* obj EINA_UNUSED,
  Test_Object_Data *pd EINA_UNUSED)
{
  fprintf(stdout, "_test_object_method_return_null_check()\n");
  fflush(stdout);
  return NULL;
}

EOLIAN static void
_test_object_event_emit(Eo* obj, Test_Object_Data *pd EINA_UNUSED)
{
   fprintf(stdout, "_test_object_event_emit()\n");
   fflush(stdout);
   static Test_Struct_Ex s = {42, TEST_ENUM_EX_THIRD};
   efl_event_callback_call(obj, TEST_OBJECT_EVENT_TEST, NULL);
   efl_event_callback_call(obj, TEST_OBJECT_EVENT_TEST_STRUCTARG, &s);
   efl_event_callback_call(obj, TEST_OBJECT_EVENT_TEST_STRINGARG, "foo");
}

EOLIAN static char *
_test_object_method_null_check(Eo* obj EINA_UNUSED,
  Test_Object_Data *pd EINA_UNUSED,
  char *in,
  char **out,
  char **inout)
{
  fprintf(stdout, "_test_object_method_null_check(%p, %p, %p)\n", in, out, inout);
  fflush(stdout);
  assert(!in);
  *out = NULL;
  *inout = NULL;
  return NULL;
}

// Arrays //

EOLIAN static int
_test_object_method_array_at_check(Eo* obj EINA_UNUSED,
  Test_Object_Data *pd EINA_UNUSED,
  Eina_Array *array,
  int index)
{
  fprintf(stdout, "_test_object_method_array_at_check(%p, %d)\n", array, index);
  fflush(stdout);
  return *((int*)eina_array_data_get(array, index));
}

EOLIAN static Eina_Array *
_test_object_method_array_with_42_check(Eo* obj EINA_UNUSED,
  Test_Object_Data *pd EINA_UNUSED)
{
  fprintf(stdout, "_test_object_method_array_with_42_check()\n");
  fflush(stdout);
  Eina_Array *arr = eina_array_new(2);
  int* n = malloc(sizeof(int));
  *n = 42;
  eina_array_push(arr, n);
  return arr;
}

EOLIAN static void
_test_object_method_array_in_array_out_check(Eo* obj EINA_UNUSED,
  Test_Object_Data *pd EINA_UNUSED,
  Eina_Array *a_in,
  Eina_Array **a_out)
{
  fprintf(stdout, "_test_object_method_array_in_array_out_check(%p, %p)\n", a_in, a_out);
  fflush(stdout);
  *a_out = a_in;
}

EOLIAN static Eina_Array *
_test_object_method_array_of_objects_check(Eo* obj,
  Test_Object_Data *pd EINA_UNUSED,
  Eina_Array *a_in)
{
  fprintf(stdout, "_test_object_method_array_of_objects_check(%p)\n", a_in);
  fflush(stdout);
  if (a_in) return a_in;
  Eina_Array *arr = eina_array_new(2);
  eina_array_push(arr, obj);
  return arr;
}


EOLIAN static Eina_Array *
_test_object_method_array_of_strings_check(Eo* obj EINA_UNUSED,
  Test_Object_Data *pd EINA_UNUSED,
  Eina_Array *a_in)
{
  fprintf(stdout, "_test_object_method_array_of_strings_check(%p)\n", a_in);
  fflush(stdout);
  if (a_in) return a_in;
  Eina_Array *arr = eina_array_new(2);
  const char* v = "foo";
  fprintf(stdout, "v = %p\n", v);
  eina_array_push(arr, v);
  return arr;
}

EOLIAN static Eina_Array *
_test_object_method_array_of_ints_check(Eo* obj EINA_UNUSED,
  Test_Object_Data *pd EINA_UNUSED,
  Eina_Array *a_in)
{
  fprintf(stdout, "_test_object_method_array_of_ints_check(%p)\n", a_in);
  fflush(stdout);
  if (a_in) return a_in;
  int *v = malloc(sizeof(int));
  *v = 42;
  Eina_Array *arr = eina_array_new(2);
  eina_array_push(arr, v);
  return arr;
}

EOLIAN static Eina_Array *
_test_object_method_array_of_bools_check(Eo* obj EINA_UNUSED,
  Test_Object_Data *pd EINA_UNUSED,
  Eina_Array *a_in)
{
  fprintf(stdout, "_test_object_method_array_of_bools_check(%p)\n", a_in);
  fflush(stdout);
  if (a_in) return a_in;
  Eina_Bool *v = malloc(sizeof(Eina_Bool));
  *v = EINA_TRUE;
  Eina_Array *arr = eina_array_new(2);
  eina_array_push(arr, v);
  return arr;
}

EOLIAN static Eina_Array *
_test_object_method_array_of_doubles_check(Eo* obj EINA_UNUSED,
  Test_Object_Data *pd EINA_UNUSED,
  Eina_Array *a_in)
{
  fprintf(stdout, "_test_object_method_array_of_doubles_check(%p)\n", a_in);
  fflush(stdout);
  if (a_in) return a_in;
  double *v = malloc(sizeof(double));
  *v = 42;
  Eina_Array *arr = eina_array_new(2);
  eina_array_push(arr, v);
  return arr;
}

EOLIAN static Eina_Array *
_test_object_method_array_of_enums_check(Eo* obj EINA_UNUSED,
  Test_Object_Data *pd EINA_UNUSED,
  Eina_Array *a_in)
{
  fprintf(stdout, "_test_object_method_array_of_enums_check(%p)\n", a_in);
  fflush(stdout);
  if (a_in) return a_in;
  Test_Enum_Ex *v = malloc(sizeof(Test_Enum_Ex));
  *v = TEST_ENUM_EX_THIRD;
  Eina_Array *arr = eina_array_new(2);
  eina_array_push(arr, v);
  return arr;
}

EOLIAN static Eina_Array *
_test_object_method_array_of_structs_check(Eo* obj EINA_UNUSED,
  Test_Object_Data *pd EINA_UNUSED,
  Eina_Array *a_in)
{
  fprintf(stdout, "_test_object_method_array_of_structs_check(%p)\n", a_in);
  fflush(stdout);
  if (a_in) return a_in;
  Test_Struct_Ex *v = malloc(sizeof(Test_Struct_Ex));
  v->value_int = 42;
  v->value_enum = TEST_ENUM_EX_THIRD;
  Eina_Array *arr = eina_array_new(2);
  eina_array_push(arr, v);
  return arr;
}

EOLIAN static Eina_Array *
_test_object_method_array_of_arrays_of_ints_check(Eo* obj EINA_UNUSED,
  Test_Object_Data *pd EINA_UNUSED,
  Eina_Array *a_in)
{
  fprintf(stdout, "_test_object_method_array_of_arrays_of_ints_check(%p)\n", a_in);
  fflush(stdout);
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
_test_object_method_list_with_42_check(Eo* obj EINA_UNUSED,
  Test_Object_Data *pd EINA_UNUSED)
{
  fprintf(stdout, "_test_object_method_list_with_42_check()\n");
  fflush(stdout);
  int* n = malloc(sizeof(int));
  *n = 42;
  return eina_list_append(NULL, n);
}

EOLIAN static void
_test_object_method_list_in_list_out_check(Eo* obj EINA_UNUSED,
  Test_Object_Data *pd EINA_UNUSED,
  Eina_List *l_in,
  Eina_List **l_out)
{
  fprintf(stdout, "_test_object_method_list_in_list_out_check(%p , %p)\n", l_in, l_out);
  fflush(stdout);
  *l_out = l_in;
}

EOLIAN static Eina_List *
_test_object_method_list_of_objects_check(Eo* obj,
  Test_Object_Data *pd EINA_UNUSED,
  Eina_List *l_in)
{
  fprintf(stdout, "_test_object_method_list_of_objects_check(%p)\n", l_in);
  fflush(stdout);
  if (l_in) return l_in;
  return eina_list_append(NULL, obj);
}


EOLIAN static Eina_List *
_test_object_method_list_of_strings_check(Eo* obj EINA_UNUSED,
  Test_Object_Data *pd EINA_UNUSED,
  Eina_List *l_in)
{
  fprintf(stdout, "_test_object_method_list_of_strings_check(%p)\n", l_in);
  fflush(stdout);
  if (l_in) return l_in;
  return eina_list_append(NULL, "foo");
}

EOLIAN static Eina_List *
_test_object_method_list_of_ints_check(Eo* obj EINA_UNUSED,
  Test_Object_Data *pd EINA_UNUSED,
  Eina_List *l_in)
{
  fprintf(stdout, "_test_object_method_list_of_ints_check(%p)\n", l_in);
  fflush(stdout);
  if (l_in) return l_in;
  int *v = malloc(sizeof(int));
  *v = 42;
  return eina_list_append(NULL, v);
}

EOLIAN static Eina_List *
_test_object_method_list_of_bools_check(Eo* obj EINA_UNUSED,
  Test_Object_Data *pd EINA_UNUSED,
  Eina_List *l_in)
{
  fprintf(stdout, "_test_object_method_list_of_bools_check(%p)\n", l_in);
  fflush(stdout);
  if (l_in) return l_in;
  Eina_Bool *v = malloc(sizeof(Eina_Bool));
  *v = EINA_TRUE;
  return eina_list_append(NULL, v);
}

EOLIAN static Eina_List *
_test_object_method_list_of_doubles_check(Eo* obj EINA_UNUSED,
  Test_Object_Data *pd EINA_UNUSED,
  Eina_List *l_in)
{
  fprintf(stdout, "_test_object_method_list_of_doubles_check(%p)\n", l_in);
  fflush(stdout);
  if (l_in) return l_in;
  double *v = malloc(sizeof(double));
  *v = 42;
  return eina_list_append(NULL, v);
}

EOLIAN static Eina_List *
_test_object_method_list_of_enums_check(Eo* obj EINA_UNUSED,
  Test_Object_Data *pd EINA_UNUSED,
  Eina_List *l_in)
{
  fprintf(stdout, "_test_object_method_list_of_enums_check(%p)\n", l_in);
  fflush(stdout);
  if (l_in) return l_in;
  Test_Enum_Ex *v = malloc(sizeof(Test_Enum_Ex));
  *v = TEST_ENUM_EX_THIRD;
  return eina_list_append(NULL, v);
}

EOLIAN static Eina_List *
_test_object_method_list_of_structs_check(Eo* obj EINA_UNUSED,
  Test_Object_Data *pd EINA_UNUSED,
  Eina_List *l_in)
{
  fprintf(stdout, "_test_object_method_list_of_structs_check(%p)\n", l_in);
  fflush(stdout);
  if (l_in) return l_in;
  Test_Struct_Ex *v = malloc(sizeof(Test_Struct_Ex));
  v->value_int = 42;
  v->value_enum = TEST_ENUM_EX_THIRD;
  return eina_list_append(NULL, v);
}

// Accessors //

EOLIAN static Eina_Accessor *
_test_object_method_accessor_of_objects_check(Eo* obj,
  Test_Object_Data *pd EINA_UNUSED,
  Eina_Accessor *a_in)
{
  fprintf(stdout, "_test_object_method_accessor_of_objects_check(%p)\n", a_in);
  fflush(stdout);
  if (a_in) return a_in;
  Eina_Array *arr = eina_array_new(2);
  eina_array_push(arr, obj);
  return eina_array_accessor_new(arr);
}

EOLIAN static Eina_Accessor *
_test_object_method_accessor_of_strings_check(Eo* obj EINA_UNUSED,
  Test_Object_Data *pd EINA_UNUSED,
  Eina_Accessor *a_in)
{
  fprintf(stdout, "_test_object_method_accessor_of_strings_check(%p)\n", a_in);
  fflush(stdout);
  if (a_in) return a_in;
  Eina_Array *arr = eina_array_new(2);
  eina_array_push(arr, "foo");
  return eina_array_accessor_new(arr);
}

EOLIAN static Eina_Accessor *
_test_object_method_accessor_of_ints_check(Eo* obj EINA_UNUSED,
  Test_Object_Data *pd EINA_UNUSED,
  Eina_Accessor *a_in)
{
  fprintf(stdout, "_test_object_method_accessor_of_ints_check(%p)\n", a_in);
  fflush(stdout);
  if (a_in) return a_in;
  int *v = malloc(sizeof(int));
  *v = 42;
  Eina_Array *arr = eina_array_new(2);
  eina_array_push(arr, v);
  return eina_array_accessor_new(arr);
}

EOLIAN static Eina_Accessor *
_test_object_method_accessor_of_bools_check(Eo* obj EINA_UNUSED,
  Test_Object_Data *pd EINA_UNUSED,
  Eina_Accessor *a_in)
{
  fprintf(stdout, "_test_object_method_accessor_of_bools_check(%p)\n", a_in);
  fflush(stdout);
  if (a_in) return a_in;
  Eina_Bool *v = malloc(sizeof(Eina_Bool));
  *v = EINA_TRUE;
  Eina_Array *arr = eina_array_new(2);
  eina_array_push(arr, v);
  return eina_array_accessor_new(arr);
}

EOLIAN static Eina_Accessor *
_test_object_method_accessor_of_doubles_check(Eo* obj EINA_UNUSED,
  Test_Object_Data *pd EINA_UNUSED,
  Eina_Accessor *a_in)
{
  fprintf(stdout, "_test_object_method_accessor_of_doubles_check(%p)\n", a_in);
  fflush(stdout);
  if (a_in) return a_in;
  double *v = malloc(sizeof(double));
  *v = 42.0;
  Eina_Array *arr = eina_array_new(2);
  eina_array_push(arr, v);
  return eina_array_accessor_new(arr);
}

EOLIAN static Eina_Accessor *
_test_object_method_accessor_of_enums_check(Eo* obj EINA_UNUSED,
  Test_Object_Data *pd EINA_UNUSED,
  Eina_Accessor *a_in)
{
  fprintf(stdout, "_test_object_method_accessor_of_enums_check(%p)\n", a_in);
  fflush(stdout);
  if (a_in) return a_in;
  Test_Enum_Ex *v = malloc(sizeof(Test_Enum_Ex));
  *v = TEST_ENUM_EX_THIRD;
  Eina_Array *arr = eina_array_new(2);
  eina_array_push(arr, v);
  return eina_array_accessor_new(arr);
}

EOLIAN static Eina_Accessor *
_test_object_method_accessor_of_structs_check(Eo* obj EINA_UNUSED,
  Test_Object_Data *pd EINA_UNUSED,
  Eina_Accessor *a_in)
{
  fprintf(stdout, "_test_object_method_accessor_of_structs_check(%p)\n", a_in);
  fflush(stdout);
  if (a_in) return a_in;
  Test_Struct_Ex *v = malloc(sizeof(Test_Struct_Ex));
  v->value_int = 42;
  v->value_enum = TEST_ENUM_EX_THIRD;
  Eina_Array *arr = eina_array_new(2);
  eina_array_push(arr, v);
  return eina_array_accessor_new(arr);
}

// Iterator //

EOLIAN static Eina_Iterator *
_test_object_method_iterator_of_objects_check(Eo* obj,
  Test_Object_Data *pd EINA_UNUSED,
  Eina_Iterator *i_in)
{
  fprintf(stdout, "_test_object_method_iterator_of_objects_check(%p)\n", i_in);
  fflush(stdout);
  if (i_in) return i_in;
  Eina_Array *arr = eina_array_new(2);
  eina_array_push(arr, obj);
  eina_array_push(arr, obj);
  return eina_array_iterator_new(arr);
}

EOLIAN static Eina_Iterator *
_test_object_method_iterator_of_strings_check(Eo* obj EINA_UNUSED,
  Test_Object_Data *pd EINA_UNUSED,
  Eina_Iterator *i_in)
{
  fprintf(stdout, "_test_object_method_iterator_of_strings_check(%p)\n", i_in);
  fflush(stdout);
  if (i_in) return i_in;
  Eina_Array *arr = eina_array_new(2);
  eina_array_push(arr, "foo");
  eina_array_push(arr, "bar");
  return eina_array_iterator_new(arr);
}

EOLIAN static Eina_Iterator *
_test_object_method_iterator_of_ints_check(Eo* obj EINA_UNUSED,
  Test_Object_Data *pd EINA_UNUSED,
  Eina_Iterator *i_in)
{
  fprintf(stdout, "_test_object_method_iterator_of_ints_check(%p)\n", i_in);
  fflush(stdout);
  if (i_in) return i_in;
  Eina_Array *arr = eina_array_new(2);
  int *v = malloc(sizeof(int));
  *v = 42;
  eina_array_push(arr, v);
  v = malloc(sizeof(int));
  *v = 24;
  eina_array_push(arr, v);
  return eina_array_iterator_new(arr);
}

EOLIAN static Eina_Iterator *
_test_object_method_iterator_of_bools_check(Eo* obj EINA_UNUSED,
  Test_Object_Data *pd EINA_UNUSED,
  Eina_Iterator *i_in)
{
  fprintf(stdout, "_test_object_method_iterator_of_bools_check(%p)\n", i_in);
  fflush(stdout);
  if (i_in) return i_in;
  Eina_Array *arr = eina_array_new(2);
  Eina_Bool *v = malloc(sizeof(Eina_Bool));
  *v = EINA_TRUE;
  eina_array_push(arr, v);
  v = malloc(sizeof(Eina_Bool));
  *v = EINA_FALSE;
  eina_array_push(arr, v);
  return eina_array_iterator_new(arr);
}

EOLIAN static Eina_Iterator *
_test_object_method_iterator_of_doubles_check(Eo* obj EINA_UNUSED,
  Test_Object_Data *pd EINA_UNUSED,
  Eina_Iterator *i_in)
{
  fprintf(stdout, "_test_object_method_iterator_of_doubles_check(%p)\n", i_in);
  fflush(stdout);
  if (i_in) return i_in;
  Eina_Array *arr = eina_array_new(2);
  double *v = malloc(sizeof(double));
  *v = 42.0;
  eina_array_push(arr, v);
  v = malloc(sizeof(double));
  *v = 24.0;
  eina_array_push(arr, v);
  return eina_array_iterator_new(arr);
}

EOLIAN static Eina_Iterator *
_test_object_method_iterator_of_enums_check(Eo* obj EINA_UNUSED,
  Test_Object_Data *pd EINA_UNUSED,
  Eina_Iterator *i_in)
{
  fprintf(stdout, "_test_object_method_iterator_of_enums_check(%p)\n", i_in);
  fflush(stdout);
  if (i_in) return i_in;
  Eina_Array *arr = eina_array_new(2);
  Test_Enum_Ex *v = malloc(sizeof(Test_Enum_Ex));
  *v = TEST_ENUM_EX_THIRD;
  eina_array_push(arr, v);
  v = malloc(sizeof(Test_Enum_Ex));
  *v = TEST_ENUM_EX_FIRST;
  eina_array_push(arr, v);
  return eina_array_iterator_new(arr);
}

EOLIAN static Eina_Iterator *
_test_object_method_iterator_of_structs_check(Eo* obj EINA_UNUSED,
  Test_Object_Data *pd EINA_UNUSED,
  Eina_Iterator *i_in)
{
  fprintf(stdout, "_test_object_method_iterator_of_structs_check(%p)\n", i_in);
  fflush(stdout);
  if (i_in) return i_in;
  Eina_Array *arr = eina_array_new(2);
  Test_Struct_Ex *v = malloc(sizeof(Test_Struct_Ex));
  v->value_int = 42;
  v->value_enum = TEST_ENUM_EX_THIRD;
  eina_array_push(arr, v);
  v = malloc(sizeof(Test_Struct_Ex));
  v->value_int = 24;
  v->value_enum = TEST_ENUM_EX_FIRST;
  eina_array_push(arr, v);
  return eina_array_iterator_new(arr);
}

// Combinations of complex types

EOLIAN static Eina_List *
_test_object_method_list_of_lists_of_ints_check(Eo* obj EINA_UNUSED,
  Test_Object_Data *pd EINA_UNUSED,
  Eina_List *l_in)
{
  fprintf(stdout, "_test_object_method_list_of_lists_of_ints_check(%p)\n", l_in);
  fflush(stdout);
  if (l_in) return l_in;
  int *v = malloc(sizeof(int));
  *v = 42;
  return eina_list_append(NULL, eina_list_append(NULL, v));
}

EOLIAN static Eina_Array *
_test_object_method_array_of_lists_of_ints_check(Eo* obj EINA_UNUSED,
  Test_Object_Data *pd EINA_UNUSED,
  Eina_Array *a_in)
{
  fprintf(stdout, "_test_object_method_array_of_lists_of_ints_check(%p)\n", a_in);
  fflush(stdout);
  if (a_in) return a_in;
  int *v = malloc(sizeof(int));
  *v = 42;
  Eina_Array *arr = eina_array_new(2);
  eina_array_push(arr, eina_list_append(NULL, v));
  return arr;
}

EOLIAN static Eina_List *
_test_object_method_list_of_arrays_of_ints_check(Eo* obj EINA_UNUSED,
  Test_Object_Data *pd EINA_UNUSED,
  Eina_List *l_in)
{
  fprintf(stdout, "_test_object_method_list_of_arrays_of_ints_check(%p)\n", l_in);
  fflush(stdout);
  if (l_in) return l_in;
  int *v = malloc(sizeof(int));
  *v = 42;
  Eina_Array *arr = eina_array_new(2);
  eina_array_push(arr, v);
  return eina_list_append(NULL, arr);
}

EOLIAN static const Eina_List *
_test_object_method_list_with_opaque_elements_check(Eo* obj EINA_UNUSED,
  Test_Object_Data *pd EINA_UNUSED)
{
  fprintf(stdout, "_test_object_method_list_with_opaque_elements_check()\n");
  fflush(stdout);
  return NULL;
}

EOLIAN static Test_Enum_Ex
_test_object_method_in_enum_return_enum_check(Eo* obj EINA_UNUSED,
  Test_Object_Data *pd EINA_UNUSED,
  Test_Enum_Ex e)
{
  fprintf(stdout, "_test_object_method_in_enum_return_enum_check(%d)\n", e);
  fflush(stdout);
  return e;
}

EOLIAN static Test_Struct_Ex *
_test_object_method_in_struct_return_struct_check(Eo* obj EINA_UNUSED,
  Test_Object_Data *pd EINA_UNUSED,
  Test_Struct_Ex *s)
{
  fprintf(stdout, "_test_object_method_in_struct_return_struct_check(%p)\n", s);
  fflush(stdout);
  return s;
}

EOLIAN static void
_test_object_event_repeated_event_name(Eo* obj EINA_UNUSED,
  Test_Object_Data *pd EINA_UNUSED)
{
  fprintf(stdout, "_test_object_event_repeated_event_name()\n");
  fflush(stdout);
}

#include <test_object.eo.c>
