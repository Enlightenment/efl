#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>

#include "Eolian.h"
#include "eolian_suite.h"

typedef struct _Eolian_Test_Case Eolian_Test_Case;

struct _Eolian_Test_Case
{
   const char *test_case;
   void      (*build)(TCase *tc);
};

START_TEST(eolian_scope)
{
   Eolian_Function fid = NULL;
   const char *class_name = "Simple";

   eolian_init();
   /* Parsing */
   fail_if(!eolian_eo_file_parse(PACKAGE_DATA_DIR"/data/scope.eo"));

   /* Property scope */
   fail_if(!(fid = eolian_class_function_find_by_name(class_name, "a", EOLIAN_PROPERTY)));
   fail_if(eolian_function_scope_get(fid) != EOLIAN_SCOPE_PROTECTED);
   fail_if(!(fid = eolian_class_function_find_by_name(class_name, "b", EOLIAN_PROPERTY)));
   fail_if(eolian_function_scope_get(fid) != EOLIAN_SCOPE_PUBLIC);
   fail_if(!(fid = eolian_class_function_find_by_name(class_name, "c", EOLIAN_PROPERTY)));
   fail_if(eolian_function_scope_get(fid) != EOLIAN_SCOPE_PUBLIC);

   /* Method scope */
   fail_if(!(fid = eolian_class_function_find_by_name(class_name, "foo", EOLIAN_METHOD)));
   fail_if(eolian_function_scope_get(fid) != EOLIAN_SCOPE_PUBLIC);
   fail_if(!(fid = eolian_class_function_find_by_name(class_name, "bar", EOLIAN_METHOD)));
   fail_if(eolian_function_scope_get(fid) != EOLIAN_SCOPE_PROTECTED);
   fail_if(!(fid = eolian_class_function_find_by_name(class_name, "foobar", EOLIAN_METHOD)));
   fail_if(eolian_function_scope_get(fid) != EOLIAN_SCOPE_PUBLIC);

   eolian_shutdown();
}
END_TEST

START_TEST(eolian_simple_parsing)
{
   Eolian_Function fid = NULL;
   const char *string = NULL, *ptype = NULL, *pname = NULL;
   Eolian_Parameter_Dir dir = EOLIAN_IN_PARAM;
   const char *class_name = "Simple";
   const Eina_List *list = NULL;
   Eolian_Function_Parameter param = NULL;

   eolian_init();
   /* Parsing */
   fail_if(!eolian_eo_file_parse(PACKAGE_DATA_DIR"/data/class_simple.eo"));
   fail_if(strcmp(eolian_class_find_by_file(PACKAGE_DATA_DIR"/data/class_simple.eo"), class_name));

   /* Class */
   fail_if(!eolian_class_exists(class_name));
   fail_if(eolian_class_type_get(class_name) != EOLIAN_CLASS_REGULAR);
   string = eolian_class_description_get(class_name);
   fail_if(!string);
   fail_if(strcmp(string, "Class Desc Simple"));
   fail_if(eolian_class_inherits_list_get(class_name) != NULL);
   fail_if(strcmp(eolian_class_legacy_prefix_get(class_name), "evas_object_simple"));
   fail_if(strcmp(eolian_class_eo_prefix_get(class_name), "evas_obj_simple"));
   fail_if(strcmp(eolian_class_data_type_get(class_name), "Evas_Simple_Data"));

   /* Property */
   fail_if(!(fid = eolian_class_function_find_by_name(class_name, "a", EOLIAN_PROPERTY)));
   fail_if(strcmp(eolian_function_name_get(fid), "a"));
   string = eolian_function_description_get(fid, EOLIAN_COMMENT_SET);
   fail_if(!string);
   fail_if(strcmp(string, "comment a::set"));
   string = eolian_function_description_get(fid, EOLIAN_COMMENT_GET);
   fail_if(string);
   /* Function return */
   string = eolian_function_return_type_get(fid, EOLIAN_PROP_SET);
   fail_if(!string);
   fail_if(strcmp(string, "Eina_Bool"));
   string = eolian_function_return_dflt_value_get(fid, EOLIAN_PROP_SET);
   fail_if(!string);
   fail_if(strcmp(string, "EINA_TRUE"));
   string = eolian_function_return_type_get(fid, EOLIAN_PROP_GET);
   fail_if(string);

   /* Function parameters */
   fail_if(eolian_property_keys_list_get(fid) != NULL);
   fail_if(!(list = eolian_property_values_list_get(fid)));
   fail_if(eina_list_count(list) != 1);
   fail_if(!(param = eina_list_data_get(list)));
   eolian_parameter_information_get(param, NULL, &ptype, &pname, &string);
   fail_if(strcmp(ptype, "int"));
   fail_if(strcmp(pname, "value"));
   fail_if(strcmp(string, "Value description"));

   /* Method */
   fail_if(!(fid = eolian_class_function_find_by_name(class_name, "foo", EOLIAN_METHOD)));
   string = eolian_function_description_get(fid, EOLIAN_COMMENT);
   fail_if(!string);
   fail_if(strcmp(string, "comment foo"));
   /* Function return */
   string = eolian_function_return_type_get(fid, EOLIAN_METHOD);
   fail_if(!string);
   fail_if(strcmp(string, "char *"));
   string = eolian_function_return_dflt_value_get(fid, EOLIAN_METHOD);
   fail_if(!string);
   fail_if(strcmp(string, "NULL"));

   /* Function parameters */
   fail_if(!(list = eolian_parameters_list_get(fid)));
   fail_if(eina_list_count(list) != 3);
   fail_if(!(param = eina_list_nth(list, 0)));
   eolian_parameter_information_get(param, &dir, &ptype, &pname, &string);
   fail_if(dir != EOLIAN_IN_PARAM);
   fail_if(strcmp(ptype, "int"));
   fail_if(strcmp(pname, "a"));
   fail_if(!string || strcmp(string, "a"));
   fail_if(!(param = eina_list_nth(list, 1)));
   eolian_parameter_information_get(param, &dir, &ptype, &pname, &string);
   fail_if(dir != EOLIAN_INOUT_PARAM);
   fail_if(strcmp(ptype, "char *"));
   fail_if(strcmp(pname, "b"));
   fail_if(string);
   fail_if(!(param = eina_list_nth(list, 2)));
   eolian_parameter_information_get(param, &dir, &ptype, &pname, &string);
   fail_if(dir != EOLIAN_OUT_PARAM);
   fail_if(strcmp(ptype, "double"));
   fail_if(strcmp(pname, "c"));
   fail_if(string);

   eolian_shutdown();
}
END_TEST

