
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <Eo.h>

#undef EOAPI
#undef EAPI
#define EOAPI EAPI EAPI_WEAK

#ifdef _WIN32
# ifdef EFL_EO_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_EO_BUILD */
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif /* ! _WIN32 */

#include "test_numberwrapper.eo.h"
#include "test_testing.eo.h"

typedef struct Test_Testing_Data
{
  SimpleCb cb;
  void *cb_data;
  Eina_Free_Cb free_cb;
  Eina_Error error_code;

} Test_Testing_Data;

typedef struct Test_Numberwrapper_Data
{
   int number;
} Test_Numberwrapper_Data;


static
void *_new_int(int v)
{
   int *r = malloc(sizeof(int));
   *r = v;
   return r;
}

static
Test_Numberwrapper *_new_obj(int n)
{
   return efl_add(TEST_NUMBERWRAPPER_CLASS, NULL, test_numberwrapper_number_set(efl_added, n));
}

// ############ //
// Test.Testing //
// ############ //

Efl_Object *_test_testing_return_object(Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
  return obj;
}

const char *_test_testing_in_string(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, const char *str)
{
  const char *ret = malloc(sizeof(char)*(strlen(str) + 1));
  return strcpy((char*)ret, str);
}

const char *_test_testing_in_own_string(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, const char *str)
{
  const char *ret = malloc(sizeof(char)*(strlen(str) + 1));
  strcpy((char*)ret, str);
  free((void*)str); // Actually take ownership of it.
  return ret;
}

Eina_Stringshare *_test_testing_return_stringshare(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
  Eina_Stringshare *str = eina_stringshare_add("stringshare");
  return str;
}

Eina_Stringshare *_test_testing_return_own_stringshare(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
  Eina_Stringshare *str = eina_stringshare_add("own_stringshare");
  return str;
}

const char *_test_testing_return_string(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
  return "string";
}

const char *_test_testing_return_own_string(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
  static const char* reference = "own_string";
  const char *ret = malloc(sizeof(char)*(strlen(reference) + 1));
  return strcpy((char*)ret, reference);
}

void _test_testing_out_string(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, const char**str)
{
  *str = "out_string";
}

void _test_testing_out_own_string(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, const char**str)
{
  static const char* reference = "out_own_string";
  *str = malloc(sizeof(char)*(strlen(reference) + 1));
  strcpy((char*)*str, reference);
}

void _test_testing_call_in_string(Eo *obj, EINA_UNUSED Test_Testing_Data *pd, const char *str)
{
  test_testing_in_string(obj, str);
}

void _test_testing_call_in_own_string(Eo *obj, EINA_UNUSED Test_Testing_Data *pd, const char *str)
{
  test_testing_in_own_string(obj, str);
}

const char *_test_testing_call_return_string(Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
  return test_testing_return_string(obj);
}

const char *_test_testing_call_return_own_string(Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
  return test_testing_return_own_string(obj);
}

const char *_test_testing_call_out_string(Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
  const char *ret = NULL;
  test_testing_out_string(obj, &ret);
  return ret;
}

const char *_test_testing_call_out_own_string(Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
  const char *ret = NULL;
  test_testing_out_own_string(obj, &ret);
  return ret;
}

// Stringshare virtual test helpers
void _test_testing_call_in_stringshare(Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Eina_Stringshare *str)
{
  test_testing_in_stringshare(obj, str);
}

void _test_testing_call_in_own_stringshare(Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Eina_Stringshare *str)
{
  test_testing_in_own_stringshare(obj, str);
  eina_stringshare_del(str);
}

Eina_Stringshare *_test_testing_in_stringshare(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Eina_Stringshare *str)
{
    return eina_stringshare_add(str);
}

Eina_Stringshare *_test_testing_in_own_stringshare(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Eina_Stringshare *str)
{
    return str;
}

void _test_testing_out_stringshare(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, const char**str)
{
  // Returning simple string but the binding shouldn't del it as it is not owned by the caller
  *str = "out_stringshare";
}

void _test_testing_out_own_stringshare(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, const char**str)
{
  *str = eina_stringshare_add("out_own_stringshare");
}

Eina_Stringshare *_test_testing_call_return_stringshare(Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
  return test_testing_return_stringshare(obj);
}

Eina_Stringshare *_test_testing_call_return_own_stringshare(Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
  return test_testing_return_own_stringshare(obj);
}

Eina_Stringshare *_test_testing_call_out_stringshare(Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
  Eina_Stringshare *ret = NULL;
  test_testing_out_stringshare(obj, &ret);
  return ret;
}

Eina_Stringshare *_test_testing_call_out_own_stringshare(Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
  Eina_Stringshare *ret = NULL;
  test_testing_out_own_stringshare(obj, &ret);
  return ret;
}


static const uint8_t base_arr[] = {0x0,0x2A,0x42};
// static const size_t base_arr_size = 3; // TODO: Use it!!!

static void *memdup(const void* mem, size_t size)
{
  void *out = malloc(size);
  memcpy(out, mem, size);
  return out;
}

Eina_Bool _test_testing_eina_slice_in(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Eina_Slice slice)
{
  uint8_t *buf = memdup(slice.mem, slice.len);
  free(buf);
  return 0 == memcmp(slice.mem, base_arr, slice.len);
}

Eina_Bool _test_testing_eina_rw_slice_in(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Eina_Rw_Slice slice)
{
  Eina_Bool r = (0 == memcmp(slice.mem, base_arr, slice.len));
  unsigned char *buf = memdup(slice.mem, slice.len);
  free(buf);
  for (unsigned i = 0; i < slice.len; ++i)
    slice.bytes[i] += 1;
  return r;
}

Eina_Bool _test_testing_eina_slice_out(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Eina_Slice *slice)
{
  if (!slice) return EINA_FALSE;
  static const Eina_Slice slc = EINA_SLICE_ARRAY(base_arr);
  slice->len = slc.len;
  slice->mem = slc.mem;
  return EINA_TRUE;
}

Eina_Bool _test_testing_eina_rw_slice_out(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Eina_Rw_Slice *slice)
{
  if (!slice) return EINA_FALSE;
  slice->len = 3;
  slice->mem = memdup(base_arr, 3);
  return EINA_TRUE;
}

Eina_Slice _test_testing_eina_slice_return(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
  Eina_Slice slc = EINA_SLICE_ARRAY(base_arr);
  return slc;
}

Eina_Rw_Slice _test_testing_eina_rw_slice_return(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
  Eina_Rw_Slice slc = { .len = 3, .mem = memdup(base_arr, 3) };
  return slc;
}

Eina_Bool _test_testing_eina_binbuf_in(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Eina_Binbuf *binbuf)
{
  Eina_Bool r = (0 == memcmp(eina_binbuf_string_get(binbuf), base_arr, eina_binbuf_length_get(binbuf)));
  eina_binbuf_insert_char(binbuf, 42, 0);
  eina_binbuf_insert_char(binbuf, 43, 0);
  eina_binbuf_append_char(binbuf, 33);
  return r;
}

Eina_Binbuf *_binbuf_in_own_to_check = NULL;

