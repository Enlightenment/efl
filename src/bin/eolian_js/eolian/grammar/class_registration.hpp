#ifndef EOLIAN_JS_CLASS_DEFINITION_HPP
#define EOLIAN_JS_CLASS_DEFINITION_HPP

#include <grammar/klass_def.hpp>
#include <grammar/indentation.hpp>
#include <grammar/string.hpp>

namespace eolian { namespace js { namespace grammar {

namespace attributes = efl::eolian::grammar::attributes;

struct class_registration_generator
{
  template <typename OutputIterator, typename Context>
  bool generate(OutputIterator sink, attributes::klass_def const& cls, Context const& context) const
  {
    using namespace efl::eolian::grammar;
    using efl::eolian::grammar::attributes::unused;

     std::string suffix;
     switch(cls.type)
       {
       case attributes::class_type::regular:
       case attributes::class_type::abstract_:
         suffix = "CLASS";
         break;
       case attributes::class_type::mixin:
         suffix = "MIXIN";
         break;
       case attributes::class_type::interface_:
         suffix = "INTERFACE";
         break;
       }
    
    std::string class_name;
    as_generator(upper_case[*(string << "_")] << upper_case[string] << "_" << suffix)
      .generate(std::back_inserter(class_name), std::make_tuple(cls.namespaces, cls.cxx_name), context_null{});

    as_generator
      (
          "namespace efl { namespace js { namespace binding { namespace {\nstatic void register_"
       << lower_case[*(string << "_")]
       << lower_case[string]
       << "(v8::Handle<v8::Object> global, v8::Isolate* isolate)\n"
       << "{\n"
       << scope_tab << "v8::Handle<v8::FunctionTemplate> constructor = ::efl::eina::js::compatibility_new<v8::FunctionTemplate>\n"
       << scope_tab << scope_tab << "(isolate, & ::efl::eo::js::constructor, ::efl::eina::js::compatibility_new<v8::External>(isolate, const_cast<Efl_Class*>("<< class_name << ")));\n"
       << scope_tab << "(void)" << class_name << ";\n"
       << scope_tab << "(void)global; (void)isolate; (void)constructor;\n"
       << "}\n"
       << "} } } } // namespace efl { namespace js { namespace binding { namespace {\n\n"
       ).generate(sink, std::make_tuple(cls.namespaces, cls.cxx_name), context);

    // constructors ?

    // v8::Handle<v8::FunctionTemplate> constructor = ::efl::eina::js::compatibility_new<v8::FunctionTemplate>
    //  (isolate, &efl::eo::js::constructor, efl::eina::js::compatibility_new<v8::External>(isolate, const_cast<Efl_Class*>(EFL_UI_WIN_CLASS)));

    
    // methods

    // events
    
    return true;
  }
};


class_registration_generator const class_registration = {};
      
} } }

namespace efl { namespace eolian { namespace grammar {

template <>
struct is_generator< ::eolian::js::grammar::class_registration_generator> : std::true_type {};
template <>
struct is_eager_generator< ::eolian::js::grammar::class_registration_generator> : std::true_type {};
      
} } }

#endif
