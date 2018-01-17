#ifndef EOLIAN_JS_CLASS_DEFINITION_HPP
#define EOLIAN_JS_CLASS_DEFINITION_HPP

#include <grammar/klass_def.hpp>
#include <grammar/indentation.hpp>
#include <grammar/string.hpp>
#include <grammar/attribute_reorder.hpp>
#include <grammar/attributed.hpp>
#include <grammar/attribute_replace.hpp>
#include <grammar/if.hpp>
#include <grammar/empty_generator.hpp>
#include <eolian/grammar/stub_function_definition.hpp>

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
       *stub_function_definition(cls)
      ).generate(sink, cls.functions, context);
    
    as_generator
      (
       attribute_reorder<0, 1, 2, 3, 0, 1, 0, 1>
       (
          "static void register_prototype_"
       << lower_case[*(string << "_")]
       << lower_case[string]
       << "(v8::Handle<v8::Object> global, v8::Local<v8::ObjectTemplate> prototype, v8::Isolate* isolate)\n"
       << "{\n"
       << *
          if_([] (attributes::function_def const& f) { return !f.is_beta && !f.is_protected; })
          [
           attribute_reorder<1, 1, 1, 1>
           (
               scope_tab << "prototype->Set( ::efl::eina::js::compatibility_new<v8::String>(isolate, \""
            << attribute_replace(&eolian::js::format::format_method)[string] << "\")\n"
            << scope_tab(2) << ", ::efl::eina::js::compatibility_new<v8::FunctionTemplate>(isolate\n"
            << scope_tab(3) << ", & ::efl::js::binding::stub_function_"
            << attributed(std::make_tuple(cls.namespaces, cls.cxx_name))
               [
                   lower_case[*(string << "_")]
                << lower_case[string] << "_"
               ]
            << lower_case[string]
            << "));\n"
           )
          ][empty_generator]           
       << 
        *(
            scope_tab << "register_prototype_"
            << lower_case[*(string << "_")]
            << lower_case[string]
            << "(global, prototype, isolate);\n"
         )
       << "}\nvoid register_constructor_"
       << lower_case[*(string << "_")]
       << lower_case[string]
       << "(v8::Handle<v8::Object> global, v8::Local<v8::FunctionTemplate> constructor, v8::Isolate* isolate)\n"
       << "{\n"
       << scope_tab << "v8::Local<v8::ObjectTemplate> instance = constructor->InstanceTemplate();\n"
       << scope_tab << "instance->SetInternalFieldCount(1);\n"
       << scope_tab << "v8::Local<v8::ObjectTemplate> prototype = constructor->PrototypeTemplate();\n"
       << scope_tab << "register_prototype_"
       << lower_case[*(string << "_")]
       << lower_case[string]
       << "(global, prototype, isolate);\n"
       << "}\n"
        )).generate(sink, std::make_tuple(cls.namespaces, cls.cxx_name, cls.functions, cls.immediate_inherits
                                          , cls.eolian_name), context);
    
    as_generator
      (
       attribute_reorder<0, 1, 0, 1, 0, 1, 0, 1>
       (
         "v8::Local<v8::Function> register_"
       << lower_case[*(string << "_")]
       << lower_case[string]
       << "(v8::Handle<v8::Object> global, v8::Isolate* isolate)\n"
       << "{\n"
       << scope_tab << "v8::Handle<v8::FunctionTemplate> constructor = ::efl::eina::js::compatibility_new<v8::FunctionTemplate>\n"
       << scope_tab(2) << "(isolate, & ::efl::eo::js::constructor, ::efl::eina::js::compatibility_new<v8::External>(isolate, const_cast<Efl_Class*>("<< class_name << ")));\n"
       << scope_tab << "register_constructor_"
       << lower_case[*(string << "_")]
       << lower_case[string]
       << "(global, constructor, isolate);\n"
       << scope_tab << "(void)" << class_name << ";\n"
       << scope_tab << "(void)global; (void)isolate; (void)constructor;\n"
       << scope_tab << "return constructor->GetFunction();\n"
       << "}\n"
       << "struct register_"
       << lower_case[*(string << "_")]
       << lower_case[string]
       << "_functor\n{\n"
       << scope_tab << "v8::Local<v8::Function> operator()(v8::Handle<v8::Object> global, v8::Isolate* isolate)\n"
       << scope_tab << "{\n" << scope_tab(2) << "return ::efl::js::binding::register_"
       << lower_case[*(string << "_")]
       << lower_case[string]
       << "(global, isolate);\n"
       << scope_tab << "}\n"
       << "};\n"
       )).generate(sink, std::make_tuple(cls.namespaces, cls.cxx_name), context);

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
