
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

struct address_of_to_pointer
{
   eo_class const& _cls;
   bool _is_const;
   address_of_to_pointer(eo_class const& cls, bool is_const)
     : _cls(cls), _is_const(is_const)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, address_of_to_pointer const& x)
{
   out << "operator " << full_name(x._cls) << (x._is_const ? " const" : "")
       << "*() const { return static_cast<" << full_name(x._cls)
       << (x._is_const ? " const" : "")
       << "*>(static_cast<D const*>(this)->p); }";
   return out;
}

struct abstract_address_of
{
   eo_class const& _cls;
   abstract_address_of(eo_class const& cls) : _cls(cls) {}
};

inline std::ostream&
operator<<(std::ostream& out, abstract_address_of const& x)
{
   out << tab(1) << "template <typename D>" << endl
       << tab(1) << "struct address_of" << endl
       << tab(1) << "{" << endl
       << tab(2) << address_of_to_pointer(x._cls, false) << endl
       << tab(2) << address_of_to_pointer(x._cls, true) << endl
       << tab(1) << "};" << endl << endl;

   out << tab(1) << "template <typename D>" << endl
       << tab(1) << "struct address_const_of" << endl
       << tab(1) << "{" << endl
       << tab(2) << address_of_to_pointer(x._cls, true) << endl
       << tab(1) << "};" << endl << endl;

   return out;
}

struct address_of_inheritance
{
   eo_class const& _cls;
   std::string _struct_name;
   address_of_inheritance(eo_class const& cls, std::string const& struct_name)
     : _cls(cls), _struct_name(struct_name)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, address_of_inheritance const& x)
{
   for (std::string const& parent : x._cls.ancestors)
     {
        out << tab(2) << ", ::" << abstract_namespace << "::" << parent << "::"
            << x._struct_name << "<" << x._struct_name << ">" << endl;
     }
   return out;
}

struct concrete_address_of
{
   eo_class const& _cls;
   concrete_address_of(eo_class const& cls) : _cls(cls) {}
};

inline std::ostream&
operator<<(std::ostream& out, concrete_address_of const& x)
{
   std::vector<std::string> names {"address_of", "address_const_of"};
   for (int is_const = 0; is_const != 2; ++is_const)
     {
        std::string const& name = names[is_const];

        out << tab(1) << "struct " << name << endl
            << tab(2) << ": " << abstract_full_name(x._cls) << "::"
            << name << "<" << name << ">" << endl
            << address_of_inheritance(x._cls, name)
            << tab(2) << ", ::efl::eo::detail::concrete_" << name << endl
            << tab(1) << "{" << endl
            << tab(2) << "explicit " << name << "(" << full_name(x._cls)
            << (is_const ? " const" : "")
            << "* p)" << endl
            << tab(3) << ": ::efl::eo::detail::concrete_" << name << "(p)" << endl
            << tab(2) << "{}" << endl
            << tab(1) << "};" << endl
            << tab(1) << name << " operator&()"
            << (is_const ? " const" : "")
            << " { return " << name << "(this); }" << endl << endl;
     }

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
       << '{' << endl
       << function_declarations(cls)
       << events(cls)
       << eo_class_getter(cls)
       << class_implicit_conversion_declaration(cls)
       << abstract_address_of(cls)
       << "private:" << endl
       << concrete_eo_ptr_getter(cls)
       << "};" << endl << endl
       << namespace_tail(cls)
       << "}" << endl << endl
       << namespace_head(cls)
       << "struct " << cls.name << endl
       << tab(2) << ": ::efl::eo::concrete" << endl
       << class_inheritance(cls)
       << '{' << endl
       << functors_constructor_methods(cls)
       << constructor_with_constructor_methods(cls)
       << constructor_eo(cls)
       << copy_constructor(cls)
       << destructor(cls)
       << constructor_method_function_declarations(cls)
       << function_declarations(cls)
       << events(cls)
       << eo_class_getter(cls)
       << concrete_address_of(cls)
        << "private:" << endl
       << function_call_constructor_methods(cls)
       << tab(2) << "Eo* _concrete_eo_ptr() const { return _eo_ptr(); }" << endl
       << "};" << endl << endl
       << "static_assert(sizeof(" << full_name(cls) << ") == sizeof(Eo*), \"\");" << endl
       << "static_assert(std::is_standard_layout<" << full_name(cls) << ">::value, \"\");" << endl
       << endl
       << namespace_tail(cls)
       << constructor_method_function_definitions(cls)
       << function_definitions(cls, true)
       << function_definitions(cls, false)
       << class_implicit_conversion_definition(cls);
}

} } } // namespace efl { namespace eolian { namespace grammar {

#endif // EOLIAN_CXX_EO_CLASS_GENERATOR_HH
