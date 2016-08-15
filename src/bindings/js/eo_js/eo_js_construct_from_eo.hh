#ifndef EFL_EO_JS_CONSTRUCT_FROM_EO_HH
#define EFL_EO_JS_CONSTRUCT_FROM_EO_HH

#include <eina_tuple.hh>
#include <eina_tuple_c.hh>
#include <eina_function.hh>
#include <Eo.h>

#include <cstdlib>
#include <functional>
#include <iostream>
#include <cassert>
#include <vector>

namespace efl { namespace eo { namespace js {

inline eina::js::compatibility_return_type construct_from_eo(eina::js::compatibility_callback_info_type args)
{
  if(args.IsConstructCall())
    {
      Eo* eo = static_cast<Eo*>(v8::External::Cast(*args[0])->Value());
      args.This()->SetInternalField(0, args[0]);
      ::efl_ref(eo);
      efl::eina::js::make_weak(args.GetIsolate(), args.This(), [eo] { efl_unref(eo); });
      return eina::js::compatibility_return();
    }
  else
    {
      std::size_t argc = args.Length();
      std::vector<v8::Local<v8::Value> > argv (argc ? argc : 1 );
      for(int i = 0; i != args.Length(); ++i)
        argv[i] = args[i];
      args.Callee()->NewInstance(argc, &argv[0]);
      return eina::js::compatibility_return();
    }
}
      
} } }

#endif
