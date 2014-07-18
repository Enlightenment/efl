
#ifndef EOLIAN_CXX_STD_INHERITANCE_GENERATOR_HH
#define EOLIAN_CXX_STD_INHERITANCE_GENERATOR_HH

#include <iosfwd>
#include <cassert>
#include <algorithm>

#include "eo_types.hh"
#include "tab.hh"
#include "parameters_generator.hh"
#include "eo_class_functions_generator.hh"

namespace efl { namespace eolian { namespace grammar {

inline std::string
_ns_as_prefix(eo_class const& cls)
{
   // XXX Use eolian_cxx::find_replace() instead.
   std::string s = cls.name_space;
   std::string::size_type found = s.find("::");
   while (found != std::string::npos)
     {
        s.replace(found, 1, "_");
        found = s.find("::");
     }
   return s;
}

struct inheritance_operation
{
   eo_class const& _cls;
   functions_container_type::size_type _idx;
   inheritance_operation(eo_class const& cls, functions_container_type::size_type idx)
     : _cls(cls), _idx(idx)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, inheritance_operation const& x)
{
   assert(x._idx < x._cls.functions.size());
   eo_function const& func = x._cls.functions[x._idx];
   out << tab(1)
       << "ops[" << x._idx << "].func = reinterpret_cast<void*>(& ::"
       << _ns_as_prefix(x._cls) << "_"
       << x._cls.name << "_" << func.name << "_wrapper<T>);" << endl
       << tab(1) << "ops[" << x._idx << "].api_func = reinterpret_cast<void*>(& ::"
       << func.impl << ");" << endl
       << tab(1) << "ops[" << x._idx << "].op = EO_OP_OVERRIDE;" << endl
       << tab(1) << "ops[" << x._idx << "].op_type = EO_OP_TYPE_REGULAR;" << endl // XXX class ops
       << tab(1) << "ops[" << x._idx << "].doc = NULL;" << endl
       << endl;
   return out;
}

struct inheritance_operations_description
{
   eo_class const& _cls;
   inheritance_operations_description(eo_class const& cls)
     : _cls(cls)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, inheritance_operations_description const& x)
{
   out << "template <typename T>"
       << endl << "int initialize_operation_description(efl::eo::detail::tag<"
       << x._cls.name_space << "::" << x._cls.name << ">" << endl
       << tab(11)
       << ", Eo_Op_Description* ops)" << endl
       << "{" << endl
       << tab(1) << "(void)ops;" << endl;
   functions_container_type::size_type n_ops = x._cls.functions.size();
   for (functions_container_type::size_type i=0; i < n_ops; ++i)
     {
        out << inheritance_operation(x._cls, i);
     }
   out << tab(1) << "return 0;" << endl
       << "}" << endl;

   return out;
}

struct inheritance_wrapper
{
   eo_class const& _cls;
   eo_function const& _func;
   inheritance_wrapper(eo_class const& cls, eo_function const& func)
     : _cls(cls), _func(func)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, inheritance_wrapper const& x)
{
   out << "template <typename T>" << endl
       << reinterpret_type(x._func.ret) << " "
       << _ns_as_prefix(x._cls) << "_"
       << x._cls.name << "_" << x._func.name
       << "_wrapper(Eo* objid EINA_UNUSED, "
       << "efl::eo::detail::Inherit_Private_Data* self"
       << (x._func.params.size() ? ", " : "")
       << parameters_c_declaration(x._func.params)
       << ")" << endl
       << "{" << endl
       << tab(1)
       << (!function_is_void(x._func) ? "return ": "")
       << "static_cast<T*>(self->this_)->"
       << x._func.name << "(" << parameters_c_list(x._func.params) << ");" << endl
       << "}" << endl << endl;

   return out;
}

struct inheritance_wrappers
{
   eo_class const& _cls;
   inheritance_wrappers(eo_class const& cls) : _cls(cls) {}
};

inline std::ostream&
operator<<(std::ostream& out, inheritance_wrappers const& x)
{
   functions_container_type::const_iterator it,
     first = x._cls.functions.begin(),
     last = x._cls.functions.end();
   for (it = first; it != last; ++it)
     {
        eo_function const& func = *it;
        out << "template <typename T>" << endl
            << reinterpret_type(func.ret) << " "
            << _ns_as_prefix(x._cls) << "_"
            << x._cls.name << "_" << func.name
            << "_wrapper(Eo* objid EINA_UNUSED, "
            << "efl::eo::detail::Inherit_Private_Data* self"
            << (func.params.size() ? ", " : "")
            << parameters_c_declaration(func.params)
            << ")" << endl
            << "{" << endl
            << tab(1)
            << (!function_is_void(func) ? "return ": "")
            << "static_cast<T*>(self->this_)->"
            << func.name << "(" << parameters_c_list(func.params) << ");" << endl
            << "}" << endl << endl;
     }
   return out;
}

struct inheritance_base_operations_size
{
   eo_class const& _cls;
   inheritance_base_operations_size(eo_class const& cls)
     : _cls(cls)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, inheritance_base_operations_size const& x)
{
   out << "template<>"
       << endl << "struct operation_description_class_size< "
       <<  x._cls.name_space << "::" << x._cls.name << " >" << endl
       << "{" << endl
       << tab(1) << "static const int value = "
       << x._cls.functions.size()
       << ";" << endl
       << "};" << endl
       << endl;
   return out;
}

struct inheritance_base_operations_function
{
   eo_class const& _cls;
   eo_function const& _func;
   inheritance_base_operations_function(eo_class const& cls, eo_function const& func)
     : _cls(cls) , _func(func)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, inheritance_base_operations_function const& x)
{
   eo_function const& func = x._func;
   bool is_void = function_is_void(func);

   out << tab(2) << "virtual " << reinterpret_type(func.ret) << " "
       << func.name << "("
       << parameters_c_declaration(func.params) << ")" << endl
       << tab(2) << "{" << endl;
   if (!is_void)
     {
        out << tab(3) << reinterpret_type(func.ret) << " _tmp_ret = {};"  << endl;
     }
   out << tab(3)
       << "eo_do_super(static_cast<T*>(this)->_eo_ptr()" << endl
       << tab(4) << ", static_cast<T*>(this)->_eo_class()," << endl
       << tab(4) << (!is_void ? "_tmp_ret = " : "")
       << "::" << x._func.impl
       << "(";
   parameter_names_enumerate(out, func.params)
       << "));" << endl;
   if (!is_void)
     {
        out << tab(3) << "return _tmp_ret;" << endl;
     }
   out << tab(2) << "}" << endl << endl;
   return out;
}

struct inheritance_base_operations
{
   eo_class const& _cls;
   inheritance_base_operations(eo_class const& cls) : _cls(cls) {}
};

inline std::ostream&
operator<<(std::ostream& out, inheritance_base_operations const& x)
{
   out << "template<>" << endl
       << "struct operations< "
       << x._cls.name_space << "::" << x._cls.name << " >" << endl
       << "{" << endl
       << tab(1) << "template <typename T>" << endl
       << tab(1) << "struct type" << endl
       << tab(1) << "{" << endl;
   functions_container_type::const_iterator it,
     first = x._cls.functions.begin(),
     last = x._cls.functions.end();
   for (it = first; it != last; ++it)
     {
        out << inheritance_base_operations_function(x._cls, *it);
     }
   out << tab(1) << "};" << endl
       << "};" << endl << endl;
   return out;
}

struct inheritance_call_constructor_arguments
{
   parameters_container_type const& _params;
   inheritance_call_constructor_arguments(parameters_container_type const& params)
     : _params(params)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, inheritance_call_constructor_arguments const& x)
{
   parameters_container_type::size_type i, n = x._params.size();
   for (i=0; i<n; i++)
     {
        if(i!=0) out << ", ";
        out << "::efl::eolian::to_c(args.get<" << i << ">())";
     }
   return out;
}

struct inheritance_call_constructors
{
   eo_class const& _cls;
   inheritance_call_constructors(eo_class const& cls) : _cls(cls) {}
};

inline std::ostream&
operator<<(std::ostream& out, inheritance_call_constructors const& x)
{
   constructors_container_type::const_iterator it,
     first = x._cls.constructors.begin(),
     last = x._cls.constructors.end();
   for (it = first; it != last; ++it)
     {
        eo_constructor const& ctor = *it;
        out << "inline void" << endl
            << "call_constructor(tag< "
            << x._cls.name_space << "::" << x._cls.name << " >" << endl
            << tab(5) << ", Eo* eo, Eo_Class const* cls EINA_UNUSED," << endl
            << tab(5) << "args_class<"
            << x._cls.name_space << "::" << x._cls.name
            << ", ::std::tuple<"
            << parameters_types(ctor.params)
            << "> > const& args)" << endl
            << "{" << endl
            << tab(1) << "(void)args;" << endl
            << tab(1)
            << "eo_do_super(eo, cls, ::" << ctor.name
            << "(" << inheritance_call_constructor_arguments(ctor.params)
            << "));" << endl
            << "}" << endl << endl;
     }
   return out;
}

struct inheritance_extension_function
{
   eo_function const& _func;
   inheritance_extension_function(eo_function const& func) : _func(func) {}
};

inline std::ostream&
operator<<(std::ostream& out, inheritance_extension_function const& x)
{
   if (parameters_count_callbacks(x._func.params) == 1)
     out << tab(1) << "template <typename F>" << endl;

   bool is_void = function_is_void(x._func);
   out << tab(2)
       << reinterpret_type(x._func.ret) << " "
       << x._func.name << "("
       << parameters_declaration(x._func.params)
       << ")" << endl
       << tab(2) << "{" << endl;

   if (!is_void)
     {
        out << tab(3) << reinterpret_type(x._func.ret) << " _tmp_ret = {};" << endl;
     }

   parameters_container_type::const_iterator callback_iter =
     parameters_find_callback(x._func.params);
   if (callback_iter != x._func.params.cend())
     {
       out << tab(2)
           << "typedef typename std::remove_reference<F>::type function_type;" << endl
           << "function_type* _tmp_f = new function_type(std::forward<F>("
           << (*callback_iter).name << "));"
           << endl;
     }

   out << tab(3) << "eo_do(static_cast<U*>(this)->_eo_ptr(), "
       << function_call(x._func) << ");" << endl;

   if (!is_void)
     {
        out << tab(3) << "return _tmp_ret;" << endl;
     }

   out << tab(2) << "}" << endl
       << endl;

   return out;
}

struct inheritance_extension
{
   eo_class const& _cls;
   inheritance_extension(eo_class const& cls) : _cls(cls) {}
};

inline std::ostream&
operator<<(std::ostream& out, inheritance_extension const& x)
{
   std::string cls = x._cls.name_space + "::" + x._cls.name;
   out << "template<>" << endl
       << "struct extension_inheritance< "
       << cls << ">" << endl
       << "{" << endl
       << tab(1) << "template <typename U>" << endl
       << tab(1) << "struct type" << endl
       << tab(1) << "{" << endl
       << tab(2) << "operator " << cls << "() const" << endl
       << tab(2) << "{" << endl
       << tab(3) << "return " << cls
       << "(eo_ref(static_cast<U const*>(this)->_eo_ptr()));" << endl
       << tab(2) << "}" << endl
       << endl;
   functions_container_type::const_iterator it,
     first = x._cls.functions.begin(),
     last = x._cls.functions.end();
   for (it = first; it != last; ++it)
     {
        out << inheritance_extension_function(*it);
     }
   out << events(x._cls, true);
   out << tab(1) << "};" << endl
       << "};" << endl
       << endl;
   return out;
}

struct inheritance_eo_class_getter
{
   eo_class const& _cls;
   inheritance_eo_class_getter(eo_class const& cls)
     : _cls(cls)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, inheritance_eo_class_getter const& x)
{
   out << "inline Eo_Class const* get_eo_class(tag<"
       << x._cls.name_space << "::" << x._cls.name << ">)" << endl
       << "{" << endl
       << tab(1) << "return (" << x._cls.eo_name << ");" << endl
       << "}" << endl << endl;
   return out;
}

inline void
eo_inheritance_detail_generator(std::ostream& out, eo_class const& cls)
{
   out << inheritance_wrappers(cls)
       << "namespace efl { namespace eo { namespace detail {" << endl << endl
       << inheritance_base_operations(cls) << endl
       << inheritance_base_operations_size(cls)
       << inheritance_operations_description(cls)
       << inheritance_call_constructors(cls)
       << inheritance_extension(cls)
       << inheritance_eo_class_getter(cls)
       <<  "} } }" << endl;
}

} } } // namespace efl { namespace eolian { namespace grammar {

#endif // EOLIAN_CXX_STD_INHERITANCE_GENERATOR_HH
