#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>

#include <Eina.h>
#include <Eolian.h>

#include "eolian_suite.h"

EFL_START_TEST(eolian_namespaces)
{
   const Eolian_Class *class11, *class112, *class21, *class_no, *impl_class,
                      *iclass;
   const Eolian_Function *fid;
   Eina_Iterator *iter;
   Eolian_Function_Type func_type;
   const char *val1, *val2;
   const Eolian_Implement *impl;
   const Eolian_Unit *unit;
   void *dummy;

   Eolian_State *eos = eolian_state_new();
   /* Parsing */
   fail_if(!eolian_state_directory_add(eos, TESTS_SRC_DIR"/data"));
   fail_if(!(unit = eolian_state_file_parse(eos, TESTS_SRC_DIR"/data/nmsp1_class1.eo")));

   /* Classes existence  */
   fail_if(!(class11 = eolian_unit_class_by_name_get(unit, "nmsp1.class1")));
   fail_if(!(class112 = eolian_unit_class_by_name_get(unit, "nmsp1.nmsp11.class2")));
   fail_if(!(class21 = eolian_unit_class_by_name_get(unit, "nmsp2.class1")));
   fail_if(!(class_no = eolian_unit_class_by_name_get(unit, "no_nmsp")));

   /* Check names and namespaces*/
   fail_if(strcmp(eolian_class_short_name_get(class11), "class1"));
   fail_if(!(iter = eolian_class_namespaces_get(class11)));
   fail_if(!(eina_iterator_next(iter, (void**)&val1)));
   fail_if(eina_iterator_next(iter, &dummy));
   fail_if(strcmp(val1, "nmsp1"));
   eina_iterator_free(iter);

   fail_if(strcmp(eolian_class_short_name_get(class112), "class2"));
   fail_if(!(iter = eolian_class_namespaces_get(class112)));
   fail_if(!(eina_iterator_next(iter, (void**)&val1)));
   fail_if(!(eina_iterator_next(iter, (void**)&val2)));
   fail_if(eina_iterator_next(iter, &dummy));
   fail_if(strcmp(val1, "nmsp1"));
   fail_if(strcmp(val2, "nmsp11"));
   eina_iterator_free(iter);

   fail_if(strcmp(eolian_class_short_name_get(class21), "class1"));
   fail_if(!(iter = eolian_class_namespaces_get(class21)));
   fail_if(!(eina_iterator_next(iter, (void**)&val1)));
   fail_if(eina_iterator_next(iter, &dummy));
   fail_if(strcmp(val1, "nmsp2"));
   eina_iterator_free(iter);

   fail_if(strcmp(eolian_class_short_name_get(class_no), "no_nmsp"));
   fail_if(eolian_class_namespaces_get(class_no));

   /* Inherits */
   fail_if(eolian_class_parent_get(class11) != class112);
   fail_if(!(iter = eolian_class_extensions_get(class11)));
   fail_if(!(eina_iterator_next(iter, (void**)&iclass)));
   fail_if(iclass != class21);
   fail_if(!(eina_iterator_next(iter, (void**)&iclass)));
   fail_if(iclass != class_no);
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

   eolian_state_free(eos);
}
EFL_END_TEST

EFL_START_TEST(eolian_events)
{
   const Eolian_Class *class;
   Eina_Iterator *iter;
   const char *name, *type_name;
   const Eolian_Type *type;
   const Eolian_Event *ev;
   const Eolian_Unit *unit;
   void *dummy;

   Eolian_State *eos = eolian_state_new();
   /* Parsing */
   fail_if(!eolian_state_directory_add(eos, TESTS_SRC_DIR"/data"));
   fail_if(!(unit = eolian_state_file_parse(eos, TESTS_SRC_DIR"/data/events.eo")));

   /* Class */
   fail_if(!(class = eolian_unit_class_by_name_get(unit, "Events")));
   fail_if(strcmp(eolian_class_event_prefix_get(class), "totally_not_events"));

   /* Events */
   fail_if(!(iter = eolian_class_events_get(class)));
   /* Clicked */
   fail_if(!(eina_iterator_next(iter, (void**)&ev)));
   fail_if(!(name = eolian_event_name_get(ev)));
   fail_if(eolian_event_type_get(ev));
   fail_if(strcmp(name, "clicked"));
   fail_if(!eolian_event_is_beta(ev));
   fail_if(eolian_event_is_hot(ev));
   fail_if(eolian_event_is_restart(ev));
   /* Clicked,double */
   fail_if(!(eina_iterator_next(iter, (void**)&ev)));
   fail_if(!(name = eolian_event_name_get(ev)));
   fail_if(!(type = eolian_event_type_get(ev)));
   fail_if(strcmp(name, "clicked,double"));
   type_name = eolian_type_short_name_get(type);
   fail_if(strcmp(type_name, "Evas_Event_Clicked_Double_Info"));
   fail_if(eolian_event_is_beta(ev));
   fail_if(eolian_event_is_hot(ev));
   fail_if(eolian_event_is_restart(ev));
   /* Hot */
   fail_if(!(eina_iterator_next(iter, (void**)&ev)));
   fail_if(!(name = eolian_event_name_get(ev)));
   fail_if(eolian_event_type_get(ev));
   fail_if(strcmp(name, "hot"));
   fail_if(eolian_event_is_beta(ev));
   fail_if(!eolian_event_is_hot(ev));
   fail_if(eolian_event_is_restart(ev));
   /* Restart */
   fail_if(!(eina_iterator_next(iter, (void**)&ev)));
   fail_if(!(name = eolian_event_name_get(ev)));
   fail_if(eolian_event_type_get(ev));
   fail_if(strcmp(name, "restart"));
   fail_if(eolian_event_is_beta(ev));
   fail_if(eolian_event_is_hot(ev));
   fail_if(!eolian_event_is_restart(ev));
   /* Hot Restart */
   fail_if(!(eina_iterator_next(iter, (void**)&ev)));
   fail_if(!(name = eolian_event_name_get(ev)));
   fail_if(eolian_event_type_get(ev));
   fail_if(strcmp(name, "hot_restart"));
   fail_if(eolian_event_is_beta(ev));
   fail_if(!eolian_event_is_hot(ev));
   fail_if(!eolian_event_is_restart(ev));

   fail_if(eina_iterator_next(iter, &dummy));
   eina_iterator_free(iter);
   /* Check eolian_class_event_by_name_get */
   fail_if(!eolian_class_event_by_name_get(class, "clicked,double"));
   fail_if(eolian_class_event_by_name_get(class, "clicked,triple"));

   eolian_state_free(eos);
}
EFL_END_TEST

EFL_START_TEST(eolian_override)
{
   Eina_Iterator *iter;
   const Eolian_Class *impl_class = NULL;
   const Eolian_Function *impl_func = NULL;
   const Eolian_Class *class, *base;
   const Eolian_Implement *impl;
   const Eolian_Unit *unit;

   Eolian_State *eos = eolian_state_new();
   /* Parsing */
   fail_if(!eolian_state_directory_add(eos, TESTS_SRC_DIR"/data"));
   fail_if(!(unit = eolian_state_file_parse(eos, TESTS_SRC_DIR"/data/override.eo")));

   /* Class */
   fail_if(!(class = eolian_unit_class_by_name_get(unit, "Override")));
   fail_if(!(base = eolian_unit_class_by_name_get(unit, "Base")));

   /* Implements */
   fail_if(!(iter = eolian_class_implements_get(class)));
   fail_if(!(eina_iterator_next(iter, (void**)&impl))); /* skip a */

   fail_if(!(eina_iterator_next(iter, (void**)&impl)));
   fail_if(!eolian_implement_is_auto(impl, EOLIAN_PROP_SET));
   fail_if(eolian_implement_is_empty(impl, EOLIAN_PROP_SET));
   fail_if(eolian_implement_is_pure_virtual(impl, EOLIAN_PROP_SET));
   fail_if(!(impl_class = eolian_implement_class_get(impl)));
   fail_if(!(impl_func = eolian_implement_function_get(impl, NULL)));
   fail_if(impl_class != class);
   fail_if(eolian_implement_implementing_class_get(impl) != class);
   fail_if(strcmp(eolian_function_name_get(impl_func), "b"));

   fail_if(!(eina_iterator_next(iter, (void**)&impl))); /* skip c */
   fail_if(!(eina_iterator_next(iter, (void**)&impl))); /* skip foo */

   fail_if(!(eina_iterator_next(iter, (void**)&impl)));
   fail_if(eolian_implement_is_auto(impl, EOLIAN_METHOD));
   fail_if(!eolian_implement_is_empty(impl, EOLIAN_METHOD));
   fail_if(eolian_implement_is_pure_virtual(impl, EOLIAN_METHOD));
   fail_if(!(impl_class = eolian_implement_class_get(impl)));
   fail_if(!(impl_func = eolian_implement_function_get(impl, NULL)));
   fail_if(impl_class != class);
   fail_if(eolian_implement_implementing_class_get(impl) != class);
   fail_if(strcmp(eolian_function_name_get(impl_func), "bar"));

   fail_if(!(eina_iterator_next(iter, (void**)&impl)));
   fail_if(eolian_implement_is_auto(impl, EOLIAN_METHOD));
   fail_if(eolian_implement_is_empty(impl, EOLIAN_METHOD));
   fail_if(eolian_implement_is_pure_virtual(impl, EOLIAN_METHOD));
   fail_if(!(impl_class = eolian_implement_class_get(impl)));
   fail_if(!(impl_func = eolian_implement_function_get(impl, NULL)));
   fail_if(impl_class != base);
   fail_if(eolian_implement_implementing_class_get(impl) != class);
   fail_if(strcmp(eolian_function_name_get(impl_func), "constructor"));

   eina_iterator_free(iter);

   eolian_state_free(eos);
}
EFL_END_TEST

EFL_START_TEST(eolian_consts)
{
   const Eolian_Function *fid = NULL;
   const Eolian_Class *class;
   const Eolian_Unit *unit;

   Eolian_State *eos = eolian_state_new();
   /* Parsing */
   fail_if(!eolian_state_directory_add(eos, TESTS_SRC_DIR"/data"));
   fail_if(!(unit = eolian_state_file_parse(eos, TESTS_SRC_DIR"/data/consts.eo")));
   fail_if(!(class = eolian_unit_class_by_name_get(unit, "Consts")));

   /* Method */
   fail_if(!(fid = eolian_class_function_by_name_get(class, "foo", EOLIAN_METHOD)));
   fail_if(EINA_FALSE == eolian_function_object_is_const(fid));

   eolian_state_free(eos);
}
EFL_END_TEST

