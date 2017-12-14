#include <Eina.h>

/**
  @brief Helpers for manipulating eina_array of stringshared strings.
*/

typedef struct _Eina_Array_Foreach_Closure_Data
{
  const char *searched_string;
  Eina_Bool *return_value;
} Eina_Array_Foreach_Closure_Data;

static void
_eina_array_string_push(Eina_Array *array, const char *string)
{
  EINA_SAFETY_ON_NULL_RETURN(string);
  eina_array_push(array, eina_stringshare_add(string));
}

static Eina_Bool
_eina_stringshare_foreach(const void *container EINA_UNUSED, void *data, void *gdata)
{
  Eina_Array_Foreach_Closure_Data *closure = gdata;
  const char *str = data;
  
  if (str && closure->searched_string && !strcmp(str, closure->searched_string))
    *closure->return_value = EINA_TRUE;

  return EINA_TRUE;
}

static Eina_Bool
_eina_array_string_contains(Eina_Array *array, const char *string)
{
  Eina_Bool found = EINA_FALSE;
  Eina_Array_Foreach_Closure_Data closure = { .searched_string = string, .return_value = &found };
  return eina_array_foreach(array, _eina_stringshare_foreach, &closure) && found;
}

static Eina_Bool
_eina_stringshare_remove(void *data, void *gdata)
{
  const char *string = data;
  const char *string_to_remove = gdata;

  if (string && string_to_remove && !strcmp(string, string_to_remove))
    return EINA_FALSE;

  return EINA_TRUE;
}

static Eina_Bool
_eina_array_string_remove(Eina_Array *array, const char *string)
{
  EINA_SAFETY_ON_NULL_RETURN_VAL(string, EINA_FALSE);
  return eina_array_remove(array, _eina_stringshare_remove, (void*)string);
}

static Eina_Bool
_eina_stringshare_del_stringshare(const void *container EINA_UNUSED, void *data, void *gdata EINA_UNUSED)
{
  const char *string = data;
  eina_stringshare_del(string);
  return EINA_TRUE;
}

static void
_eina_array_strings_clear(Eina_Array *array)
{
  eina_array_foreach(array, _eina_stringshare_del_stringshare, NULL);
}