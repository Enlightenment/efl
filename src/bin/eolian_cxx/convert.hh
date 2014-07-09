
#ifndef EOLIAN_CXX_EOLIAN_CONVERT_CLASSES_HH
#define EOLIAN_CXX_EOLIAN_CONVERT_CLASSES_HH

#include "eo_types.hh"

namespace eolian_cxx
{

///
/// @brief Retrieve a efl::eolian::eo_class from an Eolian_Class* name.
/// @param cls The Eolian class.
/// @return The @p eo_class describing @p classname.
///
efl::eolian::eo_class convert_eolian_class(const Eolian_Class& klass);

}

#endif // EOLIAN_CXX_EOLIAN_CONVERT_CLASSES_HH
