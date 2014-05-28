#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>

#include<Eina.h>
#include "Eolian.h"
#include "eolian_suite.h"

START_TEST(eolian_namespaces)
{
   Eolian_Class class11, class112, class21, class_no;
   Eolian_Function fid;
   const Eina_List *list = NULL;
   const char *class_name, *func_name;
   Eolian_Function_Type func_type;

   eolian_init();
   /* Parsing */
   fail_if(!eolian_eo_file_parse(PACKAGE_DATA_DIR"/data/namespace.eo"));

   /* Classes existence  */
   fail_if(!(class11 = eolian_class_find_by_name("nmsp1::class1")));
   fail_if(!(class112 = eolian_class_find_by_name("nmsp1::nmsp11::class2")));
   fail_if(!(class21 = eolian_class_find_by_name("nmsp2::class1")));
   fail_if(!(class_no = eolian_class_find_by_name("no_nmsp")));

   /* Check names and namespaces*/
   fail_if(strcmp(eolian_class_name_get(class11), "class1"));
   fail_if(!(list = eolian_class_namespaces_list_get(class11)));
   fail_if(eina_list_count(list) != 1);
   fail_if(strcmp(eina_list_nth(list, 0), "nmsp1"));

   fail_if(strcmp(eolian_class_name_get(class112), "class2"));
   fail_if(!(list = eolian_class_namespaces_list_get(class112)));
   fail_if(eina_list_count(list) != 2);
   fail_if(strcmp(eina_list_nth(list, 0), "nmsp1"));
   fail_if(strcmp(eina_list_nth(list, 1), "nmsp11"));

   fail_if(strcmp(eolian_class_name_get(class21), "class1"));
   fail_if(!(list = eolian_class_namespaces_list_get(class21)));
   fail_if(eina_list_count(list) != 1);
   fail_if(strcmp(eina_list_nth(list, 0), "nmsp2"));

   fail_if(strcmp(eolian_class_name_get(class_no), "no_nmsp"));
   fail_if(eolian_class_namespaces_list_get(class_no));

   /* Inherits */
   fail_if(!(list = eolian_class_inherits_list_get(class11)));
   fail_if(eina_list_count(list) != 3);
   class_name = eina_list_nth(list, 0);
   fail_if(eolian_class_find_by_name(class_name) != class112);
   class_name = eina_list_nth(list, 1);
   fail_if(eolian_class_find_by_name(class_name) != class21);
   class_name = eina_list_nth(list, 2);
   fail_if(eolian_class_find_by_name(class_name) != class_no);

   /* Implements */
   fail_if(!(list = eolian_class_implements_list_get(class11)));
   fail_if(eina_list_count(list) != 3);
   fail_if(!eolian_implement_information_get(eina_list_nth(list, 0),
            &class_name, &func_name, &func_type));
   fail_if(eolian_class_find_by_name(class_name) != class112);
   fail_if(strcmp(func_name, "a"));
   fail_if(func_type != EOLIAN_PROP_SET);
   fail_if(eolian_implement_information_get(eina_list_nth(list, 1),
            &class_name, &func_name, &func_type));
   fail_if(!eolian_implement_information_get(eina_list_nth(list, 2),
            &class_name, &func_name, &func_type));
   fail_if(eolian_class_find_by_name(class_name) != class_no);
   fail_if(strcmp(func_name, "foo"));
   fail_if(func_type != EOLIAN_METHOD);

   /* Virtual regression */
   fail_if(!(fid = eolian_class_function_find_by_name(class112, "a", EOLIAN_UNRESOLVED)));
   fail_if(!eolian_function_is_virtual_pure(fid, EOLIAN_PROP_SET));

   eolian_shutdown();
}
END_TEST

