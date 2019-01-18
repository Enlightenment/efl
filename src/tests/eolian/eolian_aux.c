#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eolian_Aux.h>

#include "eolian_suite.h"

EFL_START_TEST(eolian_aux_children)
{
   Eolian_State *eos = eolian_state_new();

   fail_if(!eolian_state_directory_add(eos, TESTS_SRC_DIR"/data_aux"));
   fail_if(!eolian_state_file_parse(eos, TESTS_SRC_DIR"/data_aux/aux_a.eo"));
   fail_if(!eolian_state_file_parse(eos, TESTS_SRC_DIR"/data_aux/aux_b.eo"));

   Eina_Hash *chash = eolian_aux_state_class_children_find(eos);
   fail_if(!chash);

   const Eolian_Class *acl = eolian_state_class_by_name_get(eos, "Aux_A");
   fail_if(!acl);

   const Eolian_Class *bcl = eolian_state_class_by_name_get(eos, "Aux_B");
   fail_if(!bcl);

   const Eolian_Class *ccl = eolian_state_class_by_name_get(eos, "Aux_C");
   fail_if(!ccl);

   Eina_List *cl = eina_hash_find(chash, &ccl);
   fail_if(!cl);

   fail_if(eina_list_count(cl) != 2);
   if (eina_list_nth(cl, 0) == bcl)
     fail_if(eina_list_nth(cl, 1) != acl);
   else
     {
        fail_if(eina_list_nth(cl, 0) != acl);
        fail_if(eina_list_nth(cl, 1) != bcl);
     }

   eina_hash_free(chash);
   eolian_state_free(eos);
}
EFL_END_TEST

EFL_START_TEST(eolian_aux_implements)
{
   Eolian_State *eos = eolian_state_new();

   fail_if(!eolian_state_directory_add(eos, TESTS_SRC_DIR"/data_aux"));
   fail_if(!eolian_state_file_parse(eos, TESTS_SRC_DIR"/data_aux/aux_a.eo"));
   fail_if(!eolian_state_file_parse(eos, TESTS_SRC_DIR"/data_aux/aux_b.eo"));

   Eina_Hash *chash = eolian_aux_state_class_children_find(eos);
   fail_if(!chash);

   const Eolian_Class *ccl = eolian_state_class_by_name_get(eos, "Aux_C");
   fail_if(!ccl);

   const Eolian_Function *fn = eolian_class_function_by_name_get(ccl, "foo", EOLIAN_METHOD);
   fail_if(!fn);

   Eina_List *imps = eolian_aux_function_all_implements_get(fn, chash);
   fail_if(!imps);
   fail_if(eina_list_count(imps) != 2);

   eina_list_free(imps);
   eina_hash_free(chash);
   eolian_state_free(eos);
}
EFL_END_TEST

EFL_START_TEST(eolian_aux_callables)
{
   Eolian_State *eos = eolian_state_new();

   fail_if(!eolian_state_directory_add(eos, TESTS_SRC_DIR"/data_aux"));
   fail_if(!eolian_state_file_parse(eos, TESTS_SRC_DIR"/data_aux/aux_a.eo"));

   const Eolian_Class *acl = eolian_state_class_by_name_get(eos, "Aux_A");
   fail_if(!acl);

   Eina_List *funcs = NULL;
   Eina_List *evs = NULL;
   size_t ofuncs = 0;
   size_t oevs = 0;

   size_t clbls = eolian_aux_class_callables_get(acl, &funcs, &evs, &ofuncs, &oevs);
   fail_if(clbls != 5);
   fail_if(ofuncs != 3);
   fail_if(oevs != 1);
   fail_if(eina_list_count(funcs) != 3);
   fail_if(eina_list_count(evs) != 2);

   eina_list_free(funcs);
   eina_list_free(evs);
   eolian_state_free(eos);
}
EFL_END_TEST

EFL_START_TEST(eolian_aux_implparent)
{
   Eolian_State *eos = eolian_state_new();

   fail_if(!eolian_state_directory_add(eos, TESTS_SRC_DIR"/data_aux"));
   fail_if(!eolian_state_file_parse(eos, TESTS_SRC_DIR"/data_aux/aux_a.eo"));

   const Eolian_Class *acl = eolian_state_class_by_name_get(eos, "Aux_A");
   fail_if(!acl);

   const Eolian_Class *ccl = eolian_state_class_by_name_get(eos, "Aux_C");
   fail_if(!ccl);

   const Eolian_Function *fn = eolian_class_function_by_name_get(ccl, "foo", EOLIAN_METHOD);
   fail_if(!fn);

   const Eolian_Implement *impl = NULL;
   Eina_Iterator *itr = eolian_class_implements_get(acl);
   EINA_ITERATOR_FOREACH(itr, impl)
     {
        if (eolian_implement_function_get(impl, NULL) == fn)
          break;
     }
   eina_iterator_free(itr);

   fail_if(!impl);
   fail_if(eolian_implement_function_get(impl, NULL) != fn);

   fail_if(eolian_implement_class_get(impl) != ccl);
   fail_if(eolian_implement_implementing_class_get(impl) != acl);

   const Eolian_Implement *pimpl = eolian_aux_implement_parent_get(impl);
   fail_if(eolian_implement_class_get(pimpl) != ccl);
   fail_if(eolian_implement_implementing_class_get(pimpl) != ccl);

   eolian_state_free(eos);
}
EFL_END_TEST

void eolian_aux_test(TCase *tc)
{
   tcase_add_test(tc, eolian_aux_children);
   tcase_add_test(tc, eolian_aux_implements);
   tcase_add_test(tc, eolian_aux_callables);
   tcase_add_test(tc, eolian_aux_implparent);
}