EFL_START_TEST(eolian_ctor_dtor)
{
   Eina_Iterator *iter;
   const Eolian_Class *impl_class = NULL;
   const Eolian_Function *impl_func = NULL;
   const Eolian_Class *class, *base;
   const Eolian_Implement *impl;
   const Eolian_Constructor *ctor;
   const Eolian_Unit *unit;
   void *dummy;

   Eolian_State *eos = eolian_state_new();
   /* Parsing */
   fail_if(!eolian_state_directory_add(eos, TESTS_SRC_DIR"/data"));
   fail_if(!(unit = eolian_state_file_parse(eos, TESTS_SRC_DIR"/data/ctor_dtor.eo")));
   fail_if(!(class = eolian_unit_class_by_name_get(unit, "Ctor_Dtor")));
   fail_if(!(base = eolian_unit_class_by_name_get(unit, "Base")));

   /* Class ctor/dtor */
   fail_if(!eolian_class_ctor_enable_get(class));
   fail_if(!eolian_class_dtor_enable_get(class));

   /* Base ctor/dtor */
   fail_if(!(iter = eolian_class_implements_get(class)));
   fail_if(!(eina_iterator_next(iter, (void**)&impl)));
   fail_if(!(eina_iterator_next(iter, (void**)&impl)));
   fail_if(!(eina_iterator_next(iter, (void**)&impl)));
   fail_if(!(eina_iterator_next(iter, (void**)&impl)));
   fail_if(!(impl_class = eolian_implement_class_get(impl)));
   fail_if(!(impl_func = eolian_implement_function_get(impl, NULL)));
   fail_if(impl_class != base);
   fail_if(strcmp(eolian_function_name_get(impl_func), "constructor"));
   fail_if(!eolian_function_is_constructor(impl_func, base));
   fail_if(!eolian_function_is_constructor(impl_func, class));
   fail_if(!(eina_iterator_next(iter, (void**)&impl)));
   fail_if(!(impl_class = eolian_implement_class_get(impl)));
   fail_if(!(impl_func = eolian_implement_function_get(impl, NULL)));
   fail_if(impl_class != base);
   fail_if(strcmp(eolian_function_name_get(impl_func), "destructor"));
   fail_if(eolian_function_is_constructor(impl_func, base));
   fail_if(eolian_function_is_constructor(impl_func, class));
   eina_iterator_free(iter);

   /* Custom ctors/dtors */
   fail_if(!eolian_class_function_by_name_get(base, "destructor", EOLIAN_METHOD));
   fail_if(!(iter = eolian_class_constructors_get(class)));
   fail_if(!(eina_iterator_next(iter, (void**)&ctor)));
   fail_if(eolian_constructor_is_optional(ctor));
   fail_if(!(impl_class = eolian_constructor_class_get(ctor)));
   fail_if(!(impl_func = eolian_constructor_function_get(ctor)));
   fail_if(impl_class != class);
   fail_if(strcmp(eolian_function_name_get(impl_func), "custom_constructor_1"));
   fail_if(!eolian_function_is_constructor(impl_func, class));
   fail_if(eolian_function_is_constructor(impl_func, base));
   fail_if(!(eina_iterator_next(iter, (void**)&ctor)));
   fail_if(!eolian_constructor_is_optional(ctor));
   fail_if(!(impl_class = eolian_constructor_class_get(ctor)));
   fail_if(!(impl_func = eolian_constructor_function_get(ctor)));
   fail_if(impl_class != class);
   fail_if(strcmp(eolian_function_name_get(impl_func), "custom_constructor_2"));
   fail_if(!eolian_function_is_constructor(impl_func, class));
   fail_if(eolian_function_is_constructor(impl_func, base));
   fail_if(!(eina_iterator_next(iter, (void**)&ctor)));
   fail_if(!eolian_constructor_is_ctor_param(ctor));
   fail_if(!(impl_class = eolian_constructor_class_get(ctor)));
   fail_if(!(impl_func = eolian_constructor_function_get(ctor)));
   fail_if(impl_class != class);
   fail_if(strcmp(eolian_function_name_get(impl_func), "custom_constructor_3"));
   fail_if(!eolian_function_is_constructor(impl_func, class));
   fail_if(eolian_function_is_constructor(impl_func, base));
   fail_if(eina_iterator_next(iter, &dummy));
   eina_iterator_free(iter);

   eolian_state_free(eos);
}
EFL_END_TEST

EFL_START_TEST(eolian_typedef)
{
   const Eolian_Type *type = NULL;
   const Eolian_Typedecl *tdl = NULL;
   const char *type_name = NULL;
   Eina_Iterator *iter = NULL;
   const Eolian_Class *class;
   const Eolian_Unit *unit;
   const char *file;

   Eolian_State *eos = eolian_state_new();
   /* Parsing */
   fail_if(!eolian_state_directory_add(eos, TESTS_SRC_DIR"/data"));
   fail_if(!(unit = eolian_state_file_parse(eos, TESTS_SRC_DIR"/data/typedef.eo")));

   /* Check that the class Dummy is still readable */
   fail_if(!(class = eolian_unit_class_by_name_get(unit, "Typedef")));
   fail_if(!eolian_class_function_by_name_get(class, "foo", EOLIAN_METHOD));

   /* Basic type */
   fail_if(!(tdl = eolian_unit_alias_by_name_get(unit, "Evas.Coord")));
   fail_if(eolian_typedecl_type_get(tdl) != EOLIAN_TYPEDECL_ALIAS);
   fail_if(!(type_name = eolian_typedecl_short_name_get(tdl)));
   fail_if(strcmp(type_name, "Coord"));
   fail_if(!(type_name = eolian_typedecl_c_type_get(tdl)));
   fail_if(strcmp(type_name, "typedef int Evas_Coord"));
   eina_stringshare_del(type_name);
   fail_if(!(type = eolian_typedecl_base_type_get(tdl)));
   fail_if(!(type_name = eolian_type_short_name_get(type)));
   fail_if(eolian_type_is_owned(type));
   fail_if(eolian_type_is_const(type));
   fail_if(eolian_type_base_type_get(type));
   fail_if(strcmp(type_name, "int"));

   /* File */
   fail_if(!(file = eolian_object_file_get((const Eolian_Object *)tdl)));
   fail_if(strcmp(file, "typedef.eo"));

   /* Lowest alias base */
   fail_if(!(tdl = eolian_unit_alias_by_name_get(unit, "Evas.Coord3")));
   fail_if(!(type = eolian_typedecl_aliased_base_get(tdl)));
   fail_if(strcmp(eolian_type_short_name_get(type), "int"));

   /* Complex type */
   fail_if(!(tdl = eolian_unit_alias_by_name_get(unit, "List_Objects")));
   fail_if(!(type_name = eolian_typedecl_short_name_get(tdl)));
   fail_if(strcmp(type_name, "List_Objects"));
   fail_if(!(type = eolian_typedecl_base_type_get(tdl)));
   fail_if(!(type_name = eolian_type_c_type_get(type, EOLIAN_C_TYPE_DEFAULT)));
   fail_if(eolian_type_is_owned(type));
   fail_if(strcmp(type_name, "Eina_List *"));
   eina_stringshare_del(type_name);
   fail_if(!(type = eolian_type_base_type_get(type)));
   fail_if(!!eolian_type_next_type_get(type));
   fail_if(!(type_name = eolian_type_c_type_get(type, EOLIAN_C_TYPE_DEFAULT)));
   fail_if(strcmp(type_name, "Eo *"));
   fail_if(eolian_type_is_owned(type));
   eina_stringshare_del(type_name);

   /* List */
   fail_if(!(iter = eolian_state_aliases_by_file_get(eos, "typedef.eo")));
   fail_if(!eina_iterator_next(iter, (void**)&tdl));
   fail_if(!(type_name = eolian_typedecl_short_name_get(tdl)));
   fail_if(strcmp(type_name, "Coord"));
   fail_if(!eina_iterator_next(iter, (void**)&tdl));
   fail_if(!(type_name = eolian_typedecl_short_name_get(tdl)));
   fail_if(strcmp(type_name, "List_Objects"));
   /* coord2 and coord3, skip */
   fail_if(!eina_iterator_next(iter, (void**)&tdl));
   fail_if(!eina_iterator_next(iter, (void**)&tdl));
   /* not generated extern, skip */
   fail_if(!eina_iterator_next(iter, (void**)&tdl));
   /* not generated undefined type, skip */
   fail_if(!eina_iterator_next(iter, (void**)&tdl));
   /* free cb, tested by generation tests */
   fail_if(!eina_iterator_next(iter, (void**)&tdl));
   fail_if(eina_iterator_next(iter, (void**)&tdl));

   eolian_state_free(eos);
}
EFL_END_TEST