Eina_Bool _test_testing_eina_binbuf_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Eina_Binbuf *binbuf)
{
  Eina_Bool r = (0 == memcmp(eina_binbuf_string_get(binbuf), base_arr, eina_binbuf_length_get(binbuf)));
  eina_binbuf_insert_char(binbuf, 42, 0);
  eina_binbuf_insert_char(binbuf, 43, 0);
  eina_binbuf_append_char(binbuf, 33);
  _binbuf_in_own_to_check = binbuf;
  return r;
}

Eina_Bool _test_testing_check_binbuf_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
    if (!_binbuf_in_own_to_check) return EINA_FALSE;
    const uint8_t mod_arr[] = {43,42,0x0,0x2A,0x42,33};
    Eina_Bool r = (0 == memcmp(eina_binbuf_string_get(_binbuf_in_own_to_check), mod_arr, eina_binbuf_length_get(_binbuf_in_own_to_check)));
    eina_binbuf_string_free(_binbuf_in_own_to_check);
    return r;
}

Eina_Binbuf *_binbuf_out_to_check = NULL;

Eina_Bool _test_testing_eina_binbuf_out(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Eina_Binbuf **binbuf)
{
  if (!binbuf) return EINA_FALSE;
  *binbuf = eina_binbuf_new();
  eina_binbuf_append_char(*binbuf, 33);
  _binbuf_out_to_check = *binbuf;
  return EINA_TRUE;
}

Eina_Bool _test_testing_check_binbuf_out(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
    if (!_binbuf_out_to_check) return EINA_FALSE;
    return 0 == memcmp(eina_binbuf_string_get(_binbuf_out_to_check), base_arr, eina_binbuf_length_get(_binbuf_out_to_check));
}

Eina_Bool _test_testing_eina_binbuf_out_own(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Eina_Binbuf **binbuf)
{
  if (!binbuf) return EINA_FALSE;
  *binbuf = eina_binbuf_new();
  eina_binbuf_append_char(*binbuf, 33);
  return EINA_TRUE;
}

Eina_Binbuf *_binbuf_return_to_check = NULL;

Eina_Binbuf *_test_testing_eina_binbuf_return(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
  Eina_Binbuf *binbuf = eina_binbuf_new();
  eina_binbuf_append_char(binbuf, 33);
  _binbuf_return_to_check = binbuf;
  return binbuf;
}

Eina_Bool _test_testing_check_binbuf_return(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
    if (!_binbuf_return_to_check) return EINA_FALSE;
    return 0 == memcmp(eina_binbuf_string_get(_binbuf_return_to_check), base_arr, eina_binbuf_length_get(_binbuf_return_to_check));
}

Eina_Binbuf *_test_testing_eina_binbuf_return_own(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
  Eina_Binbuf *binbuf = eina_binbuf_new();
  eina_binbuf_append_char(binbuf, 33);
  return binbuf;
}

static const int base_arr_int[] = {0x0,0x2A,0x42};
static const unsigned int base_arr_int_size = EINA_C_ARRAY_LENGTH(base_arr_int);
static const int modified_arr_int[] = {0x0,0x2A,0x42,42,43,33};
static const unsigned int modified_arr_int_size = EINA_C_ARRAY_LENGTH(modified_arr_int);

static const char * const base_arr_str[] = {"0x0","0x2A","0x42"};
static const unsigned int base_arr_str_size = EINA_C_ARRAY_LENGTH(base_arr_str);
static const char * const modified_arr_str[] = {"0x0","0x2A","0x42","42","43","33"};
static const unsigned int modified_arr_str_size = EINA_C_ARRAY_LENGTH(modified_arr_str);

static const Test_Numberwrapper *base_arr_obj[] = {NULL,NULL,NULL};
static const unsigned int base_arr_obj_size = EINA_C_ARRAY_LENGTH(base_arr_str);
static const Test_Numberwrapper *modified_arr_obj[] = {NULL,NULL,NULL,NULL,NULL,NULL};
static const unsigned int modified_arr_obj_size = EINA_C_ARRAY_LENGTH(modified_arr_str);

// //
// Array
//

static const unsigned int default_step = 32;

// Integer

Eina_Bool _array_int_equal(const Eina_Array *arr, const int base[], unsigned int len)
{
   if (eina_array_count(arr) != len)
     return EINA_FALSE;
   for (unsigned int i = 0; i < len; ++i)
     {
        if (*((int*)eina_array_data_get(arr, i)) != base[i])
          return EINA_FALSE;
     }
   return EINA_TRUE;
}

Eina_Bool _test_testing_eina_array_int_in(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Eina_Array *arr)
{
   Eina_Bool r = _array_int_equal(arr, base_arr_int, base_arr_int_size);
   eina_array_push(arr, _new_int(42));
   eina_array_push(arr, _new_int(43));
   eina_array_push(arr, _new_int(33));
   return r;
}

static Eina_Array *_array_int_in_own_to_check = NULL;

Eina_Bool _test_testing_eina_array_int_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Eina_Array *arr)
{
   Eina_Bool r = _array_int_equal(arr, base_arr_int, base_arr_int_size);
   eina_array_push(arr, _new_int(42));
   eina_array_push(arr, _new_int(43));
   eina_array_push(arr, _new_int(33));
   _array_int_in_own_to_check = arr;
   return r;
}

Eina_Bool _test_testing_check_eina_array_int_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
    if (!_array_int_in_own_to_check) return EINA_FALSE;

    Eina_Bool r = _array_int_equal(_array_int_in_own_to_check, modified_arr_int, modified_arr_int_size);

    unsigned int i;
    int *ele;
    Eina_Array_Iterator it;
    EINA_ARRAY_ITER_NEXT(_array_int_in_own_to_check, i, ele, it)
      free(ele);

    eina_array_free(_array_int_in_own_to_check);
    _array_int_in_own_to_check = NULL;
    return r;
}

Eina_Array *_array_int_out_to_check = NULL;

Eina_Bool _test_testing_eina_array_int_out(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Eina_Array **arr)
{
    if (!arr) return EINA_FALSE;
    *arr = eina_array_new(default_step);
    eina_array_push(*arr, _new_int(0x0));
    eina_array_push(*arr, _new_int(0x2A));
    eina_array_push(*arr, _new_int(0x42));
    _array_int_out_to_check = *arr;
    return EINA_TRUE;
}
Eina_Bool _test_testing_check_eina_array_int_out(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
   if (!_array_int_out_to_check) return EINA_FALSE;

   Eina_Bool r = _array_int_equal(_array_int_out_to_check, modified_arr_int, modified_arr_int_size);

   unsigned int i;
   int *ele;
   Eina_Array_Iterator it;
   EINA_ARRAY_ITER_NEXT(_array_int_out_to_check, i, ele, it)
     free(ele);

   eina_array_free(_array_int_out_to_check);
   _array_int_out_to_check = NULL;
   return r;
}

Eina_Bool _test_testing_eina_array_int_out_own(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Eina_Array **arr)
{
   if (!arr) return EINA_FALSE;
   *arr = eina_array_new(default_step);
   eina_array_push(*arr, _new_int(0x0));
   eina_array_push(*arr, _new_int(0x2A));
   eina_array_push(*arr, _new_int(0x42));
   return EINA_TRUE;
}

Eina_Array *_array_int_return_to_check = NULL;

