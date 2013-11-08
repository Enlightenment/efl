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

   Eina_Bool called;
   Eo *obj = eo2_add(INHERIT2_CLASS, NULL);

   eo2_do(obj, simple_a_set(1));
   Simple_Public_Data *pd = eo_data_scope_get(obj, SIMPLE_CLASS);
   fail_if(pd->a != 2);

   eo_unref(obj);

   obj = eo2_add(INHERIT3_CLASS, NULL);

   eo2_do(obj, simple_a_set(1));
   pd = eo_data_scope_get(obj, SIMPLE_CLASS);
   fail_if(pd->a != 3);

   eo_unref(obj);

   obj = eo2_add(INHERIT2_CLASS, NULL);
   eo2_do(obj, called = inherit2_print());
   fail_if(!called);
   eo2_do(obj, called = inherit2_print(), called = inherit2_print());
   fail_if(!called);
   eo_unref(obj);

   obj = eo2_add(SIMPLE_CLASS, NULL);
   eo2_do(obj, called = inherit2_print());
   fail_if(called);

#ifdef EO_DEBUG
   eo2_do(obj, called = simple_class_print());
   fail_if(called);
#endif

   eo2_do(SIMPLE_CLASS, called = simple_class_print());
   fail_if(!called);

   eo2_do(INHERIT_CLASS, called = simple_class_print());
   fail_if(!called);

   eo2_do(INHERIT2_CLASS, called = simple_class_print());
   fail_if(!called);

   eo2_do(INHERIT3_CLASS, called = simple_class_print());
   fail_if(!called);

#ifdef EO_DEBUG
   eo2_do(SIMPLE_CLASS, called = simple_a_print());
   fail_if(called);
#endif

   eo2_do_super(obj, SIMPLE_CLASS, eo2_constructor());
   eo2_do_super(obj, SIMPLE_CLASS, eo2_destructor());

   eo_unref(obj);

   eo_shutdown();
   return 0;
}

