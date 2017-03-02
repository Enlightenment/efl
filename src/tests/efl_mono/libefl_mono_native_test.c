
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

#include "test_testing.eo.c"