START_TEST(eolian_events)
{
   Eolian_Class class;
   const Eina_List *list = NULL;
   const char *name, *type, *comment;

   eolian_init();
   /* Parsing */
   fail_if(!eolian_eo_file_parse(PACKAGE_DATA_DIR"/data/events.eo"));

   /* Class */
   fail_if(!(class = eolian_class_find_by_name("Events")));

   /* Events */
   fail_if(!(list = eolian_class_events_list_get(class)));
   fail_if(eina_list_count(list) != 2);
   /* Clicked */
   fail_if(!eolian_class_event_information_get(eina_list_nth(list, 0), &name, &type, &comment));
   fail_if(strcmp(name, "clicked"));
   fail_if(type);
   fail_if(strcmp(comment, "Comment for clicked"));
   /* Clicked,double */
   fail_if(!eolian_class_event_information_get(eina_list_nth(list, 1), &name, &type, &comment));
   fail_if(strcmp(name, "clicked,double"));
   fail_if(strcmp(type, "Evas_Event_Clicked_Double_Info"));
   fail_if(comment);

   eolian_shutdown();
}
END_TEST

START_TEST(eolian_override)
{
   Eolian_Function fid = NULL;
   const Eina_List *impls = NULL;
   const char *impl_class = NULL, *impl_func = NULL;
   Eolian_Class class, base;

   eolian_init();
   /* Parsing */
   fail_if(!eolian_directory_scan(PACKAGE_DATA_DIR"/data"));
   fail_if(!eolian_eo_file_parse(PACKAGE_DATA_DIR"/data/override.eo"));

   /* Class */
   fail_if(!(class = eolian_class_find_by_name("Simple")));
   fail_if(!(base = eolian_class_find_by_name("Base")));

   /* Base ctor */
   fail_if(!(fid = eolian_class_function_find_by_name(base, "constructor", EOLIAN_UNRESOLVED)));
   fail_if(!eolian_function_is_virtual_pure(fid, EOLIAN_UNRESOLVED));
   fail_if(!(impls = eolian_class_implements_list_get(class)));
   fail_if(!eolian_implement_information_get(eina_list_nth(impls, 0), &impl_class, &impl_func, NULL));
   fail_if(strcmp(impl_class, "Base"));
   fail_if(strcmp(impl_func, "constructor"));

   /* Property */
   fail_if(!(fid = eolian_class_function_find_by_name(class, "a", EOLIAN_PROPERTY)));
   fail_if(!eolian_function_is_virtual_pure(fid, EOLIAN_PROP_SET));
   fail_if(eolian_function_is_virtual_pure(fid, EOLIAN_PROP_GET));

   /* Method */
   fail_if(!(fid = eolian_class_function_find_by_name(class, "foo", EOLIAN_METHOD)));
   fail_if(!eolian_function_is_virtual_pure(fid, EOLIAN_METHOD));

   eolian_shutdown();
}
END_TEST

START_TEST(eolian_consts)
{
   Eolian_Function fid = NULL;
   Eolian_Function_Parameter param = NULL;
   Eolian_Class class;

   eolian_init();
   /* Parsing */
   fail_if(!eolian_eo_file_parse(PACKAGE_DATA_DIR"/data/consts.eo"));
   fail_if(!(class = eolian_class_find_by_name("Const")));

   /* Property */
   fail_if(!(fid = eolian_class_function_find_by_name(class, "a", EOLIAN_PROPERTY)));
   fail_if(!(param = eolian_function_parameter_get(fid, "buffer")));
   fail_if(eolian_parameter_const_attribute_get(param, EINA_FALSE));
   fail_if(!eolian_parameter_const_attribute_get(param, EINA_TRUE));

   /* Method */
   fail_if(!(fid = eolian_class_function_find_by_name(class, "foo", EOLIAN_METHOD)));
   fail_if(EINA_FALSE == eolian_function_object_is_const(fid));

   eolian_shutdown();
}
END_TEST