EFL_START_TEST(eolian_complex_type)
{
   const Eolian_Function *fid = NULL;
   const Eolian_Function_Parameter *param = NULL;
   const Eolian_Type *type = NULL;
   const char *type_name = NULL;
   Eina_Iterator *iter = NULL;
   const Eolian_Class *class;
   const Eolian_Unit *unit;
   void *dummy;

   Eolian_State *eos = eolian_state_new();
   /* Parsing */
   fail_if(!eolian_state_directory_add(eos, TESTS_SRC_DIR"/data"));
   fail_if(!(unit = eolian_state_file_parse(eos, TESTS_SRC_DIR"/data/complex_type.eo")));
   fail_if(!(class = eolian_unit_class_by_name_get(unit, "Complex_Type")));

   /* Properties return type */
   fail_if(!(fid = eolian_class_function_by_name_get(class, "a", EOLIAN_PROPERTY)));
   fail_if(!(type = eolian_function_return_type_get(fid, EOLIAN_PROP_SET)));
   fail_if(!(type_name = eolian_type_c_type_get(type, EOLIAN_C_TYPE_RETURN)));
   fail_if(!eolian_type_is_owned(type));
   fail_if(eolian_type_builtin_type_get(type) != EOLIAN_TYPE_BUILTIN_LIST);
   fail_if(strcmp(type_name, "Eina_List *"));
   eina_stringshare_del(type_name);
   fail_if(!(type = eolian_type_base_type_get(type)));
   fail_if(!!eolian_type_next_type_get(type));
   fail_if(!(type_name = eolian_type_c_type_get(type, EOLIAN_C_TYPE_DEFAULT)));
   fail_if(eolian_type_is_owned(type));
   fail_if(eolian_type_builtin_type_get(type) != EOLIAN_TYPE_BUILTIN_ARRAY);
   fail_if(strcmp(type_name, "Eina_Array *"));
   eina_stringshare_del(type_name);
   fail_if(!(type = eolian_type_base_type_get(type)));
   fail_if(!!eolian_type_next_type_get(type));
   fail_if(!(type_name = eolian_type_c_type_get(type, EOLIAN_C_TYPE_DEFAULT)));
   fail_if(!eolian_type_is_owned(type));
   fail_if(strcmp(type_name, "Eo *"));
   eina_stringshare_del(type_name);
   /* Properties parameter type */
   fail_if(!(iter = eolian_property_values_get(fid, EOLIAN_PROP_GET)));
   fail_if(!(eina_iterator_next(iter, (void**)&param)));
   fail_if(eina_iterator_next(iter, &dummy));
   eina_iterator_free(iter);
   fail_if(strcmp(eolian_parameter_name_get(param), "value"));
   fail_if(!(type = eolian_parameter_type_get(param)));
   fail_if(!(type_name = eolian_type_c_type_get(type, EOLIAN_C_TYPE_PARAM)));
   fail_if(!eolian_type_is_owned(type));
   fail_if(strcmp(type_name, "Eina_List *"));
   eina_stringshare_del(type_name);
   fail_if(!(type = eolian_type_base_type_get(type)));
   fail_if(!!eolian_type_next_type_get(type));
   fail_if(!(type_name = eolian_type_c_type_get(type, EOLIAN_C_TYPE_DEFAULT)));
   fail_if(eolian_type_is_owned(type));
   fail_if(strcmp(type_name, "const char *"));
   eina_stringshare_del(type_name);

   /* Methods return type */
   fail_if(!(fid = eolian_class_function_by_name_get(class, "foo", EOLIAN_METHOD)));
   fail_if(!(type = eolian_function_return_type_get(fid, EOLIAN_METHOD)));
   fail_if(!(type_name = eolian_type_c_type_get(type, EOLIAN_C_TYPE_RETURN)));
   fail_if(!eolian_type_is_owned(type));
   fail_if(strcmp(type_name, "Eina_List *"));
   eina_stringshare_del(type_name);
   fail_if(!(type = eolian_type_base_type_get(type)));
   fail_if(!!eolian_type_next_type_get(type));
   fail_if(!(type_name = eolian_type_c_type_get(type, EOLIAN_C_TYPE_DEFAULT)));
   fail_if(eolian_type_is_owned(type));
   fail_if(eolian_type_builtin_type_get(type) != EOLIAN_TYPE_BUILTIN_STRINGSHARE);
   fail_if(strcmp(type_name, "Eina_Stringshare *"));
   eina_stringshare_del(type_name);
   /* Methods parameter types */
   fail_if(!(iter = eolian_function_parameters_get(fid)));

   fail_if(!(eina_iterator_next(iter, (void**)&param)));
   fail_if(strcmp(eolian_parameter_name_get(param), "buf"));
   fail_if(!(type = eolian_parameter_type_get(param)));
   fail_if(!(type_name = eolian_type_c_type_get(type, EOLIAN_C_TYPE_PARAM)));
   fail_if(!eolian_type_is_owned(type));
   fail_if(eolian_type_builtin_type_get(type) != EOLIAN_TYPE_BUILTIN_MSTRING);
   fail_if(strcmp(type_name, "char *"));
   eina_stringshare_del(type_name);

   fail_if(!(eina_iterator_next(iter, (void**)&param)));
   fail_if(strcmp(eolian_parameter_name_get(param), "sl"));
   fail_if(!(type = eolian_parameter_type_get(param)));
   fail_if(!(type_name = eolian_type_c_type_get(type, EOLIAN_C_TYPE_PARAM)));
   fail_if(eolian_type_is_owned(type));
   fail_if(eolian_type_builtin_type_get(type) != EOLIAN_TYPE_BUILTIN_SLICE);
   fail_if(strcmp(type_name, "Eina_Slice"));
   eina_stringshare_del(type_name);

   fail_if(eina_iterator_next(iter, &dummy));
   eina_iterator_free(iter);

   eolian_state_free(eos);
}
EFL_END_TEST

EFL_START_TEST(eolian_scope)
{
   const Eolian_Function *fid = NULL;
   const Eolian_Class *class;
   const Eolian_Unit *unit;

   Eolian_State *eos = eolian_state_new();
   /* Parsing */
   fail_if(!eolian_state_directory_add(eos, TESTS_SRC_DIR"/data"));
   fail_if(!(unit = eolian_state_file_parse(eos, TESTS_SRC_DIR"/data/scope.eo")));
   fail_if(!(class = eolian_unit_class_by_name_get(unit, "Scope")));

   /* Property scope */
   fail_if(eolian_class_function_by_name_get(class, "a", EOLIAN_PROPERTY));
   fail_if(!(fid = eolian_class_function_by_name_get(class, "a", EOLIAN_PROP_GET)));
   fail_if(eolian_function_scope_get(fid, EOLIAN_PROP_GET) != EOLIAN_SCOPE_PROTECTED);
   fail_if(!(fid = eolian_class_function_by_name_get(class, "b", EOLIAN_PROP_GET)));
   fail_if(eolian_function_scope_get(fid, EOLIAN_PROP_GET) != EOLIAN_SCOPE_PUBLIC);
   fail_if(!(fid = eolian_class_function_by_name_get(class, "c", EOLIAN_PROPERTY)));
   fail_if(eolian_function_scope_get(fid, EOLIAN_PROP_GET) != EOLIAN_SCOPE_PUBLIC);
   fail_if(eolian_function_scope_get(fid, EOLIAN_PROP_SET) != EOLIAN_SCOPE_PROTECTED);

   /* Method scope */
   fail_if(!(fid = eolian_class_function_by_name_get(class, "foo", EOLIAN_METHOD)));
   fail_if(eolian_function_scope_get(fid, EOLIAN_METHOD) != EOLIAN_SCOPE_PUBLIC);
   fail_if(!(fid = eolian_class_function_by_name_get(class, "bar", EOLIAN_METHOD)));
   fail_if(eolian_function_scope_get(fid, EOLIAN_METHOD) != EOLIAN_SCOPE_PROTECTED);
   fail_if(!(fid = eolian_class_function_by_name_get(class, "foobar", EOLIAN_METHOD)));
   fail_if(eolian_function_scope_get(fid, EOLIAN_METHOD) != EOLIAN_SCOPE_PUBLIC);

   eolian_state_free(eos);
}
EFL_END_TEST

EFL_START_TEST(eolian_simple_parsing)
{
   const Eolian_Function *fid = NULL;
   const char *string = NULL, *ptype = NULL;
   const Eolian_Function_Parameter *param = NULL;
   const Eolian_Expression *expr = NULL;
   const Eolian_Class *class;
   const Eolian_Type *tp;
   const Eolian_Unit *unit;
   Eina_Iterator *iter;
   Eolian_Value v;
   void *dummy;

   Eolian_State *eos = eolian_state_new();
   /* Parsing */
   fail_if(!eolian_state_directory_add(eos, TESTS_SRC_DIR"/data"));
   fail_if(!(unit = eolian_state_file_parse(eos, TESTS_SRC_DIR"/data/class_simple.eo")));
   fail_if(!(class = eolian_unit_class_by_name_get(unit, "Class_Simple")));
   fail_if(eolian_state_class_by_file_get(eos, "class_simple.eo") != class);
   fail_if(strcmp(eolian_object_file_get((const Eolian_Object *)class), "class_simple.eo"));

   /* Class */
   fail_if(eolian_class_type_get(class) != EOLIAN_CLASS_REGULAR);
   fail_if(eolian_class_parent_get(class) != NULL);
   fail_if(eolian_class_extensions_get(class) != NULL);
   fail_if(strcmp(eolian_class_legacy_prefix_get(class), "evas_object_simple"));
   fail_if(strcmp(eolian_class_eo_prefix_get(class), "efl_canvas_object_simple"));
   fail_if(strcmp(eolian_class_data_type_get(class), "Evas_Simple_Data"));
   Eina_Stringshare *dt = eolian_class_c_data_type_get(class);
   fail_if(strcmp(dt, "Evas_Simple_Data"));
   eina_stringshare_del(dt);

   /* c get func */
   fail_if(!(string = eolian_class_c_get_function_name_get(class)));
   fail_if(strcmp(string, "class_simple_class_get"));
   eina_stringshare_del(string);

   /* c name */
   fail_if(!(string = eolian_class_c_name_get(class)));
   fail_if(strcmp(string, "CLASS_SIMPLE_CLASS"));
   eina_stringshare_del(string);

   /* Property */
   fail_if(!(fid = eolian_class_function_by_name_get(class, "a", EOLIAN_PROPERTY)));
   fail_if(strcmp(eolian_function_name_get(fid), "a"));
   fail_if(!eolian_function_is_beta(fid));
   fail_if(eolian_function_class_get(fid) != class);
   /* Set return */
   tp = eolian_function_return_type_get(fid, EOLIAN_PROP_SET);
   fail_if(!tp);
   printf("BUILT %d\n", (int)eolian_type_builtin_type_get(tp));
   fail_if(eolian_type_builtin_type_get(tp) != EOLIAN_TYPE_BUILTIN_BOOL);
   fail_if(strcmp(eolian_type_short_name_get(tp), "bool"));
   expr = eolian_function_return_default_value_get(fid, EOLIAN_PROP_SET);
   fail_if(!expr);
   v = eolian_expression_eval(expr, EOLIAN_MASK_BOOL);
   fail_if(v.type != EOLIAN_EXPR_BOOL);
   /* Get return */
   tp = eolian_function_return_type_get(fid, EOLIAN_PROP_GET);
   fail_if(tp);

   /* Function parameters */
   fail_if(eolian_property_keys_get(fid, EOLIAN_PROP_GET) != NULL);
   fail_if(!(iter = eolian_property_values_get(fid, EOLIAN_PROP_GET)));
   fail_if(!(eina_iterator_next(iter, (void**)&param)));
   fail_if(eina_iterator_next(iter, &dummy));
   eina_iterator_free(iter);
   fail_if(strcmp(eolian_type_short_name_get(eolian_parameter_type_get(param)), "int"));
   fail_if(strcmp(eolian_parameter_name_get(param), "value"));
   expr = eolian_parameter_default_value_get(param);
   fail_if(!expr);
   v = eolian_expression_eval(expr, EOLIAN_MASK_INT);
   fail_if(v.type != EOLIAN_EXPR_INT);
   fail_if(v.value.i != 100);

   /* legacy only + c only */
   fail_if(eolian_class_function_by_name_get(class, "b", EOLIAN_PROPERTY));
   fail_if(!(fid = eolian_class_function_by_name_get(class, "b", EOLIAN_PROP_SET)));
   fail_if(eolian_function_is_legacy_only(fid, EOLIAN_PROP_GET));
   fail_if(!eolian_function_is_legacy_only(fid, EOLIAN_PROP_SET));
   fail_if(eolian_function_is_beta(fid));

   /* Method */
   fail_if(!(fid = eolian_class_function_by_name_get(class, "foo", EOLIAN_METHOD)));
   fail_if(!eolian_function_is_beta(fid));
   fail_if(!eolian_type_is_ptr(eolian_function_return_type_get(fid, EOLIAN_METHOD)));
   /* Function return */
   tp = eolian_function_return_type_get(fid, EOLIAN_METHOD);
   fail_if(!tp);
   string = eolian_type_c_type_get(tp, EOLIAN_C_TYPE_RETURN);
   fail_if(!string);
   fail_if(strcmp(string, "char *"));
   eina_stringshare_del(string);
   expr = eolian_function_return_default_value_get(fid, EOLIAN_METHOD);
   fail_if(!expr);
   v = eolian_expression_eval(expr, EOLIAN_MASK_NULL);
   fail_if(v.type != EOLIAN_EXPR_NULL);
   fail_if(eolian_function_is_legacy_only(fid, EOLIAN_METHOD));

   /* Function parameters */
   fail_if(!(iter = eolian_function_parameters_get(fid)));
   fail_if(!(eina_iterator_next(iter, (void**)&param)));
   fail_if(eolian_parameter_direction_get(param) != EOLIAN_IN_PARAM);
   fail_if(strcmp(eolian_type_short_name_get(eolian_parameter_type_get(param)), "int"));
   fail_if(strcmp(eolian_parameter_name_get(param), "a"));
   fail_if(!(eina_iterator_next(iter, (void**)&param)));
   fail_if(eolian_parameter_direction_get(param) != EOLIAN_INOUT_PARAM);
   ptype = eolian_type_short_name_get(eolian_parameter_type_get(param));
   fail_if(strcmp(ptype, "char"));
   fail_if(strcmp(eolian_parameter_name_get(param), "b"));
   fail_if(!(eina_iterator_next(iter, (void**)&param)));
   fail_if(eolian_parameter_direction_get(param) != EOLIAN_OUT_PARAM);
   fail_if(eolian_type_builtin_type_get(eolian_parameter_type_get(param)) != EOLIAN_TYPE_BUILTIN_DOUBLE);
   fail_if(strcmp(eolian_type_short_name_get(eolian_parameter_type_get(param)), "double"));
   fail_if(strcmp(eolian_parameter_name_get(param), "c"));
   expr = eolian_parameter_default_value_get(param);
   fail_if(!expr);
   v = eolian_expression_eval(expr, EOLIAN_MASK_FLOAT);
   fail_if(v.type != EOLIAN_EXPR_DOUBLE);
   fail_if(v.value.d != 1337.6);
   fail_if(!(eina_iterator_next(iter, (void**)&param)));
   fail_if(eolian_parameter_direction_get(param) != EOLIAN_IN_PARAM);
   fail_if(strcmp(eolian_type_short_name_get(eolian_parameter_type_get(param)), "int"));
   fail_if(!eolian_type_is_ptr(eolian_parameter_type_get(param)));
   fail_if(strcmp(eolian_parameter_name_get(param), "d"));
   fail_if(eina_iterator_next(iter, &dummy));
   eina_iterator_free(iter);

   /* legacy only + c only */
   fail_if(!(fid = eolian_class_function_by_name_get(class, "bar", EOLIAN_METHOD)));
   fail_if(!eolian_function_is_legacy_only(fid, EOLIAN_METHOD));
   fail_if(eolian_function_is_beta(fid));
   fail_if(!eolian_type_is_ptr(eolian_function_return_type_get(fid, EOLIAN_METHOD)));

   eolian_state_free(eos);
}
EFL_END_TEST

