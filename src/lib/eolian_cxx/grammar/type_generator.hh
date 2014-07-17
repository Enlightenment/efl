
#ifndef EOLIAN_CXX_GRAMMAR_TYPE_GENERATOR_HH
#define EOLIAN_CXX_GRAMMAR_TYPE_GENERATOR_HH

#include <ostream>
#include <iosfwd>

#include "eo_types.hh"

namespace efl { namespace eolian { namespace grammar {

using std::endl;

struct reinterpret_type
{
   eolian_type_instance const& _list;
   reinterpret_type(eolian_type_instance const& list)
     : _list(list)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, efl::eolian::grammar::reinterpret_type const& x)
{
   assert(x._list.size() > 0);
   std::string res;
   for (auto rit = x._list.rbegin(), last = x._list.rend(); rit != last; ++rit)
     {
        if (type_is_complex(*rit))
          res = (*rit).binding + "< " + res + " >";
        else
          res = type_is_binding(*rit) ? (*rit).binding : (*rit).native;
     }
   assert(!res.empty());
   return out << res;
}

struct type_ownership
{
   eolian_type_instance const& _type;
   type_ownership(eolian_type_instance const& type)
     : _type(type)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, type_ownership const& x)
{
   out << "std::tuple<";
   for (auto it=x._type.begin(), last=x._type.end(); it != last; ++it)
     {
        if (it != x._type.begin())
          out << ", ";
        out << ((*it).is_own ? "std::true_type" : "std::false_type");
     }
   return out << ">()";
}

struct to_cxx
{
   eolian_type_instance const& _type;
   std::string const& _varname;
   to_cxx(eolian_type_instance const& type, std::string const& varname)
     : _type(type), _varname(varname)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, to_cxx const& x)
{
   if (type_is_binding(x._type))
     {
        out << "efl::eolian::to_cxx<"
            << reinterpret_type(x._type)
            << ">(" << x._varname
            << ", " << type_ownership(x._type) << ");";
     }
   else
     out << "_tmp_ret";
   return out;
}

struct to_c
{
   eolian_type_instance const& _type;
   std::string const& _varname;
   to_c(eolian_type_instance const& type, std::string const& varname)
     : _type(type), _varname(varname)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, to_c const& x)
{
   if (type_is_callback(x._type))
     out << "efl::eolian::get_callback<" << type_to_native_str(x._type) << ", F>()";
   else if (type_is_binding(x._type))
     out << "efl::eolian::to_c(" << x._varname << ")";
   else
     out << x._varname;
   return out;
}

} } }

#endif // EOLIAN_CXX_GRAMMAR_TYPE_GENERATOR_HH
