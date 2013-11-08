#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>

#include "Eo.h"
#include "eo_suite.h"
#include "eo_test_class_simple.h"

START_TEST(eo_value)
{
   eo_init();
   char *str, *str2;

   Eina_Value val2, eo_val;
   void *tmpp = NULL;
   Eo_Dbg_Info *eo_dbg_info;
   Eo *obj = eo2_add(SIMPLE_CLASS, NULL);

   eo_dbg_info = EO_DBG_INFO_LIST_APPEND(NULL, "Root");
   eo2_do(obj, eo2_dbg_info_get(eo_dbg_info));
   fail_if(!eo_dbg_info);
   ck_assert_str_eq(eo_dbg_info->name, "Root");
   str = eina_value_to_string(&eo_dbg_info->value);
   ck_assert_str_eq(str, "[[8]]");

   eina_value_setup(&val2, EO_DBG_INFO_TYPE);
   eina_value_copy(&eo_dbg_info->value, &val2);
   str2 = eina_value_to_string(&val2);
   ck_assert_str_eq(str, str2);

   eina_value_get(&val2, &eo_val);
   eina_value_pget(&eo_val, &tmpp);
   fail_if(!tmpp);
   eina_value_free(&val2);

   eina_value_setup(&val2, EINA_VALUE_TYPE_INT);
   fail_if(eina_value_convert(&eo_dbg_info->value, &val2));
   eina_value_free(&val2);

   free(str);
   free(str2);
   eo_dbg_info_free(eo_dbg_info);
   eo_unref(obj);

   eo_shutdown();
}
END_TEST

void eo_test_value(TCase *tc)
{
   tcase_add_test(tc, eo_value);
}
