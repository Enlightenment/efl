#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Eina.hh>
#include <Eina_Js.hh>

namespace efl { namespace eina { namespace js {

EAPI
void register_destroy_iterator(v8::Isolate *isolate,
                               v8::Handle<v8::Object> global,
                               v8::Handle<v8::String> name)
{
    typedef void (*deleter_t)(void*);

    auto f = [](compatibility_callback_info_type info) -> compatibility_return_type
      {
        if (info.Length() != 1 || !info[0]->IsObject())
          return compatibility_return();

        v8::Handle<v8::Object> o = info[0]->ToObject();

        deleter_t deleter = compatibility_get_pointer_internal_field<deleter_t>(o, 1);
        deleter(compatibility_get_pointer_internal_field<>(o, 0));
        compatibility_set_pointer_internal_field(o, 0, static_cast<void*>(0));
        return compatibility_return();
      };

    global->Set(name, compatibility_new<v8::FunctionTemplate>(isolate, f)->GetFunction());
}

} } } // namespace efl { namespace js {
