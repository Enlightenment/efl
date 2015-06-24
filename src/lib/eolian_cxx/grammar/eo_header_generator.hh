
#ifndef EOLIAN_CXX_STD_EO_HEADER_GENERATOR_HH
#define EOLIAN_CXX_STD_EO_HEADER_GENERATOR_HH

#include <set>
#include <algorithm>
#include <string>
#include <ostream>
#include <iosfwd>
#include <cctype>

#include "eo_types.hh"
#include "tab.hh"
#include "eo_class_generator.hh"
#include "inheritance_base_generator.hh"

namespace {
std::string
_onceguard_key(efl::eolian::eo_class const& cls)
{
   std::string key;
   if (cls.name_space != "")
     {
        std::string ns = cls.name_space;
        size_t pos = 0;
        while ((pos = ns.find("::")) != std::string::npos)
          {
             key += ns.substr(0, pos) + "_";
             ns.erase(0, pos+2);
          }
        key += ns + "_";
     }
   key += cls.name;
   std::transform(key.begin(), key.end(), key.begin(), ::toupper);
   return key;
}
}

namespace efl { namespace eolian { namespace grammar {

struct include_dependencies
{
   eo_class const& _cls;
   eo_generator_options const& _opts;
   include_dependencies(eo_class const& cls, eo_generator_options const& opts)
     : _cls(cls)
     , _opts(opts)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, include_dependencies const& x)
{
   std::set<std::string> headers;
   eo_class const& cls = x._cls;

   for (auto it = cls.constructors.cbegin(), last = cls.constructors.cend();
        it != last; ++it)
     for (auto it_p = (*it).params.begin(), last_p = (*it).params.end();
          it_p != last_p; ++it_p)
       for (eolian_type const& subtype : (*it_p).type.parts)
         for (std::string header : subtype.includes)
           if (header != x._opts.header_decl_file_name)
             headers.insert(header);

   for (auto it = cls.functions.begin(), last  = cls.functions.end();
        it != last; ++it)
        for (auto it_p = (*it).params.begin(), last_p = (*it).params.end();
             it_p != last_p; ++it_p)
          for (eolian_type const& subtype : (*it_p).type.parts)
            for (std::string header : subtype.includes)
              if (header != x._opts.header_decl_file_name)
                headers.insert(header);

   for (std::string header : headers)
     out << "#include <" << header << ">" << endl;

   return out;
}

inline void
onceguard_head(std::ostream& out, eo_class const& cls)
{
   std::string key = ::_onceguard_key(cls);
   out << "#ifndef EFL_GENERATED_" << key << "_HH" << endl
       << "#define EFL_GENERATED_" <<  key << "_HH" << endl << endl;
}

inline void
onceguard_tail(std::ostream& out, eo_class const& cls)
{
   std::string key = ::_onceguard_key(cls);
   out << "#endif // EFL_GENERATED_" << key << "_HH" << endl;
}

inline void
include_headers(std::ostream& out,
                eo_class const& cls EINA_UNUSED,
                eo_generator_options const& opts)
{
   out << "extern \"C\"" << endl
       << "{" << endl
       << "#include <Efl.h>" << endl
       << "}" << endl
       << "#include <Eo.hh>" << endl << endl
       << "#include <eo_cxx_interop.hh>" << endl << endl
       << "extern \"C\"" << endl
       << "{" << endl;
   for (auto c_header : opts.c_headers)
     {
       out << "#include \"" << c_header << "\"" << endl;
     }
   out << "}" << endl << endl;
   for (auto cxx_header : opts.cxx_headers)
     {
       out << "#include \"" << cxx_header << "\"" << endl;
     }
   out << include_dependencies(cls, opts) << endl;
}

inline void
include_header_impl(std::ostream& out,
                eo_class const& cls EINA_UNUSED,
                eo_generator_options const& opts)
{
   out << "#include \"" << opts.header_impl_file_name << "\"" << endl << endl;
}

inline void
eo_headers_generator(std::ostream& header_decl,
                     std::ostream& header_impl,
                     eo_class const& cls,
                     eo_generator_options const& opts)
{
   onceguard_head(header_decl, cls);
   include_headers(header_decl, cls, opts);
   eo_class_declarations_generator(header_decl, cls);
   include_header_impl(header_decl, cls, opts);
   onceguard_tail(header_decl, cls);
   header_decl << endl;

   header_impl << comment("@cond EO_CXX_EO_IMPL") << endl;
   eo_class_definitions_generator(header_impl, cls);
   eo_inheritance_detail_generator(header_impl, cls);
   header_impl << endl << comment("@endcond") << endl;
}

} } } // namespace efl { namespace eolian { namespace grammar {

#endif // EOLIAN_CXX_STD_EO_HEADER_GENERATOR_HH
