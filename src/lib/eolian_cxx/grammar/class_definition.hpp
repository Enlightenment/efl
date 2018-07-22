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
#include "grammar/part_declaration.hpp"

namespace efl { namespace eolian { namespace grammar {

struct class_definition_generator
{
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::klass_def const& cls, Context const& context) const
   {
     std::vector<std::string> cpp_namespaces = attributes::cpp_namespaces(cls.namespaces);
     auto open_namespace = *("namespace " << string << " { ") << "\n";
     if(!as_generator(open_namespace).generate(sink, cpp_namespaces, add_lower_case_context(context))) return false;

#ifdef USE_EOCXX_INHERIT_ONLY
     if(!as_generator("struct " << string << " : private ::efl::eo::concrete\n"
                      << scope_tab << ", ::eo_cxx"
                      << *("::" << lower_case[string]) << "::" << string)
           .generate(sink, std::make_tuple(cls.cxx_name, attributes::cpp_namespaces(cls.namespaces), cls.cxx_name), context))
       return false;
#else
     if(!as_generator("struct " << string << " : private ::efl::eo::concrete")
        .generate(sink, cls.cxx_name, context))
       return false;
#endif

     for(auto&& i : cls.inherits)
       {
         if(!as_generator("\n" << scope_tab << ", EO_CXX_INHERIT(" << *("::" << lower_case[string]) << "::" << string << ")")
            .generate(sink, std::make_tuple(attributes::cpp_namespaces(i.namespaces), i.eolian_name), context))
           return false;
       }
     if(!as_generator("\n{\n").generate(sink, attributes::unused, context)) return false;

     // constructors
     if(!as_generator(
            scope_tab << "explicit " << string << "( ::Eo* eo)\n"
         << scope_tab << scope_tab << ": ::efl::eo::concrete(eo) {}\n"
         << scope_tab << string << "(std::nullptr_t)\n"
         << scope_tab << scope_tab << ": ::efl::eo::concrete(nullptr) {}\n"
         << scope_tab << "explicit " << string << "() = default;\n"
         << scope_tab << string << "(" << string << " const&) = default;\n"
         << scope_tab << string << "(" << string << "&&) = default;\n"
         << scope_tab << string << "& operator=(" << string << " const&) = default;\n"
         << scope_tab << string << "& operator=(" << string << "&&) = default;\n"
         << scope_tab << "template <typename Derived>\n"
         << scope_tab << string << "(Derived&& derived\n"
         << scope_tab << scope_tab << ", typename std::enable_if<\n"
         << scope_tab << scope_tab << scope_tab << "::efl::eo::is_eolian_object<Derived>::value\n"
         << scope_tab << scope_tab << scope_tab << " && std::is_base_of< " << string << ", Derived>::value>::type* = 0)\n"
         << scope_tab << scope_tab << scope_tab << ": ::efl::eo::concrete(derived._eo_ptr()) {}\n\n"
              ).generate(sink, attributes::make_infinite_tuple(cls.cxx_name), context))
       return false;

     if((cls.type == attributes::class_type::regular) && !as_generator(
            scope_tab << string << "( ::efl::eo::instantiate_t)\n"
         << scope_tab << "{\n"
         << scope_tab << scope_tab << "::efl::eolian::do_eo_add( ::efl::eo::concrete::_eo_raw, ::efl::eo::concrete{nullptr}, _eo_class());\n"
         << scope_tab << "}\n"
         << scope_tab << "template <typename T>\n"
         << scope_tab << "explicit " << string << "( ::efl::eo::instantiate_t, T&& parent, typename std::enable_if< ::efl::eo::is_eolian_object<T>::value>::type* = 0)\n"
         << scope_tab << "{\n"
         << scope_tab << scope_tab << "::efl::eolian::do_eo_add( ::efl::eo::concrete::_eo_raw, parent, _eo_class());\n"
         << scope_tab << "}\n"
         << scope_tab << "template <typename F> " << string << "( ::efl::eo::instantiate_t, F&& f, typename ::std::enable_if< ::efl::eolian::is_constructor_lambda<F, " << string << " >::value>::type* = 0)\n"
         << scope_tab << "{\n"
         << scope_tab << scope_tab << "::efl::eolian::do_eo_add( ::efl::eo::concrete::_eo_raw, ::efl::eo::concrete{nullptr}, _eo_class(), *this, std::forward<F>(f));\n"
         << scope_tab << "}\n"
         << scope_tab << "template <typename T, typename F> " << string << "(  ::efl::eo::instantiate_t, T&& parent, F&& f, typename ::std::enable_if< ::efl::eolian::is_constructor_lambda<F, " << string << " >::value && ::efl::eo::is_eolian_object<T>::value>::type* = 0)\n"
         << scope_tab << "{\n"
         << scope_tab << scope_tab << "::efl::eolian::do_eo_add( ::efl::eo::concrete::_eo_raw, parent, _eo_class(), *this, std::forward<F>(f));\n"
         << scope_tab << "}\n\n"
         // << scope_tab << "explicit " << string << "( ::efl::eo::concrete const& parent)\n"
         // << scope_tab << scope_tab << ": ::efl::eo::concrete( ::efl::eo::do_eo_add(parent)) {}\n"
         // << scope_tab << "template <typename F>\n"
         // << scope_tab << "explicit " << string << "( ::efl::eo::concrete const& parent, F f)\n"
         // << scope_tab << scope_tab << ": ::efl::eo::concrete( ::efl::eo::do_eo_add(parent, f)) {}\n"
         // << scope_tab << "template <typename F>\n"
         // << scope_tab << "explicit " << string << "(F f)\n"
         // << scope_tab << scope_tab << ": ::efl::eo::concrete( ::efl::eo::do_eo_add( ::efl::eo::concrete{nullptr}, f)) {}\n"
        ).generate(sink, attributes::make_infinite_tuple(cls.cxx_name), context)) return false;
     
#ifndef USE_EOCXX_INHERIT_ONLY
     if(!as_generator(*(function_declaration(get_klass_name(cls))))
        .generate(sink, cls.functions, context)) return false;
#endif
                                             
     // static Efl_Class const* _eo_class();
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
            scope_tab << "static Efl_Class const* _eo_class()\n"
            << scope_tab << "{\n"
            << scope_tab << scope_tab << "return "
        ).generate(sink,  attributes::unused, context)) return false;
     if(!as_generator
        (*(string << "_") << string << "_" << string)
        .generate(sink, std::make_tuple(cls.namespaces, cls.eolian_name, suffix), add_upper_case_context(context)))
       return false;
     if(!as_generator(";\n" << scope_tab << "}\n").generate(sink, attributes::unused, context)) return false;

