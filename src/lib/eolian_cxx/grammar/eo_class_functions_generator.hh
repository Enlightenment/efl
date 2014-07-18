
#ifndef EOLIAN_CXX_STD_EO_CLASS_FUNCTIONS_GENERATOR_HH
#define EOLIAN_CXX_STD_EO_CLASS_FUNCTIONS_GENERATOR_HH

#include <iosfwd>

#include "eo_types.hh"
#include "tab.hh"
#include "comment.hh"
#include "parameters_generator.hh"
#include "type_generator.hh"

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
              << "(" << parameters_list(x._func.params) << ")";
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

   out << comment(x._func.comment, 1);
   if (parameters_count_callbacks(x._func.params) == 1)
     out << tab(1) << "template <typename F>" << endl;

   if (function_is_static(func))
     out << tab(1) << "static ";

   out << tab(1)
       << reinterpret_type(func.ret) << " " << func.name << "("
       << parameters_declaration(func.params)
       << ") const" << endl
       << tab(1) << "{" << endl;

   if (!function_is_void(func))
     out << tab(2)
         << func.ret.front().native << " _tmp_ret;" << endl;

   parameters_container_type::const_iterator callback_iter =
     parameters_find_callback(func.params);
   if (callback_iter != func.params.cend())
     {
       out << tab(2)
           << "typedef typename std::remove_reference<F>::type function_type;" << endl
           << tab(2) << "function_type* _tmp_f = new function_type(std::forward<F>("
           << (*callback_iter).name << "));"
           << endl;
     }

   out << tab(2)
       << "eo_do(_eo_ptr(), " << function_call(x._func) << ");" << endl;

   if (!function_is_void(func))
     out << tab(2) << "return " << to_cxx(func.ret, "_tmp_ret") << ";" << endl;

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