START_TEST(eolian_ctor_dtor)
{
   const Eina_List *impls = NULL;
   const char *impl_class = NULL, *impl_func = NULL;
   Eolian_Class class, base;

   eolian_init();
   /* Parsing */
   fail_if(!eolian_directory_scan(PACKAGE_DATA_DIR"/data"));
   fail_if(!eolian_eo_file_parse(PACKAGE_DATA_DIR"/data/ctor_dtor.eo"));
   fail_if(!(class = eolian_class_find_by_name("Ctor_Dtor")));
   fail_if(!(base = eolian_class_find_by_name("Base")));

   /* Class ctor/dtor */
   fail_if(!eolian_class_ctor_enable_get(class));
   fail_if(!eolian_class_dtor_enable_get(class));

   /* Base ctor/dtor */
   fail_if(!(impls = eolian_class_implements_list_get(class)));
   fail_if(eina_list_count(impls) != 2);
   fail_if(!eolian_implement_information_get(eina_list_nth(impls, 0), &impl_class, &impl_func, NULL));
   fail_if(strcmp(impl_class, "Base"));
   fail_if(strcmp(impl_func, "constructor"));
   fail_if(!eolian_implement_information_get(eina_list_nth(impls, 1), &impl_class, &impl_func, NULL));
   fail_if(strcmp(impl_class, "Base"));
   fail_if(strcmp(impl_func, "destructor"));

   /* Custom ctors/dtors */
   fail_if(!eolian_class_function_find_by_name(base, "constructor", EOLIAN_CTOR));
   fail_if(!eolian_class_function_find_by_name(base, "destructor", EOLIAN_METHOD));
   fail_if(!eolian_class_function_find_by_name(class, "custom_constructor_1", EOLIAN_CTOR));
   fail_if(!eolian_class_function_find_by_name(class, "custom_constructor_2", EOLIAN_CTOR));

   eolian_shutdown();
}
END_TEST

START_TEST(eolian_typedef)
{
   Eolian_Type types_list = NULL;
   const char *type_name = NULL;
   Eina_Bool own = EINA_FALSE;
   Eolian_Class class;

   eolian_init();
   /* Parsing */
   fail_if(!eolian_eo_file_parse(PACKAGE_DATA_DIR"/data/typedef.eo"));

   /* Check that the class Dummy is still readable */
   fail_if(!(class = eolian_class_find_by_name("Dummy")));
   fail_if(!eolian_class_function_find_by_name(class, "foo", EOLIAN_METHOD));

   /* Basic type */
   fail_if(!(types_list = eolian_type_find_by_alias("Evas_Coord")));
   fail_if(eolian_type_information_get(types_list, &type_name, &own));
   fail_if(strcmp(type_name, "int"));
   fail_if(own != EINA_FALSE);

   /* Complex type */
   fail_if(!(types_list = eolian_type_find_by_alias("List_Objects")));
   fail_if(!(types_list = eolian_type_information_get(types_list, &type_name, &own)));
   fail_if(strcmp(type_name, "Eina_List *"));
   fail_if(own != EINA_TRUE);
   fail_if(eolian_type_information_get(types_list, &type_name, &own));
   fail_if(strcmp(type_name, "Eo *"));
   fail_if(own != EINA_FALSE);

   eolian_shutdown();
}
END_TEST

