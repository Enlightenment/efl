#ifndef EVAS_IMAGE_IMPL_HH
#define EVAS_IMAGE_IMPL_HH

#include "evas_image_eo.hh"

namespace evas { 
} 
namespace eo_cxx {
namespace evas { 
inline evas::Image::operator ::evas::Image() const { return *static_cast< ::evas::Image const*>(static_cast<void const*>(this)); }
inline evas::Image::operator ::evas::Image&() { return *static_cast< ::evas::Image*>(static_cast<void*>(this)); }
inline evas::Image::operator ::evas::Image const&() const { return *static_cast< ::evas::Image const*>(static_cast<void const*>(this)); }
} }
#endif
