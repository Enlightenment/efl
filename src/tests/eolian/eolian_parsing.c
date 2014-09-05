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
   const Eolian_Class *class11, *class112, *class21, *class_no, *impl_class;
   const Eolian_Function *fid;
   Eina_Iterator *iter;
   Eolian_Function_Type func_type;
   const char *class_name, *val1, *val2;
   const Eolian_Implement *impl;
   void *dummy;

   eolian_init();
   /* Parsing */
   fail_if(!eolian_directory_scan(PACKAGE_DATA_DIR"/data"));
   fail_if(!eolian_eo_file_parse(PACKAGE_DATA_DIR"/data/nmsp1_class1.eo"));

   /* Classes existence  */
   fail_if(!(class11 = eolian_class_get_by_name("nmsp1.class1")));
   fail_if(!(class112 = eolian_class_get_by_name("nmsp1.nmsp11.class2")));
   fail_if(!(class21 = eolian_class_get_by_name("nmsp2.class1")));
   fail_if(!(class_no = eolian_class_get_by_name("no_nmsp")));

   /* Check names and namespaces*/
   fail_if(strcmp(eolian_class_name_get(class11), "class1"));
   fail_if(!(iter = eolian_class_namespaces_get(class11)));
   fail_if(!(eina_iterator_next(iter, (void**)&val1)));
   fail_if(eina_iterator_next(iter, &dummy));
   fail_if(strcmp(val1, "nmsp1"));
   eina_iterator_free(iter);

   fail_if(strcmp(eolian_class_name_get(class112), "class2"));
   fail_if(!(iter = eolian_class_namespaces_get(class112)));
   fail_if(!(eina_iterator_next(iter, (void**)&val1)));
   fail_if(!(eina_iterator_next(iter, (void**)&val2)));
   fail_if(eina_iterator_next(iter, &dummy));
   fail_if(strcmp(val1, "nmsp1"));
   fail_if(strcmp(val2, "nmsp11"));
   eina_iterator_free(iter);

   fail_if(strcmp(eolian_class_name_get(class21), "class1"));
   fail_if(!(iter = eolian_class_namespaces_get(class21)));
   fail_if(!(eina_iterator_next(iter, (void**)&val1)));
   fail_if(eina_iterator_next(iter, &dummy));
   fail_if(strcmp(val1, "nmsp2"));
   eina_iterator_free(iter);

   fail_if(strcmp(eolian_class_name_get(class_no), "no_nmsp"));
   fail_if(eolian_class_namespaces_get(class_no));

   /* Inherits */
   fail_if(!(iter = eolian_class_inherits_get(class11)));
   fail_if(!(eina_iterator_next(iter, (void**)&class_name)));
   fail_if(eolian_class_get_by_name(class_name) != class112);
   fail_if(!(eina_iterator_next(iter, (void**)&class_name)));
   fail_if(eolian_class_get_by_name(class_name) != class21);
   fail_if(!(eina_iterator_next(iter, (void**)&class_name)));
   fail_if(eolian_class_get_by_name(class_name) != class_no);
   fail_if(eina_iterator_next(iter, &dummy));
   eina_iterator_free(iter);

   /* Implements */
   fail_if(!(iter = eolian_class_implements_get(class11)));
   fail_if(!(eina_iterator_next(iter, (void**)&impl)));
   fail_if(!(impl_class = eolian_implement_class_get(impl)));
   fail_if(!(fid = eolian_implement_function_get(impl, &func_type)));
   fail_if(impl_class != class112);
   fail_if(strcmp(eolian_function_name_get(fid), "a"));
   fail_if(func_type != EOLIAN_PROP_SET);

   fail_if(!(eina_iterator_next(iter, (void**)&impl)));
   fail_if(!(impl_class = eolian_implement_class_get(impl)));
   fail_if(!(fid = eolian_implement_function_get(impl, &func_type)));
   fail_if(impl_class != class_no);
   fail_if(strcmp(eolian_function_name_get(fid), "foo"));
   fail_if(func_type != EOLIAN_METHOD);
   fail_if(eina_iterator_next(iter, &dummy));
   eina_iterator_free(iter);

   /* Virtual regression */
   fail_if(!(fid = eolian_class_function_get_by_name(class112, "a", EOLIAN_UNRESOLVED)));
   fail_if(!eolian_function_is_virtual_pure(fid, EOLIAN_PROP_SET));

   eolian_shutdown();
}
END_TEST

START_TEST(eolian_events)
{
   const Eolian_Class *class;
   Eina_Iterator *iter;
   const char *name, *comment, *type_name;
   const Eolian_Type *type;
   const Eolian_Event *ev;
   void *dummy;

   eolian_init();
   /* Parsing */
   fail_if(!eolian_eo_file_parse(PACKAGE_DATA_DIR"/data/events.eo"));

   /* Class */
   fail_if(!(class = eolian_class_get_by_name("Events")));

   /* Events */
   fail_if(!(iter = eolian_class_events_get(class)));
   /* Clicked */
   fail_if(!(eina_iterator_next(iter, (void**)&ev)));
   fail_if(!(name = eolian_event_name_get(ev)));
   fail_if(eolian_event_type_get(ev));
   fail_if(!(comment = eolian_event_description_get(ev)));
   fail_if(strcmp(name, "clicked"));
   fail_if(strcmp(comment, "Comment for clicked"));
   /* Clicked,double */
   fail_if(!(eina_iterator_next(iter, (void**)&ev)));
   fail_if(!(name = eolian_event_name_get(ev)));
   fail_if(!(type = eolian_event_type_get(ev)));
   fail_if(comment = eolian_event_description_get(ev));
   fail_if(strcmp(name, "clicked,double"));
   type_name = eolian_type_name_get(type);
   fail_if(strcmp(type_name, "Evas_Event_Clicked_Double_Info"));
   fail_if(eina_iterator_next(iter, &dummy));
   eina_iterator_free(iter);

   eolian_shutdown();
}
END_TEST

