
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
  return strcpy((char*)ret, str);
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

const char *_test_testing_call_in_string(Eo *obj, EINA_UNUSED Test_Testing_Data *pd, const char *str)
{
  return test_testing_in_string(obj, str);
}

const char *_test_testing_call_in_own_string(Eo *obj, EINA_UNUSED Test_Testing_Data *pd, const char *str)
{
  return test_testing_in_own_string(obj, str);
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
Eina_Stringshare *_test_testing_call_in_stringshare(Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Eina_Stringshare *str)
{
  return test_testing_in_stringshare(obj, str);
}

Eina_Stringshare *_test_testing_call_in_own_stringshare(Eo *obj, EINA_UNUSED Test_Testing_Data *pd, Eina_Stringshare *str)
{
  return test_testing_in_own_stringshare(obj, str);
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
  if (slice) return EINA_FALSE;
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

#include "test_testing.eo.c"