     if(!as_generator(
         scope_tab << "Eo* _eo_ptr() const { return *(reinterpret_cast<Eo **>"
              << "(const_cast<" << string << " *>(this))); }\n"
        ).generate(sink, cls.cxx_name, context))
       return false;

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
                << scope_tab << scope_tab << "static Efl_Event_Description const* description()\n"
                << scope_tab << scope_tab << "{ return " << string << "; }\n"
                << scope_tab << scope_tab << "typedef "
                << (attribute_conditional([] (eina::optional<attributes::type_def> const& t) { return !!t; })
                    [attribute_replace([] (eina::optional<attributes::type_def> const& t) { return *t; }) [type]]
                    | "void")
                << " parameter_type;\n"
                << scope_tab << "} const " << string_replace(',', '_') << "_event;\n"
             ))).generate(sink, std::vector<attributes::event_def>{e}, context))
            return false;

          /* EXPERIMENTAL: event_cb_add */
          if (!as_generator("#ifdef EFL_CXXPERIMENTAL\n").generate(sink, attributes::unused, context))
            return false;
          if (!as_generator(
                 scope_tab << "template <typename F>\n"
                 << scope_tab << "typename std::enable_if<std::is_bind_expression<F>::value, ::efl::eolian::signal_connection>::type\n"
                 << scope_tab << string_replace(',', '_') << "_event_cb_add(F function)\n"
                 << scope_tab << "{\n"
                 << scope_tab << scope_tab << "return ::efl::eolian::event_add(" << string_replace(',', '_') << "_event, *this, function);\n"
                 << scope_tab << "}\n")
              .generate(sink, std::make_tuple(e.name, e.name), context))
            return false;
          if (!as_generator(
                 scope_tab << "template <typename F>\n"
                 << scope_tab << "typename std::enable_if<!std::is_bind_expression<F>::value, ::efl::eolian::signal_connection>::type\n"
                 << scope_tab << string_replace(',', '_') << "_event_cb_add(F function)\n"
                 << scope_tab << "{\n"
                 << scope_tab << scope_tab << "return ::efl::eolian::event_add(" << string_replace(',', '_') << "_event, *this, std::bind(function));\n"
                 << scope_tab << "}\n")
              .generate(sink, std::make_tuple(e.name, e.name), context))
            return false;
          if (!as_generator("#endif\n").generate(sink, attributes::unused, context))
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

     // EXPERIMENTAL: Parts
     if (!cls.parts.empty())
       {
          if(!as_generator("#ifdef EFL_CXXPERIMENTAL\n").generate(sink, attributes::unused, context)) return false;
          if(!as_generator(*(scope_tab << part_declaration << ";\n"))
                .generate(sink, cls.parts, context)) return false;
          if(!as_generator("#endif \n").generate(sink, attributes::unused, context)) return false;
       }

     if(!as_generator(   scope_tab << "::efl::eo::wref<" << string << "> _get_wref() const { "
                         "return ::efl::eo::wref<" << string << ">(*this); }\n"
                         ).generate(sink, std::make_tuple(cls.cxx_name, cls.cxx_name), context)) return false;

     // eo_concrete
     if(!as_generator(   scope_tab << "::efl::eo::concrete const& _get_concrete() const { return *this; }\n"
                      << scope_tab << "::efl::eo::concrete& _get_concrete() { return *this; }\n"
                     ).generate(sink, attributes::unused, context)) return false;
     
     if(!as_generator(   scope_tab << "using ::efl::eo::concrete::_eo_ptr;\n"
                      << scope_tab << "using ::efl::eo::concrete::_release;\n"
                      << scope_tab << "using ::efl::eo::concrete::_reset;\n"
                      << scope_tab << "using ::efl::eo::concrete::_delete;\n"
                      << scope_tab << "using ::efl::eo::concrete::operator bool;\n"
                      ).generate(sink, attributes::unused, context)) return false;

     // EXPERIMENTAL: wref and implicit conversion to Eo*
     if(!as_generator("#ifdef EFL_CXXPERIMENTAL\n").generate(sink, attributes::unused, context)) return false;
     // For easy wref, operator-> in wref needs to also return a pointer type
     if(!as_generator(   scope_tab << "const " << string << "* operator->() const { return this; }\n"
                     ).generate(sink, std::make_tuple(cls.cxx_name, cls.cxx_name), context)) return false;
     if(!as_generator(   scope_tab << string << "* operator->() { return this; }\n"
                     ).generate(sink, std::make_tuple(cls.cxx_name, cls.cxx_name), context)) return false;
     // For easy interfacing with C: no need to use _eo_ptr()
     if(!as_generator(   scope_tab << "operator Eo*() const { return _eo_ptr(); }\n"
                     ).generate(sink, attributes::unused, context)) return false;
     if(!as_generator("#endif \n").generate(sink, attributes::unused, context)) return false;

     if(!as_generator(   scope_tab << "friend bool operator==(" << string << " const& lhs, " << string << " const& rhs)\n"
                      << scope_tab << "{ return lhs._get_concrete() == rhs._get_concrete(); }\n"
                      << scope_tab << "friend bool operator!=(" << string << " const& lhs, " << string << " const& rhs)\n"
                      << scope_tab << "{ return !(lhs == rhs); }\n"
                     << "};\n").generate(sink, attributes::make_infinite_tuple(cls.cxx_name), context)) return false;
     
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
template <>
struct is_generator<class_definition_generator> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed<class_definition_generator> : std::integral_constant<int, 1> {};
}
      
class_definition_generator const class_definition = {};
      
} } }

#endif
