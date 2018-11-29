#ifndef EOLIAN_MONO_UTILS_HPP
#define EOLIAN_MONO_UTILS_HPP

#include <string>
#include <vector>
#include <sstream>
#include <iterator>
#include <algorithm>

/* Compared to the helpers.hh and name_helpers headers, these functions are
 * lower level, not dealing with binding-specific structures or knowledge */

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

   std::vector<std::string> split(std::string const &input, char delim)
   {
      std::stringstream ss(input);
      std::string name;
      std::vector<std::string> names;

      while (std::getline(ss, name, delim))
        {
           if (!name.empty())
             names.push_back(name);
        }
      return names;
   }

   std::string to_pascal_case(const std::vector<std::string> &names, std::string const& delim="")
   {
     std::vector<std::string> outv(names.size());
        std::stringstream osstream;

        std::transform(names.begin(), names.end(), outv.begin(),
              [](std::string name) {
                name[0] = std::toupper(name[0]);
                return name;
              });

        std::copy(outv.begin(), outv.end(), std::ostream_iterator<std::string>(osstream, delim.c_str()));

        std::string ret = osstream.str();

        if (delim != "")
            ret.pop_back(); // We could implement an infix_iterator but this pop is enough for now.

        return ret;
    }

   inline std::string remove_all(std::string name, char target)
   {
       name.erase(std::remove(name.begin(), name.end(), target), name.end());
       return name;
   }
} }

#endif
