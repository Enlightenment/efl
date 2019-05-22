#include "libefl_mono_native_test.h"

typedef struct Dummy_Child_Data
{
  int iface_prop;
  const char* a;
  double b;
  Eina_Bool iface_was_set;
  Eina_Bool obligatory_beta_ctor_was_called;
  Eina_Bool optional_beta_ctor_was_called;
} Dummy_Child_Data;

/// Dummy.Child

static Efl_Object *
_dummy_child_efl_object_constructor(Eo *obj, Dummy_Child_Data *pd)
{
    efl_constructor(efl_super(obj, DUMMY_CHILD_CLASS));

    pd->iface_prop = 1984;
    pd->iface_was_set = EINA_FALSE;
    pd->obligatory_beta_ctor_was_called = EINA_FALSE;
    pd->optional_beta_ctor_was_called = EINA_FALSE;
    return obj;
}

void _dummy_child_dummy_test_iface_iface_prop_set(EINA_UNUSED Eo *obj, Dummy_Child_Data *pd, int value)
{
    pd->iface_prop = value;
    pd->iface_was_set = EINA_TRUE;
}

int _dummy_child_dummy_test_iface_iface_prop_get(EINA_UNUSED const Eo *obj, Dummy_Child_Data *pd)
{
    return pd->iface_prop;
}

void _dummy_child_double_params(EINA_UNUSED Eo* obj, Dummy_Child_Data *pd, const char* a, double b)
{
    if (pd->a)
      free((void*)pd->a);
    pd->a = malloc(sizeof(char)*(strlen(a) + 1));
    strcpy((char*)pd->a, a);

    pd->b = b;
}

Eina_Bool _dummy_child_iface_was_set_get(EINA_UNUSED const Eo* obj, Dummy_Child_Data *pd)
{
    return pd->iface_was_set;
}

void _dummy_child_obligatory_beta_ctor(EINA_UNUSED Eo* obj, Dummy_Child_Data *pd, EINA_UNUSED int a)
{
    pd->obligatory_beta_ctor_was_called = EINA_TRUE;
}

void _dummy_child_optional_beta_ctor(EINA_UNUSED Eo* obj, Dummy_Child_Data *pd, EINA_UNUSED int a)
{
    pd->optional_beta_ctor_was_called = EINA_TRUE;
}

Eina_Bool _dummy_child_obligatory_beta_ctor_was_called_get(EINA_UNUSED const Eo* obj, Dummy_Child_Data *pd)
{
    return pd->obligatory_beta_ctor_was_called;
}

Eina_Bool _dummy_child_optional_beta_ctor_was_called_get(EINA_UNUSED const Eo* obj, Dummy_Child_Data *pd)
{
    return pd->optional_beta_ctor_was_called;
}

EOLIAN static void
_dummy_child_class_constructor(Efl_Class *klass)
{
    (void)klass;
}

EOLIAN static void
_dummy_child_class_destructor(Efl_Class *klass)
{
    (void)klass;
}

#include "dummy_child.eo.c"

