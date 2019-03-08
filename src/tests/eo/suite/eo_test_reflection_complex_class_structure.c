#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>

#include <Eo.h>

#include "eo_suite.h"
#include "eo_test_class_simple.h"
#include "eo_test_reflection_complex_class_structure.h"

typedef struct {
   int i;
} Complex_Class_Data;

typedef struct {
   int i;
} Complex_Mixin_Data;

static void
_complex_class_complex_interface_i_test_set(Eo *obj EINA_UNUSED, Complex_Class_Data *pd, int i)
{
   pd->i = i;
}

static int
_complex_class_complex_interface_i_test_get(const Eo *obj EINA_UNUSED, Complex_Class_Data *pd)
{
   return pd->i;
}

static int
_complex_mixin_m_test_get(const Eo *obj EINA_UNUSED, Complex_Mixin_Data *pd)
{
   return pd->i;
}

static void
_complex_mixin_m_test_set(Eo *obj EINA_UNUSED, Complex_Mixin_Data *pd, int i)
{
   pd->i = i;
}

static Eina_Bool
_complex_class_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef COMPLEX_CLASS_EXTRA_OPS
#define COMPLEX_CLASS_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(complex_interface_i_test_set, _complex_class_complex_interface_i_test_set),
      EFL_OBJECT_OP_FUNC(complex_interface_i_test_get, _complex_class_complex_interface_i_test_get),
      COMPLEX_CLASS_EXTRA_OPS
   );
   opsp = &ops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _complex_class_class_desc = {
   EO_VERSION,
   "Complex_Class",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Complex_Class_Data),
   _complex_class_class_initializer,
   NULL,
   NULL
};

EFL_DEFINE_CLASS(complex_class_class_get, &_complex_class_class_desc, EO_CLASS, COMPLEX_INTERFACE_INTERFACE, COMPLEX_MIXIN_MIXIN, NULL);

static Eina_Error
__eolian_complex_interface_i_test_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   int cval;
   if (!eina_value_int_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   complex_interface_i_test_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(complex_interface_i_test_set, EFL_FUNC_CALL(i), int i);

static Eina_Value
__eolian_complex_interface_i_test_get_reflect(Eo *obj)
{
   int val = complex_interface_i_test_get(obj);
   return eina_value_int_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(complex_interface_i_test_get, int, 0);

static Eina_Bool
_complex_interface_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef COMPLEX_INTERFACE_EXTRA_OPS
#define COMPLEX_INTERFACE_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(complex_interface_i_test_set, NULL),
      EFL_OBJECT_OP_FUNC(complex_interface_i_test_get, NULL),
      COMPLEX_INTERFACE_EXTRA_OPS
   );
   opsp = &ops;

   static const Efl_Object_Property_Reflection refl_table[] = {
      {"i_test", __eolian_complex_interface_i_test_set_reflect, __eolian_complex_interface_i_test_get_reflect},
   };
   static const Efl_Object_Property_Reflection_Ops rops = {
      refl_table, EINA_C_ARRAY_LENGTH(refl_table)
   };
   ropsp = &rops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _complex_interface_class_desc = {
   EO_VERSION,
   "Complex_Interface",
   EFL_CLASS_TYPE_INTERFACE,
   0,
   _complex_interface_class_initializer,
   NULL,
   NULL
};

EFL_DEFINE_CLASS(complex_interface_interface_get, &_complex_interface_class_desc, NULL, NULL);

static Eina_Error
__eolian_complex_mixin_m_test_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   int cval;
   if (!eina_value_int_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   complex_mixin_m_test_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(complex_mixin_m_test_set, EFL_FUNC_CALL(i), int i);


static Eina_Value
__eolian_complex_mixin_m_test_get_reflect(Eo *obj)
{
   int val = complex_mixin_m_test_get(obj);
   return eina_value_int_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(complex_mixin_m_test_get, int, 0);

static Eina_Bool
_complex_mixin_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef COMPLEX_MIXIN_EXTRA_OPS
#define COMPLEX_MIXIN_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(complex_mixin_m_test_set, _complex_mixin_m_test_set),
      EFL_OBJECT_OP_FUNC(complex_mixin_m_test_get, _complex_mixin_m_test_get),
      COMPLEX_MIXIN_EXTRA_OPS
   );
   opsp = &ops;

   static const Efl_Object_Property_Reflection refl_table[] = {
      {"m_test", __eolian_complex_mixin_m_test_set_reflect, __eolian_complex_mixin_m_test_get_reflect},
   };
   static const Efl_Object_Property_Reflection_Ops rops = {
      refl_table, EINA_C_ARRAY_LENGTH(refl_table)
   };
   ropsp = &rops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _complex_mixin_class_desc = {
   EO_VERSION,
   "Complex_Mixin",
   EFL_CLASS_TYPE_MIXIN,
   sizeof(Complex_Mixin_Data),
   _complex_mixin_class_initializer,
   NULL,
   NULL
};

EFL_DEFINE_CLASS(complex_mixin_mixin_get, &_complex_mixin_class_desc, NULL, NULL);