EFL_START_TEST(eolian_struct)
{
   const Eolian_Struct_Type_Field *field = NULL;
   const Eolian_Type *type = NULL, *ftype = NULL;
   const Eolian_Typedecl *tdl = NULL;
   const Eolian_Class *class;
   const Eolian_Function *func;
   const Eolian_Unit *unit;
   const char *type_name;
   const char *file;

   Eolian_State *eos = eolian_state_new();

   /* Parsing */
   fail_if(!eolian_state_directory_add(eos, TESTS_SRC_DIR"/data"));
   fail_if(!(unit = eolian_state_file_parse(eos, TESTS_SRC_DIR"/data/struct.eo")));

   /* Check that the class Dummy is still readable */
   fail_if(!(class = eolian_unit_class_by_name_get(unit, "Struct")));
   fail_if(!eolian_class_function_by_name_get(class, "foo", EOLIAN_METHOD));

   /* named struct */
   fail_if(!(tdl = eolian_unit_struct_by_name_get(unit, "Named")));
   fail_if(!(type_name = eolian_typedecl_short_name_get(tdl)));
   fail_if(!(file = eolian_object_file_get((const Eolian_Object *)tdl)));
   fail_if(eolian_typedecl_type_get(tdl) != EOLIAN_TYPEDECL_STRUCT);
   fail_if(strcmp(type_name, "Named"));
   fail_if(strcmp(file, "struct.eo"));
   fail_if(!(field = eolian_typedecl_struct_field_get(tdl, "field")));
   fail_if(!(ftype = eolian_typedecl_struct_field_type_get(field)));
   fail_if(!eolian_type_is_ptr(ftype));
   fail_if(!(type_name = eolian_type_short_name_get(ftype)));
   fail_if(strcmp(type_name, "int"));
   fail_if(!(field = eolian_typedecl_struct_field_get(tdl, "something")));
   fail_if(!(ftype = eolian_typedecl_struct_field_type_get(field)));
   fail_if(eolian_type_is_ptr(ftype));
   fail_if(!(type_name = eolian_type_c_type_get(ftype, EOLIAN_C_TYPE_DEFAULT)));
   fail_if(strcmp(type_name, "const char *"));
   eina_stringshare_del(type_name);

   /* referencing */
   fail_if(!(tdl = eolian_unit_struct_by_name_get(unit, "Another")));
   fail_if(!(type_name = eolian_typedecl_short_name_get(tdl)));
   fail_if(!(file = eolian_object_file_get((const Eolian_Object *)tdl)));
   fail_if(eolian_typedecl_type_get(tdl) != EOLIAN_TYPEDECL_STRUCT);
   fail_if(strcmp(type_name, "Another"));
   fail_if(strcmp(file, "struct.eo"));
   fail_if(!(field = eolian_typedecl_struct_field_get(tdl, "field")));
   fail_if(!(ftype = eolian_typedecl_struct_field_type_get(field)));
   fail_if(!(type_name = eolian_type_short_name_get(ftype)));
   fail_if(strcmp(type_name, "Named"));
   fail_if(eolian_type_type_get(ftype) != EOLIAN_TYPE_REGULAR);
   fail_if(eolian_typedecl_type_get(eolian_type_typedecl_get(ftype))
       != EOLIAN_TYPEDECL_STRUCT);

   /* opaque struct */
   fail_if(!(tdl = eolian_unit_struct_by_name_get(unit, "Opaque")));
   fail_if(eolian_typedecl_type_get(tdl) != EOLIAN_TYPEDECL_STRUCT_OPAQUE);

   /* use in function */
   fail_if(!(func = eolian_class_function_by_name_get(class, "bar", EOLIAN_METHOD)));
   fail_if(!(type = eolian_function_return_type_get(func, EOLIAN_METHOD)));
   fail_if(eolian_type_type_get(type) != EOLIAN_TYPE_REGULAR);
   fail_if(!eolian_type_is_ptr(type));
   fail_if(!(tdl = eolian_type_typedecl_get(type)));
   fail_if(eolian_typedecl_type_get(tdl) != EOLIAN_TYPEDECL_STRUCT);

   eolian_state_free(eos);
}
EFL_END_TEST

EFL_START_TEST(eolian_extern)
{
   const Eolian_Typedecl *tdl = NULL;
   const Eolian_Class *class;
   const Eolian_Unit *unit;

   Eolian_State *eos = eolian_state_new();

   /* Parsing */
   fail_if(!eolian_state_directory_add(eos, TESTS_SRC_DIR"/data"));
   fail_if(!(unit = eolian_state_file_parse(eos, TESTS_SRC_DIR"/data/extern.eo")));

   /* Check that the class Dummy is still readable */
   fail_if(!(class = eolian_unit_class_by_name_get(unit, "Extern")));
   fail_if(!eolian_class_function_by_name_get(class, "foo", EOLIAN_METHOD));

   /* regular type */
   fail_if(!(tdl = eolian_unit_alias_by_name_get(unit, "Foo")));
   fail_if(eolian_typedecl_is_extern(tdl));

   /* extern type */
   fail_if(!(tdl = eolian_unit_alias_by_name_get(unit, "Evas.Coord")));
   fail_if(!eolian_typedecl_is_extern(tdl));

   /* regular struct */
   fail_if(!(tdl = eolian_unit_struct_by_name_get(unit, "X")));
   fail_if(eolian_typedecl_is_extern(tdl));

   /* extern struct */
   fail_if(!(tdl = eolian_unit_struct_by_name_get(unit, "Y")));
   fail_if(!eolian_typedecl_is_extern(tdl));

   eolian_state_free(eos);
}
EFL_END_TEST

