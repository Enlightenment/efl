#ifndef EOLIAN_JS_FORMAT_HH
#define EOLIAN_JS_FORMAT_HH

#include <eolian/js/domain.hh>

#include <algorithm>
#include <string>
#include <cctype>

namespace eolian { namespace js {

namespace format {

std::string generic(std::string const& in)
{
   std::string s = in;
   auto i = s.find('_');
   while (i != std::string::npos)
     {
        if (i <= 0 || i+1 >= s.size() ||
            !::isalnum(s[i-1]) || !::isalnum(s[i+1]))
          {
             EINA_CXX_DOM_LOG_WARN(eolian::js::domain) << "Entity '" << in
               << "' can't be conveniently converted to a JavaScript name.";
             return in;
          }
        s[i+1] = static_cast<char>(::toupper(s[i+1]));
        s.erase(i, 1);
        i = s.find('_', i);
     }
   return s;
}

std::string constant(std::string in)
{
   std::transform(in.begin(), in.end(), in.begin(), ::toupper);
   return in;
}

}

} }

#endif