Eina_Array *_test_testing_eina_array_int_return(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
   Eina_Array *arr = eina_array_new(default_step);
   eina_array_push(arr, _new_int(0x0));
   eina_array_push(arr, _new_int(0x2A));
   eina_array_push(arr, _new_int(0x42));
   _array_int_return_to_check = arr;
   return arr;
}
Eina_Bool _test_testing_check_eina_array_int_return(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
   if (!_array_int_return_to_check) return EINA_FALSE;

   Eina_Bool r = _array_int_equal(_array_int_return_to_check, modified_arr_int, modified_arr_int_size);

   unsigned int i;
   int *ele;
   Eina_Array_Iterator it;
   EINA_ARRAY_ITER_NEXT(_array_int_return_to_check, i, ele, it)
     free(ele);

   eina_array_free(_array_int_return_to_check);
   _array_int_return_to_check = NULL;
   return r;
}

Eina_Array *_test_testing_eina_array_int_return_own(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
   Eina_Array *arr = eina_array_new(default_step);
   eina_array_push(arr, _new_int(0x0));
   eina_array_push(arr, _new_int(0x2A));
   eina_array_push(arr, _new_int(0x42));
   return arr;
}

// String

Eina_Bool _array_str_equal(const Eina_Array *arr, const char * const base[], unsigned int len)
{
   if (eina_array_count(arr) != len)
     return EINA_FALSE;
   for (unsigned int i = 0; i < len; ++i)
     {
        if (0 != strcmp(eina_array_data_get(arr, i), base[i]))
          return EINA_FALSE;
     }
   return EINA_TRUE;
}

Eina_Bool _test_testing_eina_array_str_in(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Eina_Array *arr)
{
   Eina_Bool r = _array_str_equal(arr, base_arr_str, base_arr_str_size);
   eina_array_push(arr, strdup("42"));
   eina_array_push(arr, strdup("43"));
   eina_array_push(arr, strdup("33"));
   return r;
}

static Eina_Array *_array_str_in_own_to_check = NULL;

Eina_Bool _test_testing_eina_array_str_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Eina_Array *arr)
{
   Eina_Bool r = _array_str_equal(arr, base_arr_str, base_arr_str_size);
   eina_array_push(arr, strdup("42"));
   eina_array_push(arr, strdup("43"));
   eina_array_push(arr, strdup("33"));
   _array_str_in_own_to_check = arr;
   return r;
}

Eina_Bool _test_testing_check_eina_array_str_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
   if (!_array_str_in_own_to_check) return EINA_FALSE;

   Eina_Bool r = _array_str_equal(_array_str_in_own_to_check, modified_arr_str, modified_arr_str_size);

   unsigned int i;
   char *ele;
   Eina_Array_Iterator it;
   EINA_ARRAY_ITER_NEXT(_array_str_in_own_to_check, i, ele, it)
     free(ele);

   eina_array_free(_array_str_in_own_to_check);
   _array_str_in_own_to_check = NULL;
   return r;
}

Eina_Array *_array_str_out_to_check = NULL;

Eina_Bool _test_testing_eina_array_str_out(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Eina_Array **arr)
{
   if (!arr) return EINA_FALSE;
   *arr = eina_array_new(default_step);
   eina_array_push(*arr, strdup("0x0"));
   eina_array_push(*arr, strdup("0x2A"));
   eina_array_push(*arr, strdup("0x42"));
   _array_str_out_to_check = *arr;
   return EINA_TRUE;
}
Eina_Bool _test_testing_check_eina_array_str_out(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
   if (!_array_str_out_to_check) return EINA_FALSE;

   Eina_Bool r = _array_str_equal(_array_str_out_to_check, modified_arr_str, modified_arr_str_size);

   unsigned int i;
   char *ele;
   Eina_Array_Iterator it;
   EINA_ARRAY_ITER_NEXT(_array_str_out_to_check, i, ele, it)
     free(ele);

   eina_array_free(_array_str_out_to_check);
   _array_str_out_to_check = NULL;
   return r;
}

Eina_Bool _test_testing_eina_array_str_out_own(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Eina_Array **arr)
{
   if (!arr) return EINA_FALSE;
   *arr = eina_array_new(default_step);
   eina_array_push(*arr, strdup("0x0"));
   eina_array_push(*arr, strdup("0x2A"));
   eina_array_push(*arr, strdup("0x42"));
   return EINA_TRUE;
}

Eina_Array *_array_str_return_to_check = NULL;

Eina_Array *_test_testing_eina_array_str_return(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
   Eina_Array *arr = eina_array_new(default_step);
   eina_array_push(arr, strdup("0x0"));
   eina_array_push(arr, strdup("0x2A"));
   eina_array_push(arr, strdup("0x42"));
   _array_str_return_to_check = arr;
   return arr;
}
Eina_Bool _test_testing_check_eina_array_str_return(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
   if (!_array_str_return_to_check) return EINA_FALSE;

   Eina_Bool r = _array_str_equal(_array_str_return_to_check, modified_arr_str, modified_arr_str_size);

   unsigned int i;
   char *ele;
   Eina_Array_Iterator it;
   EINA_ARRAY_ITER_NEXT(_array_str_return_to_check, i, ele, it)
     free(ele);

   eina_array_free(_array_str_return_to_check);
   _array_str_return_to_check = NULL;
   return r;
}

Eina_Array *_test_testing_eina_array_str_return_own(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
   Eina_Array *arr = eina_array_new(default_step);
   eina_array_push(arr, strdup("0x0"));
   eina_array_push(arr, strdup("0x2A"));
   eina_array_push(arr, strdup("0x42"));
   return arr;
}

// Object

Eina_Bool _array_obj_equal(const Eina_Array *arr, const Test_Numberwrapper * const base[], unsigned int len)
{
   if (eina_array_count(arr) != len)
     return EINA_FALSE;
   for (unsigned int i = 0; i < len; ++i)
     {
        const Test_Numberwrapper *eo = eina_array_data_get(arr, i);
        int a = test_numberwrapper_number_get(eo);
        int b = test_numberwrapper_number_get(base[i]);
        if (a != b)
          return EINA_FALSE;
     }
   return EINA_TRUE;
}

Eina_Bool _test_testing_eina_array_obj_in(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Eina_Array *arr)
{
   Eina_Bool r = _array_obj_equal(arr, base_arr_obj, base_arr_obj_size);
   if (!r) return r;
   eina_array_push(arr, _new_obj(42));
   eina_array_push(arr, _new_obj(43));
   eina_array_push(arr, _new_obj(33));
   return r;
}

static Eina_Array *_array_obj_in_own_to_check = NULL;

Eina_Bool _test_testing_eina_array_obj_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Eina_Array *arr)
{
   Eina_Bool r = _array_obj_equal(arr, base_arr_obj, base_arr_obj_size);
   if (!r) return r;
   eina_array_push(arr, _new_obj(42));
   eina_array_push(arr, _new_obj(43));
   eina_array_push(arr, _new_obj(33));
   _array_obj_in_own_to_check = arr;
   return r;
}

Eina_Bool _test_testing_check_eina_array_obj_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
   if (!_array_obj_in_own_to_check) return EINA_FALSE;

   Eina_Bool r = _array_obj_equal(_array_obj_in_own_to_check, modified_arr_obj, modified_arr_obj_size);
   if (!r) return r;

   unsigned int i;
   Test_Numberwrapper *ele;
   Eina_Array_Iterator it;
   EINA_ARRAY_ITER_NEXT(_array_obj_in_own_to_check, i, ele, it)
     efl_unref(ele);

   eina_array_free(_array_obj_in_own_to_check);
   _array_obj_in_own_to_check = NULL;
   return r;
}