START_TEST(eolian_complex_type)
{
   Eolian_Function fid = NULL;
   Eolian_Function_Parameter param = NULL;
   const Eina_List *params_list = NULL;
   Eolian_Type types_list = NULL;
   const char *type_name = NULL;
   Eina_Bool own = EINA_FALSE;
   Eolian_Class class;

   eolian_init();
   /* Parsing */
   fail_if(!eolian_eo_file_parse(PACKAGE_DATA_DIR"/data/complex_type.eo"));
   fail_if(!(class = eolian_class_find_by_name("Complex_Type")));

   /* Properties return type */
   fail_if(!(fid = eolian_class_function_find_by_name(class, "a", EOLIAN_PROPERTY)));
   fail_if(!(types_list = eolian_function_return_types_list_get(fid, EOLIAN_PROP_SET)));
   fail_if(!(types_list = eolian_type_information_get(types_list, &type_name, &own)));
   fail_if(strcmp(type_name, "Eina_List *"));
   fail_if(own != EINA_TRUE);
   fail_if(!(types_list = eolian_type_information_get(types_list, &type_name, &own)));
   fail_if(strcmp(type_name, "Eina_Array *"));
   fail_if(own != EINA_FALSE);
   fail_if(eolian_type_information_get(types_list, &type_name, &own));
   fail_if(strcmp(type_name, "Eo **"));
   fail_if(own != EINA_TRUE);
   /* Properties parameter type */
   fail_if(!(params_list = eolian_parameters_list_get(fid)));
   fail_if(eina_list_count(params_list) != 1);
   fail_if(!(param = eina_list_nth(params_list, 0)));
   fail_if(strcmp(eolian_parameter_name_get(param), "value"));
   fail_if(!(types_list = eolian_parameter_types_list_get(param)));
   fail_if(!(types_list = eolian_type_information_get(types_list, &type_name, &own)));
   fail_if(strcmp(type_name, "Eina_List *"));
   fail_if(own != EINA_TRUE);
   fail_if(eolian_type_information_get(types_list, &type_name, &own));
   fail_if(strcmp(type_name, "int"));
   fail_if(own != EINA_FALSE);

   /* Methods return type */
   fail_if(!(fid = eolian_class_function_find_by_name(class, "foo", EOLIAN_METHOD)));
   fail_if(!(types_list = eolian_function_return_types_list_get(fid, EOLIAN_METHOD)));
   fail_if(!(types_list = eolian_type_information_get(types_list, &type_name, &own)));
   fail_if(strcmp(type_name, "Eina_List *"));
   fail_if(own != EINA_TRUE);
   fail_if(eolian_type_information_get(types_list, &type_name, &own));
   fail_if(strcmp(type_name, "Eina_Stringshare *"));
   fail_if(own != EINA_FALSE);
   /* Methods parameter type */
   fail_if(!(params_list = eolian_parameters_list_get(fid)));
   fail_if(eina_list_count(params_list) != 1);
   fail_if(!(param = eina_list_nth(params_list, 0)));
   fail_if(strcmp(eolian_parameter_name_get(param), "buf"));
   fail_if(!(types_list = eolian_parameter_types_list_get(param)));
   fail_if(eolian_type_information_get(types_list, &type_name, &own));
   fail_if(strcmp(type_name, "char *"));
   fail_if(own != EINA_TRUE);

   eolian_shutdown();
}
END_TEST

START_TEST(eolian_scope)
{
   Eolian_Function fid = NULL;
   Eolian_Class class;

   eolian_init();
   /* Parsing */
   fail_if(!eolian_eo_file_parse(PACKAGE_DATA_DIR"/data/scope.eo"));
   fail_if(!(class = eolian_class_find_by_name("Simple")));

   /* Property scope */
   fail_if(!(fid = eolian_class_function_find_by_name(class, "a", EOLIAN_PROPERTY)));
   fail_if(eolian_function_scope_get(fid) != EOLIAN_SCOPE_PROTECTED);
   fail_if(!(fid = eolian_class_function_find_by_name(class, "b", EOLIAN_PROPERTY)));
   fail_if(eolian_function_scope_get(fid) != EOLIAN_SCOPE_PUBLIC);
   fail_if(!(fid = eolian_class_function_find_by_name(class, "c", EOLIAN_PROPERTY)));
   fail_if(eolian_function_scope_get(fid) != EOLIAN_SCOPE_PUBLIC);

   /* Method scope */
   fail_if(!(fid = eolian_class_function_find_by_name(class, "foo", EOLIAN_METHOD)));
   fail_if(eolian_function_scope_get(fid) != EOLIAN_SCOPE_PUBLIC);
   fail_if(!(fid = eolian_class_function_find_by_name(class, "bar", EOLIAN_METHOD)));
   fail_if(eolian_function_scope_get(fid) != EOLIAN_SCOPE_PROTECTED);
   fail_if(!(fid = eolian_class_function_find_by_name(class, "foobar", EOLIAN_METHOD)));
   fail_if(eolian_function_scope_get(fid) != EOLIAN_SCOPE_PUBLIC);

   eolian_shutdown();
}
END_TEST

