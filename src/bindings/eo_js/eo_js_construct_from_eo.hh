#ifndef EFL_EO_JS_CONSTRUCT_FROM_EO_HH
#define EFL_EO_JS_CONSTRUCT_FROM_EO_HH

#include V8_INCLUDE_HEADER

#include <eina_tuple.hh>
#include <eina_tuple_c.hh>
#include <eina_function.hh>
#include <Eo.h>

#include <eina_js_get_value.hh>
#include <eina_js_get_value_from_c.hh>

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
  if(args.IsConstructCall())
    {
      args.This()->SetInternalField(0, args[0]);
      return v8::Handle<v8::Value>();
    }
  else
    {
      std::size_t argc = args.Length();
      std::vector<v8::Local<v8::Value> > argv (argc ? argc : 1 );
      for(int i = 0; i != args.Length(); ++i)
        argv[i] = args[i];
      args.Callee()->NewInstance(argc, &argv[0]);
      return v8::Handle<v8::Value>();
    }
}
#endif
      
} } }

#endif
