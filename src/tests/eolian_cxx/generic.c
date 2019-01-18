#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eo.h>
#include <Ecore.h>

#define GENERIC_BETA
#define GENERIC_PROTECTED

#include "generic.eo.h"
#include "generic_interface.eo.h"

#include <check.h>

struct _Generic_Data
{
   int        req_ctor_a_val;
   int        req_ctor_b_val;
   int        opt_ctor_a_val;
   int        opt_ctor_b_val;
};
typedef struct _Generic_Data Generic_Data;

#define MY_CLASS GENERIC_CLASS

static Eo *_generic_efl_object_constructor(Eo *obj, Generic_Data *pd)
{
   pd->req_ctor_a_val = 0;
   pd->opt_ctor_a_val = 0;
   return efl_constructor(efl_super(obj, MY_CLASS));
}

static void _generic_required_ctor_a(Eo *obj EINA_UNUSED, Generic_Data *pd, int value)
{
   pd->req_ctor_a_val = value;
}

static void _generic_required_ctor_b(Eo *obj EINA_UNUSED, Generic_Data *pd EINA_UNUSED, int value)
{
   pd->req_ctor_b_val = value;
}

static void _generic_optional_ctor_a(Eo *obj EINA_UNUSED, Generic_Data *pd, int value)
{
   pd->opt_ctor_a_val = value;
}

static void _generic_optional_ctor_b(Eo *obj EINA_UNUSED, Generic_Data *pd EINA_UNUSED, int value)
{
   pd->opt_ctor_b_val = value;
}

static int _generic_req_ctor_a_value_get(const Eo *obj EINA_UNUSED, Generic_Data *pd)
{
   return pd->req_ctor_a_val;
}

static int _generic_req_ctor_b_value_get(const Eo *obj EINA_UNUSED, Generic_Data *pd)
{
   return pd->req_ctor_b_val;
}

static int _generic_opt_ctor_a_value_get(const Eo *obj EINA_UNUSED, Generic_Data *pd)
{
   return pd->opt_ctor_a_val;
}

static int _generic_opt_ctor_b_value_get(const Eo *obj EINA_UNUSED, Generic_Data *pd)
{
   return pd->opt_ctor_b_val;
}

/* static void _generic_req_ctor_a_value_set(Eo *obj EINA_UNUSED, Generic_Data* pd EINA_UNUSED, int value EINA_UNUSED) */
/* { */
/* } */

/* static void _generic_opt_ctor_a_value_set(Eo *obj EINA_UNUSED, Generic_Data* pd EINA_UNUSED, int value EINA_UNUSED) */
/* { */
/* } */

static void _generic_out_required_ctor_a(Eo *obj EINA_UNUSED, Generic_Data* pd, int *value)
{
   *value = pd->req_ctor_a_val;
}

static void _generic_out_required_ctor_b(Eo *obj EINA_UNUSED, Generic_Data* pd, int *value)
{
   *value = pd->req_ctor_b_val;
}

static void _generic_out_optional_ctor_a(Eo *obj EINA_UNUSED, Generic_Data* pd, int *value)
{
   *value = pd->opt_ctor_a_val;
}

static void _generic_out_optional_ctor_b(Eo *obj EINA_UNUSED, Generic_Data* pd, int *value)
{
   *value = pd->opt_ctor_b_val;
}

static void _generic_call_event1(Eo *obj, Generic_Data* pd EINA_UNUSED)
{
  efl_event_callback_call(obj, GENERIC_EVENT_PREFIX_EVENT1, NULL);
}
static void _generic_call_event2(Eo *obj, Generic_Data* pd EINA_UNUSED)
{
  efl_event_callback_call(obj, GENERIC_EVENT_PREFIX_EVENT2, obj);
}
static void _generic_call_event3(Eo *obj, Generic_Data* pd EINA_UNUSED)
{
  int p = 42;
  efl_event_callback_call(obj, GENERIC_EVENT_PREFIX_EVENT3, &p);
}
static void _generic_call_event4(Eo *obj, Generic_Data* pd EINA_UNUSED)
{
  int i = 42;
  Eina_List* p = eina_list_append(NULL, &i);
  ck_assert(p != NULL);
  efl_event_callback_call(obj, GENERIC_EVENT_PREFIX_EVENT4, p);
  eina_list_free(p);
}
static void _generic_call_event5(Eo *obj, Generic_Data* pd EINA_UNUSED)
{
  int i = 42;
  Eina_List* p = eina_list_append(NULL, &i);

  Generic_Event e = {.field1 = 42, .field2 = p};
  efl_event_callback_call(obj, GENERIC_EVENT_PREFIX_EVENT5, &e);
  eina_list_free(p);
}
static void _generic_protected_method1(Eo *obj EINA_UNUSED, Generic_Data* pd EINA_UNUSED)
{
}
static void _generic_protected_beta_method1(Eo *obj EINA_UNUSED, Generic_Data* pd EINA_UNUSED)
{
}
static void _generic_beta_method1(Eo *obj EINA_UNUSED, Generic_Data* pd EINA_UNUSED)
{
}
#include "generic.eo.c"
#include "generic_interface.eo.c"
