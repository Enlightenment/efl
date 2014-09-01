
#include <v8.h>
#include <Eina.h>
#include <cstdlib>

#include <iostream>

namespace efl { namespace js { namespace {

struct eina_list
{
  Eina_List* _list;
};

void new_eina_list(v8::FunctionCallbackInfo<v8::Value> const& args)
{
  std::cerr << "called eina list constructor" << std::endl;
  args.This()->SetAlignedPointerInInternalField(0, new eina_list);
}

} } }

EAPI void eina_list_register(v8::Handle<v8::ObjectTemplate> global, v8::Isolate* isolate)
{
  v8::Local<v8::FunctionTemplate> constructor = v8::FunctionTemplate::New(isolate, &efl::js::new_eina_list );
  v8::Local<v8::ObjectTemplate> instance_t = constructor->InstanceTemplate();
  instance_t->SetInternalFieldCount(1);
  global->Set(v8::String::NewFromUtf8(isolate, "List"), constructor);
}