START_TEST(eolian_override)
{
   Eina_Iterator *iter;
   const Eolian_Function *fid = NULL;
   const Eolian_Class *impl_class = NULL;
   const Eolian_Function *impl_func = NULL;
   const Eolian_Class *class, *base;
   const Eolian_Implement *impl;

   eolian_init();
   /* Parsing */
   fail_if(!eolian_directory_scan(PACKAGE_DATA_DIR"/data"));
   fail_if(!eolian_eo_file_parse(PACKAGE_DATA_DIR"/data/override.eo"));

   /* Class */
   fail_if(!(class = eolian_class_get_by_name("Override")));
   fail_if(!(base = eolian_class_get_by_name("Base")));

   /* Base ctor */
   fail_if(!(fid = eolian_class_function_get_by_name(base, "constructor", EOLIAN_UNRESOLVED)));
   fail_if(!eolian_function_is_virtual_pure(fid, EOLIAN_UNRESOLVED));

   /* Property */
   fail_if(!(fid = eolian_class_function_get_by_name(class, "a", EOLIAN_PROPERTY)));
   fail_if(!eolian_function_is_virtual_pure(fid, EOLIAN_PROP_SET));
   fail_if(eolian_function_is_virtual_pure(fid, EOLIAN_PROP_GET));

   /* Method */
   fail_if(!(fid = eolian_class_function_get_by_name(class, "foo", EOLIAN_METHOD)));
   fail_if(!eolian_function_is_virtual_pure(fid, EOLIAN_METHOD));

   /* Implements */
   fail_if(!(iter = eolian_class_implements_get(class)));

   fail_if(!(eina_iterator_next(iter, (void**)&impl)));
   fail_if(eolian_implement_is_auto(impl));
   fail_if(eolian_implement_is_empty(impl));
   fail_if(eolian_implement_is_virtual(impl));
   fail_if(!(impl_class = eolian_implement_class_get(impl)));
   fail_if(!(impl_func = eolian_implement_function_get(impl, NULL)));
   fail_if(impl_class != base);
   fail_if(strcmp(eolian_function_name_get(impl_func), "constructor"));

   fail_if(!(eina_iterator_next(iter, (void**)&impl)));
   fail_if(eolian_implement_is_auto(impl));
   fail_if(eolian_implement_is_empty(impl));
   fail_if(!eolian_implement_is_virtual(impl));
   fail_if(!(impl_class = eolian_implement_class_get(impl)));
   fail_if(!(impl_func = eolian_implement_function_get(impl, NULL)));
   fail_if(impl_class != class);
   fail_if(strcmp(eolian_function_name_get(impl_func), "a"));
   fail_if(eolian_function_is_auto(impl_func, EOLIAN_PROP_SET));
   fail_if(eolian_function_is_auto(impl_func, EOLIAN_PROP_GET));
   fail_if(eolian_function_is_empty(impl_func, EOLIAN_PROP_SET));
   fail_if(eolian_function_is_empty(impl_func, EOLIAN_PROP_GET));
   fail_if(!eolian_function_is_virtual_pure(impl_func, EOLIAN_PROP_SET));
   fail_if(eolian_function_is_virtual_pure(impl_func, EOLIAN_PROP_GET));

   fail_if(!(eina_iterator_next(iter, (void**)&impl)));
   fail_if(eolian_implement_is_auto(impl));
   fail_if(eolian_implement_is_empty(impl));
   fail_if(!eolian_implement_is_virtual(impl));
   fail_if(!(impl_class = eolian_implement_class_get(impl)));
   fail_if(!(impl_func = eolian_implement_function_get(impl, NULL)));
   fail_if(impl_class != class);
   fail_if(strcmp(eolian_function_name_get(impl_func), "foo"));
   fail_if(eolian_function_is_auto(impl_func, EOLIAN_METHOD));
   fail_if(eolian_function_is_empty(impl_func, EOLIAN_METHOD));
   fail_if(!eolian_function_is_virtual_pure(impl_func, EOLIAN_METHOD));

   fail_if(!(eina_iterator_next(iter, (void**)&impl)));
   fail_if(!eolian_implement_is_auto(impl));
   fail_if(eolian_implement_is_empty(impl));
   fail_if(eolian_implement_is_virtual(impl));
   fail_if(!(impl_class = eolian_implement_class_get(impl)));
   fail_if(!(impl_func = eolian_implement_function_get(impl, NULL)));
   fail_if(impl_class != class);
   fail_if(strcmp(eolian_function_name_get(impl_func), "b"));
   fail_if(!eolian_function_is_auto(impl_func, EOLIAN_PROP_SET));
   fail_if(eolian_function_is_auto(impl_func, EOLIAN_PROP_GET));
   fail_if(eolian_function_is_empty(impl_func, EOLIAN_PROP_SET));
   fail_if(eolian_function_is_empty(impl_func, EOLIAN_PROP_GET));
   fail_if(eolian_function_is_virtual_pure(impl_func, EOLIAN_PROP_SET));
   fail_if(eolian_function_is_virtual_pure(impl_func, EOLIAN_PROP_GET));

   fail_if(!(eina_iterator_next(iter, (void**)&impl)));
   fail_if(eolian_implement_is_auto(impl));
   fail_if(!eolian_implement_is_empty(impl));
   fail_if(eolian_implement_is_virtual(impl));
   fail_if(!(impl_class = eolian_implement_class_get(impl)));
   fail_if(!(impl_func = eolian_implement_function_get(impl, NULL)));
   fail_if(impl_class != class);
   fail_if(strcmp(eolian_function_name_get(impl_func), "bar"));
   fail_if(eolian_function_is_auto(impl_func, EOLIAN_METHOD));
   fail_if(!eolian_function_is_empty(impl_func, EOLIAN_METHOD));
   fail_if(eolian_function_is_virtual_pure(impl_func, EOLIAN_METHOD));

   eina_iterator_free(iter);

   eolian_shutdown();
}
END_TEST

