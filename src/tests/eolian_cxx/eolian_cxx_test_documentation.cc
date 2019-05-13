#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <iostream>
#include <cassert>
#include <iterator>
#include <algorithm>

#include <Eina.hh>
#include <Eolian_Cxx.hh>

#include "eolian_cxx_suite.h"

#include "grammar/klass_def.hpp"

using efl::eolian::grammar::attributes::klass_def;
using efl::eolian::grammar::attributes::documentation_def;
using efl::eolian::grammar::attributes::function_def;
using efl::eolian::grammar::attributes::enum_def;
using efl::eolian::grammar::attributes::struct_def;

klass_def init_test_data(efl::eolian::eolian_state const& state)
{
   ck_assert(::eolian_state_directory_add(state.value, TESTS_SRC_DIR));
   ck_assert(::eolian_state_all_eot_files_parse(state.value));
   ck_assert(::eolian_state_file_parse(state.value, "docs.eo"));

   const Eolian_Class *c_klass = ::eolian_state_class_by_name_get(state.value, "Docs");
   ck_assert_ptr_ne(c_klass, NULL);

   klass_def klass(c_klass, state.as_unit());
   return klass;
}

EFL_START_TEST(eolian_cxx_test_class_docs)
{
   efl::eina::eina_init eina_init;
   efl::eolian::eolian_init eolian_init;
   efl::eolian::eolian_state eolian_state;

   klass_def klass = init_test_data(eolian_state);
   documentation_def doc = klass.documentation;
   ck_assert_str_eq(doc.summary.c_str(), "Docs for class.");
   ck_assert_str_eq(doc.description.c_str(),
           "More docs for class. Testing references now. "
           "@Foo "
           "@Bar "
           "@Alias "
           "@pants "
           "@Docs.meth "
           "@Docs.prop "
           "@Docs.prop.get "
           "@Docs.prop.set "
           "@Foo.field1 "
           "@Bar.foo "
           "@Docs");
   ck_assert_str_eq(doc.since.c_str(), "1.17");
}
EFL_END_TEST

EFL_START_TEST(eolian_cxx_test_function_docs)
{
   efl::eina::eina_init eina_init;
   efl::eolian::eolian_init eolian_init;
   efl::eolian::eolian_state eolian_state;

   klass_def klass = init_test_data(eolian_state);
   function_def func = *std::find_if(klass.functions.begin(), klass.functions.end(),
           [](const function_def& f) {
             return f.name == "meth";
           });

   documentation_def doc = func.documentation;
   ck_assert_str_eq(doc.summary.c_str(), "Method documentation.");
   ck_assert_str_eq(doc.description.c_str(), "");
   ck_assert_str_eq(doc.since.c_str(), "1.17"); // Since is inherited from parent if not present, except when no doc is present ofr this member.

   doc = func.return_documentation;
   ck_assert_str_eq(doc.summary.c_str(), "Return documentation.");
   ck_assert_str_eq(doc.description.c_str(), "");
   ck_assert_str_eq(doc.since.c_str(), "1.17");

   auto param_iter = func.parameters.begin();

   // a int
   doc = param_iter->documentation;
   ck_assert_str_eq(doc.summary.c_str(), "Param documentation.");
   ck_assert_str_eq(doc.description.c_str(), "");
   ck_assert_str_eq(doc.since.c_str(), "1.17");

   param_iter++;

   // b float (no doc)
   doc = param_iter->documentation;
   ck_assert_str_eq(doc.summary.c_str(), "");
   ck_assert_str_eq(doc.description.c_str(), "");
   ck_assert_str_eq(doc.since.c_str(), "");

   param_iter++;

   // c long
   doc = param_iter->documentation;
   ck_assert_str_eq(doc.summary.c_str(), "Another param documentation.");
   ck_assert_str_eq(doc.description.c_str(), "");
   ck_assert_str_eq(doc.since.c_str(), "1.17");
}
EFL_END_TEST

EFL_START_TEST(eolian_cxx_test_property_docs)
{
   efl::eina::eina_init eina_init;
   efl::eolian::eolian_init eolian_init;
   efl::eolian::eolian_state eolian_state;

   klass_def klass = init_test_data(eolian_state);
   auto func_iter = std::find_if(klass.functions.begin(), klass.functions.end(),
           [](const function_def& f) {
             return f.name == "prop_get";
           });

   ck_assert_msg(func_iter != klass.functions.end(), "Failed to find prop_get function");
   function_def func = *func_iter;

   documentation_def doc = func.property_documentation;
   ck_assert_str_eq(doc.summary.c_str(), "Property common documentation.");
   ck_assert_str_eq(doc.description.c_str(), "");
   ck_assert_str_eq(doc.since.c_str(), "1.18");

   doc = func.documentation; // Actual getdocumentation.
   ck_assert_str_eq(doc.summary.c_str(), "Get documentation.");
   ck_assert_str_eq(doc.description.c_str(), "");
   ck_assert_str_eq(doc.since.c_str(), "1.17"); // Members inherit from parent *class*

   func_iter = std::find_if(klass.functions.begin(), klass.functions.end(),
           [](const function_def& f) {
             return f.name == "prop_set";
           });

   ck_assert_msg(func_iter != klass.functions.end(), "Failed to find prop_set function");
   func = *func_iter;

   doc = func.documentation; // Actual getdocumentation.
   ck_assert_str_eq(doc.summary.c_str(), "Set documentation.");
   ck_assert_str_eq(doc.description.c_str(), "");
   ck_assert_str_eq(doc.since.c_str(), "1.17"); // Members inherit from parent *class*

   auto property_iter = klass.properties.begin();
   auto property = *property_iter;
   doc = property.documentation;
   ck_assert_str_eq(doc.summary.c_str(), "Property common documentation.");
   ck_assert_str_eq(doc.since.c_str(), "1.18");
}
EFL_END_TEST