EFL_START_TEST(eolian_var)
{
   const Eolian_Variable *var = NULL;
   const Eolian_Expression *exp = NULL;
   const Eolian_Type *type = NULL;
   const Eolian_Class *class;
   const Eolian_Unit *unit;
   Eolian_Value v;
   const char *name;

   Eolian_State *eos = eolian_state_new();

   /* Parsing */
   fail_if(!eolian_state_directory_add(eos, TESTS_SRC_DIR"/data"));
   fail_if(!(unit = eolian_state_file_parse(eos, TESTS_SRC_DIR"/data/var.eo")));

   /* Check that the class Dummy is still readable */
   fail_if(!(class = eolian_unit_class_by_name_get(unit, "Var")));
   fail_if(!eolian_class_function_by_name_get(class, "foo", EOLIAN_METHOD));

   /* regular constant */
   fail_if(!(var = eolian_unit_constant_by_name_get(unit, "Foo")));
   fail_if(eolian_variable_type_get(var) != EOLIAN_VAR_CONSTANT);
   fail_if(eolian_variable_is_extern(var));
   fail_if(!(type = eolian_variable_base_type_get(var)));
   fail_if(!(name = eolian_type_short_name_get(type)));
   fail_if(strcmp(name, "int"));
   fail_if(!(exp = eolian_variable_value_get(var)));
   v = eolian_expression_eval_type(exp, type);
   fail_if(v.type != EOLIAN_EXPR_INT);
   fail_if(v.value.i != 5);

   /* regular global */
   fail_if(!(var = eolian_unit_global_by_name_get(unit, "Bar")));
   fail_if(eolian_variable_type_get(var) != EOLIAN_VAR_GLOBAL);
   fail_if(eolian_variable_is_extern(var));
   fail_if(!(type = eolian_variable_base_type_get(var)));
   fail_if(!(name = eolian_type_short_name_get(type)));
   fail_if(strcmp(name, "float"));
   fail_if(!(exp = eolian_variable_value_get(var)));
   v = eolian_expression_eval_type(exp, type);
   fail_if(v.type != EOLIAN_EXPR_FLOAT);
   fail_if(((int)v.value.f) != 10);

   /* no-value global */
   fail_if(!(var = eolian_unit_global_by_name_get(unit, "Baz")));
   fail_if(eolian_variable_type_get(var) != EOLIAN_VAR_GLOBAL);
   fail_if(eolian_variable_is_extern(var));
   fail_if(!(type = eolian_variable_base_type_get(var)));
   fail_if(!(name = eolian_type_short_name_get(type)));
   fail_if(strcmp(name, "long"));
   fail_if(eolian_variable_value_get(var));

   /* extern global  */
   fail_if(!(var = eolian_unit_global_by_name_get(unit, "Bah")));
   fail_if(eolian_variable_type_get(var) != EOLIAN_VAR_GLOBAL);
   fail_if(!eolian_variable_is_extern(var));
   fail_if(!(type = eolian_variable_base_type_get(var)));
   fail_if(!(name = eolian_type_short_name_get(type)));
   fail_if(strcmp(name, "double"));
   fail_if(eolian_variable_value_get(var));

   eolian_state_free(eos);
}
EFL_END_TEST

EFL_START_TEST(eolian_enum)
{
   const Eolian_Enum_Type_Field *field = NULL;
   const Eolian_Variable *var = NULL;
   const Eolian_Typedecl *tdl = NULL;
   const Eolian_Type *type = NULL;
   const Eolian_Class *class;
   const Eolian_Expression *exp;
   const Eolian_Unit *unit;
   Eina_Stringshare *cname;
   const char *name;
   Eolian_Value v;

   Eolian_State *eos = eolian_state_new();

   /* Parsing */
   fail_if(!eolian_state_directory_add(eos, TESTS_SRC_DIR"/data"));
   fail_if(!(unit = eolian_state_file_parse(eos, TESTS_SRC_DIR"/data/enum.eo")));

   /* Check that the class Dummy is still readable */
   fail_if(!(class = eolian_unit_class_by_name_get(unit, "Enum")));
   fail_if(!eolian_class_function_by_name_get(class, "foo", EOLIAN_METHOD));

   fail_if(!(tdl = eolian_unit_enum_by_name_get(unit, "Foo")));

   fail_if(!(field = eolian_typedecl_enum_field_get(tdl, "first")));
   fail_if(!(exp = eolian_typedecl_enum_field_value_get(field, EINA_FALSE)));
   v = eolian_expression_eval(exp, EOLIAN_MASK_ALL);
   fail_if(v.type != EOLIAN_EXPR_INT);
   fail_if(v.value.i != 0);

   fail_if(!(field = eolian_typedecl_enum_field_get(tdl, "bar")));
   fail_if(eolian_typedecl_enum_field_value_get(field, EINA_FALSE));

   fail_if(!(field = eolian_typedecl_enum_field_get(tdl, "baz")));
   fail_if(!(exp = eolian_typedecl_enum_field_value_get(field, EINA_FALSE)));
   v = eolian_expression_eval(exp, EOLIAN_MASK_ALL);
   fail_if(v.type != EOLIAN_EXPR_INT);
   fail_if(v.value.i != 15);

   fail_if(!(tdl = eolian_unit_enum_by_name_get(unit, "Bar")));
   fail_if(strcmp(eolian_typedecl_enum_legacy_prefix_get(tdl), "test"));

   fail_if(!(field = eolian_typedecl_enum_field_get(tdl, "foo")));
   fail_if(!(exp = eolian_typedecl_enum_field_value_get(field, EINA_FALSE)));
   v = eolian_expression_eval(exp, EOLIAN_MASK_ALL);
   fail_if(v.type != EOLIAN_EXPR_INT);
   fail_if(v.value.i != 15);

   cname = eolian_typedecl_enum_field_c_name_get(field);
   fail_if(strcmp(cname, "TEST_FOO"));
   eina_stringshare_del(cname);

   fail_if(!(tdl = eolian_unit_enum_by_name_get(unit, "Baz")));

   fail_if(!(field = eolian_typedecl_enum_field_get(tdl, "flag1")));
   fail_if(!(exp = eolian_typedecl_enum_field_value_get(field, EINA_FALSE)));
   v = eolian_expression_eval(exp, EOLIAN_MASK_ALL);
   fail_if(v.type != EOLIAN_EXPR_INT);
   fail_if(v.value.i != (1 << 0));

   fail_if(!(field = eolian_typedecl_enum_field_get(tdl, "flag2")));
   fail_if(!(exp = eolian_typedecl_enum_field_value_get(field, EINA_FALSE)));
   v = eolian_expression_eval(exp, EOLIAN_MASK_ALL);
   fail_if(v.type != EOLIAN_EXPR_INT);
   fail_if(v.value.i != (1 << 1));

   fail_if(!(field = eolian_typedecl_enum_field_get(tdl, "flag3")));
   fail_if(!(exp = eolian_typedecl_enum_field_value_get(field, EINA_FALSE)));
   v = eolian_expression_eval(exp, EOLIAN_MASK_ALL);
   fail_if(v.type != EOLIAN_EXPR_INT);
   fail_if(v.value.i != (1 << 2));

   fail_if(!(tdl = eolian_unit_enum_by_name_get(unit, "Name.Spaced")));
   fail_if(!(field = eolian_typedecl_enum_field_get(tdl, "pants")));

   cname = eolian_typedecl_enum_field_c_name_get(field);
   fail_if(strcmp(cname, "NAME_SPACED_PANTS"));
   eina_stringshare_del(cname);

   fail_if(!(var = eolian_unit_constant_by_name_get(unit, "Bah")));
   fail_if(eolian_variable_type_get(var) != EOLIAN_VAR_CONSTANT);
   fail_if(eolian_variable_is_extern(var));
   fail_if(!(type = eolian_variable_base_type_get(var)));
   fail_if(!(name = eolian_type_short_name_get(type)));
   fail_if(strcmp(name, "Baz"));
   fail_if(!(exp = eolian_variable_value_get(var)));
   v = eolian_expression_eval(exp, EOLIAN_MASK_ALL);
   fail_if(v.type != EOLIAN_EXPR_INT);
   fail_if(v.value.i != (1 << 0));

   fail_if(!(var = eolian_unit_constant_by_name_get(unit, "Pants")));
   fail_if(eolian_variable_type_get(var) != EOLIAN_VAR_CONSTANT);
   fail_if(!(exp = eolian_variable_value_get(var)));
   v = eolian_expression_eval(exp, EOLIAN_MASK_ALL);
   fail_if(v.type != EOLIAN_EXPR_INT);
   fail_if(v.value.i != 5);

   eolian_state_free(eos);
}
EFL_END_TEST

EFL_START_TEST(eolian_class_funcs)
{
   const Eolian_Function *fid = NULL;
   const Eolian_Class *class;
   const Eolian_Unit *unit;

   Eolian_State *eos = eolian_state_new();
   /* Parsing */
   fail_if(!eolian_state_directory_add(eos, TESTS_SRC_DIR"/data"));
   fail_if(!(unit = eolian_state_file_parse(eos, TESTS_SRC_DIR"/data/class_funcs.eo")));
   fail_if(!(class = eolian_unit_class_by_name_get(unit, "Class_Funcs")));

   /* Class properties */
   fail_if(eolian_class_function_by_name_get(class, "a", EOLIAN_PROPERTY));
   fail_if(!(fid = eolian_class_function_by_name_get(class, "a", EOLIAN_PROP_GET)));
   fail_if(!eolian_function_is_class(fid));
   fail_if(!(fid = eolian_class_function_by_name_get(class, "b", EOLIAN_PROP_GET)));
   fail_if(eolian_function_is_class(fid));

   /* Class methods */
   fail_if(!(fid = eolian_class_function_by_name_get(class, "foo", EOLIAN_METHOD)));
   fail_if(!eolian_function_is_class(fid));
   fail_if(eolian_function_scope_get(fid, EOLIAN_METHOD) != EOLIAN_SCOPE_PUBLIC);
   fail_if(!(fid = eolian_class_function_by_name_get(class, "bar", EOLIAN_METHOD)));
   fail_if(eolian_function_is_class(fid));
   fail_if(eolian_function_scope_get(fid, EOLIAN_METHOD) != EOLIAN_SCOPE_PUBLIC);
   fail_if(!(fid = eolian_class_function_by_name_get(class, "baz", EOLIAN_METHOD)));
   fail_if(!eolian_function_is_class(fid));
   fail_if(eolian_function_scope_get(fid, EOLIAN_METHOD) != EOLIAN_SCOPE_PROTECTED);
   fail_if(!(fid = eolian_class_function_by_name_get(class, "bah", EOLIAN_METHOD)));
   fail_if(eolian_function_is_class(fid));
   fail_if(eolian_function_scope_get(fid, EOLIAN_METHOD) != EOLIAN_SCOPE_PROTECTED);

   eolian_state_free(eos);
}
EFL_END_TEST

