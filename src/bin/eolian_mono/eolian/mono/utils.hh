#ifndef EOLIAN_MONO_UTILS_HPP
#define EOLIAN_MONO_UTILS_HPP

#include <string>
#include <algorithm>

/* Compared to the helpers.hh header, these functions are lower level, not dealing with
 * binding-specific structures or knowledge */

namespace eolian_mono { namespace utils {

   // Helper method to avoid multiple as_generator calls when mixing case strings
   inline std::string to_uppercase(std::string s)
   {
       std::transform(s.begin(), s.end(), s.begin(), ::toupper);
       return s;
   }
   inline std::string to_lowercase(std::string s)
   {
       std::transform(s.begin(), s.end(), s.begin(), ::tolower);
       return s;
   }
   inline std::string capitalize(std::string const &s)
   {
      std::string ret = s;
      ret[0] = std::toupper(ret[0]);
      return ret;
   }
} }

#endif
