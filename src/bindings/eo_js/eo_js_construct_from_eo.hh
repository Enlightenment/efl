#ifndef EFL_EO_JS_CONSTRUCT_FROM_EO_HH
#define EFL_EO_JS_CONSTRUCT_FROM_EO_HH

#include <v8.h>

#include <eina_tuple.hh>
#include <eina_tuple_c.hh>
#include <eina_function.hh>
#include <Eo.h>

#include <eo_js_get_value.hh>
#include <eo_js_get_value_from_c.hh>

#include <cstdlib>
#include <functional>
#include <iostream>
#include <cassert>
#include <vector>

namespace efl { namespace eo { namespace js {

#if 0
#else
inline v8::Handle<v8::Value> construct_from_eo(v8::Arguments const& args)
{
  std::cout << "construct_from_eo " << __LINE__ << std::endl;
  if(args.IsConstructCall())
    {
  std::cout << "construct_from_eo " << __LINE__ << std::endl;
      args.This()->SetInternalField(0, args[0]);
  std::cout << "construct_from_eo " << __LINE__ << std::endl;
      return v8::Handle<v8::Value>();
    }
  else
    {
  std::cout << "construct_from_eo " << __LINE__ << std::endl;
      std::size_t argc = args.Length();
  std::cout << "construct_from_eo " << __LINE__ << std::endl;
      std::vector<v8::Local<v8::Value> > argv (argc ? argc : 1 );
  std::cout << "construct_from_eo " << __LINE__ << std::endl;
      for(int i = 0; i != args.Length(); ++i)
        argv[i] = args[i];
  std::cout << "construct_from_eo " << __LINE__ << std::endl;
      args.Callee()->NewInstance(argc, &argv[0]);
  std::cout << "construct_from_eo " << __LINE__ << std::endl;
      return v8::Handle<v8::Value>();
    }
  std::cout << "construct_from_eo " << __LINE__ << std::endl;
}
#endif
      
} } }

#endif