START_TEST(eolian_consts)
{
   const Eolian_Function *fid = NULL;
   const Eolian_Function_Parameter *param = NULL;
   const Eolian_Class *class;

   eolian_init();
   /* Parsing */
   fail_if(!eolian_eo_file_parse(PACKAGE_DATA_DIR"/data/consts.eo"));
   fail_if(!(class = eolian_class_get_by_name("Consts")));

   /* Property */
   fail_if(!(fid = eolian_class_function_get_by_name(class, "a", EOLIAN_PROPERTY)));
   fail_if(!(param = eolian_function_parameter_get_by_name(fid, "buffer")));
   fail_if(eolian_parameter_const_attribute_get(param, EINA_FALSE));
   fail_if(!eolian_parameter_const_attribute_get(param, EINA_TRUE));

   /* Method */
   fail_if(!(fid = eolian_class_function_get_by_name(class, "foo", EOLIAN_METHOD)));
   fail_if(EINA_FALSE == eolian_function_object_is_const(fid));

   eolian_shutdown();
}
END_TEST

START_TEST(eolian_ctor_dtor)
{
   Eina_Iterator *iter;
   const Eolian_Class *impl_class = NULL;
   const Eolian_Function *impl_func = NULL;
   const Eolian_Class *class, *base;
   const Eolian_Implement *impl;
   const Eolian_Constructor *ctor;
   void *dummy;

   eolian_init();
   /* Parsing */
   fail_if(!eolian_directory_scan(PACKAGE_DATA_DIR"/data"));
   fail_if(!eolian_eo_file_parse(PACKAGE_DATA_DIR"/data/ctor_dtor.eo"));
   fail_if(!(class = eolian_class_get_by_name("Ctor_Dtor")));
   fail_if(!(base = eolian_class_get_by_name("Base")));

   /* Class ctor/dtor */
   fail_if(!eolian_class_ctor_enable_get(class));
   fail_if(!eolian_class_dtor_enable_get(class));

   /* Base ctor/dtor */
   fail_if(!(iter = eolian_class_implements_get(class)));
   fail_if(!(eina_iterator_next(iter, (void**)&impl)));
   fail_if(!(impl_class = eolian_implement_class_get(impl)));
   fail_if(!(impl_func = eolian_implement_function_get(impl, NULL)));
   fail_if(impl_class != base);
   fail_if(strcmp(eolian_function_name_get(impl_func), "constructor"));
   fail_if(!(eina_iterator_next(iter, (void**)&impl)));
   fail_if(!(impl_class = eolian_implement_class_get(impl)));
   fail_if(!(impl_func = eolian_implement_function_get(impl, NULL)));
   fail_if(impl_class != base);
   fail_if(strcmp(eolian_function_name_get(impl_func), "destructor"));
   eina_iterator_free(iter);

   /* Custom ctors/dtors */
   fail_if(!eolian_class_function_get_by_name(base, "destructor", EOLIAN_METHOD));
   fail_if(!(iter = eolian_class_constructors_get(class)));
   fail_if(!(eina_iterator_next(iter, (void**)&ctor)));
   fail_if(!(impl_class = eolian_constructor_class_get(ctor)));
   fail_if(!(impl_func = eolian_constructor_function_get(ctor)));
   fail_if(impl_class != class);
   fail_if(strcmp(eolian_function_name_get(impl_func), "custom_constructor_1"));
   fail_if(!(eina_iterator_next(iter, (void**)&ctor)));
   fail_if(!(impl_class = eolian_constructor_class_get(ctor)));
   fail_if(!(impl_func = eolian_constructor_function_get(ctor)));
   fail_if(impl_class != class);
   fail_if(strcmp(eolian_function_name_get(impl_func), "custom_constructor_2"));
   fail_if(eina_iterator_next(iter, &dummy));
   eina_iterator_free(iter);

   eolian_shutdown();
}
END_TEST

START_TEST(eolian_typedef)
{
   const Eolian_Type *atype = NULL, *type = NULL;
   const char *type_name = NULL;
   Eina_Iterator *iter = NULL;
   const Eolian_Class *class;
   const char *file;

   eolian_init();
   /* Parsing */
   fail_if(!eolian_eo_file_parse(PACKAGE_DATA_DIR"/data/typedef.eo"));

   /* Check that the class Dummy is still readable */
   fail_if(!(class = eolian_class_get_by_name("Typedef")));
   fail_if(!eolian_class_function_get_by_name(class, "foo", EOLIAN_METHOD));

   /* Basic type */
   fail_if(!(atype = eolian_type_alias_get_by_name("Evas.Coord")));
   fail_if(eolian_type_type_get(atype) != EOLIAN_TYPE_ALIAS);
   fail_if(!(type_name = eolian_type_name_get(atype)));
   fail_if(strcmp(type_name, "Coord"));
   fail_if(!(type_name = eolian_type_c_type_get(atype)));
   fail_if(strcmp(type_name, "typedef int Evas_Coord"));
   eina_stringshare_del(type_name);
   fail_if(!(type = eolian_type_base_type_get(atype)));
   fail_if(!(type_name = eolian_type_name_get(type)));
   fail_if(eolian_type_is_own(type));
   fail_if(eolian_type_is_const(type));
   fail_if(eolian_type_base_type_get(type));
   fail_if(strcmp(type_name, "int"));

   /* File */
   fail_if(!(file = eolian_type_file_get(atype)));
   fail_if(strcmp(file, "typedef.eo"));

   /* Complex type */
   fail_if(!(atype = eolian_type_alias_get_by_name("List_Objects")));
   fail_if(!(type_name = eolian_type_name_get(atype)));
   fail_if(strcmp(type_name, "List_Objects"));
   fail_if(!(type = eolian_type_base_type_get(atype)));
   fail_if(!(type_name = eolian_type_c_type_get(type)));
   fail_if(!eolian_type_is_own(type));
   fail_if(strcmp(type_name, "Eina_List *"));
   eina_stringshare_del(type_name);
   fail_if(!(iter = eolian_type_subtypes_get(type)));
   fail_if(!eina_iterator_next(iter, (void**)&type));
   fail_if(!(type_name = eolian_type_c_type_get(type)));
   fail_if(strcmp(type_name, "Eo *"));
   fail_if(eolian_type_is_own(type));
   eina_stringshare_del(type_name);
   eina_iterator_free(iter);

   /* List */
   fail_if(!(iter = eolian_type_aliases_get_by_file("typedef.eo")));
   fail_if(!eina_iterator_next(iter, (void**)&atype));
   fail_if(!(type_name = eolian_type_name_get(atype)));
   fail_if(strcmp(type_name, "Coord"));
   fail_if(!eina_iterator_next(iter, (void**)&atype));
   fail_if(!(type_name = eolian_type_name_get(atype)));
   fail_if(strcmp(type_name, "List_Objects"));
   fail_if(eina_iterator_next(iter, (void**)&atype));

   eolian_shutdown();
}
END_TEST