Eina_Array *_array_obj_out_to_check = NULL;

Eina_Bool _test_testing_eina_array_obj_out(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Eina_Array **arr)
{
   if (!arr) return EINA_FALSE;
   *arr = eina_array_new(default_step);
   eina_array_push(*arr, _new_obj(0x0));
   eina_array_push(*arr, _new_obj(0x2A));
   eina_array_push(*arr, _new_obj(0x42));
   _array_obj_out_to_check = *arr;
   return EINA_TRUE;
}
Eina_Bool _test_testing_check_eina_array_obj_out(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
   if (!_array_obj_out_to_check) return EINA_FALSE;

   Eina_Bool r = _array_obj_equal(_array_obj_out_to_check, modified_arr_obj, modified_arr_obj_size);
   if (!r) return r;

   unsigned int i;
   Test_Numberwrapper *ele;
   Eina_Array_Iterator it;
   EINA_ARRAY_ITER_NEXT(_array_obj_out_to_check, i, ele, it)
     efl_unref(ele);

   eina_array_free(_array_obj_out_to_check);
   _array_obj_out_to_check = NULL;
   return r;
}

Eina_Bool _test_testing_eina_array_obj_out_own(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Eina_Array **arr)
{
   if (!arr) return EINA_FALSE;
   *arr = eina_array_new(default_step);
   eina_array_push(*arr, _new_obj(0x0));
   eina_array_push(*arr, _new_obj(0x2A));
   eina_array_push(*arr, _new_obj(0x42));
   return EINA_TRUE;
}

Eina_Array *_array_obj_return_to_check = NULL;

Eina_Array *_test_testing_eina_array_obj_return(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
   Eina_Array *arr = eina_array_new(default_step);
   eina_array_push(arr, _new_obj(0x0));
   eina_array_push(arr, _new_obj(0x2A));
   eina_array_push(arr, _new_obj(0x42));
   _array_obj_return_to_check = arr;
   return arr;
}
Eina_Bool _test_testing_check_eina_array_obj_return(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
   if (!_array_obj_return_to_check) return EINA_FALSE;

   Eina_Bool r = _array_obj_equal(_array_obj_return_to_check, modified_arr_obj, modified_arr_obj_size);
   if (!r) return r;

   unsigned int i;
   Test_Numberwrapper *ele;
   Eina_Array_Iterator it;
   EINA_ARRAY_ITER_NEXT(_array_obj_return_to_check, i, ele, it)
     efl_unref(ele);

   eina_array_free(_array_obj_return_to_check);
   _array_obj_return_to_check = NULL;
   return r;
}

Eina_Array *_test_testing_eina_array_obj_return_own(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
   Eina_Array *arr = eina_array_new(default_step);
   eina_array_push(arr, _new_obj(0x0));
   eina_array_push(arr, _new_obj(0x2A));
   eina_array_push(arr, _new_obj(0x42));
   return arr;
}

Eina_Array *_test_testing_eina_array_obj_return_in(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Eina_Array *arr)
{
   return arr;
}


// //
// List
//

// Integer

Eina_Bool _list_int_equal(const Eina_List *lst, const int base[], unsigned int len)
{
   if (eina_list_count(lst) != len)
     return EINA_FALSE;

   const Eina_List *l;
   int *data;
   int i = 0;
   EINA_LIST_FOREACH(lst, l, data)
     {
        if (*data != base[i])
          return EINA_FALSE;
        ++i;
     }
   return EINA_TRUE;
}

Eina_Bool _test_testing_eina_list_int_in(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Eina_List *lst)
{
   Eina_Bool r = _list_int_equal(lst, base_arr_int, base_arr_int_size);
   return r;
}

static Eina_List *_list_int_in_own_to_check = NULL;

Eina_Bool _test_testing_eina_list_int_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Eina_List *lst)
{
   Eina_Bool r = _list_int_equal(lst, base_arr_int, base_arr_int_size);
   if (!r) return r;

   lst = eina_list_append(lst, _new_int(42));
   lst = eina_list_append(lst, _new_int(43));
   lst = eina_list_append(lst, _new_int(33));
   _list_int_in_own_to_check = lst;
   return r;
}

Eina_Bool _test_testing_check_eina_list_int_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
   if (!_list_int_in_own_to_check) return EINA_FALSE;

   Eina_Bool r = _list_int_equal(_list_int_in_own_to_check, modified_arr_int, modified_arr_int_size);
   if (!r) return r;

   int *ele;
   EINA_LIST_FREE(_list_int_in_own_to_check, ele)
     free(ele);
   _list_int_in_own_to_check = NULL;
   return r;
}

Eina_List *_list_int_out_to_check = NULL;

Eina_Bool _test_testing_eina_list_int_out(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Eina_List **lst)
{
    if (!lst) return EINA_FALSE;
    *lst = eina_list_append(*lst, _new_int(0x0));
    *lst = eina_list_append(*lst, _new_int(0x2A));
    *lst = eina_list_append(*lst, _new_int(0x42));
    _list_int_out_to_check = *lst;
    return EINA_TRUE;
}
Eina_Bool _test_testing_check_eina_list_int_out(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
   if (!_list_int_out_to_check) return EINA_FALSE;

   Eina_Bool r = _list_int_equal(_list_int_out_to_check, base_arr_int, base_arr_int_size);
   if (!r) return r;

   int *ele;
   EINA_LIST_FREE(_list_int_out_to_check, ele)
     free(ele);

   _list_int_out_to_check = NULL;
   return r;
}

Eina_Bool _test_testing_eina_list_int_out_own(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Eina_List **lst)
{
   if (!lst) return EINA_FALSE;
   *lst = eina_list_append(*lst, _new_int(0x0));
   *lst = eina_list_append(*lst, _new_int(0x2A));
   *lst = eina_list_append(*lst, _new_int(0x42));
   return EINA_TRUE;
}

Eina_List *_list_int_return_to_check = NULL;

Eina_List *_test_testing_eina_list_int_return(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
   Eina_List *lst = NULL;
   lst = eina_list_append(lst, _new_int(0x0));
   lst = eina_list_append(lst, _new_int(0x2A));
   lst = eina_list_append(lst, _new_int(0x42));
   _list_int_return_to_check = lst;
   return lst;
}
Eina_Bool _test_testing_check_eina_list_int_return(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
   if (!_list_int_return_to_check) return EINA_FALSE;

   Eina_Bool r = _list_int_equal(_list_int_return_to_check, base_arr_int, base_arr_int_size);
   if (!r) return r;

   int *ele;
   EINA_LIST_FREE(_list_int_return_to_check, ele)
     free(ele);

   _list_int_return_to_check = NULL;
   return r;
}

Eina_List *_test_testing_eina_list_int_return_own(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
   Eina_List *lst = NULL;
   lst = eina_list_append(lst, _new_int(0x0));
   lst = eina_list_append(lst, _new_int(0x2A));
   lst = eina_list_append(lst, _new_int(0x42));
   return lst;
}

// String

