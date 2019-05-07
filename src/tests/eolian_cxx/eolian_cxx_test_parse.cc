#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <cassert>
#include <tuple>

#include <Eolian_Cxx.hh>

#include "eolian_cxx_suite.h"

EFL_START_TEST(eolian_cxx_test_parse_complex_types)
{
   efl::eolian::eolian_init init;
   efl::eolian::eolian_state state;
   fail_if(!::eolian_state_directory_add(state.value, EO_SRC_DIR));
   fail_if(!::eolian_state_directory_add(state.value, TESTS_SRC_DIR));
   fail_if(!::eolian_state_file_parse(state.value, "complex.eo"));
   // TODO finish
}
EFL_END_TEST

EFL_START_TEST(eolian_cxx_test_parse_qualifiers)
{
   efl::eolian::eolian_init init;
   // XXX: implement
   //eolian_cxx::type_qualify("");
   //fail_if();
}
EFL_END_TEST

void
eolian_cxx_test_parse(TCase* tc)
{
   tcase_add_test(tc, eolian_cxx_test_parse_qualifiers);
   tcase_add_test(tc, eolian_cxx_test_parse_complex_types);
}
