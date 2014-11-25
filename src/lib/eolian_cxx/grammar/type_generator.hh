
#ifndef EOLIAN_CXX_GRAMMAR_TYPE_GENERATOR_HH
#define EOLIAN_CXX_GRAMMAR_TYPE_GENERATOR_HH

#include <ostream>
#include <iosfwd>

#include "eo_types.hh"

namespace efl { namespace eolian { namespace grammar {

using std::endl;

struct c_type
{
   eolian_type_instance const& _list;
   c_type(eolian_type_instance const& list)
     : _list(list)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, efl::eolian::grammar::c_type const& x)
{
   assert(x._list.size() > 0);
   std::string res;
   for (auto rit = x._list.rbegin(), last = x._list.rend(); rit != last; ++rit)
     {
       res = /*type_is_binding(*rit) ? (*rit).binding :*/ (*rit).native;
     }
   assert(!res.empty());
   return out << res;
}

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
        eolian_type const& t = *rit;
        if (type_is_complex(t))
          res = t.binding + "< " + res + " >" + (t.is_out ? "*" : "");
        else
          res = type_is_binding(t) ? t.binding + (t.is_out ? "*" : "")
                                   : t.native;
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

struct
template_parameter_type
{
   eolian_type_instance const& _type;
   std::string const& _name;
   template_parameter_type(eolian_type_instance const& type, std::string const& name)
     : _type(type)
     , _name(name)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, template_parameter_type const& x)
{
   return out << "F_" << x._name;
}

struct
parameter_type
{
   eolian_type_instance const& _type;
   std::string const& _name;
   parameter_type(eolian_type_instance const& t, std::string const& name)
     : _type(t)
     , _name(name)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, parameter_type const& x)
{
   if(type_is_callback(x._type))
      out << template_parameter_type(x._type, x._name);
   else
      out << reinterpret_type(x._type);
   return out;
}

struct
parameter_no_ref_type
{
   eolian_type_instance const& _type;
   std::string const& _name;
   parameter_no_ref_type(eolian_type_instance const& type, std::string const& name)
     : _type(type)
     , _name(name)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, parameter_no_ref_type const& x)
{
   return out << "_no_ref_" << parameter_type(x._type, x._name);
}

struct
parameter_remove_reference_typedef
{
   eolian_type_instance const& _type;
   std::string const& _name;
   parameter_remove_reference_typedef(eolian_type_instance const& type, std::string const& name)
     : _type(type)
     , _name(name)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, parameter_remove_reference_typedef const& x)
{
   out << "typedef typename std::remove_reference<"
       << parameter_type(x._type, x._name)
       << ">::type " << parameter_no_ref_type(x._type, x._name) << ";";
   return out;
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
            << ", " << type_ownership(x._type) << ")";
     }
   else
     out << x._varname;
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
     out << "efl::eolian::get_callback<" << type_to_native_str(x._type)
         << ", " << parameter_no_ref_type(x._type, x._varname) << " >()";
   else if (type_is_complex(x._type) && type_is_binding(x._type))
     out << "efl::eolian::to_native<" << c_type(x._type) << ">(" << x._varname << ")";
   else if (type_is_binding(x._type))
     out << "efl::eolian::to_c(" << x._varname << ")";
   else
     out << x._varname;
   return out;
}

} } }

#endif // EOLIAN_CXX_GRAMMAR_TYPE_GENERATOR_HH
