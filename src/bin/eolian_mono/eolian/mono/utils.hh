#ifndef EOLIAN_MONO_UTILS_HPP
#define EOLIAN_MONO_UTILS_HPP

#include <string>
#include <algorithm>

namespace eolian_mono { namespace utils {

   // Helper method to avoid multiple as_generator calls when mixing case strings
   std::string to_uppercase(std::string s)
   {
       std::transform(s.begin(), s.end(), s.begin(), ::toupper);
       return s;
   }
   std::string to_lowercase(std::string s)
   {
       std::transform(s.begin(), s.end(), s.begin(), ::tolower);
       return s;
   }
} }

#endif
