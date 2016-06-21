#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Efl.h>

static Efl_Config *_main_config = NULL;

EAPI void
__efl_internal_elm_config_set(Efl_Config *cfg)
{
   _main_config = cfg;
}

EOLIAN static Eina_Bool
_efl_config_config_bool_set(Eo *obj, void *_pd EINA_UNUSED, const char * name, Eina_Bool val)
{
   Eina_Value *v = eina_value_new(EINA_VALUE_TYPE_UCHAR);
   Eina_Bool b;
   eina_value_set(v, val);
   b = efl_config_set(obj, name, v);
   eina_value_free(v);
   return b;
}

EOLIAN static Eina_Bool
_efl_config_config_bool_get(Eo *obj, void *_pd EINA_UNUSED, const char * name)
{
   Eina_Value *v = efl_config_get(obj, name);
   Eina_Bool b = 0;
   eina_value_get(v, &b);
   eina_value_free(v);
   return b;
}

EOLIAN static Eina_Bool
_efl_config_config_int_set(Eo *obj, void *_pd EINA_UNUSED, const char * name, int val)
{
   Eina_Value *v = eina_value_new(EINA_VALUE_TYPE_INT);
   Eina_Bool b;
   eina_value_set(v, val);
   b = efl_config_set(obj, name, v);
   eina_value_free(v);
   return b;
}

EOLIAN static int
_efl_config_config_int_get(Eo *obj, void *_pd EINA_UNUSED, const char * name)
{
   Eina_Value *v = efl_config_get(obj, name);
   int b = 0;
   eina_value_get(v, &b);
   eina_value_free(v);
   return b;
}

EOLIAN static Eina_Bool
_efl_config_config_uint_set(Eo *obj, void *_pd EINA_UNUSED, const char * name, unsigned int val)
{
   Eina_Value *v = eina_value_new(EINA_VALUE_TYPE_UINT);
   Eina_Bool b;
   eina_value_set(v, val);
   b = efl_config_set(obj, name, v);
   eina_value_free(v);
   return b;
}

EOLIAN static unsigned int
_efl_config_config_uint_get(Eo *obj, void *_pd EINA_UNUSED, const char * name)
{
   Eina_Value *v = efl_config_get(obj, name);
   unsigned int b = 0;
   eina_value_get(v, &b);
   eina_value_free(v);
   return b;
}

EOLIAN static Eina_Bool
_efl_config_config_double_set(Eo *obj, void *_pd EINA_UNUSED, const char * name, double val)
{
   Eina_Value *v = eina_value_new(EINA_VALUE_TYPE_DOUBLE);
   Eina_Bool b;
   eina_value_set(v, val);
   b = efl_config_set(obj, name, v);
   eina_value_free(v);
   return b;
}

EOLIAN static double
_efl_config_config_double_get(Eo *obj, void *_pd EINA_UNUSED, const char * name)
{
   Eina_Value *v = efl_config_get(obj, name);
   double b = 0;
   eina_value_get(v, &b);
   eina_value_free(v);
   return b;
}

EOLIAN static Eina_Bool
_efl_config_config_string_set(Eo *obj, void *_pd EINA_UNUSED, const char *name, const char *val)
{
   Eina_Value *v = eina_value_new(EINA_VALUE_TYPE_STRING);
   Eina_Bool b;
   eina_value_set(v, val);
   b = efl_config_set(obj, name, v);
   eina_value_free(v);
   return b;
}

EOLIAN static const char *
_efl_config_config_string_get(Eo *obj, void *_pd EINA_UNUSED, const char *name)
{
   Eina_Value *v = efl_config_get(obj, name);
   Eina_Stringshare *s;
   const char *b = 0;
   eina_value_get(v, &b);
   s = eina_stringshare_add(b);
   eina_value_free(v);
   return s;
}

EOLIAN static Efl_Config *
_efl_config_config_global_get(Eo *klass EINA_UNUSED, void *_pd EINA_UNUSED)
{
   return _main_config;
}

#include "interfaces/efl_config.eo.c"