EFL_START_TEST(eolian_free_func)
{
   const Eolian_Class *class;
   const Eolian_Typedecl *tdl;
   const Eolian_Unit *unit;

   Eolian_State *eos = eolian_state_new();

   /* Parsing */
   fail_if(!eolian_state_directory_add(eos, TESTS_SRC_DIR"/data"));
   fail_if(!(unit = eolian_state_file_parse(eos, TESTS_SRC_DIR"/data/free_func.eo")));

   /* Check that the class Dummy is still readable */
   fail_if(!(class = eolian_unit_class_by_name_get(unit, "Free_Func")));
   fail_if(!eolian_class_function_by_name_get(class, "foo", EOLIAN_METHOD));

   /* regular struct */
   fail_if(!(tdl = eolian_unit_struct_by_name_get(unit, "Named1")));
   fail_if(eolian_typedecl_free_func_get(tdl));
   fail_if(!(tdl = eolian_unit_struct_by_name_get(unit, "Named2")));
   fail_if(strcmp(eolian_typedecl_free_func_get(tdl), "test_free"));

   /* opaque struct */
   fail_if(!(tdl = eolian_unit_struct_by_name_get(unit, "Opaque1")));
   fail_if(eolian_typedecl_free_func_get(tdl));
   fail_if(!(tdl = eolian_unit_struct_by_name_get(unit, "Opaque2")));
   fail_if(strcmp(eolian_typedecl_free_func_get(tdl), "opaque_free"));

   eolian_state_free(eos);
}
EFL_END_TEST

EFL_START_TEST(eolian_null)
{
   const Eolian_Class *class;
   const Eolian_Function *func;
   const Eolian_Function_Parameter *param;
   const Eolian_Unit *unit;
   Eina_Iterator *iter;

   Eolian_State *eos = eolian_state_new();

   /* Parsing */
   fail_if(!eolian_state_directory_add(eos, TESTS_SRC_DIR"/data"));
   fail_if(!(unit = eolian_state_file_parse(eos, TESTS_SRC_DIR"/data/null.eo")));

   fail_if(!(class = eolian_unit_class_by_name_get(unit, "Null")));
   fail_if(!(func = eolian_class_function_by_name_get(class, "foo", EOLIAN_METHOD)));

   fail_if(!(iter = eolian_function_parameters_get(func)));

   /* no qualifiers */
   fail_if(!(eina_iterator_next(iter, (void**)&param)));
   fail_if(strcmp(eolian_parameter_name_get(param), "x"));
   fail_if(eolian_parameter_is_nullable(param));
   fail_if(eolian_parameter_is_optional(param));

   /* nullable */
   fail_if(!(eina_iterator_next(iter, (void**)&param)));
   fail_if(strcmp(eolian_parameter_name_get(param), "y"));
   fail_if(!eolian_parameter_is_nullable(param));
   fail_if(eolian_parameter_is_optional(param));

   /* optional */
   fail_if(!(eina_iterator_next(iter, (void**)&param)));
   fail_if(strcmp(eolian_parameter_name_get(param), "z"));
   fail_if(eolian_parameter_is_nullable(param));
   fail_if(!eolian_parameter_is_optional(param));

   /* both */
   fail_if(!(eina_iterator_next(iter, (void**)&param)));
   fail_if(strcmp(eolian_parameter_name_get(param), "w"));
   fail_if(!eolian_parameter_is_nullable(param));
   fail_if(!eolian_parameter_is_optional(param));

   fail_if(eina_iterator_next(iter, (void**)&param));
   eina_iterator_free(iter);

   eolian_state_free(eos);
}
EFL_END_TEST

EFL_START_TEST(eolian_import)
{
   const Eolian_Class *class;
   const Eolian_Typedecl *tdl;
   const Eolian_Unit *unit;

   Eolian_State *eos = eolian_state_new();

   fail_if(!eolian_state_directory_add(eos, TESTS_SRC_DIR"/data"));

   fail_if(!(unit = eolian_state_file_parse(eos, TESTS_SRC_DIR"/data/import.eo")));
   fail_if(!(class = eolian_unit_class_by_name_get(unit, "Import")));

   fail_if(!(tdl = eolian_unit_alias_by_name_get(unit, "Imported")));
   fail_if(strcmp(eolian_object_file_get((const Eolian_Object *)tdl), "import_types.eot"));

   fail_if(!(tdl = eolian_unit_struct_by_name_get(unit, "Imported_Struct")));
   fail_if(strcmp(eolian_object_file_get((const Eolian_Object *)tdl), "import_types.eot"));

   eolian_state_free(eos);
}
EFL_END_TEST