Eina_Bool _list_str_equal(const Eina_List *lst, const char * const base[], unsigned int len)
{
   if (eina_list_count(lst) != len)
     return EINA_FALSE;

   const Eina_List *l;
   char *data;
   int i = 0;
   EINA_LIST_FOREACH(lst, l, data)
     {
        if (0 != strcmp(data, base[i]))
          return EINA_FALSE;
        ++i;
     }
   return EINA_TRUE;
}

Eina_Bool _test_testing_eina_list_str_in(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Eina_List *lst)
{
   Eina_Bool r = _list_str_equal(lst, base_arr_str, base_arr_str_size);
   return r;
}

static Eina_List *_list_str_in_own_to_check = NULL;

Eina_Bool _test_testing_eina_list_str_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Eina_List *lst)
{
   Eina_Bool r = _list_str_equal(lst, base_arr_str, base_arr_str_size);
   if (!r) return r;
   lst = eina_list_append(lst, strdup("42"));
   lst = eina_list_append(lst, strdup("43"));
   lst = eina_list_append(lst, strdup("33"));
   _list_str_in_own_to_check = lst;
   return r;
}

Eina_Bool _test_testing_check_eina_list_str_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
   if (!_list_str_in_own_to_check) return EINA_FALSE;

   Eina_Bool r = _list_str_equal(_list_str_in_own_to_check, modified_arr_str, modified_arr_str_size);
   if (!r) return r;

   char *ele;
   EINA_LIST_FREE(_list_str_in_own_to_check, ele)
     free(ele);

   _list_str_in_own_to_check = NULL;
   return r;
}

Eina_List *_list_str_out_to_check = NULL;

Eina_Bool _test_testing_eina_list_str_out(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Eina_List **lst)
{
   if (!lst) return EINA_FALSE;
   *lst = eina_list_append(*lst, strdup("0x0"));
   *lst = eina_list_append(*lst, strdup("0x2A"));
   *lst = eina_list_append(*lst, strdup("0x42"));
   _list_str_out_to_check = *lst;
   return EINA_TRUE;
}
Eina_Bool _test_testing_check_eina_list_str_out(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
   if (!_list_str_out_to_check) return EINA_FALSE;

   Eina_Bool r = _list_str_equal(_list_str_out_to_check, base_arr_str, base_arr_str_size);
   if (!r) return r;

   char *ele;
   EINA_LIST_FREE(_list_str_out_to_check, ele)
     free(ele);

   _list_str_out_to_check = NULL;
   return r;
}

Eina_Bool _test_testing_eina_list_str_out_own(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Eina_List **lst)
{
   if (!lst) return EINA_FALSE;
   *lst = eina_list_append(*lst, strdup("0x0"));
   *lst = eina_list_append(*lst, strdup("0x2A"));
   *lst = eina_list_append(*lst, strdup("0x42"));
   return EINA_TRUE;
}

Eina_List *_list_str_return_to_check = NULL;

Eina_List *_test_testing_eina_list_str_return(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
   Eina_List *lst = NULL;
   lst = eina_list_append(lst, strdup("0x0"));
   lst = eina_list_append(lst, strdup("0x2A"));
   lst = eina_list_append(lst, strdup("0x42"));
   _list_str_return_to_check = lst;
   return lst;
}
Eina_Bool _test_testing_check_eina_list_str_return(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
   if (!_list_str_return_to_check) return EINA_FALSE;

   Eina_Bool r = _list_str_equal(_list_str_return_to_check, base_arr_str, base_arr_str_size);
   if (!r) return r;

   char *ele;
   EINA_LIST_FREE(_list_str_return_to_check, ele)
     free(ele);

   _list_str_return_to_check = NULL;
   return r;
}

Eina_List *_test_testing_eina_list_str_return_own(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
   Eina_List *lst = NULL;
   lst = eina_list_append(lst, strdup("0x0"));
   lst = eina_list_append(lst, strdup("0x2A"));
   lst = eina_list_append(lst, strdup("0x42"));
   return lst;
}

// Object

Eina_Bool _list_obj_equal(const Eina_List *lst, const Test_Numberwrapper * const base[], unsigned int len)
{
   if (eina_list_count(lst) != len)
     return EINA_FALSE;

   const Eina_List *l;
   Test_Numberwrapper *eo;
   int i = 0;
   EINA_LIST_FOREACH(lst, l, eo)
     {
        int a = test_numberwrapper_number_get(eo);
        int b = test_numberwrapper_number_get(base[i]);
        if (a != b)
          return EINA_FALSE;
        ++i;
     }
   return EINA_TRUE;
}

Eina_Bool _test_testing_eina_list_obj_in(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Eina_List *lst)
{
   Eina_Bool r = _list_obj_equal(lst, base_arr_obj, base_arr_obj_size);
   return r;
}

static Eina_List *_list_obj_in_own_to_check = NULL;

Eina_Bool _test_testing_eina_list_obj_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Eina_List *lst)
{
   Eina_Bool r = _list_obj_equal(lst, base_arr_obj, base_arr_obj_size);
   if (!r) return r;
   lst = eina_list_append(lst, _new_obj(42));
   lst = eina_list_append(lst, _new_obj(43));
   lst = eina_list_append(lst, _new_obj(33));
   _list_obj_in_own_to_check = lst;
   return r;
}

Eina_Bool _test_testing_check_eina_list_obj_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
   if (!_list_obj_in_own_to_check) return EINA_FALSE;

   Eina_Bool r = _list_obj_equal(_list_obj_in_own_to_check, modified_arr_obj, modified_arr_obj_size);
   if (!r) return r;

   Test_Numberwrapper *ele;
   EINA_LIST_FREE(_list_obj_in_own_to_check, ele)
     efl_unref(ele);

   _list_obj_in_own_to_check = NULL;
   return r;
}

Eina_List *_list_obj_out_to_check = NULL;

Eina_Bool _test_testing_eina_list_obj_out(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Eina_List **lst)
{
   if (!lst) return EINA_FALSE;
   *lst = eina_list_append(*lst, _new_obj(0x0));
   *lst = eina_list_append(*lst, _new_obj(0x2A));
   *lst = eina_list_append(*lst, _new_obj(0x42));
   _list_obj_out_to_check = *lst;
   return EINA_TRUE;
}
Eina_Bool _test_testing_check_eina_list_obj_out(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
   if (!_list_obj_out_to_check) return EINA_FALSE;

   Eina_Bool r = _list_obj_equal(_list_obj_out_to_check, base_arr_obj, base_arr_obj_size);
   if (!r) return r;

   Test_Numberwrapper *ele;
   EINA_LIST_FREE(_list_obj_out_to_check, ele)
     efl_unref(ele);

   _list_obj_out_to_check = NULL;
   return r;
}

Eina_Bool _test_testing_eina_list_obj_out_own(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Eina_List **lst)
{
   if (!lst) return EINA_FALSE;
   *lst = eina_list_append(*lst, _new_obj(0x0));
   *lst = eina_list_append(*lst, _new_obj(0x2A));
   *lst = eina_list_append(*lst, _new_obj(0x42));
   return EINA_TRUE;
}

Eina_List *_list_obj_return_to_check = NULL;

