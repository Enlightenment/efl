#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>

#include "eobj_suite.h"
#include "Eobj.h"

#include "class_simple.h"

START_TEST(eobj_simple)
{
   eobj_init();
   Eobj *obj = eobj_add(EOBJ_CLASS_BASE, NULL);
   fail_if(obj);

   eobj_shutdown();
}
END_TEST

START_TEST(eobj_weak_reference)
{
   eobj_init();

   Eobj *obj = eobj_add(SIMPLE_CLASS, NULL);
   Eobj_Weak_Ref *wref = eobj_weak_ref_new(obj);
   fail_if(!eobj_weak_ref_get(wref));

   eobj_unref(obj);
   fail_if(eobj_weak_ref_get(wref));

   eobj_weak_ref_free(wref);

   obj = eobj_add(SIMPLE_CLASS, NULL);
   wref = eobj_weak_ref_new(obj);

   eobj_ref(obj);
   fail_if(!eobj_weak_ref_get(wref));

   eobj_del(obj);
   fail_if(eobj_weak_ref_get(wref));

   eobj_unref(obj);
   fail_if(eobj_weak_ref_get(wref));

   eobj_weak_ref_free(wref);


   eobj_shutdown();
}
END_TEST

START_TEST(eobj_op_errors)
{
   eobj_init();
   Eobj *obj = eobj_add(SIMPLE_CLASS, NULL);

   /* Out of bounds op for a legal class. */
   fail_if(eobj_do(obj, 0x00010111));

   /* Ilegal class. */
   fail_if(eobj_do(obj, 0x0F010111));

   fail_if(eobj_ref_get(obj) != 1);

   eobj_ref(obj);
   fail_if(eobj_ref_get(obj) != 2);

   eobj_ref(obj);
   fail_if(eobj_ref_get(obj) != 3);

   eobj_unref(obj);
   fail_if(eobj_ref_get(obj) != 2);

   eobj_unref(obj);
   fail_if(eobj_ref_get(obj) != 1);

   eobj_unref(obj);
   eobj_shutdown();
}
END_TEST

START_TEST(eobj_generic_data)
{
   eobj_init();
   Eobj *obj = eobj_add(SIMPLE_CLASS, NULL);
   void *data;

   eobj_do(obj, EOBJ_BASE_DATA_SET("test1", (void *) 1, NULL));
   eobj_do(obj, EOBJ_BASE_DATA_GET("test1", &data));
   fail_if(1 != (int) data);
   eobj_do(obj, EOBJ_BASE_DATA_DEL("test1"));
   eobj_do(obj, EOBJ_BASE_DATA_GET("test1", &data));
   fail_if(data);

   eobj_do(obj, EOBJ_BASE_DATA_SET("test1", (void *) 1, NULL));
   eobj_do(obj, EOBJ_BASE_DATA_SET("test2", (void *) 2, NULL));
   eobj_do(obj, EOBJ_BASE_DATA_GET("test1", &data));
   fail_if(1 != (int) data);
   eobj_do(obj, EOBJ_BASE_DATA_GET("test2", &data));
   fail_if(2 != (int) data);

   eobj_do(obj, EOBJ_BASE_DATA_GET("test2", &data));
   fail_if(2 != (int) data);
   eobj_do(obj, EOBJ_BASE_DATA_DEL("test2"));
   eobj_do(obj, EOBJ_BASE_DATA_GET("test2", &data));
   fail_if(data);

   eobj_do(obj, EOBJ_BASE_DATA_GET("test1", &data));
   fail_if(1 != (int) data);
   eobj_do(obj, EOBJ_BASE_DATA_DEL("test1"));
   eobj_do(obj, EOBJ_BASE_DATA_GET("test1", &data));
   fail_if(data);

   eobj_unref(obj);
   eobj_shutdown();
}
END_TEST

void eobj_test_general(TCase *tc)
{
   tcase_add_test(tc, eobj_generic_data);
   tcase_add_test(tc, eobj_op_errors);
   tcase_add_test(tc, eobj_simple);
   tcase_add_test(tc, eobj_weak_reference);
}
