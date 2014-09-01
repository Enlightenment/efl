#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Eina.hh>
#include <Eina_Js.hh>

namespace efl { namespace eina { namespace js {

std::map<std::string, v8::Local<v8::Function>> constructors_map_;

} } }
