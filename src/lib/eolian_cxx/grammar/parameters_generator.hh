
#ifndef EOLIAN_CXX_STD_PARAMETERS_GENERATOR_HH
#define EOLIAN_CXX_STD_PARAMETERS_GENERATOR_HH

#include <iosfwd>

#include "tab.hh"
#include "eo_types.hh"
#include "type_generator.hh"

namespace efl { namespace eolian { namespace grammar {

struct
parameter_forward
{
   eolian_type_instance const& _type;
   std::string const& _name;
   parameter_forward(eolian_type_instance const& type, std::string const& name)
     : _type(type)
     , _name(name)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, parameter_forward const& x)
{
   if (type_is_callback(x._type))
     {
        out << "std::forward<" << template_parameter_type(x._type, x._name) << ">(" << x._name << ")";
     }
   else
     out << x._name;
   return out;
}

struct param_data
{
   std::ostream& out;
   unsigned pos;
   eolian_type_instance const& type;
   std::string const& name;
   int cb_idx;
   bool is_cb;
   param_data(std::ostream& out_, unsigned pos_, eolian_type_instance const& type_, std::string const& name_, int cb_idx_)
      : out(out_)
      , pos(pos_)
      , type(type_)
      , name(name_)
      , cb_idx(cb_idx_)
      , is_cb(cb_idx_ >= 0)
   {}
};

template <typename T>
struct
_parameters_cxx_generic
{
   parameters_container_type const& _params;
   T _fparam;
   _parameters_cxx_generic(parameters_container_type const& params, T fparam)
     : _params(params)
     , _fparam(fparam)
   {}
};

template <typename T>
std::ostream&
operator<<(std::ostream& out, _parameters_cxx_generic<T> const& x)
{
   int cb_idx = 0u;
   auto first = x._params.cbegin(),
     last = x._params.cend();
   for (auto it = first; it != last; ++it)
     {
        if (type_is_callback((*it).type) && it+1 != last)
          {
             x._fparam(param_data(out, it - first, (*it).type, (*it).name, cb_idx++));
             ++it; // skip next.
          }
        else
          x._fparam(param_data(out, it - first, (*it).type, (*it).name, -1));
     }
   return out;
}

template <typename T>
_parameters_cxx_generic<T>
parameters_cxx_generic(parameters_container_type const& params, T fparam)
{
   return _parameters_cxx_generic<T>(params, fparam);
}

struct
callback_tmp
{
   std::string const& _name;
   callback_tmp(std::string const& name)
     : _name(name)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, callback_tmp const& x)
{
   return out << "_tmp_" << x._name;
}

struct
callback_parameter_heap_alloc
{
   eolian_type_instance const& _type;
   std::string const& _name;
   int _tab;
   callback_parameter_heap_alloc(eolian_type_instance const& type, std::string const& name, int tab)
     : _type(type)
     , _name(name)
     , _tab(tab)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, callback_parameter_heap_alloc const& x)
{
   out << tab(x._tab) << parameter_remove_reference_typedef(x._type, x._name) << endl
       << tab(x._tab) << parameter_no_ref_type(x._type, x._name) << "* "
       << callback_tmp(x._name) << " = new "
       << parameter_no_ref_type(x._type, x._name) << "(std::forward<"
       << template_parameter_type(x._type, x._name) << ">(" << x._name << "));" << endl;
   return out;
}

struct
callback_parameter_free_ev_add
{
   std::string const& _eo_raw_expr;
   eolian_type_instance const& _type;
   std::string const& _name;
   callback_parameter_free_ev_add(std::string const& eo_raw_expr, eolian_type_instance const& type, std::string const& name)
     : _eo_raw_expr(eo_raw_expr)
     , _type(type)
     , _name(name)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, callback_parameter_free_ev_add const& x)
{
   out << "eo_do(" << x._eo_raw_expr
       << ", eo_event_callback_add(EO_EV_DEL, &::efl::eolian::free_callback_calback<"
       << parameter_no_ref_type(x._type, x._name) << ">, "
       << callback_tmp(x._name) << "));";
   return out;
}

struct
callbacks_heap_alloc
{
   std::string const& _eo_raw_expr;
   parameters_container_type const& _params;
   int _tab;
   callbacks_heap_alloc(std::string const& eo_raw_expr, parameters_container_type const& params, int tab)
     : _eo_raw_expr(eo_raw_expr)
     , _params(params)
     , _tab(tab)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, callbacks_heap_alloc const& x)
{
   auto first = x._params.cbegin(), last = x._params.cend();
   for (auto it = first; it != last; ++it)
     {
        if (type_is_callback((*it).type) && it+1 != last)
          {
             out << callback_parameter_heap_alloc((*it).type, (*it).name, x._tab)
                 << tab(x._tab)
                 << callback_parameter_free_ev_add(x._eo_raw_expr, (*it).type, (*it).name)
                 << endl << endl;
             ++it; // skip next.
          }
     }
   return out;
}

struct
template_parameters_declaration
{
   parameters_container_type const& _params;
   int _tab;
   template_parameters_declaration(parameters_container_type const& params, int tab)
     : _params(params)
     , _tab(tab)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, template_parameters_declaration const& x)
{
   if (parameters_count_callbacks(x._params) == 0)
     return out;

   bool comma = false;
   out << tab(x._tab) << "template <";
   auto first = x._params.cbegin(), last = x._params.cend();
   for (auto it = first; it != last; ++it)
     {
        if (type_is_callback((*it).type) && it+1 != last)
          {
             if (comma)
               out << ", ";
             else
               comma = true;
             out << "typename " << template_parameter_type((*it).type, (*it).name);
             ++it; // skip next.
          }
     }
   return out << ">" << endl;
}

struct
parameters_declaration
{
   parameters_container_type const& _params;
   parameters_declaration(parameters_container_type const& params)
     : _params(params)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, parameters_declaration const& x)
{
   auto first = x._params.cbegin(),
     last = x._params.cend();
   for (auto it = first; it != last; ++it)
     {
        if (it != first)
          out << ", ";
        // TODO What to do when callback happens in the middle of parameters
        //      and does not have a following userdata pointer ?
        if (type_is_callback((*it).type) && it+1 != last)
          {
            out << template_parameter_type((*it).type, (*it).name) << " && " << (*it).name;
            ++it; // skip next.
          }
        else
          out << reinterpret_type((*it).type) << " " << (*it).name;
     }
   return out;
}

struct
parameters_c_declaration
{
   parameters_container_type const& _params;
   parameters_c_declaration(parameters_container_type const& params)
     : _params(params)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, parameters_c_declaration const& x)
{
   auto first = x._params.cbegin(),
     last = x._params.cend();
   for (auto it = first; it != last; ++it)
     {
        if (it != first)
          out << ", ";
        out << c_type(it->type) << " " << (*it).name;
     }
   return out;
}

struct
parameters_types
{
   parameters_container_type const& _params;
   parameters_types(parameters_container_type const& params)
     : _params(params)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, parameters_types const& x)
{
   parameters_container_type::const_iterator it,
     first = x._params.begin(),
     last = x._params.end();
   for (it = first; it != last; ++it)
     {
        if(it != first) out << ", ";
        out << reinterpret_type((*it).type);
     }
   return out;
}

struct
parameters_c_list
{
   parameters_container_type const& _params;
   parameters_c_list(parameters_container_type const& params)
     : _params(params)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, parameters_c_list const& x)
{
   auto first = x._params.cbegin(), last = x._params.cend();
   for (auto it = first; it != last; ++it)
     {
        if (it != first)
          out << ", ";
        out << it->name;
     }
   return out;
}

struct
parameters_cxx_list
{
   parameters_container_type const& _params;
   parameters_cxx_list(parameters_container_type const& params)
     : _params(params)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, parameters_cxx_list const& x)
{
   auto first = x._params.cbegin(), last = x._params.cend();
   for (auto it = first; it != last; ++it)
     {
        if (it != first)
          out << ", ";
        out << to_cxx(it->type, it->name);
     }
   return out;
}

struct
parameters_forward
{
   parameters_container_type const& _params;
   parameters_forward(parameters_container_type const& params)
     : _params(params)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, parameters_forward const& x)
{
   auto first = x._params.cbegin(), last = x._params.cend();
   for (auto it = first; it != last; ++it)
     {
        if (it != first)
          out << ", ";
        out << parameter_forward((*it).type, (*it).name);
        if (type_is_callback((*it).type) && it+1 != last)
          {
             ++it; // skip next.
          }
     }
   return out;
}

struct
parameters_forward_to_c
{
   parameters_container_type const& _params;
   parameters_forward_to_c(parameters_container_type const& params)
     : _params(params)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, parameters_forward_to_c const& x)
{
   auto first = x._params.cbegin(), last = x._params.cend();
   for (auto it = first; it != last; ++it)
     {
        if (it != first)
          out << ", ";
        if (type_is_callback((*it).type))
          {
             // TODO: Is it correct to simple not translate the callback type
             //       when it is the last paramenter?
             if(it + 1 != last)
               {
                  out << to_c((*it).type, (*it).name) << ", " << callback_tmp((*it).name);
                  ++it; // skip next
               }
             else
               out << (*it).name;
          }
        else
          out << to_c((*it).type, (*it).name);
     }
   return out;
}

} } } // namespace efl { namespace eolian { namespace grammar {


#endif // EOLIAN_CXX_STD_PARAMETERS_GENERATOR_HH
