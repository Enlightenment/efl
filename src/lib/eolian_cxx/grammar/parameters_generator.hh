
#ifndef EOLIAN_CXX_STD_PARAMETERS_GENERATOR_HH
#define EOLIAN_CXX_STD_PARAMETERS_GENERATOR_HH

#include <iosfwd>

#include "tab.hh"
#include "eo_types.hh"

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
   parameters_container_type::const_iterator it, first = x._params.cbegin();
   parameters_container_type::const_iterator last = x._params.cend();
   for (it = first; it != last; ++it)
     {
        if (it != first) out << ", ";
        out << (*it).type << " " << (*it).name;
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
        out << (*it).type;
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
   parameters_container_type::const_iterator it, first = x._params.cbegin();
   parameters_container_type::const_iterator last = x._params.cend();
   for (it = first; it != last; ++it)
     {
        if (it != first) out << ", ";
        out << (*it).name;
     }
   return out;
}

} } } // namespace efl { namespace eolian { namespace grammar {


#endif // EOLIAN_CXX_STD_PARAMETERS_GENERATOR_HH
