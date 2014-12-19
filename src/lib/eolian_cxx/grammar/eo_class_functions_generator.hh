
#ifndef EOLIAN_CXX_STD_EO_CLASS_FUNCTIONS_GENERATOR_HH
#define EOLIAN_CXX_STD_EO_CLASS_FUNCTIONS_GENERATOR_HH

#include <iosfwd>

#include "eo_types.hh"
#include "tab.hh"
#include "comment.hh"
#include "parameters_generator.hh"
#include "type_generator.hh"
#include "namespace_generator.hh"

namespace efl { namespace eolian { namespace grammar {

struct function_call
{
   eo_function const& _func;
   function_call(eo_function const& func) : _func(func) {}
};

inline std::ostream&
operator<<(std::ostream& out, function_call const& x)
{
   bool is_void = function_is_void(x._func);
   return out << (!is_void ? "_tmp_ret = " : "")
              << "::" << x._func.impl
              << "(" << parameters_forward_to_c(x._func.params) << ")";
}

struct function_declaration
{
   eo_class const& _cls;
   eo_function const& _func;
   function_declaration(eo_class const& cls, eo_function const& func)
     : _cls(cls), _func(func)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, function_declaration const& x)
{
   eo_function const& func = x._func;

   out << comment(x._func.comment, 1)
       << template_parameters_declaration(func.params, 1)
       << tab(1);

   bool is_static = function_is_static(func);
   if (is_static)
     out << "static ";

   out << reinterpret_type(func.ret) << " " << func.name << "("
       << parameters_declaration(func.params)
       << (is_static ? ");" : ") const;") << endl << endl;

   return out;
}

struct function_definition
{
   eo_class const& _cls;
   eo_function const& _func;
   bool _concrete;
   function_definition(eo_class const& cls, eo_function const& func, bool concrete)
     : _cls(cls), _func(func), _concrete(concrete)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, function_definition const& x)
{
   eo_function const& func = x._func;

   bool is_static = function_is_static(func);

   out << template_parameters_declaration(func.params, 0)
       << "inline " << reinterpret_type(func.ret) << " ";

   if (x._concrete)
     out << full_name(x._cls, false);
   else
     out << abstract_full_name(x._cls, false);

   out << "::" << func.name << "("
       << parameters_declaration(func.params)
       << (is_static ? ")" : ") const") << endl
       << "{" << endl;

   if (!function_is_void(func))
     out << tab(1)
         << func.ret.front().native << " _tmp_ret;" << endl;

   if (!is_static)
      out << callbacks_heap_alloc("_concrete_eo_ptr()", func.params, 1);

   // TODO : register free callback for static methods

   out << tab(1) << "eo_do("
       << (is_static ? "_eo_class(), " : "_concrete_eo_ptr(), ")
       << function_call(x._func) << ");" << endl;

   if (!function_is_void(func))
     out << tab(1) << "return " << to_cxx(func.ret, "_tmp_ret") << ";" << endl;

   out << "}" << endl << endl;
   return out;
}

struct function_declarations
{
   eo_class const& _cls;
   function_declarations(eo_class const& cls)
     : _cls(cls)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, function_declarations const& x)
{
   for (eo_function const& f : x._cls.functions)
     {
        out << function_declaration(x._cls, f) << endl;
     }
   return out;
}

struct function_definitions
{
   eo_class const& _cls;
   bool _concrete;
   function_definitions(eo_class const& cls, bool concrete)
     : _cls(cls)
     , _concrete(concrete)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, function_definitions const& x)
{
   for (eo_function const& f : x._cls.functions)
     {
        out << function_definition(x._cls, f, x._concrete) << endl;
     }
   return out;
}

} } } // namespace efl { namespace eolian { namespace grammar {

#endif // EOLIAN_CXX_STD_EO_CLASS_FUNCTIONS_GENERATOR_HH