EFL_START_TEST(eolian_docs)
{
   const Eolian_Typedecl *tdl;
   const Eolian_Class *class;
   const Eolian_Event *event;
   const Eolian_Variable *var;
   const Eolian_Function *fid;
   const Eolian_Documentation *doc;
   const Eolian_Function_Parameter *par;
   const Eolian_Struct_Type_Field *sfl;
   const Eolian_Enum_Type_Field *efl;
   const Eolian_Unit *unit;
   Eina_Iterator *itr;

   Eolian_State *eos = eolian_state_new();

   fail_if(!eolian_state_directory_add(eos, TESTS_SRC_DIR"/data"));

   fail_if(!(unit = eolian_state_file_parse(eos, TESTS_SRC_DIR"/data/docs.eo")));

   fail_if(!(tdl = eolian_unit_struct_by_name_get(unit, "Foo")));
   fail_if(!(doc = eolian_typedecl_documentation_get(tdl)));
   fail_if(strcmp(eolian_documentation_summary_get(doc),
                  "This is struct Foo. It does stuff."));
   fail_if(strcmp(eolian_documentation_description_get(doc),
                  "Note: This is a note.\n\n"
                  "This is a longer description for struct Foo.\n\n"
                  "Warning: This is a warning. You can only use Warning: "
                  "and Note: at the beginning of a paragraph.\n\n"
                  "This is another paragraph."));
   fail_if(strcmp(eolian_documentation_since_get(doc),
                  "1.66"));

   const char *sdesc = eolian_documentation_description_get(doc);
   Eina_List *sdoc = eolian_documentation_string_split(sdesc);

   char *dpar = eina_list_data_get(sdoc);
   fail_if(strcmp(dpar, "Note: This is a note."));
   sdoc = eina_list_remove_list(sdoc, sdoc);
   dpar = eina_list_data_get(sdoc);
   fail_if(strcmp(dpar, "This is a longer description for struct Foo."));
   EINA_LIST_FREE(sdoc, dpar)
     free(dpar);

   const char *tdoc = "Note: This is $something, see @pants, "
                      "@Docs.meth, \\@ref foo and @[Things.Stuffs.foo,bar].";

   Eolian_Doc_Token tok;
   eolian_doc_token_init(&tok);
   fail_if(eolian_doc_token_type_get(&tok) != EOLIAN_DOC_TOKEN_UNKNOWN);
   tdoc = eolian_documentation_tokenize(tdoc, &tok);
   fail_if(eolian_doc_token_type_get(&tok) != EOLIAN_DOC_TOKEN_MARK_NOTE);
   char *txt = eolian_doc_token_text_get(&tok);
   fail_if(strcmp(txt, "Note: "));
   free(txt);
   tdoc = eolian_documentation_tokenize(tdoc, &tok);
   fail_if(eolian_doc_token_type_get(&tok) != EOLIAN_DOC_TOKEN_TEXT);
   txt = eolian_doc_token_text_get(&tok);
   fail_if(strcmp(txt, "This is "));
   free(txt);
   tdoc = eolian_documentation_tokenize(tdoc, &tok);
   fail_if(eolian_doc_token_type_get(&tok) != EOLIAN_DOC_TOKEN_MARKUP_MONOSPACE);
   txt = eolian_doc_token_text_get(&tok);
   fail_if(strcmp(txt, "something"));
   free(txt);
   tdoc = eolian_documentation_tokenize(tdoc, &tok);
   fail_if(eolian_doc_token_type_get(&tok) != EOLIAN_DOC_TOKEN_TEXT);
   txt = eolian_doc_token_text_get(&tok);
   fail_if(strcmp(txt, ", see "));
   free(txt);
   tdoc = eolian_documentation_tokenize(tdoc, &tok);
   fail_if(eolian_doc_token_type_get(&tok) != EOLIAN_DOC_TOKEN_REF);
   txt = eolian_doc_token_text_get(&tok);
   fail_if(strcmp(txt, "pants"));
   fail_if(eolian_doc_token_ref_resolve(&tok, eos, NULL, NULL) != EOLIAN_OBJECT_VARIABLE);
   free(txt);
   tdoc = eolian_documentation_tokenize(tdoc, &tok);
   fail_if(eolian_doc_token_type_get(&tok) != EOLIAN_DOC_TOKEN_TEXT);
   txt = eolian_doc_token_text_get(&tok);
   fail_if(strcmp(txt, ", "));
   free(txt);
   tdoc = eolian_documentation_tokenize(tdoc, &tok);
   fail_if(eolian_doc_token_type_get(&tok) != EOLIAN_DOC_TOKEN_REF);
   txt = eolian_doc_token_text_get(&tok);
   fail_if(strcmp(txt, "Docs.meth"));
   fail_if(eolian_doc_token_ref_resolve(&tok, eos, NULL, NULL) != EOLIAN_OBJECT_FUNCTION);
   free(txt);
   tdoc = eolian_documentation_tokenize(tdoc, &tok);
   fail_if(eolian_doc_token_type_get(&tok) != EOLIAN_DOC_TOKEN_TEXT);
   txt = eolian_doc_token_text_get(&tok);
   fail_if(strcmp(txt, ", @ref foo and "));
   free(txt);
   tdoc = eolian_documentation_tokenize(tdoc, &tok);
   fail_if(eolian_doc_token_type_get(&tok) != EOLIAN_DOC_TOKEN_REF);
   txt = eolian_doc_token_text_get(&tok);
   fail_if(strcmp(txt, "[Things.Stuffs.foo,bar]"));
   free(txt);
   tdoc = eolian_documentation_tokenize(tdoc, &tok);
   fail_if(tdoc[0] != '\0');
   fail_if(eolian_doc_token_type_get(&tok) != EOLIAN_DOC_TOKEN_TEXT);
   txt = eolian_doc_token_text_get(&tok);
   fail_if(strcmp(txt, "."));
   free(txt);
   tdoc = eolian_documentation_tokenize(tdoc, &tok);
   fail_if(tdoc != NULL);
   fail_if(eolian_doc_token_type_get(&tok) != EOLIAN_DOC_TOKEN_UNKNOWN);

   fail_if(!(sfl = eolian_typedecl_struct_field_get(tdl, "field1")));
   fail_if(!(doc = eolian_typedecl_struct_field_documentation_get(sfl)));
   fail_if(strcmp(eolian_documentation_summary_get(doc),
                  "Field documentation."));
   fail_if(eolian_documentation_description_get(doc));

   fail_if(!(sfl = eolian_typedecl_struct_field_get(tdl, "field2")));
   fail_if(eolian_typedecl_struct_field_documentation_get(sfl));

   fail_if(!(sfl = eolian_typedecl_struct_field_get(tdl, "field3")));
   fail_if(!(doc = eolian_typedecl_struct_field_documentation_get(sfl)));
   fail_if(strcmp(eolian_documentation_summary_get(doc),
                  "Another field documentation."));
   fail_if(eolian_documentation_description_get(doc));

   fail_if(!(tdl = eolian_unit_enum_by_name_get(unit, "Bar")));
   fail_if(!(doc = eolian_typedecl_documentation_get(tdl)));
   fail_if(strcmp(eolian_documentation_summary_get(doc),
                  "Docs for enum Bar."));
   fail_if(eolian_documentation_description_get(doc));
   fail_if(eolian_documentation_since_get(doc));

   fail_if(!(efl = eolian_typedecl_enum_field_get(tdl, "blah")));
   fail_if(eolian_typedecl_enum_field_documentation_get(efl));

   fail_if(!(efl = eolian_typedecl_enum_field_get(tdl, "foo")));
   fail_if(!(doc = eolian_typedecl_enum_field_documentation_get(efl)));
   fail_if(strcmp(eolian_documentation_summary_get(doc),
                  "Docs for foo."));
   fail_if(eolian_documentation_description_get(doc));

   fail_if(!(efl = eolian_typedecl_enum_field_get(tdl, "bar")));
   fail_if(!(doc = eolian_typedecl_enum_field_documentation_get(efl)));
   fail_if(strcmp(eolian_documentation_summary_get(doc),
                  "Docs for bar."));
   fail_if(eolian_documentation_description_get(doc));

   fail_if(!(tdl = eolian_unit_alias_by_name_get(unit, "Alias")));
   fail_if(!(doc = eolian_typedecl_documentation_get(tdl)));
   fail_if(strcmp(eolian_documentation_summary_get(doc),
                  "Docs for typedef."));
   fail_if(strcmp(eolian_documentation_description_get(doc),
                  "More docs for typedef. See @Bar."));
   fail_if(strcmp(eolian_documentation_since_get(doc),
                  "2.0"));

   fail_if(!(var = eolian_unit_global_by_name_get(unit, "pants")));
   fail_if(!(doc = eolian_variable_documentation_get(var)));
   fail_if(strcmp(eolian_documentation_summary_get(doc),
                  "Docs for var."));
   fail_if(eolian_documentation_description_get(doc));

   fail_if(!(tdl = eolian_unit_struct_by_name_get(unit, "Opaque")));
   fail_if(!(doc = eolian_typedecl_documentation_get(tdl)));
   fail_if(strcmp(eolian_documentation_summary_get(doc),
                  "Opaque struct docs. See @Foo for another struct."));
   fail_if(eolian_documentation_description_get(doc));

   fail_if(!(class = eolian_unit_class_by_name_get(unit, "Docs")));
   fail_if(!(doc = eolian_class_documentation_get(class)));
   fail_if(strcmp(eolian_documentation_summary_get(doc),
                  "Docs for class."));
   fail_if(strcmp(eolian_documentation_description_get(doc),
                  "More docs for class. Testing references now. "
                  "@Foo @Bar @Alias @pants @Docs.meth @Docs.prop "
                  "@Docs.prop.get @Docs.prop.set @Foo.field1 @Bar.foo @Docs"));

   fail_if(!(fid = eolian_class_function_by_name_get(class, "meth", EOLIAN_METHOD)));
   const Eolian_Implement *fimp = eolian_function_implement_get(fid);
   fail_if(!(doc = eolian_implement_documentation_get(fimp, EOLIAN_METHOD)));
   fail_if(strcmp(eolian_documentation_summary_get(doc),
                  "Method documentation."));
   fail_if(eolian_documentation_description_get(doc));

   fail_if(!(itr = eolian_function_parameters_get(fid)));

   fail_if(!eina_iterator_next(itr, (void**)&par));
   fail_if(!(doc = eolian_parameter_documentation_get(par)));
   fail_if(strcmp(eolian_documentation_summary_get(doc),
                  "Param documentation."));
   fail_if(eolian_documentation_description_get(doc));

   fail_if(!eina_iterator_next(itr, (void**)&par));
   fail_if(eolian_parameter_documentation_get(par));

   fail_if(!eina_iterator_next(itr, (void**)&par));
   fail_if(!(doc = eolian_parameter_documentation_get(par)));
   fail_if(strcmp(eolian_documentation_summary_get(doc),
                  "Another param documentation."));
   fail_if(eolian_documentation_description_get(doc));

   fail_if(eina_iterator_next(itr, (void**)&par));
   eina_iterator_free(itr);

   fail_if(!(doc = eolian_function_return_documentation_get(fid, EOLIAN_METHOD)));
   fail_if(strcmp(eolian_documentation_summary_get(doc),
                  "Return documentation."));
   fail_if(eolian_documentation_description_get(doc));

   fail_if(!(fid = eolian_class_function_by_name_get(class, "prop", EOLIAN_PROPERTY)));
   fimp = eolian_function_implement_get(fid);
   fail_if(!(doc = eolian_implement_documentation_get(fimp, EOLIAN_PROPERTY)));
   fail_if(strcmp(eolian_documentation_summary_get(doc),
                  "Property common documentation."));
   fail_if(eolian_documentation_description_get(doc));
   fail_if(strcmp(eolian_documentation_since_get(doc),
                  "1.18"));
   fail_if(!(doc = eolian_implement_documentation_get(fimp, EOLIAN_PROP_GET)));
   fail_if(strcmp(eolian_documentation_summary_get(doc),
                  "Get documentation."));
   fail_if(eolian_documentation_description_get(doc));
   fail_if(!(doc = eolian_implement_documentation_get(fimp, EOLIAN_PROP_SET)));
   fail_if(strcmp(eolian_documentation_summary_get(doc),
                  "Set documentation."));
   fail_if(eolian_documentation_description_get(doc));

   fail_if(!(itr = eolian_property_values_get(fid, EOLIAN_PROP_GET)));

   fail_if(!eina_iterator_next(itr, (void**)&par));
   fail_if(!(doc = eolian_parameter_documentation_get(par)));
   fail_if(strcmp(eolian_documentation_summary_get(doc),
                  "Value documentation."));
   fail_if(eolian_documentation_description_get(doc));

   fail_if(eina_iterator_next(itr, (void**)&par));
   eina_iterator_free(itr);

   fail_if(!(event = eolian_class_event_by_name_get(class, "clicked")));
   fail_if(!(doc = eolian_event_documentation_get(event)));
   fail_if(strcmp(eolian_documentation_summary_get(doc),
                  "Event docs."));
   fail_if(eolian_documentation_description_get(doc));

   eolian_state_free(eos);
}
EFL_END_TEST

EFL_START_TEST(eolian_function_types)
{
   const Eolian_Typedecl *decl = NULL;
   const Eolian_Typedecl *arg_decl = NULL;
   const Eolian_Type *type = NULL;
   const Eolian_Function *fid = NULL;
   const Eolian_Function_Parameter *param = NULL;
   const Eolian_Unit *unit = NULL;
   Eina_Iterator *iter = NULL;
   const char* type_name = NULL;
   void *dummy;

   Eolian_State *eos = eolian_state_new();

   fail_if(!eolian_state_directory_add(eos, TESTS_SRC_DIR"/data"));

   /* Parsing */
   fail_if(!(unit = eolian_state_file_parse(eos, TESTS_SRC_DIR"/data/function_types.eot")));

   /* void func(void); */
   fail_if(!(decl = eolian_unit_alias_by_name_get(unit, "VoidFunc")));
   fail_if(eolian_typedecl_type_get(decl) != EOLIAN_TYPEDECL_FUNCTION_POINTER);

   fail_if(!(fid = eolian_typedecl_function_pointer_get(decl)));
   fail_if(eolian_function_type_get(fid) != EOLIAN_FUNCTION_POINTER);

   fail_if((eolian_function_return_type_get(fid, EOLIAN_FUNCTION_POINTER))); // void is null_return_type?
   fail_if((eolian_function_parameters_get(fid)));

   /* Function pointer with return and parameters */
   fail_if(!(decl = eolian_unit_alias_by_name_get(unit, "SimpleFunc")));
   fail_if(eolian_typedecl_type_get(decl) != EOLIAN_TYPEDECL_FUNCTION_POINTER);

   fail_if(!(fid = eolian_typedecl_function_pointer_get(decl)));
   fail_if(eolian_function_type_get(fid) != EOLIAN_FUNCTION_POINTER);
   fail_if(strcmp(eolian_function_name_get(fid), "SimpleFunc"));

   fail_if(!(type = eolian_function_return_type_get(fid, EOLIAN_FUNCTION_POINTER))); // void is null_return_type?
   fail_if(!(type_name = eolian_type_c_type_get(type, EOLIAN_C_TYPE_RETURN)));
   fail_if(strcmp(type_name, "const char *"));
   fail_if(!(iter = (eolian_function_parameters_get(fid))));

   fail_if(!(eina_iterator_next(iter, (void**)&param)));
   fail_if(strcmp(eolian_parameter_name_get(param), "a"));
   fail_if(!(type = eolian_parameter_type_get(param)));
   fail_if(!(type_name = eolian_type_c_type_get(type, EOLIAN_C_TYPE_PARAM)));
   fail_if(strcmp(type_name, "int"));

   fail_if(!(eina_iterator_next(iter, (void**)&param)));
   fail_if(strcmp(eolian_parameter_name_get(param), "b"));
   fail_if(!(type = eolian_parameter_type_get(param)));
   fail_if(!(type_name = eolian_type_c_type_get(type, EOLIAN_C_TYPE_PARAM)));
   fail_if(strcmp(type_name, "double"));

   fail_if(eina_iterator_next(iter, &dummy));

   /* Function pointer with parameter attributes (in/out) */
   fail_if(!(decl = eolian_unit_alias_by_name_get(unit, "ComplexFunc")));
   fail_if(eolian_typedecl_type_get(decl) != EOLIAN_TYPEDECL_FUNCTION_POINTER);

   fail_if(!(fid = eolian_typedecl_function_pointer_get(decl)));
   fail_if(eolian_function_type_get(fid) != EOLIAN_FUNCTION_POINTER);

   fail_if(!(type = eolian_function_return_type_get(fid, EOLIAN_FUNCTION_POINTER)));
   fail_if(!(type_name = eolian_type_c_type_get(type, EOLIAN_C_TYPE_RETURN)));
   fail_if(strcmp(type_name, "double"));
   fail_if(!(iter = (eolian_function_parameters_get(fid))));

   /* in string */
   fail_if(!(eina_iterator_next(iter, (void**)&param)));
   fail_if(strcmp(eolian_parameter_name_get(param), "c"));
   fail_if(!(type = eolian_parameter_type_get(param)));
   fail_if(eolian_parameter_direction_get(param) != EOLIAN_IN_PARAM);
   fail_if(eolian_type_is_owned(type));
   fail_if(!(type_name = eolian_type_c_type_get(type, EOLIAN_C_TYPE_PARAM)));
   fail_if(strcmp(type_name, "const char *"));

   /*out own string */
   fail_if(!(eina_iterator_next(iter, (void**)&param)));
   fail_if(strcmp(eolian_parameter_name_get(param), "d"));
   fail_if(eolian_parameter_direction_get(param) != EOLIAN_OUT_PARAM);
   fail_if(!(type = eolian_parameter_type_get(param)));
   fail_if(!eolian_type_is_owned(type));
   fail_if(!(type_name = eolian_type_c_type_get(type, EOLIAN_C_TYPE_PARAM)));
   fail_if(strcmp(type_name, "char *"));

   fail_if(eina_iterator_next(iter, &dummy));

   /* Function pointer receiving another function pointer */
   fail_if(!(decl = eolian_unit_alias_by_name_get(unit, "FuncAsArgFunc")));
   fail_if(eolian_typedecl_type_get(decl) != EOLIAN_TYPEDECL_FUNCTION_POINTER);

   fail_if(!(fid = eolian_typedecl_function_pointer_get(decl)));
   fail_if(eolian_function_type_get(fid) != EOLIAN_FUNCTION_POINTER);

   fail_if(eolian_function_return_type_get(fid, EOLIAN_FUNCTION_POINTER));

   fail_if(!(iter = (eolian_function_parameters_get(fid))));

   fail_if(!(eina_iterator_next(iter, (void**)&param)));
   fail_if(strcmp(eolian_parameter_name_get(param), "cb"));
   fail_if(!(type = eolian_parameter_type_get(param)));
   fail_if(eolian_parameter_direction_get(param) != EOLIAN_IN_PARAM);
   fail_if(eolian_type_is_owned(type));
   fail_if(!(type_name = eolian_type_short_name_get(type)));
   fail_if(strcmp(type_name, "VoidFunc"));
   fail_if(!(arg_decl = eolian_type_typedecl_get(type)));
   fail_if(eolian_typedecl_type_get(arg_decl) != EOLIAN_TYPEDECL_FUNCTION_POINTER);

   fail_if(!(eina_iterator_next(iter, (void**)&param)));
   fail_if(strcmp(eolian_parameter_name_get(param), "another_cb"));
   fail_if(eolian_parameter_direction_get(param) != EOLIAN_IN_PARAM);
   fail_if(!(type = eolian_parameter_type_get(param)));
   fail_if(eolian_type_is_owned(type));
   fail_if(eolian_type_type_get(type) != EOLIAN_TYPE_REGULAR);
   fail_if(!(type_name = eolian_type_short_name_get(type)));
   fail_if(strcmp(type_name, "SimpleFunc"));
   fail_if(!(arg_decl = eolian_type_typedecl_get(type)));
   fail_if(eolian_typedecl_type_get(arg_decl) != EOLIAN_TYPEDECL_FUNCTION_POINTER);

   fail_if(eina_iterator_next(iter, &dummy));

   eolian_state_free(eos);
}
EFL_END_TEST

