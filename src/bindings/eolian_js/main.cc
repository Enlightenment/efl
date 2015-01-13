
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_NODEJS

#include <Eina.hh>
#include EINA_STRINGIZE(NODE_INCLUDE_HEADER)
#include EINA_STRINGIZE(UV_INCLUDE_HEADER)

#include <iostream>

#include <Efl.h>
#include <Eo.h>
#include <Ecore.h>
#include <Ecore_Evas.h>

namespace evas {
EAPI void register_box(v8::Handle<v8::Object> global, v8::Isolate* isolate);
}

namespace {

Ecore_Evas* ee;
  
void init(v8::Handle<v8::Object> exports)
{
  std::cout << "init hey " << std::endl;
  ::eina_init();
  ::ecore_init();
  ::ecore_evas_init();
  ::eo_init();

  // ee = ecore_evas_new(NULL, 100, 100, 200, 200, NULL);
  // ecore_evas_show(ee);
  try
    {
      evas::register_box(exports, v8::Isolate::GetCurrent());
    }
  catch(...)
    {
      std::cout << "Exception" << std::endl;
    }

  std::cout << "inited " << std::endl;
}
  
}

NODE_MODULE(efl, init)

#endif
