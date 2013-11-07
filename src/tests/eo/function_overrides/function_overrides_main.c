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
   inherit2_print_called = EINA_FALSE;
   eo2_do(obj, inherit2_print());
   eo2_do(obj, inherit2_print(), inherit2_print());
   fail_if(!inherit2_print_called);
   eo_unref(obj);

   obj = eo2_add(SIMPLE_CLASS, NULL);
   inherit2_print_called = EINA_FALSE;
   eo2_do(obj, inherit2_print());
   fail_if(inherit2_print_called);

#ifdef EO_DEBUG
   class_print_called = EINA_FALSE;
   eo2_do(obj, simple_class_print());
   fail_if(class_print_called);
#endif

   class_print_called = EINA_FALSE;
   eo2_do(SIMPLE_CLASS, simple_class_print());
   fail_if(!class_print_called);
   class_print_called = EINA_FALSE;
   eo2_do(INHERIT_CLASS, simple_class_print());
   fail_if(!class_print_called);
   class_print_called = EINA_FALSE;
   eo2_do(INHERIT2_CLASS, simple_class_print());
   fail_if(!class_print_called);
   class_print_called = EINA_FALSE;
   eo2_do(INHERIT3_CLASS, simple_class_print());
   fail_if(!class_print_called);

#ifdef EO_DEBUG
   pd->a_print_called = EINA_FALSE;
   eo2_do(SIMPLE_CLASS, simple_a_print());
   fail_if(pd->a_print_called);
#endif

   eo2_do_super(obj, SIMPLE_CLASS, eo2_constructor());
   eo2_do_super(obj, SIMPLE_CLASS, eo2_destructor());

   eo_unref(obj);

   eo_shutdown();
   return 0;
}

