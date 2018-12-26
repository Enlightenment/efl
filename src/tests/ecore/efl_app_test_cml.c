#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define EFL_CORE_COMMAND_LINE_PROTECTED

#include <stdio.h>
#include <unistd.h>
#define EFL_NOLEGACY_API_SUPPORT
#include <Efl_Core.h>
#include "efl_app_suite.h"
#include "../efl_check.h"

typedef struct {

} Efl_App_Test_CML_Data;

#include "efl_app_test_cml.eo.h"
#include "efl_app_test_cml.eo.c"

static Eina_Array*
_construct_array(void)
{
   Eina_Array *array = eina_array_new(16);

   eina_array_push(array, "/bin/sh");
   eina_array_push(array, "-C");
   eina_array_push(array, "foo");
   eina_array_push(array, "--test");
   eina_array_push(array, "--option=done");
   eina_array_push(array, "--");
   eina_array_push(array, "asdf --test");
   return array;
}

static const char*
_construct_string(void)
{
   return "/bin/sh -C foo --test --option=done -- \"asdf --test\"";
}

EFL_START_TEST(efl_core_cml_string)
{
   Efl_App_Test_CML *cml = efl_add_ref(EFL_APP_TEST_CML_CLASS, NULL);
   Eina_Array *content = _construct_array();
   Eina_Stringshare *str;
   Eina_Bool b;
   int i = 0;

   b = efl_core_command_line_command_string_set(cml, _construct_string());
   ck_assert_int_ne(b, 0);

   EINA_ACCESSOR_FOREACH(efl_core_command_line_command_access(cml), i, str)
     {
        ck_assert_str_eq(eina_array_data_get(content, i), str);
     }
   ck_assert_str_eq(efl_core_command_line_command_get(cml), _construct_string());
}
EFL_END_TEST

EFL_START_TEST(efl_core_cml_array)
{
   Efl_App_Test_CML *cml = efl_add_ref(EFL_APP_TEST_CML_CLASS, NULL);
   Eina_Array *content1 = _construct_array();
   Eina_Array *content2 = _construct_array();
   Eina_Stringshare *str;
   Eina_Bool b;
   int i = 0;

   b = efl_core_command_line_command_array_set(cml, content1);
   ck_assert_int_ne(b, 0);

   EINA_ACCESSOR_FOREACH(efl_core_command_line_command_access(cml), i, str)
     {
        ck_assert_str_eq(eina_array_data_get(content2, i), str);
     }
   ck_assert_str_eq(efl_core_command_line_command_get(cml), _construct_string());
}
EFL_END_TEST

void efl_test_efl_cml(TCase *tc)
{
   tcase_add_test(tc, efl_core_cml_string);
   tcase_add_test(tc, efl_core_cml_array);
}
