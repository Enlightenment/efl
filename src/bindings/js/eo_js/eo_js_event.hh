#ifndef EFL_EO_JS_EVENT_HH
#define EFL_EO_JS_EVENT_HH

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

struct event_information
{
  eina::js::global_ref<v8::Function>* constructor;
  Efl_Event_Description const* event;
  Efl_Event_Cb event_callback;
  const char* class_name;
};

typedef std::map<std::string, event_information*> event_information_map;

struct event_callback_information
{
  event_information* event_info;
  eina::js::global_ref<v8::Function> function;
};

template <typename T>
v8::Local<v8::Value> get_event_info(void* event_info, v8::Isolate* isolate, const char* class_name)
{
  using no_tag_type = typename eina::js::remove_tag<T>::type;
  return eina::js::get_value_from_c(
    eina::js::wrap_value<T>(*static_cast<no_tag_type*>(event_info), eina::js::value_tag<T>{}),
    isolate,
    class_name);
}

// FIXME: This shouldn't be necessary. Reveiew Eolian standards.
template <>
inline v8::Local<v8::Value> get_event_info<const char*>(void* event_info, v8::Isolate* isolate, const char*)
{
  return eina::js::get_value_from_c(static_cast<const char*>(event_info), isolate, "");
}

template <>
inline v8::Local<v8::Value> get_event_info<void>(void*, v8::Isolate* isolate, const char*)
{
  return v8::Undefined(isolate);
}

template <typename T>
inline void event_callback(void* data, Efl_Event const* eo_event)
{
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);
  event_callback_information* event = static_cast<event_callback_information*>(data);
  v8::Handle<v8::Value> a[] = {eina::js::compatibility_new<v8::External>(isolate, eo_event->object)};
  v8::Local<v8::Object> self = (event->event_info->constructor->handle())->NewInstance(1, a);

  v8::Local<v8::Value> call_args[] = {
    self,
    get_event_info<T>(eo_event->info, isolate, event->event_info->class_name)
  };
  event->function.handle()->Call(eina::js::compatibility_global(), 2, call_args);
}

inline eina::js::compatibility_return_type on_event(eina::js::compatibility_callback_info_type args)
{
  if (args.Length() >= 2)
    {
      v8::Local<v8::Value> ev_name = args[0];
      v8::Local<v8::Value> f = args[1];
      if (ev_name->IsString() && f->IsFunction())
        {
          v8::Local<v8::Value> data = args.Data();
          auto ev_map =
            static_cast<event_information_map*>
            (v8::External::Cast(*data)->Value());

          v8::String::Utf8Value str(ev_name->ToString());
          auto found = ev_map->find(*str);
          if (found == ev_map->end())
            return eina::js::compatibility_return();

          auto event = found->second;

          v8::Local<v8::Object> self = args.This();
          v8::Local<v8::Value> external = self->GetInternalField(0);
          Eo* eo = static_cast<Eo*>(v8::External::Cast(*external)->Value());

          auto isolate = args.GetIsolate();

          event_callback_information* i = new event_callback_information
            {event, {isolate, eina::js::compatibility_cast<v8::Function>(f)}};
          efl_event_callback_add(eo, event->event, event->event_callback, i);
          efl::eina::js::make_weak(isolate, self, [i]{ delete i; });
        }
      else
        {
           eina::js::compatibility_throw
             (v8::Exception::TypeError
               (eina::js::compatibility_new<v8::String>(args.GetIsolate(), "Invalid argument type")));
           throw std::logic_error("");
        }
    }
  else
    {
       eina::js::compatibility_throw
         (v8::Exception::TypeError
           (eina::js::compatibility_new<v8::String>(args.GetIsolate(), "Expected more arguments for this call")));
       throw std::logic_error("");
    }
  return eina::js::compatibility_return();
}

} } }

#endif
