#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Eldbus_Js.hh>

namespace efl { namespace eldbus { namespace js {

namespace {

void register_init(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                   v8::Handle<v8::String> name)
{
    using v8::Integer;
    using v8::FunctionTemplate;

    auto init = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 0)
            return compatibility_return();

        auto isolate = args.GetIsolate();
        auto ret = eldbus_init();
        return compatibility_return(compatibility_new<Integer>(isolate, ret),
                                    args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, init)
                ->GetFunction());
}

void register_shutdown(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                       v8::Handle<v8::String> name)
{
    using v8::Integer;
    using v8::FunctionTemplate;

    auto shutdown = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 0)
            return compatibility_return();

        auto isolate = args.GetIsolate();
        auto ret = eldbus_shutdown();
        return compatibility_return(compatibility_new<Integer>(isolate, ret),
                                    args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, shutdown)
                ->GetFunction());
}

void register_fdo_bus(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                      v8::Handle<v8::String> name)
{
    using v8::String;
    global->Set(name, compatibility_new<String>(isolate, ELDBUS_FDO_BUS));
}

void register_fdo_path(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                       v8::Handle<v8::String> name)
{
    using v8::String;
    global->Set(name, compatibility_new<String>(isolate, ELDBUS_FDO_PATH));
}

void register_fdo_interface(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                            v8::Handle<v8::String> name)
{
    using v8::String;
    global->Set(name, compatibility_new<String>(isolate, ELDBUS_FDO_INTERFACE));
}

void register_fdo_interface_properties(v8::Isolate *isolate,
                                       v8::Handle<v8::Object> global,
                                       v8::Handle<v8::String> name)
{
    using v8::String;
    global->Set(name,
                compatibility_new<String>(isolate,
                                          ELDBUS_FDO_INTERFACE_PROPERTIES));
}

void register_fdo_interface_object_manager(v8::Isolate *isolate,
                                           v8::Handle<v8::Object> global,
                                           v8::Handle<v8::String> name)
{
    using v8::String;
    global->Set(name,
                compatibility_new<String>(isolate,
                                          ELDBUS_FDO_INTERFACE_OBJECT_MANAGER));
}

void register_fdo_interface_introspectable(v8::Isolate *isolate,
                                           v8::Handle<v8::Object> global,
                                           v8::Handle<v8::String> name)
{
    using v8::String;
    global->Set(name,
                compatibility_new<String>(isolate,
                                          ELDBUS_FDO_INTERFACE_INTROSPECTABLE));
}

void register_fdo_inteface_peer(v8::Isolate *isolate,
                                v8::Handle<v8::Object> global,
                                v8::Handle<v8::String> name)
{
    using v8::String;
    global->Set(name,
                compatibility_new<String>(isolate, ELDBUS_FDO_INTEFACE_PEER));
}

void register_error_pending_canceled(v8::Isolate *isolate,
                                     v8::Handle<v8::Object> global,
                                     v8::Handle<v8::String> name)
{
    using v8::String;
    global->Set(name,
                compatibility_new<String>(isolate,
                                          ELDBUS_ERROR_PENDING_CANCELED));
}

void register_error_pending_timeout(v8::Isolate *isolate,
                                    v8::Handle<v8::Object> global,
                                    v8::Handle<v8::String> name)
{
    using v8::String;
    global->Set(name,
                compatibility_new<String>(isolate,
                                          ELDBUS_ERROR_PENDING_TIMEOUT));
}

}

EAPI
void register_eldbus_core(v8::Isolate* isolate, v8::Handle<v8::Object> exports)
{
   using v8::String;
   register_init(isolate, exports,
                 compatibility_new<String>(isolate, "eldbus_init"));
   register_shutdown(isolate, exports,
                     compatibility_new<String>(isolate, "eldbus_shutdown"));
   register_fdo_bus(isolate, exports,
                    compatibility_new<String>(isolate, "ELDBUS_FDO_BUS"));
   register_fdo_path(isolate, exports,
                     compatibility_new<String>(isolate, "ELDBUS_FDO_PATH"));
   register_fdo_interface(isolate, exports,
                          compatibility_new<String>(isolate,
                                                    "ELDBUS_FDO_INTERFACE"));
   register_fdo_interface_properties(isolate, exports,
                                     compatibility_new<String>
                                     (isolate,
                                      "ELDBUS_FDO_INTERFACE_PROPERTIES"));
   register_fdo_interface_object_manager(isolate, exports,
                                         compatibility_new<String>
                                         (isolate, "ELDBUS_FDO_INTERFACE_OBJECT_MANAGER"));
   
   register_fdo_interface_introspectable
     (isolate, exports,
      compatibility_new<String>(isolate,
                                "ELDBUS_FDO_INTERFACE_INTROSPECTABLE"));
   register_fdo_inteface_peer(isolate, exports,
                              compatibility_new<String>
                              (isolate, "ELDBUS_FDO_INTEFACE_PEER"));
   register_error_pending_canceled
     (isolate, exports,
      compatibility_new<String>(isolate, "ELDBUS_ERROR_PENDING_CANCELED"));
   register_error_pending_timeout
     (isolate, exports,
      compatibility_new<String>(isolate, "ELDBUS_ERROR_PENDING_TIMEOUT"));
}

EAPI void register_eldbus(v8::Isolate* isolate, v8::Handle<v8::Object> exports)
{
  register_eldbus_connection(isolate, exports);
  register_eldbus_core(isolate, exports);
  register_eldbus_message(isolate, exports);
  register_eldbus_object_mapper(isolate, exports);
}
      
} } } // namespace efl { namespace eldbus { namespace js {
