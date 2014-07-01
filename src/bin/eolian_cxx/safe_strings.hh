
#ifndef EOLIAN_CXX_BIN_SAFE_STRINGS_HH
#define EOLIAN_CXX_BIN_SAFE_STRINGS_HH

#include <string>
#include <algorithm>
#include <cstddef>
#include <cctype>
#include <iterator>

extern "C"
{
#include <Eina.h>
}

/// @brief Safely convert const char* to std::string.
///
inline std::string
safe_str(const char* str)
{
   return (str != NULL) ? str : "";
}

/// @brief Safely convert Eina_Stringshare to std::string.
///
inline std::string
safe_strshare(Eina_Stringshare* strsh)
{
   std::string ret = strsh != NULL ? strsh : "";
   eina_stringshare_del(strsh);
   return ret;
}

/// @brief Get a lower-case copy of string.
///
inline std::string
safe_lower(std::string const& s)
{
   std::string res;
   res.resize(s.size());
   std::transform(s.begin(), s.end(), res.begin(), ::tolower);
   return res;
}

/// @brief Get a lower-case copy of string.
///
inline std::string
safe_lower(const char *s)
{
   return safe_lower(safe_str(s));
}

/// @brief Get a upper-case copy of string.
///
inline std::string
safe_upper(std::string const& s)
{
   std::string res;
   res.resize(s.size());
   std::transform(s.begin(), s.end(), res.begin(), ::toupper);
   return res;
}

/// @brief Get a upper-case copy of string.
///
inline std::string
safe_upper(const char* s)
{
   return safe_upper(safe_str(s));
}

/// @brief Trim both ends of the string and replaces any
/// subsequence of contiguous spaces with a single space.
///
inline std::string
normalize_spaces(std::string const& s)
{
   std::ostringstream os;
   bool prev_is_space = true;
   std::remove_copy_if
     (s.begin(), s.end(),
      std::ostream_iterator<char>(os),
      [&prev_is_space] (char c)
      {
         bool r = ::isspace(c);
         if (r && prev_is_space)
           return true;
            prev_is_space = r;
         return false;
      });
   std::string r = os.str();
   if (!r.empty() && ::isspace(r.back()))
     r.erase(r.end()-1, r.end());
   return r;
}

/// @brief Return the basename of a path.
///
inline std::string
path_base(std::string path)
{
   std::string::reverse_iterator
     slash = std::find(path.rbegin(), path.rend(), '/');
   return std::string(slash.base(), path.end());
}

/// @brief Find-and-replace patterns in a string.
///
inline std::string
find_replace(std::string const& s_,
             std::string const& old,
             std::string const& new_)
{
   std::string s = s_;
   std::string::size_type found = s.find(old);
   std::string::size_type len = new_.length();
   while (found != std::string::npos)
     {
        s.replace(found, len, new_);
        found = s.find(old);
     }
   return s;
}

#endif // EOLIAN_CXX_BIN_SAFE_STRINGS_HH
