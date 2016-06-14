#ifndef EOLIAN_CXX_CLASS_DEFINITION_HH
#define EOLIAN_CXX_CLASS_DEFINITION_HH

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"

#include "grammar/indentation.hpp"
#include "grammar/list.hpp"
#include "grammar/alternative.hpp"
#include "grammar/type.hpp"
#include "grammar/parameter.hpp"
#include "grammar/function_declaration.hpp"
#include "grammar/case.hpp"
#include "grammar/address_of.hpp"
#include "grammar/attribute_reorder.hpp"
#include "grammar/attribute_conditional.hpp"
#include "grammar/attribute_replace.hpp"

namespace efl { namespace eolian { namespace grammar {

struct class_definition_generator
{
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::klass_def const& cls, Context const& context) const
   {
     std::vector<std::string> cpp_namespaces = attributes::cpp_namespaces(cls.namespaces);
     auto open_namespace = *("namespace " << string << " { ") << "\n";
     if(!as_generator(open_namespace).generate(sink, cpp_namespaces, add_lower_case_context(context))) return false;

     if(!as_generator
        (
         "struct " << string << " : ::efl::eo::concrete"
         )
        .generate(sink, cls.cxx_name, context))
       return false;
     for(auto&& i : cls.inherits)
       {
         if(!as_generator("\n" << scope_tab << ", EO_CXX_INHERIT(" << *(" ::" << lower_case[string]) << "::" << string << ")")
            .generate(sink, std::make_tuple(attributes::cpp_namespaces(i.namespaces), i.eolian_name), context))
           return false;
       }
     if(!as_generator("\n{\n").generate(sink, attributes::unused, context)) return false;

     // constructors
     if(!as_generator
        (
            scope_tab << "explicit " << string << "( ::Eo* eo)\n"
         << scope_tab << scope_tab << ": ::efl::eo::concrete(eo) {}\n"
         << scope_tab << "explicit " << string << "(std::nullptr_t)\n"
         << scope_tab << scope_tab << ": ::efl::eo::concrete(nullptr) {}\n"
         << scope_tab << string << "(" << string << " const& other) = default;\n"
         << scope_tab << string << "(" << string << "&& other) = default;\n"
         << scope_tab << string << "& operator=(" << string << " const& other) = default;\n"
         << scope_tab << string << "& operator=(" << string << "&& other) = default;\n"
         << scope_tab << string << "()\n"
         << scope_tab << "{\n"
         << scope_tab << scope_tab << "::efl::eolian::do_eo_add( ::efl::eo::concrete::_eo_raw, ::efl::eo::concrete{nullptr}, _eo_class());\n"
         << scope_tab << "}\n"
         << scope_tab << string << "( ::efl::eo::concrete parent)\n"
         << scope_tab << "{\n"
         << scope_tab << scope_tab << "::efl::eolian::do_eo_add( ::efl::eo::concrete::_eo_raw, parent, _eo_class());\n"
         << scope_tab << "}\n"
         << scope_tab << "template <typename F> " << string << "(F f, typename ::std::enable_if< ::efl::eolian::is_callable<F>::value>::type* = 0)\n"
         << scope_tab << "{\n"
         << scope_tab << scope_tab << "::efl::eolian::do_eo_add( ::efl::eo::concrete::_eo_raw, ::efl::eo::concrete{nullptr}, _eo_class(), f);\n"
         << scope_tab << "}\n"
         // << scope_tab << "explicit " << string << "( ::efl::eo::concrete const& parent)\n"
         // << scope_tab << scope_tab << ": ::efl::eo::concrete( ::efl::eo::do_eo_add(parent)) {}\n"
         // << scope_tab << "template <typename F>\n"
         // << scope_tab << "explicit " << string << "( ::efl::eo::concrete const& parent, F f)\n"
         // << scope_tab << scope_tab << ": ::efl::eo::concrete( ::efl::eo::do_eo_add(parent, f)) {}\n"
         // << scope_tab << "template <typename F>\n"
         // << scope_tab << "explicit " << string << "(F f)\n"
         // << scope_tab << scope_tab << ": ::efl::eo::concrete( ::efl::eo::do_eo_add( ::efl::eo::concrete{nullptr}, f)) {}\n"
        ).generate(sink, attributes::make_infinite_tuple(cls.cxx_name), context)) return false;
     
     if(!as_generator(*(scope_tab << function_declaration))
        .generate(sink, cls.functions, context)) return false;
                                             
     // static Eo_Class const* _eo_class();
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

     if(!as_generator
        (
            scope_tab << "static Eo_Class const* _eo_class()\n"
            << scope_tab << "{\n"
            << scope_tab << scope_tab << "return "
        ).generate(sink,  attributes::unused, context)) return false;
     if(!as_generator
        (*(string << "_") << string << "_" << string)
        .generate(sink, std::make_tuple(cls.namespaces, cls.eolian_name, suffix), add_upper_case_context(context)))
       return false;
     if(!as_generator(";\n" << scope_tab << "}\n").generate(sink, attributes::unused, context)) return false;

     if(!as_generator
        (
         scope_tab << "Eo* _eo_ptr() const { return *(Eo**)this; }\n"
        ).generate(sink, attributes::unused, context)) return false;
     
     for (auto&& e : cls.events)
       {
          if (e.beta)
            {
               suffix = "BETA";
               if(!as_generator
                     ("#ifdef " << *(string << "_") << string << "_" << string << "\n")
                     .generate(sink, std::make_tuple(cls.namespaces, cls.eolian_name, suffix), add_upper_case_context(context)))
                 return false;
            }
          if (e.protect)
            {
               suffix = "PROTECTED";
               if(!as_generator
                     ("#ifdef " << *(string << "_") << string << "_" << string << "\n")
                     .generate(sink, std::make_tuple(cls.namespaces, cls.eolian_name, suffix), add_upper_case_context(context)))
                 return false;
            }
          if(!as_generator
             (
              *attribute_reorder<1, 2, 0, 1>
              ((scope_tab << "static struct " << string_replace(',', '_') << "_event\n"
                << scope_tab << "{\n"
                << scope_tab << scope_tab << "static Eo_Event_Description const* description()\n"
                << scope_tab << scope_tab << "{ return " << string << "; }\n"
                << scope_tab << scope_tab << "typedef "
                << (attribute_conditional([] (eina::optional<attributes::type_def> t) { return !!t; })
                    [attribute_replace([] (eina::optional<attributes::type_def> t) { return *t; }) [type]]
                    | "void")
                << " parameter_type;\n"
                << scope_tab << "} const " << string_replace(',', '_') << "_event;\n"
             ))).generate(sink, std::vector<attributes::event_def>{e}, context))
            return false;
          if (e.beta && !as_generator("#endif\n").generate(sink, attributes::unused, context))
            return false;
          if (e.protect && !as_generator("#endif\n").generate(sink, attributes::unused, context))
            return false;
       }
     
     // /// @cond LOCAL
     if(!as_generator(scope_tab << "/// @cond LOCAL\n").generate(sink, attributes::unused, context)) return false;
     
     if(!as_generator(address_of).generate(sink, cls, context)) return false;

     // /// @endcond
     if(!as_generator(scope_tab << "/// @endcond\n").generate(sink, attributes::unused, context)) return false;

     if(!as_generator("};\n").generate(sink, attributes::unused, context)) return false;

     // static asserts
     if(!as_generator("static_assert(sizeof(" << string << ") == sizeof(Eo*), \"\");\n")
        .generate(sink, cls.cxx_name, context)) return false;
     if(!as_generator("static_assert(std::is_standard_layout<" << string << ">::value, \"\");\n")
        .generate(sink, cls.cxx_name, context)) return false;

     auto close_namespace = *(lit("} ")) << "\n";
     if(!as_generator(close_namespace).generate(sink, cpp_namespaces, context)) return false;
     
     return true;
   }
};

template <>
struct is_eager_generator<class_definition_generator> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed<class_definition_generator> : std::integral_constant<int, 1> {};
}
      
class_definition_generator const class_definition = {};
      
} } }

#endif
