#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <iostream>
#include <cassert>
#include <iterator>

#include "eolian_cxx_suite.h"

#include <grammar/header.hpp>

EFL_START_TEST(eolian_cxx_test_generate_complex_types)
{
   using efl::eolian::grammar::class_header;
   using efl::eolian::grammar::attributes::unused_type;
   using efl::eolian::grammar::attributes::regular_type_def;
   using efl::eolian::grammar::attributes::klass_name;
   using efl::eolian::grammar::attributes::complex_type_def;
   using efl::eolian::grammar::attributes::parameter_direction;
   using efl::eolian::grammar::attributes::qualifier_info;
   using efl::eolian::grammar::context_null;;

   // efl::eolian::grammar::attributes::klass_def my_class
   // {
   //      "Class_Name", "Class_Name", {"Namespace1", "Namesapce2"}
   //    , {
   //        {{regular_type_def{"int", {qualifier_info::is_none, {}}, {}}}
   //        , "function_name"
   //        , {
   //            {parameter_direction::in,    {regular_type_def{"unsigned", {qualifier_info::is_none, {}}, {}}}, "param1", ""}
   //          , {parameter_direction::out,   {klass_name{{"Namespace1","Namesapce2"}, "Class_Name",
   //                                                                                    {qualifier_info::is_none, {}}, {}, {}}}
   //              , "param2", ""}
   //          , {parameter_direction::inout, {complex_type_def
   //                  {{
   //                    {regular_type_def{"list", {qualifier_info::is_none, {}}, {}}}
   //                  , {regular_type_def{"int",  {qualifier_info::is_none, {}}, {}}}
   //                  }}}
   //              , "param3", ""}
   //          }
   //        }
   //      }
   //    , {}
   //    , {}
   // };
   
   // std::tuple<std::string, std::vector<std::string>
   //            , std::vector<std::string>, std::vector<efl::eolian::grammar::attributes::klass_def>
   //            , std::vector<efl::eolian::grammar::attributes::klass_def>
   //            , std::vector<efl::eolian::grammar::attributes::klass_def>> attributes
   //   {"GUARD_HEADER_HH", {"abc.h", "def.h"}, {"abc.hh", "def.hh"}, {my_class}, {my_class}, {my_class}};
   // std::vector<char> buffer;
   // class_header.generate(std::back_inserter<std::vector<char>>(buffer), attributes, context_null());

   // const char result[] =
   //   "#ifndef GUARD_HEADER_HH\n"
   //   "#define GUARD_HEADER_HH\n"
   //   "#endif\n"
   //   ;
   
   // std::cout << "Beginning of generated file" << std::endl;
   // std::copy(buffer.begin(), buffer.end(), std::ostream_iterator<char>(std::cout));
   // std::cout << "\n End of generated file" << std::endl;

   // ck_assert(buffer.size() == (sizeof(result) - 1));
   // ck_assert(std::equal(buffer.begin(), buffer.end(), result));
}
EFL_END_TEST

void
eolian_cxx_test_generate(TCase* tc)
{
   tcase_add_test(tc, eolian_cxx_test_generate_complex_types);
}