START_TEST(eolian_complex_type)
{
   const Eolian_Function *fid = NULL;
   const Eolian_Function_Parameter *param = NULL;
   const Eolian_Type *type = NULL;
   const char *type_name = NULL;
   Eina_Iterator *iter = NULL;
   const Eolian_Class *class;
   void *dummy;

   eolian_init();
   /* Parsing */
   fail_if(!eolian_eo_file_parse(PACKAGE_DATA_DIR"/data/complex_type.eo"));
   fail_if(!(class = eolian_class_get_by_name("Complex_Type")));

   /* Properties return type */
   fail_if(!(fid = eolian_class_function_get_by_name(class, "a", EOLIAN_PROPERTY)));
   fail_if(!(type = eolian_function_return_type_get(fid, EOLIAN_PROP_SET)));
   fail_if(!(type_name = eolian_type_c_type_get(type)));
   fail_if(!eolian_type_is_own(type));
   fail_if(strcmp(type_name, "Eina_List *"));
   eina_stringshare_del(type_name);
   fail_if(!(iter = eolian_type_subtypes_get(type)));
   fail_if(!eina_iterator_next(iter, (void**)&type));
   fail_if(!(type_name = eolian_type_c_type_get(type)));
   fail_if(eolian_type_is_own(type));
   fail_if(strcmp(type_name, "Eina_Array *"));
   eina_stringshare_del(type_name);
   eina_iterator_free(iter);
   fail_if(!(iter = eolian_type_subtypes_get(type)));
   fail_if(!eina_iterator_next(iter, (void**)&type));
   fail_if(!(type_name = eolian_type_c_type_get(type)));
   fail_if(!eolian_type_is_own(type));
   fail_if(strcmp(type_name, "Eo **"));
   eina_stringshare_del(type_name);
   eina_iterator_free(iter);
   /* Properties parameter type */
   fail_if(!(iter = eolian_function_parameters_get(fid)));
   fail_if(!(eina_iterator_next(iter, (void**)&param)));
   fail_if(eina_iterator_next(iter, &dummy));
   eina_iterator_free(iter);
   fail_if(strcmp(eolian_parameter_name_get(param), "value"));
   fail_if(!(type = eolian_parameter_type_get(param)));
   fail_if(!(type_name = eolian_type_c_type_get(type)));
   fail_if(!eolian_type_is_own(type));
   fail_if(strcmp(type_name, "Eina_List *"));
   eina_stringshare_del(type_name);
   fail_if(!(iter = eolian_type_subtypes_get(type)));
   fail_if(!eina_iterator_next(iter, (void**)&type));
   fail_if(!(type_name = eolian_type_c_type_get(type)));
   fail_if(eolian_type_is_own(type));
   fail_if(strcmp(type_name, "int"));
   eina_stringshare_del(type_name);
   eina_iterator_free(iter);

   /* Methods return type */
   fail_if(!(fid = eolian_class_function_get_by_name(class, "foo", EOLIAN_METHOD)));
   fail_if(!(type = eolian_function_return_type_get(fid, EOLIAN_METHOD)));
   fail_if(!(type_name = eolian_type_c_type_get(type)));
   fail_if(!eolian_type_is_own(type));
   fail_if(strcmp(type_name, "Eina_List *"));
   eina_stringshare_del(type_name);
   fail_if(!(iter = eolian_type_subtypes_get(type)));
   fail_if(!eina_iterator_next(iter, (void**)&type));
   fail_if(!(type_name = eolian_type_c_type_get(type)));
   fail_if(eolian_type_is_own(type));
   fail_if(strcmp(type_name, "Eina_Stringshare *"));
   eina_stringshare_del(type_name);
   eina_iterator_free(iter);
   /* Methods parameter type */
   fail_if(!(iter = eolian_function_parameters_get(fid)));
   fail_if(!(eina_iterator_next(iter, (void**)&param)));
   fail_if(eina_iterator_next(iter, &dummy));
   eina_iterator_free(iter);
   fail_if(strcmp(eolian_parameter_name_get(param), "buf"));
   fail_if(!(type = eolian_parameter_type_get(param)));
   fail_if(!(type_name = eolian_type_c_type_get(type)));
   fail_if(!eolian_type_is_own(type));
   fail_if(strcmp(type_name, "char *"));
   eina_stringshare_del(type_name);

   eolian_shutdown();
}
END_TEST

