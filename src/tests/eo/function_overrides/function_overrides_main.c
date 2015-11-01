#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "function_overrides_simple.h"
#include "function_overrides_inherit.h"
#include "function_overrides_inherit2.h"
#include "function_overrides_inherit3.h"

#include "../eunit_tests.h"

int
main(int argc, char *argv[])
{
   (void) argc;
   (void) argv;
   eo_init();

   Eina_Bool called = EINA_FALSE;
   Eo *obj;
   eo_add(obj, INHERIT2_CLASS, NULL);

   eo_do(obj, simple_a_set(obj, 1));
   Simple_Public_Data *pd = eo_data_scope_get(obj, SIMPLE_CLASS);
   fail_if(pd->a != 2);

   eo_unref(obj);

   eo_add(obj, INHERIT3_CLASS, NULL);

   eo_do(obj, simple_a_set(obj, 1));
   pd = eo_data_scope_get(obj, SIMPLE_CLASS);
   fail_if(pd->a != 3);

   eo_unref(obj);

   eo_add(obj, INHERIT2_CLASS, NULL);
   eo_do(obj, called = inherit2_print(obj));
   fail_if(!called);
   eo_do(obj, called = inherit2_print(obj), called = inherit2_print(obj));
   fail_if(!called);
   eo_unref(obj);

   eo_add(obj, SIMPLE_CLASS, NULL);
   eo_do(obj, called = inherit2_print(obj));
   fail_if(called);

#ifdef EO_DEBUG
   eo_do(obj, called = simple_class_print(obj));
   fail_if(called);
#endif

   eo_do(SIMPLE_CLASS, called = simple_class_print(obj));
   fail_if(!called);

   eo_do(INHERIT_CLASS, called = simple_class_print(obj));
   fail_if(!called);

   eo_do(INHERIT2_CLASS, called = simple_class_print(obj));
   fail_if(!called);

   eo_do(INHERIT3_CLASS, called = simple_class_print(obj));
   fail_if(!called);

#ifdef EO_DEBUG
   eo_do(SIMPLE_CLASS, called = simple_a_print(obj));
   fail_if(called);
#endif

   eo_super_eo_constructor(SIMPLE_CLASS, obj);
   eo_super_eo_destructor(SIMPLE_CLASS, obj);

   eo_unref(obj);

   eo_shutdown();
   return 0;
}

