
#include <Eolian_Cxx.hh>

#include <cassert>
#include <check.h>
#include <tuple>

START_TEST(eolian_cxx_test_parse_complex_types)
{
   efl::eolian::eolian_init init;
   // fail_if(!::eolian_directory_scan(PACKAGE_DATA_DIR"/data"));
   // fail_if(!::eolian_eo_file_parse(PACKAGE_DATA_DIR"/data/complex_type.eo"));
   // TODO finish
}
END_TEST

START_TEST(eolian_cxx_test_parse_qualifiers)
{
   efl::eolian::eolian_init init;
   // XXX: implement
   //eolian_cxx::type_qualify("");
   //fail_if();
}
END_TEST

void
eolian_cxx_test_parse(TCase* tc)
{
   tcase_add_test(tc, eolian_cxx_test_parse_qualifiers);
   tcase_add_test(tc, eolian_cxx_test_parse_complex_types);
}