START_TEST(eolian_simple_parsing)
{
   Eolian_Function fid = NULL;
   const char *string = NULL, *ptype = NULL, *pname = NULL;
   Eolian_Parameter_Dir dir = EOLIAN_IN_PARAM;
   const Eina_List *list = NULL;
   Eolian_Function_Parameter param = NULL;
   Eolian_Class class;

   eolian_init();
   /* Parsing */
   fail_if(!eolian_eo_file_parse(PACKAGE_DATA_DIR"/data/class_simple.eo"));
   fail_if(!(class = eolian_class_find_by_name("Simple")));
   fail_if(eolian_class_find_by_file(PACKAGE_DATA_DIR"/data/class_simple.eo") != class);
   fail_if(strcmp(eolian_class_file_get(class), PACKAGE_DATA_DIR"/data/class_simple.eo"));

   /* Class */
   fail_if(eolian_class_type_get(class) != EOLIAN_CLASS_REGULAR);
   string = eolian_class_description_get(class);
   fail_if(!string);
   fail_if(strcmp(string, "Class Desc Simple"));
   fail_if(eolian_class_inherits_list_get(class) != NULL);
   fail_if(strcmp(eolian_class_legacy_prefix_get(class), "evas_object_simple"));
   fail_if(strcmp(eolian_class_eo_prefix_get(class), "evas_obj_simple"));
   fail_if(strcmp(eolian_class_data_type_get(class), "Evas_Simple_Data"));

   /* Property */
   fail_if(!(fid = eolian_class_function_find_by_name(class, "a", EOLIAN_PROPERTY)));
   fail_if(strcmp(eolian_function_name_get(fid), "a"));
   string = eolian_function_description_get(fid, EOLIAN_COMMENT_SET);
   fail_if(!string);
   fail_if(strcmp(string, "comment a::set"));
   string = eolian_function_description_get(fid, EOLIAN_COMMENT_GET);
   fail_if(string);
   /* Set return */
   string = eolian_function_return_type_get(fid, EOLIAN_PROP_SET);
   fail_if(!string);
   fail_if(strcmp(string, "Eina_Bool"));
   string = eolian_function_return_dflt_value_get(fid, EOLIAN_PROP_SET);
   fail_if(!string);
   fail_if(strcmp(string, "EINA_TRUE"));
   string = eolian_function_return_comment_get(fid, EOLIAN_PROP_SET);
   fail_if(!string);
   fail_if(strcmp(string, "comment for property set return"));
   /* Get return */
   string = eolian_function_return_type_get(fid, EOLIAN_PROP_GET);
   fail_if(string);
   string = eolian_function_return_comment_get(fid, EOLIAN_PROP_GET);
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
   fail_if(!(fid = eolian_class_function_find_by_name(class, "foo", EOLIAN_METHOD)));
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
   string = eolian_function_return_comment_get(fid, EOLIAN_METHOD);
   fail_if(!string);
   fail_if(strcmp(string, "comment for method return"));

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

void eolian_parsing_test(TCase *tc)
{
   tcase_add_test(tc, eolian_simple_parsing);
   tcase_add_test(tc, eolian_ctor_dtor);
   tcase_add_test(tc, eolian_scope);
   tcase_add_test(tc, eolian_complex_type);
   tcase_add_test(tc, eolian_typedef);
   tcase_add_test(tc, eolian_consts);
   tcase_add_test(tc, eolian_override);
   tcase_add_test(tc, eolian_events);
   tcase_add_test(tc, eolian_namespaces);
}