Eina_List *_test_testing_eina_list_obj_return(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
   Eina_List *lst = NULL;
   lst = eina_list_append(lst, _new_obj(0x0));
   lst = eina_list_append(lst, _new_obj(0x2A));
   lst = eina_list_append(lst, _new_obj(0x42));
   _list_obj_return_to_check = lst;
   return lst;
}
Eina_Bool _test_testing_check_eina_list_obj_return(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
   if (!_list_obj_return_to_check) return EINA_FALSE;

   Eina_Bool r = _list_obj_equal(_list_obj_return_to_check, base_arr_obj, base_arr_obj_size);
   if (!r) return r;

   Test_Numberwrapper *ele;
   EINA_LIST_FREE(_list_obj_return_to_check, ele)
     efl_unref(ele);

   _list_obj_return_to_check = NULL;
   return r;
}

Eina_List *_test_testing_eina_list_obj_return_own(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
   Eina_List *lst = NULL;
   lst = eina_list_append(lst, _new_obj(0x0));
   lst = eina_list_append(lst, _new_obj(0x2A));
   lst = eina_list_append(lst, _new_obj(0x42));
   return lst;
}

Eina_List *_test_testing_eina_list_obj_return_in(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Eina_List *lst)
{
   return lst;
}



//      //
// Hash //
//      //

// Integer //

Eina_Bool _hash_int_check(const Eina_Hash *hsh, int key, int expected_val)
{
   int *val = eina_hash_find(hsh, &key);
   return val && (*val == expected_val);
}


// int in

Eina_Bool _test_testing_eina_hash_int_in(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Eina_Hash *hsh)
{
   if (!_hash_int_check(hsh, 22, 222))
     return EINA_FALSE;

   int key = 44;
   return eina_hash_add(hsh, &key, _new_int(444));
}


// int in own

static Eina_Bool _hash_int_in_own_free_flag = EINA_FALSE;
static void _hash_int_in_own_free_cb(void *data)
{
   _hash_int_in_own_free_flag = EINA_TRUE;
   free(data);
}
static Eina_Hash *_hash_int_in_own_to_check = NULL;

Eina_Bool _test_testing_eina_hash_int_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Eina_Hash *hsh)
{
   eina_hash_free_cb_set(hsh, _hash_int_in_own_free_cb);

   if (!_hash_int_check(hsh, 22, 222))
     return EINA_FALSE;

   _hash_int_in_own_to_check = hsh;

   int key = 44;
   return eina_hash_add(hsh, &key, _new_int(444));
}
Eina_Bool _test_testing_check_eina_hash_int_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
   if (!_hash_int_in_own_to_check) return EINA_FALSE;

   Eina_Hash *hsh = _hash_int_in_own_to_check;

   if (!_hash_int_check(hsh, 22, 222)
       || !_hash_int_check(hsh, 44, 444)
       || !_hash_int_check(hsh, 88, 888))
     return EINA_FALSE;

   eina_hash_free(hsh);

   return _hash_int_in_own_free_flag;
}


// int out

static Eina_Bool _hash_int_out_free_flag = EINA_FALSE;
static void _hash_int_out_free_cb(void *data)
{
   _hash_int_out_free_flag = EINA_TRUE;
   free(data);
}
Eina_Hash *_hash_int_out_to_check = NULL;

Eina_Bool _test_testing_eina_hash_int_out(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Eina_Hash **hsh)
{
   if (!hsh) return EINA_FALSE;

   *hsh = eina_hash_int32_new(_hash_int_out_free_cb);

   _hash_int_out_to_check = *hsh;

   int key = 22;
   return eina_hash_add(*hsh, &key, _new_int(222));
}
Eina_Bool _test_testing_check_eina_hash_int_out(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
   if (!_hash_int_out_to_check) return EINA_FALSE;

   Eina_Hash *hsh = _hash_int_out_to_check;

   if (!_hash_int_check(hsh, 22, 222)
       || !_hash_int_check(hsh, 44, 444))
     return EINA_FALSE;

   eina_hash_free(hsh);

   _hash_int_out_to_check = NULL;
   return _hash_int_out_free_flag;
}


// int out own

static Eina_Bool _hash_int_out_own_free_flag = EINA_FALSE;
static void _hash_int_out_own_free_cb(void *data)
{
   _hash_int_out_own_free_flag = EINA_TRUE;
   free(data);
}
Eina_Bool _test_testing_eina_hash_int_out_own(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Eina_Hash **hsh)
{
   if (!hsh) return EINA_FALSE;

   *hsh = eina_hash_int32_new(_hash_int_out_own_free_cb);

   int key = 22;
   return eina_hash_add(*hsh, &key, _new_int(222));
}
Eina_Bool _test_testing_check_eina_hash_int_out_own(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
   return !_hash_int_out_own_free_flag;
}


// int return

static Eina_Bool _hash_int_return_free_flag = EINA_FALSE;
static void _hash_int_return_free_cb(void *data)
{
   _hash_int_return_free_flag = EINA_TRUE;
   free(data);
}
Eina_Hash *_hash_int_return_to_check = NULL;

Eina_Hash *_test_testing_eina_hash_int_return(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
   Eina_Hash *hsh = eina_hash_int32_new(_hash_int_return_free_cb);

   int key = 22;
   eina_hash_add(hsh, &key, _new_int(222));

   _hash_int_return_to_check = hsh;

   return hsh;
}
Eina_Bool _test_testing_check_eina_hash_int_return(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
   if (!_hash_int_return_to_check) return EINA_FALSE;

   Eina_Hash *hsh = _hash_int_return_to_check;

   if (!_hash_int_check(hsh, 22, 222)
       || !_hash_int_check(hsh, 44, 444))
     return EINA_FALSE;

   eina_hash_free(hsh);

   _hash_int_return_to_check = NULL;
   return _hash_int_return_free_flag;
}


// int return own

static Eina_Bool _hash_int_return_own_free_flag = EINA_FALSE;
static void _hash_int_return_own_free_cb(void *data)
{
   _hash_int_return_own_free_flag = EINA_TRUE;
   free(data);
}
Eina_Hash *_test_testing_eina_hash_int_return_own(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
   Eina_Hash *hsh = eina_hash_int32_new(_hash_int_return_own_free_cb);

   int key = 22;
   eina_hash_add(hsh, &key, _new_int(222));

   return hsh;
}
Eina_Bool _test_testing_check_eina_hash_int_return_own(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
   return !_hash_int_return_own_free_flag;
}


// String //

Eina_Bool _hash_str_check(const Eina_Hash *hsh, const char *key, const char *expected_val)
{
   const char *val = eina_hash_find(hsh, key);
   return val && 0 == strcmp(val, expected_val);
}


// str in

Eina_Bool _test_testing_eina_hash_str_in(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Eina_Hash *hsh)
{
   if (!_hash_str_check(hsh, "aa", "aaa"))
     return EINA_FALSE;

   return eina_hash_add(hsh, "bb", strdup("bbb"));
}


// str in own

static Eina_Bool _hash_str_in_own_free_flag = EINA_FALSE;
static void _hash_str_in_own_free_cb(void *data)
{
   _hash_str_in_own_free_flag = EINA_TRUE;
   free(data);
}
static Eina_Hash *_hash_str_in_own_to_check = NULL;