START_TEST(eolian_scope)
{
   const Eolian_Function *fid = NULL;
   const Eolian_Class *class;

   eolian_init();
   /* Parsing */
   fail_if(!eolian_eo_file_parse(PACKAGE_DATA_DIR"/data/scope.eo"));
   fail_if(!(class = eolian_class_get_by_name("Scope")));

   /* Property scope */
   fail_if(!(fid = eolian_class_function_get_by_name(class, "a", EOLIAN_PROPERTY)));
   fail_if(eolian_function_scope_get(fid) != EOLIAN_SCOPE_PROTECTED);
   fail_if(!(fid = eolian_class_function_get_by_name(class, "b", EOLIAN_PROPERTY)));
   fail_if(eolian_function_scope_get(fid) != EOLIAN_SCOPE_PUBLIC);
   fail_if(!(fid = eolian_class_function_get_by_name(class, "c", EOLIAN_PROPERTY)));
   fail_if(eolian_function_scope_get(fid) != EOLIAN_SCOPE_PUBLIC);

   /* Method scope */
   fail_if(!(fid = eolian_class_function_get_by_name(class, "foo", EOLIAN_METHOD)));
   fail_if(eolian_function_scope_get(fid) != EOLIAN_SCOPE_PUBLIC);
   fail_if(!(fid = eolian_class_function_get_by_name(class, "bar", EOLIAN_METHOD)));
   fail_if(eolian_function_scope_get(fid) != EOLIAN_SCOPE_PROTECTED);
   fail_if(!(fid = eolian_class_function_get_by_name(class, "foobar", EOLIAN_METHOD)));
   fail_if(eolian_function_scope_get(fid) != EOLIAN_SCOPE_PUBLIC);

   eolian_shutdown();
}
END_TEST

START_TEST(eolian_simple_parsing)
{
   const Eolian_Function *fid = NULL;
   const char *string = NULL, *ptype = NULL;
   const Eolian_Function_Parameter *param = NULL;
   const Eolian_Expression *expr = NULL;
   const Eolian_Class *class;
   const Eolian_Type *tp;
   Eina_Iterator *iter;
   Eolian_Value v;
   void *dummy;

   eolian_init();
   /* Parsing */
   fail_if(!eolian_eo_file_parse(PACKAGE_DATA_DIR"/data/class_simple.eo"));
   fail_if(!(class = eolian_class_get_by_name("Class_Simple")));
   fail_if(eolian_class_get_by_file("class_simple.eo") != class);
   fail_if(strcmp(eolian_class_file_get(class), "class_simple.eo"));

   /* Class */
   fail_if(eolian_class_type_get(class) != EOLIAN_CLASS_REGULAR);
   string = eolian_class_description_get(class);
   fail_if(!string);
   fail_if(strcmp(string, "Class Desc Simple"));
   fail_if(eolian_class_inherits_get(class) != NULL);
   fail_if(strcmp(eolian_class_legacy_prefix_get(class), "evas_object_simple"));
   fail_if(strcmp(eolian_class_eo_prefix_get(class), "evas_obj_simple"));
   fail_if(strcmp(eolian_class_data_type_get(class), "Evas_Simple_Data"));

   /* Property */
   fail_if(!(fid = eolian_class_function_get_by_name(class, "a", EOLIAN_PROPERTY)));
   fail_if(strcmp(eolian_function_name_get(fid), "a"));
   string = eolian_function_description_get(fid, EOLIAN_PROP_SET);
   fail_if(!string);
   fail_if(strcmp(string, "comment a.set"));
   string = eolian_function_description_get(fid, EOLIAN_PROP_GET);
   fail_if(string);
   /* Set return */
   tp = eolian_function_return_type_get(fid, EOLIAN_PROP_SET);
   fail_if(!tp);
   fail_if(strcmp(eolian_type_name_get(tp), "bool"));
   expr = eolian_function_return_default_value_get(fid, EOLIAN_PROP_SET);
   fail_if(!expr);
   v = eolian_expression_eval(expr, EOLIAN_MASK_BOOL);
   fail_if(v.type != EOLIAN_EXPR_BOOL);
   string = eolian_function_return_comment_get(fid, EOLIAN_PROP_SET);
   fail_if(!string);
   fail_if(strcmp(string, "comment for property set return"));
   /* Get return */
   tp = eolian_function_return_type_get(fid, EOLIAN_PROP_GET);
   fail_if(tp);
   string = eolian_function_return_comment_get(fid, EOLIAN_PROP_GET);
   fail_if(string);

   /* Function parameters */
   fail_if(eolian_property_keys_get(fid) != NULL);
   fail_if(!(iter = eolian_property_values_get(fid)));
   fail_if(!(eina_iterator_next(iter, (void**)&param)));
   fail_if(eina_iterator_next(iter, &dummy));
   eina_iterator_free(iter);
   fail_if(strcmp(eolian_type_name_get(eolian_parameter_type_get(param)), "int"));
   fail_if(strcmp(eolian_parameter_name_get(param), "value"));
   fail_if(strcmp(eolian_parameter_description_get(param), "Value description"));
   expr = eolian_parameter_default_value_get(param);
   fail_if(!expr);
   v = eolian_expression_eval(expr, EOLIAN_MASK_INT);
   fail_if(v.type != EOLIAN_EXPR_INT);
   fail_if(v.value.i != 100);

   /* legacy only */
   fail_if(!(fid = eolian_class_function_get_by_name(class, "b", EOLIAN_PROPERTY)));
   fail_if(eolian_function_is_legacy_only(fid, EOLIAN_PROP_GET));
   fail_if(!eolian_function_is_legacy_only(fid, EOLIAN_PROP_SET));

   /* Method */
   fail_if(!(fid = eolian_class_function_get_by_name(class, "foo", EOLIAN_METHOD)));
   string = eolian_function_description_get(fid, EOLIAN_METHOD);
   fail_if(!string);
   fail_if(strcmp(string, "comment foo"));
   /* Function return */
   tp = eolian_function_return_type_get(fid, EOLIAN_METHOD);
   fail_if(!tp);
   string = eolian_type_c_type_get(tp);
   fail_if(!string);
   fail_if(strcmp(string, "char *"));
   eina_stringshare_del(string);
   expr = eolian_function_return_default_value_get(fid, EOLIAN_METHOD);
   fail_if(!expr);
   v = eolian_expression_eval(expr, EOLIAN_MASK_NULL);
   fail_if(v.type != EOLIAN_EXPR_NULL);
   string = eolian_function_return_comment_get(fid, EOLIAN_METHOD);
   fail_if(!string);
   fail_if(strcmp(string, "comment for method return"));
   fail_if(eolian_function_is_legacy_only(fid, EOLIAN_METHOD));

   /* Function parameters */
   fail_if(!(iter = eolian_property_values_get(fid)));
   fail_if(!(eina_iterator_next(iter, (void**)&param)));
   fail_if(eolian_parameter_direction_get(param) != EOLIAN_IN_PARAM);
   fail_if(strcmp(eolian_type_name_get(eolian_parameter_type_get(param)), "int"));
   fail_if(strcmp(eolian_parameter_name_get(param), "a"));
   fail_if(strcmp(eolian_parameter_description_get(param), "a"));
   fail_if(!(eina_iterator_next(iter, (void**)&param)));
   fail_if(eolian_parameter_direction_get(param) != EOLIAN_INOUT_PARAM);
   ptype = eolian_type_name_get(eolian_parameter_type_get(param));
   fail_if(strcmp(ptype, "char"));
   fail_if(strcmp(eolian_parameter_name_get(param), "b"));
   fail_if(eolian_parameter_description_get(param));
   fail_if(!(eina_iterator_next(iter, (void**)&param)));
   fail_if(eolian_parameter_direction_get(param) != EOLIAN_OUT_PARAM);
   fail_if(strcmp(eolian_type_name_get(eolian_parameter_type_get(param)), "double"));
   fail_if(strcmp(eolian_parameter_name_get(param), "c"));
   fail_if(eolian_parameter_description_get(param));
   expr = eolian_parameter_default_value_get(param);
   fail_if(!expr);
   v = eolian_expression_eval(expr, EOLIAN_MASK_FLOAT);
   fail_if(v.type != EOLIAN_EXPR_DOUBLE);
   fail_if(v.value.d != 1337.6);
   fail_if(eina_iterator_next(iter, &dummy));
   eina_iterator_free(iter);

   /* legacy only */
   fail_if(!(fid = eolian_class_function_get_by_name(class, "bar", EOLIAN_METHOD)));
   fail_if(!eolian_function_is_legacy_only(fid, EOLIAN_METHOD));

   eolian_shutdown();
}
END_TEST

