
#ifndef EOLIAN_CXX_EOLIAN_CONVERT_COMMENTS_HH
#define EOLIAN_CXX_EOLIAN_CONVERT_COMMENTS_HH

#include <string>

extern "C"
{
#include <Eina.h>
#include <Eolian.h>
}

#include <Eolian_Cxx.hh>

namespace detail {

std::string eolian_class_comment(const char *classname);

std::string eolian_constructor_comment(Eolian_Function constructor);

std::string eolian_function_comment(Eolian_Function function);

std::string eolian_property_getter_comment(Eolian_Function function);

std::string eolian_property_setter_comment(Eolian_Function function);

}

#endif // EOLIAN_CXX_EOLIAN_CONVERT_COMMENTS_HH
