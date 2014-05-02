
#ifndef EOLIAN_CXX_STD_EO_CLASS_FUNCTIONS_GENERATOR_HH
#define EOLIAN_CXX_STD_EO_CLASS_FUNCTIONS_GENERATOR_HH

#include <iosfwd>

#include "eo_types.hh"
#include "tab.hh"
#include "comment.hh"
#include "parameters_generator.hh"

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
   out << (!is_void ? "_tmp_ret = " : "")
       << "::" << x._func.impl
       << "(" << parameters_list(x._func.params) << ")";
   return out;
}

struct function
{
   eo_function const& _func;
   function(eo_function const& func) : _func(func) {}
};

inline std::ostream&
operator<<(std::ostream& out, function const& x)
{
   eo_function const& func = x._func;
   bool is_void = function_is_void(func);
   out << comment(x._func.comment, 1)
       << tab(1)
       << ( func.type == eo_function::class_ ? "static " : "" )
       << func.ret << " " << func.name << "("
       << parameters_declaration(func.params)
       << ") const" << endl
       << tab(1) << "{" << endl;
   if (!is_void)
     {
        out << tab(2) << func.ret << " _tmp_ret;" << endl;
     }
   out << tab(2) << "eo_do(_eo_ptr(), "
       << function_call(x._func)
       << ");" << endl;
   if (!is_void)
     {
        out << tab(2) << "return _tmp_ret;" << endl;
     }
   out << tab(1) << "}" << endl;
   return out;
}

struct functions
{
   functions_container_type const& _funcs;
   functions(functions_container_type const& funcs) : _funcs(funcs) {}
};

inline std::ostream&
operator<<(std::ostream& out, functions const& x)
{
   functions_container_type::const_iterator it,
     first = x._funcs.begin(),
     last = x._funcs.end();
   for (it = first; it != last; it++)
     {
        out << function(*it) << endl;
     }
   return out;
}

} } } // namespace efl { namespace eolian { namespace grammar {

#endif // EOLIAN_CXX_STD_EO_CLASS_FUNCTIONS_GENERATOR_HH
