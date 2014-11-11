
#ifndef EOLIAN_CXX_STD_PARAMETERS_GENERATOR_HH
#define EOLIAN_CXX_STD_PARAMETERS_GENERATOR_HH

#include <iosfwd>

#include "tab.hh"
#include "eo_types.hh"
#include "type_generator.hh"

namespace efl { namespace eolian { namespace grammar {

struct
parameter_type
{
   eolian_type_instance const& _type;
   parameter_type(eolian_type_instance const& t)
     : _type(t)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, parameter_type const& x)
{
   if(type_is_callback(x._type))
      out << "F";
   else
      out << reinterpret_type(x._type);
   return out;
}

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
        out << "std::forward<F>(" << x._name << ")";
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
            out << "F && " << (*it).name;
            assert(it+1 != last);
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
parameters_list
{
   parameters_container_type const& _params;
   parameters_list(parameters_container_type const& params)
     : _params(params)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, parameters_list const& x)
{
   auto first = x._params.cbegin(), last = x._params.cend();
   for (auto it = first; it != last; ++it)
     {
        if (it != first)
          out << ", ";
        if (type_is_callback((*it).type))
          {
            if(it + 1 != last)
              {
                out << to_c((*it).type, (*it).name)
                    << ", _tmp_f";
                ++it; // skip next
              }
            else
              out << it->name;
          }
        else
          out << to_c((*it).type, (*it).name);
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
constructor_parameters_list
{
   parameters_container_type const& _params;
   constructor_parameters_list(parameters_container_type const& params)
     : _params(params)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, constructor_parameters_list const& x)
{
   auto first = x._params.cbegin(),
     last = x._params.cend();
   for (auto it = first; it != last; ++it)
     {
        if (it != first)
          out << ", ";
        if (type_is_callback((*it).type) && it + 1 != last)
          {
             out << "std::forward<F>(" << (*it).name << ")";
             ++it; // skip next.
          }
        else
          out << (*it).name;
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
        if (type_is_callback((*it).type) && it + 1 != last)
          {
             out << to_c((*it).type, (*it).name) << ", _tmp_f";
             ++it; // skip next
          }
        else
          out << to_c((*it).type, (*it).name);
     }
   return out;
}

} } } // namespace efl { namespace eolian { namespace grammar {


#endif // EOLIAN_CXX_STD_PARAMETERS_GENERATOR_HH
