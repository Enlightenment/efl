#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eo.h>
#include <Ecore.h>

#define GENERIC_BETA
#define GENERIC_PROTECTED

#define EOLIANCXXTEST_API
#define EOLIANCXXTEST_API_WEAK

#include "generic.eo.h"
#include "generic_interface.eo.h"

#include <check.h>

struct _Generic_Data
{
   int        req_ctor_a_val;
   int        req_ctor_b_val;
   int        opt_ctor_a_val;
   int        opt_ctor_b_val;
   /* What the last array handed in held, so a caller can ask whether what it
      sent is what arrived. */
   char       seen[256];
   /* Handed out by string_array_out, which does not pass ownership: the
      object keeps it, and the strings in it are literals. */
   Eina_Array *kept;
};
typedef struct _Generic_Data Generic_Data;

#define MY_CLASS GENERIC_CLASS

static Eo *_generic_efl_object_constructor(Eo *obj, Generic_Data *pd)
{
   pd->req_ctor_a_val = 0;
   pd->opt_ctor_a_val = 0;
   pd->seen[0] = '\0';
   pd->kept = NULL;
   return efl_constructor(efl_super(obj, MY_CLASS));
}

static void _generic_efl_object_destructor(Eo *obj, Generic_Data *pd)
{
   /* Only the array: what it holds are string literals. */
   if (pd->kept) eina_array_free(pd->kept);
   efl_destructor(efl_super(obj, MY_CLASS));
}

/* Records the contents so a test can check that what it sent is what arrived,
   rather than only that the call went through. */
static void _generic_record(Generic_Data *pd, Eina_Array *l)
{
   unsigned int i;
   Eina_Array_Iterator it;
   void *item;

   pd->seen[0] = '\0';
   EINA_ARRAY_ITER_NEXT(l, i, item, it)
     {
        if (i) strncat(pd->seen, ",", sizeof(pd->seen) - strlen(pd->seen) - 1);
        strncat(pd->seen, item ? (const char *)item : "(null)",
                sizeof(pd->seen) - strlen(pd->seen) - 1);
     }
}

static Eina_Array *_generic_make_owned(void)
{
   Eina_Array *a = eina_array_new(2);
   eina_array_push(a, strdup("given-one"));
   eina_array_push(a, strdup("given-two"));
   return a;
}

static void _generic_string_array_in(Eo *obj EINA_UNUSED, Generic_Data *pd, Eina_Array *l)
{
   _generic_record(pd, l);
}

static void _generic_string_array_in_own(Eo *obj EINA_UNUSED, Generic_Data *pd, Eina_Array *l)
{
   unsigned int i;
   Eina_Array_Iterator it;
   void *item;

   _generic_record(pd, l);

   /* @move: the strings and the array are the callee's now. */
   EINA_ARRAY_ITER_NEXT(l, i, item, it)
     free(item);
   eina_array_free(l);
}

static void _generic_string_array_out(Eo *obj EINA_UNUSED, Generic_Data *pd, Eina_Array **l)
{
   if (!pd->kept)
     {
        pd->kept = eina_array_new(2);
        eina_array_push(pd->kept, "kept-one");
        eina_array_push(pd->kept, "kept-two");
     }
   *l = pd->kept;
}

static void _generic_string_array_out_own(Eo *obj EINA_UNUSED, Generic_Data *pd EINA_UNUSED, Eina_Array **l)
{
   *l = _generic_make_owned();
}

static Eina_Array *_generic_string_array_return_own(Eo *obj EINA_UNUSED, Generic_Data *pd EINA_UNUSED)
{
   return _generic_make_owned();
}

static const char *_generic_string_array_seen_get(const Eo *obj EINA_UNUSED, Generic_Data *pd)
{
   return pd->seen;
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
  static const char *s = "42";
  Eina_Array* p = eina_array_new(1);
  ck_assert(eina_array_push(p, s));
  efl_event_callback_call(obj, GENERIC_EVENT_PREFIX_EVENT4, p);
  eina_array_free(p);
}
static void _generic_call_event5(Eo *obj, Generic_Data* pd EINA_UNUSED)
{
  const char *s = "42";
  Eina_List* p = eina_list_append(NULL, s);

  Generic_Beta_Event e = {.field1 = 42, .field2 = p};
  efl_event_callback_call(obj, GENERIC_EVENT_PREFIX_EVENT5, &e);
  eina_list_free(p);
}
static void _generic_call_event6(Eo *obj, Generic_Data* pd EINA_UNUSED)
{
  const char *s = "42";
  Eina_Array* p = eina_array_new(1);

  eina_array_push(p, s);

  Generic_Event e = {.field1 = 42, .field2 = p};
  efl_event_callback_call(obj, GENERIC_EVENT_PREFIX_EVENT6, &e);
  eina_array_free(p);
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
void _generic_event_param(Eo *obj EINA_UNUSED, Generic_Data *pd EINA_UNUSED, Efl_Event *value EINA_UNUSED)
{
}
void _generic_const_event_param(Eo *obj EINA_UNUSED, Generic_Data *pd EINA_UNUSED, const Efl_Event *value EINA_UNUSED)
{
}
void _generic_binbuf_param(Eo *obj EINA_UNUSED, Generic_Data *pd EINA_UNUSED, Eina_Binbuf *value EINA_UNUSED)
{
}
void _generic_const_binbuf_param(Eo *obj EINA_UNUSED, Generic_Data *pd EINA_UNUSED, const Eina_Binbuf *value EINA_UNUSED)
{
}



#include "generic.eo.c"
#include "generic_interface.eo.c"