START_TEST(eolian_struct)
{
   const Eolian_Struct_Type_Field *field = NULL;
   const Eolian_Type *atype = NULL, *type = NULL, *ftype = NULL;
   const Eolian_Class *class;
   const char *type_name;
   const char *file;

   eolian_init();

   /* Parsing */
   fail_if(!eolian_eo_file_parse(PACKAGE_DATA_DIR"/data/struct.eo"));

   /* Check that the class Dummy is still readable */
   fail_if(!(class = eolian_class_get_by_name("Struct")));
   fail_if(!eolian_class_function_get_by_name(class, "foo", EOLIAN_METHOD));

   /* named struct */
   fail_if(!(type = eolian_type_struct_get_by_name("Named")));
   fail_if(!(type_name = eolian_type_name_get(type)));
   fail_if(!(file = eolian_type_file_get(type)));
   fail_if(eolian_type_type_get(type) != EOLIAN_TYPE_STRUCT);
   fail_if(eolian_type_is_own(type));
   fail_if(eolian_type_is_const(type));
   fail_if(strcmp(type_name, "Named"));
   fail_if(strcmp(file, "struct.eo"));
   fail_if(!(field = eolian_type_struct_field_get(type, "field")));
   fail_if(!(ftype = eolian_type_struct_field_type_get(field)));
   fail_if(!(type_name = eolian_type_name_get(ftype)));
   fail_if(strcmp(type_name, "int"));
   fail_if(!(field = eolian_type_struct_field_get(type, "something")));
   fail_if(!(ftype = eolian_type_struct_field_type_get(field)));
   fail_if(!(type_name = eolian_type_c_type_get(ftype)));
   fail_if(strcmp(type_name, "const char *"));
   eina_stringshare_del(type_name);

   /* referencing */
   fail_if(!(type = eolian_type_struct_get_by_name("Another")));
   fail_if(!(type_name = eolian_type_name_get(type)));
   fail_if(!(file = eolian_type_file_get(type)));
   fail_if(eolian_type_type_get(type) != EOLIAN_TYPE_STRUCT);
   fail_if(strcmp(type_name, "Another"));
   fail_if(strcmp(file, "struct.eo"));
   fail_if(!(field = eolian_type_struct_field_get(type, "field")));
   fail_if(!(ftype = eolian_type_struct_field_type_get(field)));
   fail_if(!(type_name = eolian_type_name_get(ftype)));
   fail_if(strcmp(type_name, "Named"));
   fail_if(eolian_type_type_get(ftype) != EOLIAN_TYPE_REGULAR_STRUCT);

   /* typedef */
   fail_if(!(atype = eolian_type_alias_get_by_name("Foo")));
   fail_if(!(type = eolian_type_base_type_get(atype)));
   fail_if(!(type_name = eolian_type_name_get(type)));
   fail_if(eolian_type_type_get(type) != EOLIAN_TYPE_STRUCT);
   fail_if(strcmp(type_name, "_Foo"));

   /* typedef - anon */
   fail_if(!(atype = eolian_type_alias_get_by_name("Bar")));
   fail_if(!(type = eolian_type_base_type_get(atype)));
   fail_if(!!(type_name = eolian_type_name_get(type)));
   fail_if(eolian_type_type_get(type) != EOLIAN_TYPE_STRUCT);

   /* opaque struct */
   fail_if(!(type = eolian_type_struct_get_by_name("Opaque")));
   fail_if(eolian_type_type_get(type) != EOLIAN_TYPE_STRUCT_OPAQUE);

   eolian_shutdown();
}
END_TEST

