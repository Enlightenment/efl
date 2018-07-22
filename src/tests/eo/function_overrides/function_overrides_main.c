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
   efl_object_init();

   Eina_Bool called = EINA_FALSE;
   Eo *obj = efl_add_ref(INHERIT2_CLASS, NULL);

   simple_a_set(obj, 1);
   Simple_Public_Data *pd = efl_data_scope_get(obj, SIMPLE_CLASS);
   fail_if(pd->a != 2);

   efl_unref(obj);

   obj = efl_add_ref(INHERIT3_CLASS, NULL);

   simple_a_set(obj, 1);
   pd = efl_data_scope_get(obj, SIMPLE_CLASS);
   fail_if(pd->a != 3);

   efl_unref(obj);

   obj = efl_add_ref(INHERIT2_CLASS, NULL);
   called = inherit2_print(obj);
   fail_if(!called);
   called = inherit2_print(obj);
   called = inherit2_print(obj);
   fail_if(!called);
   efl_unref(obj);

   obj = efl_add_ref(SIMPLE_CLASS, NULL);
   called = inherit2_print(obj);
   fail_if(called);

#ifdef EO_DEBUG
   called = simple_class_print(obj);
   fail_if(called);
#endif

   called = simple_class_print(SIMPLE_CLASS);
   fail_if(!called);

   called = simple_class_print(INHERIT_CLASS);
   fail_if(!called);

   called = simple_class_print(INHERIT2_CLASS);
   fail_if(!called);

   called = simple_class_print(INHERIT3_CLASS);
   fail_if(!called);

#ifdef EO_DEBUG
   called = simple_a_print(SIMPLE_CLASS);
   fail_if(called);
#endif

   efl_constructor(efl_super(obj, SIMPLE_CLASS));
   efl_destructor(efl_super(obj, SIMPLE_CLASS));

   efl_unref(obj);

   efl_object_shutdown();
   return 0;
}

