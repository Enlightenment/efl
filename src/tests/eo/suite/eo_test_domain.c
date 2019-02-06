#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eina.h"
#include "Eo.h"
#include "eo_test_domain.h"

#define MY_CLASS DOMAIN_CLASS

EAPI const Efl_Event_Description _EV_DOMAIN_A_CHANGED =
        EFL_EVENT_DESCRIPTION("domain,a,changed");

static void
_a_set(Eo *obj EINA_UNUSED, void *class_data, int a)
{
   Domain_Public_Data *pd = class_data;
   printf("Set Begin\n");
   pd->a = a;
   printf("Set End\n");
   printf("Call Events\n");
   efl_event_callback_legacy_call(obj, EV_DOMAIN_A_CHANGED, &pd->a);
   printf("Call Events End\n");
}

static int
_a_get(Eo *obj EINA_UNUSED, void *class_data)
{
   Domain_Public_Data *pd = class_data;
   printf("Get Begin\n");
   return pd->a;
}

//return obj = efl_add_ref(DOMAIN_CLASS, NULL);

EFL_VOID_FUNC_BODYV(domain_recursive, EFL_FUNC_CALL(n), int n);

static void
_recursive(Eo *obj, void *class_data EINA_UNUSED, int n)
{
   static int count = 0;

   if (count < n)
     {
        count++;
        domain_recursive(obj, n);
     }
   else
     count = 0;
}

EFL_VOID_FUNC_BODYV(domain_a_set, EFL_FUNC_CALL(a), int a);
EFL_FUNC_BODY(domain_a_get, int, 0);
EFL_FUNC_BODY(domain_a_print, Eina_Bool, EINA_FALSE);
EFL_FUNC_BODY_CONST(domain_class_hi_print, Eina_Bool, EINA_FALSE);
EFL_VOID_FUNC_BODY(domain_pure_virtual);
EFL_VOID_FUNC_BODY(domain_no_implementation);

static Eina_Bool
_class_initializer(Efl_Class *klass)
{
   EFL_OPS_DEFINE(ops,
         EFL_OBJECT_OP_FUNC(domain_a_set, _a_set),
         EFL_OBJECT_OP_FUNC(domain_a_get, _a_get),
         EFL_OBJECT_OP_FUNC(domain_recursive, _recursive)
   );
   return efl_class_functions_set(klass, &ops, NULL, NULL);
}

static const Efl_Class_Description class_desc = {
     EO_VERSION,
     "Domain",
     EFL_CLASS_TYPE_REGULAR,
     sizeof(Domain_Public_Data),
     _class_initializer,
     NULL,
     NULL
};

EFL_DEFINE_CLASS(domain_class_get, &class_desc, EO_CLASS, NULL)
