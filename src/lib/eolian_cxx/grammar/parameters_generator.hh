
#ifndef EOLIAN_CXX_STD_PARAMETERS_GENERATOR_HH
#define EOLIAN_CXX_STD_PARAMETERS_GENERATOR_HH

#include <iosfwd>

#include "tab.hh"
#include "eo_types.hh"
#include "type_generator.hh"

namespace efl { namespace eolian { namespace grammar {

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


} } } // namespace efl { namespace eolian { namespace grammar {


#endif // EOLIAN_CXX_STD_PARAMETERS_GENERATOR_HH