START_TEST(eolian_extern)
{
   const Eolian_Type *type = NULL;
   const Eolian_Class *class;

   eolian_init();

   /* Parsing */
   fail_if(!eolian_eo_file_parse(PACKAGE_DATA_DIR"/data/extern.eo"));

   /* Check that the class Dummy is still readable */
   fail_if(!(class = eolian_class_get_by_name("Extern")));
   fail_if(!eolian_class_function_get_by_name(class, "foo", EOLIAN_METHOD));

   /* regular type */
   fail_if(!(type = eolian_type_alias_get_by_name("Foo")));
   fail_if(eolian_type_is_extern(type));

   /* extern type */
   fail_if(!(type = eolian_type_alias_get_by_name("Evas.Coord")));
   fail_if(!eolian_type_is_extern(type));

   /* regular struct */
   fail_if(!(type = eolian_type_struct_get_by_name("X")));
   fail_if(eolian_type_is_extern(type));

   /* extern struct */
   fail_if(!(type = eolian_type_struct_get_by_name("Y")));
   fail_if(!eolian_type_is_extern(type));

   eolian_shutdown();
}
END_TEST

START_TEST(eolian_var)
{
   const Eolian_Variable *var = NULL;
   const Eolian_Expression *exp = NULL;
   const Eolian_Type *type = NULL;
   const Eolian_Class *class;
   Eolian_Value v;
   const char *name;

   eolian_init();

   /* Parsing */
   fail_if(!eolian_eo_file_parse(PACKAGE_DATA_DIR"/data/var.eo"));

   /* Check that the class Dummy is still readable */
   fail_if(!(class = eolian_class_get_by_name("Var")));
   fail_if(!eolian_class_function_get_by_name(class, "foo", EOLIAN_METHOD));

   /* regular constant */
   fail_if(!(var = eolian_variable_constant_get_by_name("Foo")));
   fail_if(eolian_variable_type_get(var) != EOLIAN_VAR_CONSTANT);
   fail_if(eolian_variable_is_extern(var));
   fail_if(!(type = eolian_variable_base_type_get(var)));
   fail_if(!(name = eolian_type_name_get(type)));
   fail_if(strcmp(name, "int"));
   fail_if(!(exp = eolian_variable_value_get(var)));
   v = eolian_expression_eval_type(exp, type);
   fail_if(v.type != EOLIAN_EXPR_INT);
   fail_if(v.value.i != 5);

   /* regular global */
   fail_if(!(var = eolian_variable_global_get_by_name("Bar")));
   fail_if(eolian_variable_type_get(var) != EOLIAN_VAR_GLOBAL);
   fail_if(eolian_variable_is_extern(var));
   fail_if(!(type = eolian_variable_base_type_get(var)));
   fail_if(!(name = eolian_type_name_get(type)));
   fail_if(strcmp(name, "float"));
   fail_if(!(exp = eolian_variable_value_get(var)));
   v = eolian_expression_eval_type(exp, type);
   fail_if(v.type != EOLIAN_EXPR_FLOAT);
   fail_if(((int)v.value.f) != 10);

   /* no-value global */
   fail_if(!(var = eolian_variable_global_get_by_name("Baz")));
   fail_if(eolian_variable_type_get(var) != EOLIAN_VAR_GLOBAL);
   fail_if(eolian_variable_is_extern(var));
   fail_if(!(type = eolian_variable_base_type_get(var)));
   fail_if(!(name = eolian_type_name_get(type)));
   fail_if(strcmp(name, "long"));
   fail_if(eolian_variable_value_get(var));

   /* extern global  */
   fail_if(!(var = eolian_variable_global_get_by_name("Bah")));
   fail_if(eolian_variable_type_get(var) != EOLIAN_VAR_GLOBAL);
   fail_if(!eolian_variable_is_extern(var));
   fail_if(!(type = eolian_variable_base_type_get(var)));
   fail_if(!(name = eolian_type_name_get(type)));
   fail_if(strcmp(name, "double"));
   fail_if(eolian_variable_value_get(var));

   eolian_shutdown();
}
END_TEST

START_TEST(eolian_enum)
{
   const Eolian_Enum_Type_Field *field = NULL;
   const Eolian_Variable *var = NULL;
   const Eolian_Type *type = NULL;
   const Eolian_Class *class;
   const Eolian_Expression *exp;
   const char *name;
   Eolian_Value v;

   eolian_init();

   /* Parsing */
   fail_if(!eolian_eo_file_parse(PACKAGE_DATA_DIR"/data/enum.eo"));

   /* Check that the class Dummy is still readable */
   fail_if(!(class = eolian_class_get_by_name("Enum")));
   fail_if(!eolian_class_function_get_by_name(class, "foo", EOLIAN_METHOD));

   fail_if(!(type = eolian_type_enum_get_by_name("Foo")));

   fail_if(!(field = eolian_type_enum_field_get(type, "first")));
   fail_if(!(exp = eolian_type_enum_field_value_get(field)));
   v = eolian_expression_eval(exp, EOLIAN_MASK_ALL);
   fail_if(v.type != EOLIAN_EXPR_INT);
   fail_if(v.value.i != 0);

   fail_if(!(field = eolian_type_enum_field_get(type, "bar")));
   fail_if(eolian_type_enum_field_value_get(field));

   fail_if(!(field = eolian_type_enum_field_get(type, "baz")));
   fail_if(!(exp = eolian_type_enum_field_value_get(field)));
   v = eolian_expression_eval(exp, EOLIAN_MASK_ALL);
   fail_if(v.type != EOLIAN_EXPR_INT);
   fail_if(v.value.i != 15);

   fail_if(!(type = eolian_type_enum_get_by_name("Bar")));

   fail_if(!(field = eolian_type_enum_field_get(type, "foo")));
   fail_if(!(exp = eolian_type_enum_field_value_get(field)));
   v = eolian_expression_eval(exp, EOLIAN_MASK_ALL);
   fail_if(v.type != EOLIAN_EXPR_INT);
   fail_if(v.value.i != 15);

   fail_if(!(type = eolian_type_alias_get_by_name("Baz")));
   fail_if(!(type = eolian_type_base_type_get(type)));

   fail_if(!(field = eolian_type_enum_field_get(type, "flag1")));
   fail_if(!(exp = eolian_type_enum_field_value_get(field)));
   v = eolian_expression_eval(exp, EOLIAN_MASK_ALL);
   fail_if(v.type != EOLIAN_EXPR_INT);
   fail_if(v.value.i != (1 << 0));

   fail_if(!(field = eolian_type_enum_field_get(type, "flag2")));
   fail_if(!(exp = eolian_type_enum_field_value_get(field)));
   v = eolian_expression_eval(exp, EOLIAN_MASK_ALL);
   fail_if(v.type != EOLIAN_EXPR_INT);
   fail_if(v.value.i != (1 << 1));

   fail_if(!(field = eolian_type_enum_field_get(type, "flag3")));
   fail_if(!(exp = eolian_type_enum_field_value_get(field)));
   v = eolian_expression_eval(exp, EOLIAN_MASK_ALL);
   fail_if(v.type != EOLIAN_EXPR_INT);
   fail_if(v.value.i != (1 << 2));

   fail_if(!(var = eolian_variable_constant_get_by_name("Bah")));
   fail_if(eolian_variable_type_get(var) != EOLIAN_VAR_CONSTANT);
   fail_if(eolian_variable_is_extern(var));
   fail_if(!(type = eolian_variable_base_type_get(var)));
   fail_if(!(name = eolian_type_name_get(type)));
   fail_if(strcmp(name, "int"));
   fail_if(!(exp = eolian_variable_value_get(var)));
   v = eolian_expression_eval(exp, EOLIAN_MASK_ALL);
   fail_if(v.type != EOLIAN_EXPR_INT);
   fail_if(v.value.i != (1 << 0));

   eolian_shutdown();
}
END_TEST

