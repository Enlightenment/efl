
#ifndef EOLIAN_CXX_EO_CLASS_GENERATOR_HH
#define EOLIAN_CXX_EO_CLASS_GENERATOR_HH

#include <iosfwd>
#include <string>

#include "eo_types.hh"
#include "tab.hh"
#include "comment.hh"
#include "namespace_generator.hh"
#include "eo_class_constructors_generator.hh"
#include "eo_class_functions_generator.hh"
#include "eo_class_events_generator.hh"

namespace efl { namespace eolian { namespace grammar {

struct eo_class_getter
{
   eo_class const& _cls;
   eo_class_getter(eo_class const& cls) : _cls(cls) {}
};

inline std::ostream&
operator<<(std::ostream& out, eo_class_getter const& x)
{
   out << tab(1) << "static Eo_Class const* _eo_class()" << endl
       << tab(1) << "{" << endl
       << tab(2) << "return("<< x._cls.eo_name << ");" << endl
       << tab(1) << "}" << endl << endl;
   return out;
}

struct concrete_eo_ptr_getter
{
   eo_class const& _cls;
   concrete_eo_ptr_getter(eo_class const& cls) : _cls(cls) {}
};

inline std::ostream&
operator<<(std::ostream& out, concrete_eo_ptr_getter const&)
{
   out << tab(1) << "Eo* _concrete_eo_ptr() const" << endl
       << tab(1) << "{" << endl
       << tab(2) << "return static_cast<::efl::eo::concrete const*>(static_cast<void const*>(this))->_eo_ptr();" << endl
       << tab(1) << "}" << endl << endl;
   return out;
}

struct class_implicit_conversion_declaration
{
   eo_class const& _cls;
   class_implicit_conversion_declaration(eo_class const& cls) : _cls(cls) {}
};

inline std::ostream&
operator<<(std::ostream& out, class_implicit_conversion_declaration const& x)
{
   out << tab(1) << "operator " << full_name(x._cls) << "() const;" << endl;
   out << tab(1) << "operator " << full_name(x._cls) << "&();" << endl;
   out << tab(1) << "operator " << full_name(x._cls) << " const&() const;" << endl;
   return out << endl;
}

struct class_implicit_conversion_definition
{
   eo_class const& _cls;
   class_implicit_conversion_definition(eo_class const& cls) : _cls(cls) {}
};

inline std::ostream&
operator<<(std::ostream& out, class_implicit_conversion_definition const& x)
{
   out << "inline " << abstract_full_name(x._cls) << "::operator "
       << full_name(x._cls) << "() const" << endl
       << "{" << endl
       << tab(1) << "return *static_cast<" << full_name(x._cls)
       << " const*>(static_cast<void const*>(this));" << endl
       << "}" << endl << endl;

   out << "inline " << abstract_full_name(x._cls) << "::operator "
       << full_name(x._cls) << "&()" << endl
       << "{" << endl
       << tab(1) << "return *static_cast<" << full_name(x._cls)
       << "*>(static_cast<void*>(this));" << endl
       << "}" << endl << endl;

   out << "inline " << abstract_full_name(x._cls) << "::operator "
       << full_name(x._cls) << " const&() const" << endl
       << "{" << endl
       << tab(1) << "return *static_cast<" << full_name(x._cls)
       << " const*>(static_cast<void const*>(this));" << endl
       << "}" << endl << endl;

   return out;
}

inline void
eo_class_generator(std::ostream& out, eo_class const& cls)
{
   out << namespace_head(cls)
       << "struct " << cls.name << ";" << endl << endl
       << namespace_tail(cls)
       << "namespace " << abstract_namespace << " {" << endl << endl
       << namespace_head(cls)
       << comment(cls.comment)
       << "struct " << cls.name << endl
       << class_inheritance(cls)
       << '{' << endl
       << functors_constructor_methods(cls)
       << constructor_method_function_declarations(cls)
       << function_declarations(cls)
       << events(cls)
       << eo_class_getter(cls)
       << class_implicit_conversion_declaration(cls)
       << "private:" << endl
       << concrete_eo_ptr_getter(cls)
       << "};" << endl << endl
       << namespace_tail(cls)
       << "}" << endl << endl
       << namespace_head(cls)
       << "struct " << cls.name << endl
       << tab(2) << ": " << abstract_full_name(cls) << endl
       << tab(2) << ", ::efl::eo::concrete" << endl
       << '{' << endl
       << constructor_with_constructor_methods(cls)
       << constructor_eo(cls)
       << copy_constructor(cls)
       << destructor(cls)
        << "private:" << endl
       << function_call_constructor_methods(cls)
       << "};" << endl << endl
       << "static_assert(sizeof(" << full_name(cls) << ") == sizeof(Eo*), \"\");" << endl
       << "static_assert(std::is_standard_layout<" << full_name(cls) << ">::value, \"\");" << endl
       << endl
       << namespace_tail(cls)
       << constructor_method_function_definitions(cls)
       << function_definitions(cls)
       << class_implicit_conversion_definition(cls);
}

} } } // namespace efl { namespace eolian { namespace grammar {

#endif // EOLIAN_CXX_EO_CLASS_GENERATOR_HH
