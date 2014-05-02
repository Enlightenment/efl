
#ifndef EOLIAN_CXX_BIN_SAFE_STRINGS_HH
#define EOLIAN_CXX_BIN_SAFE_STRINGS_HH

#include <string>

extern "C"
{
#include <Eina.h>
}

/// @brief Safely convert an const char* to std::string.
inline std::string
safe_str(const char* str)
{
   return (str != NULL) ? str : "";
}

/// @brief Safely convert an Eina_Stringshare to std::string.
inline std::string
safe_strshare(Eina_Stringshare* strsh)
{
   std::string ret = strsh != NULL ? strsh : "";
   eina_stringshare_del(strsh);
   return ret;
}

#endif // EOLIAN_CXX_BIN_SAFE_STRINGS_HH