EFL_START_TEST(eolian_cxx_test_event_docs)
{
   efl::eina::eina_init eina_init;
   efl::eolian::eolian_init eolian_init;
   efl::eolian::eolian_state eolian_state;

   klass_def klass = init_test_data(eolian_state);

   auto event = klass.events.front();
   documentation_def doc = event.documentation;
   ck_assert_str_eq(doc.summary.c_str(), "Event docs.");
   ck_assert_str_eq(doc.description.c_str(), "");
   ck_assert_str_eq(doc.since.c_str(), "1.17"); // Members inherit from parent *class*
}
EFL_END_TEST

EFL_START_TEST(eolian_cxx_test_enum_docs)
{
   efl::eina::eina_init eina_init;
   efl::eolian::eolian_init eolian_init;
   efl::eolian::eolian_state eolian_state;

   klass_def klass = init_test_data(eolian_state);

   auto unit = eolian_state.as_unit();
   enum_def _enum(::eolian_state_enum_by_name_get(eolian_state.value, "Bar"), unit);

   documentation_def doc = _enum.documentation;
   ck_assert_str_eq(doc.summary.c_str(), "Docs for enum Bar.");
   ck_assert_str_eq(doc.description.c_str(), "");
   ck_assert_str_eq(doc.since.c_str(), "");

   // fields
   auto field_iter = _enum.fields.begin();

   // blah - no docs
   doc = field_iter->documentation;
   ck_assert_str_eq(doc.summary.c_str(), "");
   ck_assert_str_eq(doc.description.c_str(), "");
   ck_assert_str_eq(doc.since.c_str(), "");

   field_iter++;

   // foo - docs
   doc = field_iter->documentation;
   ck_assert_str_eq(doc.summary.c_str(), "Docs for foo.");
   ck_assert_str_eq(doc.description.c_str(), "");
   ck_assert_str_eq(doc.since.c_str(), "");

   field_iter++;

   // bar - docs
   doc = field_iter->documentation;
   ck_assert_str_eq(doc.summary.c_str(), "Docs for bar.");
   ck_assert_str_eq(doc.description.c_str(), "");
   ck_assert_str_eq(doc.since.c_str(), "");
}
EFL_END_TEST

EFL_START_TEST(eolian_cxx_test_struct_docs)
{
   efl::eina::eina_init eina_init;
   efl::eolian::eolian_init eolian_init;
   efl::eolian::eolian_state eolian_state;

   klass_def klass = init_test_data(eolian_state);

   auto unit = eolian_state.as_unit();
   struct_def _struct(::eolian_state_struct_by_name_get(eolian_state.value, "Foo"), unit);

   documentation_def doc = _struct.documentation;
   ck_assert_str_eq(doc.summary.c_str(), "This is struct Foo. "
                                         "It does stuff.");
   ck_assert_str_eq(doc.description.c_str(),
                    "Note: This is a note.\n"
                    "\n"
                    "This is a longer description for struct Foo.\n"
                    "\n"
                    "Warning: This is a warning. You can only use Warning: and "
                    "Note: at the beginning of a paragraph.\n"
                    "\n"
                    "This is another paragraph.");
   ck_assert_str_eq(doc.since.c_str(), "1.66");

   std::vector<std::string> ref_paragraphs = {
       "Note: This is a note.",
       "This is a longer description for struct Foo.",
       "Warning: This is a warning. You can only use Warning: and "
       "Note: at the beginning of a paragraph.",
       "This is another paragraph."
   };

   auto paragraph_it = doc.desc_paragraphs.begin();
   auto ref_paragraph_it = ref_paragraphs.begin();

   while (ref_paragraph_it != ref_paragraphs.end())
     {
        ck_assert_str_eq(paragraph_it->c_str(), ref_paragraph_it->c_str());
        paragraph_it++;
        ref_paragraph_it++;
     }

   ck_assert(paragraph_it == doc.desc_paragraphs.end());


   // fields
   auto field_iter = _struct.fields.begin();

   doc = field_iter->documentation;
   ck_assert_str_eq(doc.summary.c_str(), "Field documentation.");
   ck_assert_str_eq(doc.description.c_str(), "");
   ck_assert_str_eq(doc.since.c_str(), "");

   field_iter++;

   doc = field_iter->documentation;
   ck_assert_str_eq(doc.summary.c_str(), "");
   ck_assert_str_eq(doc.description.c_str(), "");
   ck_assert_str_eq(doc.since.c_str(), "");

   field_iter++;

   doc = field_iter->documentation;
   ck_assert_str_eq(doc.summary.c_str(), "Another field documentation.");
   ck_assert_str_eq(doc.description.c_str(), "");
   ck_assert_str_eq(doc.since.c_str(), "");
}
EFL_END_TEST

void
eolian_cxx_test_documentation(TCase* tc)
{
   tcase_add_test(tc, eolian_cxx_test_class_docs);
   tcase_add_test(tc, eolian_cxx_test_function_docs);
   tcase_add_test(tc, eolian_cxx_test_property_docs);
   tcase_add_test(tc, eolian_cxx_test_event_docs);
   tcase_add_test(tc, eolian_cxx_test_enum_docs);
   tcase_add_test(tc, eolian_cxx_test_struct_docs);
}

