#ifndef EFL_EO_JS_EVENT_HH
#define EFL_EO_JS_EVENT_HH

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

struct event_information
{
  v8::Handle<v8::Function>* constructor;
  Eo_Event_Description const* event;
};

struct event_callback_information
{
  event_information* event_info;
  v8::Persistent<v8::Function> function;
};

inline Eina_Bool event_callback(void* data, Eo* obj, Eo_Event_Description const*
                                , void* /*event_info*/)
{
  event_callback_information* event = static_cast<event_callback_information*>(data);
  v8::Handle<v8::Value> a[] = {v8::External::New(/*isolate,*/ obj)};
  // v8::Local<v8::Function> f = (*event->event_info->constructor)->GetFunction();
  v8::Local<v8::Object> self = (*event->event_info->constructor)->NewInstance(1, a);

  v8::Handle<v8::Value> call_args[] = {self};
  event->function->Call(v8::Context::GetCurrent()->Global(), 1, call_args);
  
  return EO_CALLBACK_CONTINUE;
}
      
#if 0

#else
inline v8::Handle<v8::Value> event_call(v8::Arguments const& args)
{
  if(args.Length() >= 1)
    {
      v8::Local<v8::Value> arg1 = args[0];
      if(arg1->IsFunction())
        {
          v8::Local<v8::Value> data = args.Data();
          event_information* event =
            static_cast<event_information*>
            (v8::External::Cast(*data)->Value());

          v8::Local<v8::Object> self = args.This();
          v8::Local<v8::Value> external = self->GetInternalField(0);
          Eo* eo = static_cast<Eo*>(v8::External::Cast(*external)->Value());

          event_callback_information* i = new event_callback_information
            {event, v8::Persistent<v8::Function>::New(v8::Handle<v8::Function>
                                                      (v8::Function::Cast(*arg1->ToObject())))};
          
          eo_do(eo, eo_event_callback_priority_add
                (event->event, EO_CALLBACK_PRIORITY_DEFAULT, &event_callback, i));
        }
    }
  else
    {
    }
  return v8::Handle<v8::Value>();
}
#endif

} } }

#endif