Eina_Bool _test_testing_eina_hash_str_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Eina_Hash *hsh)
{
   eina_hash_free_cb_set(hsh, _hash_str_in_own_free_cb);

   if (!_hash_str_check(hsh, "aa", "aaa"))
     return EINA_FALSE;

   _hash_str_in_own_to_check = hsh;

   return eina_hash_add(hsh, "bb", strdup("bbb"));
}
Eina_Bool _test_testing_check_eina_hash_str_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
   if (!_hash_str_in_own_to_check) return EINA_FALSE;

   Eina_Hash *hsh = _hash_str_in_own_to_check;

   if (!_hash_str_check(hsh, "aa", "aaa")
       || !_hash_str_check(hsh, "bb", "bbb")
       || !_hash_str_check(hsh, "cc", "ccc"))
     return EINA_FALSE;

   eina_hash_free(hsh);

   return _hash_str_in_own_free_flag;
}


// str out

static Eina_Bool _hash_str_out_free_flag = EINA_FALSE;
static void _hash_str_out_free_cb(void *data)
{
   _hash_str_out_free_flag = EINA_TRUE;
   free(data);
}
Eina_Hash *_hash_str_out_to_check = NULL;

Eina_Bool _test_testing_eina_hash_str_out(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Eina_Hash **hsh)
{
   if (!hsh) return EINA_FALSE;

   *hsh = eina_hash_string_superfast_new(_hash_str_out_free_cb);

   _hash_str_out_to_check = *hsh;

   return eina_hash_add(*hsh, "aa", strdup("aaa"));
}
Eina_Bool _test_testing_check_eina_hash_str_out(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
   if (!_hash_str_out_to_check) return EINA_FALSE;

   Eina_Hash *hsh = _hash_str_out_to_check;

   if (!_hash_str_check(hsh, "aa", "aaa")
       || !_hash_str_check(hsh, "bb", "bbb"))
     return EINA_FALSE;

   eina_hash_free(hsh);

   _hash_str_out_to_check = NULL;
   return _hash_str_out_free_flag;
}


// str out own

static Eina_Bool _hash_str_out_own_free_flag = EINA_FALSE;
static void _hash_str_out_own_free_cb(void *data)
{
   _hash_str_out_own_free_flag = EINA_TRUE;
   free(data);
}
Eina_Bool _test_testing_eina_hash_str_out_own(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Eina_Hash **hsh)
{
   if (!hsh) return EINA_FALSE;

   *hsh = eina_hash_string_superfast_new(_hash_str_out_own_free_cb);

   return eina_hash_add(*hsh, "aa", strdup("aaa"));
}
Eina_Bool _test_testing_check_eina_hash_str_out_own(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
   return !_hash_str_out_own_free_flag;
}


// str return

static Eina_Bool _hash_str_return_free_flag = EINA_FALSE;
static void _hash_str_return_free_cb(void *data)
{
   _hash_str_return_free_flag = EINA_TRUE;
   free(data);
}
Eina_Hash *_hash_str_return_to_check = NULL;

Eina_Hash *_test_testing_eina_hash_str_return(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
   Eina_Hash *hsh = eina_hash_string_superfast_new(_hash_str_return_free_cb);

   eina_hash_add(hsh, "aa", strdup("aaa"));

   _hash_str_return_to_check = hsh;

   return hsh;
}
Eina_Bool _test_testing_check_eina_hash_str_return(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
   if (!_hash_str_return_to_check) return EINA_FALSE;

   Eina_Hash *hsh = _hash_str_return_to_check;

   if (!_hash_str_check(hsh, "aa", "aaa")
       || !_hash_str_check(hsh, "bb", "bbb"))
     return EINA_FALSE;

   eina_hash_free(hsh);

   _hash_str_return_to_check = NULL;
   return _hash_str_return_free_flag;
}


// str return own

static Eina_Bool _hash_str_return_own_free_flag = EINA_FALSE;
static void _hash_str_return_own_free_cb(void *data)
{
   _hash_str_return_own_free_flag = EINA_TRUE;
   free(data);
}
Eina_Hash *_test_testing_eina_hash_str_return_own(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
   Eina_Hash *hsh = eina_hash_string_superfast_new(_hash_str_return_own_free_cb);

   eina_hash_add(hsh, "aa", strdup("aaa"));

   return hsh;
}
Eina_Bool _test_testing_check_eina_hash_str_return_own(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
   return !_hash_str_return_own_free_flag;
}


// Object //

Eina_Bool _hash_obj_check(const Eina_Hash *hsh, Test_Numberwrapper *key, Test_Numberwrapper *expected_val, int knum, int vnum)
{
   Test_Numberwrapper *val = eina_hash_find(hsh, &key);
   return val && (val == expected_val) && (test_numberwrapper_number_get(key) == knum) && (test_numberwrapper_number_get(val) == vnum);
}


// obj in

Eina_Bool _test_testing_eina_hash_obj_in(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Eina_Hash *hsh, Test_Numberwrapper *nwk1, Test_Numberwrapper *nwv1, Test_Numberwrapper **nwk2, Test_Numberwrapper **nwv2)
{
   if (!_hash_obj_check(hsh, nwk1, nwv1, 22, 222))
     return EINA_FALSE;

   *nwk2 = _new_obj(44);
   *nwv2 = _new_obj(444);

   return eina_hash_add(hsh, nwk2, *nwv2);
}


// obj in own

static Eina_Bool _hash_obj_in_own_free_flag = EINA_FALSE;
static void _hash_obj_in_own_free_cb(void *data)
{
   _hash_obj_in_own_free_flag = EINA_TRUE;
   efl_unref(data);
}
static Eina_Hash *_hash_obj_in_own_to_check = NULL;

Eina_Bool _test_testing_eina_hash_obj_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Eina_Hash *hsh, Test_Numberwrapper *nwk1, Test_Numberwrapper *nwv1, Test_Numberwrapper **nwk2, Test_Numberwrapper **nwv2)
{
   eina_hash_free_cb_set(hsh, _hash_obj_in_own_free_cb);

   if (!_hash_obj_check(hsh, nwk1, nwv1, 22, 222))
     return EINA_FALSE;

   _hash_obj_in_own_to_check = hsh;

   *nwk2 = _new_obj(44);
   *nwv2 = _new_obj(444);

   return eina_hash_add(hsh, nwk2, *nwv2);
}
Eina_Bool _test_testing_check_eina_hash_obj_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Test_Numberwrapper *nwk1, Test_Numberwrapper *nwv1, Test_Numberwrapper *nwk2, Test_Numberwrapper *nwv2)
{
   if (!_hash_obj_in_own_to_check) return EINA_FALSE;

   Eina_Hash *hsh = _hash_obj_in_own_to_check;

   if (!_hash_obj_check(hsh, nwk1, nwv1, 22, 222)
       || !_hash_obj_check(hsh, nwk2, nwv2, 44, 444))
     return EINA_FALSE;

   eina_hash_free(hsh);

   return _hash_obj_in_own_free_flag;
}


// obj out

static Eina_Bool _hash_obj_out_free_flag = EINA_FALSE;
static void _hash_obj_out_free_cb(void *data)
{
   _hash_obj_out_free_flag = EINA_TRUE;
   efl_unref(data);
}
Eina_Hash *_hash_obj_out_to_check = NULL;

