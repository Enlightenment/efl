#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Eldbus_Js.hh>

namespace efl { namespace eldbus { namespace js {

namespace {
      
void register_object_event_iface_added(v8::Isolate *isolate,
                                       v8::Handle<v8::Object> global,
                                       v8::Handle<v8::String> name)
{
    using v8::Integer;
    global->Set(name,
                compatibility_new<Integer>(isolate,
                                           ELDBUS_OBJECT_EVENT_IFACE_ADDED));
}

void register_object_event_iface_removed(v8::Isolate *isolate,
                                         v8::Handle<v8::Object> global,
                                         v8::Handle<v8::String> name)
{
    using v8::Integer;
    global->Set(name,
                compatibility_new<Integer>(isolate,
                                           ELDBUS_OBJECT_EVENT_IFACE_REMOVED));
}

void register_object_event_property_changed(v8::Isolate *isolate,
                                            v8::Handle<v8::Object> global,
                                            v8::Handle<v8::String> name)
{
    using v8::Integer;
    global->Set(name,
                compatibility_new<Integer>
                (isolate, ELDBUS_OBJECT_EVENT_PROPERTY_CHANGED));
}

void register_object_event_property_removed(v8::Isolate *isolate,
                                            v8::Handle<v8::Object> global,
                                            v8::Handle<v8::String> name)
{
    using v8::Integer;
    global->Set(name,
                compatibility_new<Integer>
                (isolate, ELDBUS_OBJECT_EVENT_PROPERTY_REMOVED));
}

void register_object_event_del(v8::Isolate *isolate,
                               v8::Handle<v8::Object> global,
                               v8::Handle<v8::String> name)
{
    using v8::Integer;
    global->Set(name,
                compatibility_new<Integer>(isolate, ELDBUS_OBJECT_EVENT_DEL));
}

void register_object_event_last(v8::Isolate *isolate,
                                v8::Handle<v8::Object> global,
                                v8::Handle<v8::String> name)
{
    using v8::Integer;
    global->Set(name,
                compatibility_new<Integer>(isolate, ELDBUS_OBJECT_EVENT_LAST));
}

}

EAPI
void register_eldbus_object_mapper(v8::Isolate* isolate, v8::Handle<v8::Object> exports)
{
  register_object_event_iface_added
    (isolate, exports, compatibility_new<v8::String>
     (isolate, "object_event_iface_added"));

  register_object_event_iface_removed
    (isolate, exports, compatibility_new<v8::String>
     (isolate, "object_event_iface_removed"));

  register_object_event_property_changed
    (isolate, exports, compatibility_new<v8::String>
     (isolate, "object_event_property_changed"));

  register_object_event_property_removed
    (isolate, exports, compatibility_new<v8::String>
     (isolate, "object_event_property_removed"));

  register_object_event_del
    (isolate, exports, compatibility_new<v8::String>
     (isolate, "object_event_del"));

  register_object_event_last
    (isolate, exports, compatibility_new<v8::String>
     (isolate, "object_event_last"));
}


} } } // namespace efl { namespace eldbus { namespace js {
