
#ifndef EOLIAN_CXX_CONVERT_COMMENTS_HH
#define EOLIAN_CXX_CONVERT_COMMENTS_HH

#include <string>

#include <Eolian.h>
#include <Eolian_Cxx.hh>

#include "eolian_wrappers.hh"

namespace eolian_cxx {

std::string convert_comments_class(Eolian_Class const& klass);

std::string convert_comments_function(Eolian_Class const& klass,
                                      Eolian_Function const& function,
                                      Eolian_Function_Type func_type);

inline std::string
convert_comments_function(Eolian_Class const& klass, Eolian_Function const& function_, method_t func_type_)
{
   return convert_comments_function(klass, function_, func_type_.value);
}

inline std::string
convert_comments_function(Eolian_Class const& klass, Eolian_Function const& property_, getter_t func_type_)
{
   return convert_comments_function(klass, property_, func_type_.value);
}

inline std::string
convert_comments_function(Eolian_Class const& klass, Eolian_Function const& property_, setter_t func_type_)
{
   return convert_comments_function(klass, property_, func_type_.value);
}

inline std::string
convert_comments_function(Eolian_Class const& klass, Eolian_Function const& function_)
{
   return convert_comments_function(klass, function_, eolian_cxx::method);
}

}

#endif // EOLIAN_CXX_CONVERT_COMMENTS_HH