Eina_Bool _test_testing_eina_hash_obj_out(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Eina_Hash **hsh, Test_Numberwrapper **nwk, Test_Numberwrapper **nwv)
{
   if (!hsh) return EINA_FALSE;

   *hsh = eina_hash_pointer_new(_hash_obj_out_free_cb);

   _hash_obj_out_to_check = *hsh;

   *nwk = _new_obj(22);
   *nwv = _new_obj(222);
   return eina_hash_add(*hsh, nwk, *nwv);
}
Eina_Bool _test_testing_check_eina_hash_obj_out(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Test_Numberwrapper *nwk1, Test_Numberwrapper *nwv1, Test_Numberwrapper *nwk2, Test_Numberwrapper *nwv2)
{
   if (!_hash_obj_out_to_check) return EINA_FALSE;

   Eina_Hash *hsh = _hash_obj_out_to_check;

   if (!_hash_obj_check(hsh, nwk1, nwv1, 22, 222)
       || !_hash_obj_check(hsh, nwk2, nwv2, 44, 444))
     return EINA_FALSE;

   eina_hash_free(hsh);

   _hash_obj_out_to_check = NULL;
   return _hash_obj_out_free_flag;
}


// obj out own

static Eina_Bool _hash_obj_out_own_free_flag = EINA_FALSE;
static void _hash_obj_out_own_free_cb(void *data)
{
   _hash_obj_out_own_free_flag = EINA_TRUE;
   efl_unref(data);
}
Eina_Bool _test_testing_eina_hash_obj_out_own(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Eina_Hash **hsh, Test_Numberwrapper **nwk, Test_Numberwrapper **nwv)
{
   if (!hsh) return EINA_FALSE;

   *hsh = eina_hash_pointer_new(_hash_obj_out_own_free_cb);

   *nwk = _new_obj(22);
   *nwv = _new_obj(222);
   return eina_hash_add(*hsh, nwk, *nwv);
}
Eina_Bool _test_testing_check_eina_hash_obj_out_own(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
   return !_hash_obj_out_own_free_flag;
}


// obj return

static Eina_Bool _hash_obj_return_free_flag = EINA_FALSE;
static void _hash_obj_return_free_cb(void *data)
{
   _hash_obj_return_free_flag = EINA_TRUE;
   efl_unref(data);
}
Eina_Hash *_hash_obj_return_to_check = NULL;

Eina_Hash *_test_testing_eina_hash_obj_return(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Test_Numberwrapper **nwk, Test_Numberwrapper **nwv)
{
   Eina_Hash *hsh = eina_hash_pointer_new(_hash_obj_return_free_cb);

   *nwk = _new_obj(22);
   *nwv = _new_obj(222);

   eina_hash_add(hsh, nwk, *nwv);

   _hash_obj_return_to_check = hsh;

   return hsh;
}
Eina_Bool _test_testing_check_eina_hash_obj_return(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Test_Numberwrapper *nwk1, Test_Numberwrapper *nwv1, Test_Numberwrapper *nwk2, Test_Numberwrapper *nwv2)
{
   if (!_hash_obj_return_to_check) return EINA_FALSE;

   Eina_Hash *hsh = _hash_obj_return_to_check;

   if (!_hash_obj_check(hsh, nwk1, nwv1, 22, 222)
       || !_hash_obj_check(hsh, nwk2, nwv2, 44, 444))
     return EINA_FALSE;

   eina_hash_free(hsh);

   _hash_obj_return_to_check = NULL;
   return _hash_obj_return_free_flag;
}


// obj return own

static Eina_Bool _hash_obj_return_own_free_flag = EINA_FALSE;
static void _hash_obj_return_own_free_cb(void *data)
{
   _hash_obj_return_own_free_flag = EINA_TRUE;
   efl_unref(data);
}
Eina_Hash *_test_testing_eina_hash_obj_return_own(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Test_Numberwrapper **nwk, Test_Numberwrapper **nwv)
{
   Eina_Hash *hsh = eina_hash_pointer_new(_hash_obj_return_own_free_cb);

   *nwk = _new_obj(22);
   *nwv = _new_obj(222);
   eina_hash_add(hsh, nwk, *nwv);

   return hsh;
}
Eina_Bool _test_testing_check_eina_hash_obj_return_own(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
   return !_hash_obj_return_own_free_flag;
}


//                   //
// Class constructor
//                   //
EOLIAN static void
_test_testing_class_constructor(Efl_Class *klass)
{
   (void)klass;
   modified_arr_obj[0] = base_arr_obj[0] = _new_obj(0x0);
   modified_arr_obj[1] = base_arr_obj[1] = _new_obj(0x2A);
   modified_arr_obj[2] = base_arr_obj[2] = _new_obj(0x42);
   modified_arr_obj[3] = _new_obj(42);
   modified_arr_obj[4] = _new_obj(43);
   modified_arr_obj[5] = _new_obj(33);
}

EOLIAN static void
_test_testing_class_destructor(Efl_Class *klass)
{
   (void)klass;
   for (unsigned i = 0; i < base_arr_obj_size; ++i)
     efl_unref(base_arr_obj[i]);
   for (unsigned i = 0; i < modified_arr_obj_size; ++i)
     efl_unref(modified_arr_obj[i]);
}


// ################## //
// Test.Numberwrapper //
// ################## //


void _test_numberwrapper_number_set(EINA_UNUSED Eo *obj, Test_Numberwrapper_Data *pd, int n)
{
   pd->number = n;
}

int _test_numberwrapper_number_get(EINA_UNUSED Eo *obj, Test_Numberwrapper_Data *pd)
{
   return pd->number;
}

void _test_testing_set_callback(EINA_UNUSED Eo *obj, Test_Testing_Data *pd, SimpleCb cb, void *cb_data, Eina_Free_Cb cb_free_cb)
{
   if (pd->free_cb)
      pd->free_cb(pd->cb_data);

   pd->cb = cb;
   pd->cb_data = cb_data;
   pd->free_cb = cb_free_cb;
}

int _test_testing_call_callback(EINA_UNUSED Eo *obj, Test_Testing_Data *pd, int a)
{
   if (!pd->cb)
     {
       EINA_LOG_ERR("Trying to call with no callback set");
       return -1; // FIXME Maybe use Eina error when exceptions are supported?
     }

   return pd->cb(pd->cb_data, a);
}

// Global var used due to the current issue of calling methods from the GC thread
static Eina_Bool _free_called = EINA_FALSE;

EAPI Eina_Bool free_called_get() {
   return _free_called;
}

EAPI void free_called_set(Eina_Bool b) {
   _free_called = b;
}

static void _free_callback(void *data)
{
   Eo *obj = data;

   free_called_set(EINA_TRUE);
   efl_unref(obj);
}

int _wrapper_cb(EINA_UNUSED void *data, int a)
{
    return a * 3;
}

void _test_testing_call_set_callback(Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
   test_testing_set_callback(obj, _wrapper_cb, efl_ref(obj), _free_callback);
}

void _test_testing_raises_eina_error(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
   eina_error_set(EIO);
}

void _test_testing_children_raise_error(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
    // Native method shouldn't throw any error. Children must raise it.
}

void _test_testing_call_children_raise_error(Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
    test_testing_children_raise_error(obj);
}

void _test_testing_error_ret_set(EINA_UNUSED Eo *obj, Test_Testing_Data *pd, Eina_Error error)
{
    pd->error_code = error;
}

Eina_Error _test_testing_returns_error(EINA_UNUSED Eo *obj, Test_Testing_Data *pd)
{
   return pd->error_code;
}

#include "test_testing.eo.c"
#include "test_numberwrapper.eo.c"

