
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

typedef struct Test_Testing_Data
{
} Test_Testing_Data;

#include "test_testing.eo.h"

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

// //
// Array
//

static const int base_arr_int[] = {0x0,0x2A,0x42};
static const unsigned int base_arr_int_size = 3;

Eina_Bool _array_int_equal(const Eina_Array *arr, const int *base, unsigned int len)
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

void *_new_int(int v)
{
   int *r = malloc(sizeof(int));
   *r = v;
   return r;
}

Eina_Bool _test_testing_eina_array_int_in(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Eina_Array *arr)
{
   Eina_Bool r = _array_int_equal(arr, base_arr_int, base_arr_int_size);
   eina_array_push(arr, _new_int(42));
   eina_array_push(arr, _new_int(43));
   eina_array_push(arr, _new_int(33));
   return r;
}

static Eina_Array *_array_in_own_to_check = NULL;

Eina_Bool _test_testing_eina_array_int_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Eina_Array *arr)
{
   Eina_Bool r = _array_int_equal(arr, base_arr_int, base_arr_int_size);
   eina_array_push(arr, _new_int(42));
   eina_array_push(arr, _new_int(43));
   eina_array_push(arr, _new_int(33));
   _array_in_own_to_check = arr;
   return r;
}


Eina_Bool _test_testing_check_eina_array_int_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
    if (!_array_in_own_to_check) return EINA_FALSE;

    const int mod_arr[] = {0x0,0x2A,0x42,42,43,33};
    Eina_Bool r = _array_int_equal(_array_in_own_to_check, mod_arr, 6);

    unsigned int i;
    int *ele;
    Eina_Array_Iterator it;
    EINA_ARRAY_ITER_NEXT(_array_in_own_to_check, i, ele, it)
      free(ele);

    eina_array_free(_array_in_own_to_check);
    return r;
}

Eina_Bool _test_testing_eina_array_int_out(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Eina_Array **arr)
{
   (void) arr;
   return EINA_FALSE;
}

Eina_Bool _test_testing_eina_array_int_out_own(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Eina_Array **arr)
{
   (void) arr;
   return EINA_FALSE;
}

Eina_Array *_test_testing_eina_array_int_return(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
   Eina_Array *arr = NULL;
   return arr;
}

Eina_Array *_test_testing_eina_array_int_return_own(EINA_UNUSED Eo *obj, EINA_UNUSED Test_Testing_Data *pd)
{
   Eina_Array *arr = NULL;
   return arr;
}

#include "test_testing.eo.c"