START_TEST(eolian_class_funcs)
{
   const Eolian_Function *fid = NULL;
   const Eolian_Class *class;

   eolian_init();
   /* Parsing */
   fail_if(!eolian_eo_file_parse(PACKAGE_DATA_DIR"/data/class_funcs.eo"));
   fail_if(!(class = eolian_class_get_by_name("Class_Funcs")));

   /* Class properties */
   fail_if(!(fid = eolian_class_function_get_by_name(class, "a", EOLIAN_PROPERTY)));
   fail_if(!eolian_function_is_class(fid));
   fail_if(!(fid = eolian_class_function_get_by_name(class, "b", EOLIAN_PROPERTY)));
   fail_if(eolian_function_is_class(fid));

   /* Class methods */
   fail_if(!(fid = eolian_class_function_get_by_name(class, "foo", EOLIAN_METHOD)));
   fail_if(!eolian_function_is_class(fid));
   fail_if(eolian_function_scope_get(fid) != EOLIAN_SCOPE_PUBLIC);
   fail_if(!(fid = eolian_class_function_get_by_name(class, "bar", EOLIAN_METHOD)));
   fail_if(eolian_function_is_class(fid));
   fail_if(eolian_function_scope_get(fid) != EOLIAN_SCOPE_PUBLIC);
   fail_if(!(fid = eolian_class_function_get_by_name(class, "baz", EOLIAN_METHOD)));
   fail_if(!eolian_function_is_class(fid));
   fail_if(eolian_function_scope_get(fid) != EOLIAN_SCOPE_PROTECTED);
   fail_if(!(fid = eolian_class_function_get_by_name(class, "bah", EOLIAN_METHOD)));
   fail_if(eolian_function_is_class(fid));
   fail_if(eolian_function_scope_get(fid) != EOLIAN_SCOPE_PROTECTED);

   eolian_shutdown();
}
END_TEST

START_TEST(eolian_free_func)
{
   const Eolian_Class *class;
   const Eolian_Type *type;

   eolian_init();

   /* Parsing */
   fail_if(!eolian_eo_file_parse(PACKAGE_DATA_DIR"/data/free_func.eo"));

   /* Check that the class Dummy is still readable */
   fail_if(!(class = eolian_class_get_by_name("Free_Func")));
   fail_if(!eolian_class_function_get_by_name(class, "foo", EOLIAN_METHOD));

   /* regular struct */
   fail_if(!(type = eolian_type_struct_get_by_name("Named1")));
   fail_if(eolian_type_free_func_get(type));
   fail_if(!(type = eolian_type_struct_get_by_name("Named2")));
   fail_if(strcmp(eolian_type_free_func_get(type), "test_free"));

   /* typedef */
   fail_if(!(type = eolian_type_alias_get_by_name("Typedef1")));
   fail_if(eolian_type_free_func_get(type));
   fail_if(!(type = eolian_type_alias_get_by_name("Typedef2")));
   fail_if(strcmp(eolian_type_free_func_get(type), "def_free"));

   /* anon struct */
   fail_if(!(type = eolian_type_alias_get_by_name("Anon1")));
   fail_if(!(type = eolian_type_base_type_get(type)));
   fail_if(eolian_type_free_func_get(type));
   fail_if(!(type = eolian_type_alias_get_by_name("Anon2")));
   fail_if(!(type = eolian_type_base_type_get(type)));
   fail_if(strcmp(eolian_type_free_func_get(type), "anon_free"));

   /* opaque struct */
   fail_if(!(type = eolian_type_struct_get_by_name("Opaque1")));
   fail_if(eolian_type_free_func_get(type));
   fail_if(!(type = eolian_type_struct_get_by_name("Opaque2")));
   fail_if(strcmp(eolian_type_free_func_get(type), "opaque_free"));

   /* pointer */
   fail_if(!(type = eolian_type_alias_get_by_name("Pointer1")));
   fail_if(!(type = eolian_type_base_type_get(type)));
   fail_if(eolian_type_free_func_get(type));
   fail_if(!(type = eolian_type_alias_get_by_name("Pointer2")));
   fail_if(!(type = eolian_type_base_type_get(type)));
   fail_if(strcmp(eolian_type_free_func_get(type), "ptr_free"));

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
   tcase_add_test(tc, eolian_struct);
   tcase_add_test(tc, eolian_extern);
   tcase_add_test(tc, eolian_var);
   tcase_add_test(tc, eolian_enum);
   tcase_add_test(tc, eolian_class_funcs);
   tcase_add_test(tc, eolian_free_func);
}