EFL_START_TEST(eolian_function_as_arguments)
{
   const Eolian_Class *cls = NULL;
   const Eolian_Function *fid = NULL;
   const Eolian_Type *type = NULL;
   const Eolian_Typedecl *arg_decl = NULL;
   const Eolian_Function_Parameter *param = NULL;
   const Eolian_Unit *unit = NULL;
   Eina_Iterator *iter = NULL;
   const char *type_name = NULL;
   void *dummy;

   Eolian_State *eos = eolian_state_new();

   fail_if(!eolian_state_directory_add(eos, TESTS_SRC_DIR"/data"));

   fail_if(!(unit = eolian_state_file_parse(eos, TESTS_SRC_DIR"/data/function_as_argument.eo")));

   fail_if(!(cls = eolian_unit_class_by_name_get(unit, "Function_As_Argument")));

   fail_if(!(fid = eolian_class_function_by_name_get(cls, "set_cb", EOLIAN_METHOD)));

   fail_if(!(iter = (eolian_function_parameters_get(fid))));

   fail_if(!(eina_iterator_next(iter, (void**)&param)));
   fail_if(strcmp(eolian_parameter_name_get(param), "cb"));
   fail_if(!(type = eolian_parameter_type_get(param)));
   fail_if(eolian_parameter_direction_get(param) != EOLIAN_IN_PARAM);
   fail_if(eolian_type_is_owned(type));
   fail_if(!(type_name = eolian_type_short_name_get(type)));
   fail_if(strcmp(type_name, "SimpleFunc"));
   fail_if(!(arg_decl = eolian_type_typedecl_get(type)));
   fail_if(eolian_typedecl_type_get(arg_decl) != EOLIAN_TYPEDECL_FUNCTION_POINTER);

   fail_if(eina_iterator_next(iter, &dummy));

   eolian_state_free(eos);
}
EFL_END_TEST

EFL_START_TEST(eolian_parts)
{
   const Eolian_Unit *unit;
   const Eolian_Class *cls;
   Eina_Iterator *iter;
   Eolian_Part *part;
   int i = 0;

   static const char *part_classes[] = {
      "Override", "Parts"
   };

   Eolian_State *eos = eolian_state_new();

   fail_if(!eolian_state_directory_add(eos, TESTS_SRC_DIR"/data"));

   fail_if(!(unit = eolian_state_file_parse(eos, TESTS_SRC_DIR"/data/parts.eo")));

   fail_if(!(cls = eolian_unit_class_by_name_get(unit, "Parts")));

   fail_if(!(iter = eolian_class_parts_get(cls)));

   EINA_ITERATOR_FOREACH(iter, part)
     {
        const Eolian_Documentation *doc;
        const Eolian_Class *klass;
        char pattern[24];

        sprintf(pattern, "part%d", i + 1);
        ck_assert_str_eq(pattern, eolian_part_name_get(part));

        sprintf(pattern, "Part %d", i + 1);
        fail_if(!(doc = eolian_part_documentation_get(part)));
        ck_assert_str_eq(pattern, eolian_documentation_summary_get(doc));

        fail_if(!(klass = eolian_part_class_get(part)));
        ck_assert_str_eq(part_classes[i], eolian_class_short_name_get(klass));
        i++;
     }
   eina_iterator_free(iter);

   eolian_state_free(eos);
}
EFL_END_TEST

EFL_START_TEST(eolian_mixins_require)
{
   const Eolian_Unit *unit;
   const Eolian_Class *cl;
   const Eolian_Class *base;

   Eolian_State *eos = eolian_state_new();

   fail_if(!eolian_state_directory_add(eos, TESTS_SRC_DIR"/data"));

   fail_if(!(unit = eolian_state_file_parse(eos, TESTS_SRC_DIR"/data/mixins_require.eo")));

   fail_if (!(cl = eolian_state_class_by_name_get(eos, "Mixins.Require")));
   fail_if (!(base = eolian_state_class_by_name_get(eos, "Base")));

   ck_assert_ptr_eq(eolian_class_parent_get(cl), NULL);

   //Check that implements is empty
   {
      Eolian_Class *extc;
      Eina_Iterator *ext = eolian_class_extensions_get (cl);

      EINA_ITERATOR_FOREACH(ext, extc)
        {
           ck_abort_msg("Iterator should be empty");
        }
      eina_iterator_free(ext);
   }
   //check that implements contains this one class
   {
      Eolian_Implement *impl;
      Eina_Iterator *i = eolian_class_implements_get(cl);
      Eina_Array *tmp = eina_array_new(1);

      EINA_ITERATOR_FOREACH(i, impl)
        {
           if (eolian_implement_class_get(impl) != cl)
             {
                eina_array_push(tmp, eolian_implement_class_get(impl));
                ck_assert_ptr_eq(eolian_implement_class_get(impl), base);
             }
        }
      ck_assert_int_eq(eina_array_count(tmp), 1);
      eina_array_free(tmp);
      eina_iterator_free(i);
   }
   //check that the mixins has the right require
   {
      Eina_Iterator *iter = eolian_class_requires_get(cl);
      Eina_Array *tmp = eina_array_new(1);

      EINA_ITERATOR_FOREACH(iter, cl)
        {
           eina_array_push(tmp, cl);
        }
      ck_assert_int_eq(eina_array_count(tmp), 1);
      ck_assert_ptr_eq(eina_array_data_get(tmp, 0), base);
      eina_array_free(tmp);
      eina_iterator_free(iter);
   }
   eolian_state_free(eos);
}
EFL_END_TEST

EFL_START_TEST(eolian_class_requires_classes)
{
   const Eolian_Unit *unit;
   const Eolian_Class *cl;

   Eolian_State *eos = eolian_state_new();

   fail_if(!eolian_state_directory_add(eos, TESTS_SRC_DIR"/data"));

   fail_if(!(unit = eolian_state_file_parse(eos, TESTS_SRC_DIR"/data/class_requires.eo")));

   fail_if (!(cl = eolian_state_class_by_name_get(eos, "Class.Requires")));

   eolian_state_free(eos);
}
EFL_END_TEST

EFL_START_TEST(eolian_class_unimpl)
{
   Eolian_State *eos = eolian_state_new();

   fail_if(!eolian_state_directory_add(eos, TESTS_SRC_DIR"/data"));

   setenv("EOLIAN_CLASS_UNIMPLEMENTED_WARN", "1", 1);
   const Eolian_Unit *unit = eolian_state_file_parse(eos, TESTS_SRC_DIR"/data/unimpl.eo");
   unsetenv("EOLIAN_CLASS_UNIMPLEMENTED_WARN");
   fail_if(!unit);

   eolian_state_free(eos);
}
EFL_END_TEST

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
   tcase_add_test(tc, eolian_null);
   tcase_add_test(tc, eolian_import);
   tcase_add_test(tc, eolian_docs);
   tcase_add_test(tc, eolian_function_types);
   tcase_add_test(tc, eolian_function_as_arguments);
   tcase_add_test(tc, eolian_parts);
   tcase_add_test(tc, eolian_mixins_require);
   tcase_add_test(tc, eolian_class_requires_classes);
   tcase_add_test(tc, eolian_class_unimpl);
}