static void eolian_parsing_test(TCase *tc)
{
   tcase_add_test(tc, eolian_simple_parsing);
   tcase_add_test(tc, eolian_scope);
}

static const Eolian_Test_Case etc[] = {
  { "Eolian Parsing", eolian_parsing_test},
  { NULL, NULL }
};

static void
_list_tests(void)
{
  const Eolian_Test_Case *itr;

   itr = etc;
   fputs("Available Test Cases:\n", stderr);
   for (; itr->test_case; itr++)
     fprintf(stderr, "\t%s\n", itr->test_case);
}

static Eina_Bool
_use_test(int argc, const char **argv, const char *test_case)
{
   if (argc < 1)
     return 1;

   for (; argc > 0; argc--, argv++)
     if (strcmp(test_case, *argv) == 0)
       return 1;
   return 0;
}

static Suite *
eolian_suite_build(int argc, const char **argv)
{
   TCase *tc;
   Suite *s;
   int i;

   s = suite_create("Eolian");

   for (i = 0; etc[i].test_case; ++i)
     {
	if (!_use_test(argc, argv, etc[i].test_case)) continue;
	tc = tcase_create(etc[i].test_case);

	etc[i].build(tc);

	suite_add_tcase(s, tc);
	tcase_set_timeout(tc, 0);
     }

   return s;
}

int
main(int argc, char **argv)
{
   Suite *s;
   SRunner *sr;
   int i, failed_count;
   eolian_init();
   setenv("CK_FORK", "no", 0);

   for (i = 1; i < argc; i++)
     if ((strcmp(argv[i], "-h") == 0) ||
	 (strcmp(argv[i], "--help") == 0))
       {
	  fprintf(stderr, "Usage:\n\t%s [test_case1 .. [test_caseN]]\n",
		  argv[0]);
	  _list_tests();
	  return 0;
       }
     else if ((strcmp(argv[i], "-l") == 0) ||
	      (strcmp(argv[i], "--list") == 0))
       {
	  _list_tests();
	  return 0;
       }

   putenv("EFL_RUN_IN_TREE=1");

   s = eolian_suite_build(argc - 1, (const char **)argv + 1);
   sr = srunner_create(s);

   srunner_set_xml(sr, TESTS_BUILD_DIR "/check-results.xml");

   srunner_run_all(sr, CK_ENV);
   failed_count = srunner_ntests_failed(sr);
   srunner_free(sr);

   eolian_shutdown();

   return (failed_count == 0) ? 0 : 255;
}
