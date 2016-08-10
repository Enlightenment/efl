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
   Eo *obj = eo_add(INHERIT2_CLASS, NULL);

   simple_a_set(obj, 1);
   Simple_Public_Data *pd = eo_data_scope_get(obj, SIMPLE_CLASS);
   fail_if(pd->a != 2);

   eo_unref(obj);

   obj = eo_add(INHERIT3_CLASS, NULL);

   simple_a_set(obj, 1);
   pd = eo_data_scope_get(obj, SIMPLE_CLASS);
   fail_if(pd->a != 3);

   eo_unref(obj);

   obj = eo_add(INHERIT2_CLASS, NULL);
   called = inherit2_print(obj);
   fail_if(!called);
   called = inherit2_print(obj);
   called = inherit2_print(obj);
   fail_if(!called);
   eo_unref(obj);

   obj = eo_add(SIMPLE_CLASS, NULL);
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

   efl_constructor(eo_super(obj, SIMPLE_CLASS));
   efl_destructor(eo_super(obj, SIMPLE_CLASS));

   eo_unref(obj);

   eo_shutdown();
   return 0;
}

