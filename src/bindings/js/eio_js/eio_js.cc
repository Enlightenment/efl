#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Eio_Js.hh>

namespace efl { namespace eio { namespace js {

using ::efl::eina::js::compatibility_new;
using ::efl::eina::js::compatibility_return_type;
using ::efl::eina::js::compatibility_callback_info_type;
using ::efl::eina::js::compatibility_return;
using ::efl::eina::js::compatibility_get_pointer_internal_field;
using ::efl::eina::js::compatibility_set_pointer_internal_field;

namespace {
      
Eio_File* extract_eio_file(v8::Local<v8::Object> object)
{
    return compatibility_get_pointer_internal_field<Eio_File*>(object, 0);
}

v8::Local<v8::Object> wrap_eio_file(Eio_File *file, v8::Isolate *isolate)
{
    using v8::String;
    using v8::Boolean;
    using v8::ObjectTemplate;
    using v8::FunctionTemplate;

    auto obj_tpl = compatibility_new<ObjectTemplate>(isolate);
    obj_tpl->SetInternalFieldCount(1);
    auto ret = obj_tpl->NewInstance();

    auto cancel = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 0)
            return compatibility_return();

        auto isolate = info.GetIsolate();

        auto ret = eio_file_cancel(extract_eio_file(info.This()));
        return compatibility_return(compatibility_new<Boolean>(isolate, ret),
                                    info);
    };

    auto check = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 0)
            return compatibility_return();

        auto isolate = info.GetIsolate();

        auto ret = eio_file_check(extract_eio_file(info.This()));
        return compatibility_return(compatibility_new<Boolean>(isolate, ret),
                                    info);
    };

    ret->Set(compatibility_new<String>(isolate, "cancel"),
             compatibility_new<FunctionTemplate>(isolate, cancel)
             ->GetFunction());
    ret->Set(compatibility_new<String>(isolate, "check"),
             compatibility_new<FunctionTemplate>(isolate, check)
             ->GetFunction());

    compatibility_set_pointer_internal_field(ret, 0, file);

    return ret;
}

static Eina_File* extract_eina_file(v8::Local<v8::Object> object)
{
    return compatibility_get_pointer_internal_field<Eina_File*>(object, 0);
}

static v8::Local<v8::Object> wrap_eina_file(Eina_File *file,
                                            v8::Isolate *isolate)
{
    using v8::String;
    using v8::ObjectTemplate;
    using v8::Object;
    using v8::FunctionTemplate;

    auto obj_tpl = compatibility_new<ObjectTemplate>(isolate);
    obj_tpl->SetInternalFieldCount(1);
    auto ret = obj_tpl->NewInstance();

    /* TODO:

       Eina_File only handle files open in read-only mode. Althought there
       aren't many useful functions exposed to a JavaScript binding, the exposed
       `file->fd` can be used to expose a few read operations found in
       `<cstdio>`.
       */

    auto close = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 0)
            return compatibility_return();

        auto o = info.This();

        auto file = extract_eina_file(o);
        if (!file)
            return compatibility_return();

        eina_file_close(file);
        compatibility_set_pointer_internal_field<void*>(o, 0, nullptr);
        return compatibility_return();
    };

    ret->Set(compatibility_new<String>(isolate, "close"),
             compatibility_new<FunctionTemplate>(isolate, close)
             ->GetFunction());

    compatibility_set_pointer_internal_field(ret, 0, file);

    auto on_gc = [ret]() {
      auto file = extract_eina_file(ret);

      if (file) {
        eina_file_close(file);
        compatibility_set_pointer_internal_field<void*>(ret, 0, nullptr);
      }
    };
    
    efl::eina::js::make_weak(isolate, ret, on_gc);

    return ret;
}

static Eio_Monitor* extract_monitor(v8::Local<v8::Object> object)
{
    return compatibility_get_pointer_internal_field<Eio_Monitor*>(object, 0);
}

static v8::Local<v8::Object> wrap_monitor(Eio_Monitor *monitor,
                                          v8::Isolate *isolate)
{
    using v8::String;
    using v8::ObjectTemplate;
    using v8::FunctionTemplate;

    auto obj_tpl = compatibility_new<ObjectTemplate>(isolate);
    obj_tpl->SetInternalFieldCount(1);
    auto ret = obj_tpl->NewInstance();

    auto del = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 0)
            return compatibility_return();

        eio_monitor_del(extract_monitor(info.This()));
        return compatibility_return();
    };

    auto path_get = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 0)
            return compatibility_return();

        auto ret = eio_monitor_path_get(extract_monitor(info.This()));
        return compatibility_return(compatibility_new<String>(info.GetIsolate(),
                                                              ret),
                                    info);
    };

    ret->Set(compatibility_new<String>(isolate, "del"),
             compatibility_new<FunctionTemplate>(isolate, del)->GetFunction());
    ret->Set(compatibility_new<String>(isolate, "getPath"),
             compatibility_new<FunctionTemplate>(isolate, path_get)
             ->GetFunction());

    compatibility_set_pointer_internal_field(ret, 0, monitor);

    return ret;
}

static Ecore_Event_Handler *extract_event_handler(v8::Local<v8::Object> object)
{
    return compatibility_get_pointer_internal_field<Ecore_Event_Handler*>
        (object, 0);
}

static v8::Local<v8::Object> wrap_event_handler(Ecore_Event_Handler *handler,
                                                v8::Isolate *isolate)
{
    using v8::String;
    using v8::ObjectTemplate;
    using v8::FunctionTemplate;
    using v8::Value;

    auto obj_tpl = compatibility_new<ObjectTemplate>(isolate);
    obj_tpl->SetInternalFieldCount(1);
    auto ret = obj_tpl->NewInstance();

    auto del = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 0)
            return compatibility_return();

        auto p = ecore_event_handler_del(extract_event_handler(info.This()));

        auto per = static_cast<efl::eina::js::global_ref<Value>*>(p);
        per->dispose();
        delete per;
        return compatibility_return();
    };

    ret->Set(compatibility_new<String>(isolate, "del"),
             compatibility_new<FunctionTemplate>(isolate, del)->GetFunction());

    compatibility_set_pointer_internal_field(ret, 0, handler);

    return ret;
}

static
v8::Local<v8::Object> wrap_eina_file_direct_info(const Eina_File_Direct_Info*
                                                 info,
                                                 v8::Isolate *isolate)
{
    using v8::Object;
    using v8::String;
    using v8::Integer;

    auto wrapped_info = compatibility_new<Object>(isolate);

    wrapped_info->Set(compatibility_new<String>(isolate, "pathLength"),
                      compatibility_new<Integer>(isolate, info->path_length));
    wrapped_info->Set(compatibility_new<String>(isolate, "nameLength"),
                      compatibility_new<Integer>(isolate, info->name_length));
    wrapped_info->Set(compatibility_new<String>(isolate, "nameStart"),
                      compatibility_new<Integer>(isolate, info->name_start));
    wrapped_info->Set(compatibility_new<String>(isolate, "type"),
                      compatibility_new<Integer>(isolate, info->type));
    wrapped_info->Set(compatibility_new<String>(isolate, "path"),
                      compatibility_new<String>(isolate, info->path));

    return wrapped_info;
}

EAPI
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
        auto ret = eio_init();
        return compatibility_return(compatibility_new<Integer>(isolate, ret),
                                    args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, init)
                ->GetFunction());
}

EAPI
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
        auto ret = eio_shutdown();
        return compatibility_return(compatibility_new<Integer>(isolate, ret),
                                    args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, shutdown)
                ->GetFunction());
}

EAPI
void register_op_file_copy(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                           v8::Handle<v8::String> name)
{
    using v8::Integer;
    global->Set(name, compatibility_new<Integer>(isolate, EIO_FILE_COPY));
}

EAPI
void register_op_file_move(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                           v8::Handle<v8::String> name)
{
    using v8::Integer;
    global->Set(name, compatibility_new<Integer>(isolate, EIO_FILE_MOVE));
}

EAPI
void register_op_dir_copy(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                          v8::Handle<v8::String> name)
{
    using v8::Integer;
    global->Set(name, compatibility_new<Integer>(isolate, EIO_DIR_COPY));
}

EAPI
void register_op_dir_move(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                          v8::Handle<v8::String> name)
{
    using v8::Integer;
    global->Set(name, compatibility_new<Integer>(isolate, EIO_DIR_MOVE));
}

EAPI
void register_op_unlink(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                        v8::Handle<v8::String> name)
{
    using v8::Integer;
    global->Set(name, compatibility_new<Integer>(isolate, EIO_UNLINK));
}

EAPI
void register_op_file_getpwnam(v8::Isolate *isolate,
                               v8::Handle<v8::Object> global,
                               v8::Handle<v8::String> name)
{
    using v8::Integer;
    global->Set(name, compatibility_new<Integer>(isolate, EIO_FILE_GETPWNAM));
}

EAPI
void register_op_file_getgrnam(v8::Isolate *isolate,
                               v8::Handle<v8::Object> global,
                               v8::Handle<v8::String> name)
{
    using v8::Integer;
    global->Set(name, compatibility_new<Integer>(isolate, EIO_FILE_GETGRNAM));
}

EAPI
void register_file_open(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                        v8::Handle<v8::String> name)
{
    using v8::Integer;
    using v8::Value;
    using v8::String;
    using v8::FunctionTemplate;
    using v8::Function;
    using v8::Handle;
    using std::unique_ptr;

    typedef efl::eina::js::global_ref<Value> persistent_t;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 4 || !args[0]->IsString() || !args[1]->IsBoolean()
            || !args[2]->IsFunction() || !args[3]->IsFunction())
            return compatibility_return();

        auto isolate = args.GetIsolate();

        auto cb_data = unique_ptr<persistent_t[]>(new persistent_t[2]);
        cb_data[0] = persistent_t(isolate, args[2]);
        cb_data[1] = persistent_t(isolate, args[3]);

        auto open_cb = [](void *data, Eio_File *handler, Eina_File *file) {
            auto persistent = reinterpret_cast<persistent_t*>(data);
            auto o = persistent->handle();

            auto isolate = v8::Isolate::GetCurrent();

            Handle<Value> args[2] = {
                wrap_eio_file(handler, isolate),
                wrap_eina_file(file, isolate)
            };

            Function::Cast(*o)->Call(o->ToObject(), 2, args);

            delete[] persistent;
        };
        auto error_cb = [](void *data, Eio_File *handler, int error) {
            auto persistent = reinterpret_cast<persistent_t*>(data) + 1;
            auto o = persistent->handle();

            auto isolate = v8::Isolate::GetCurrent();

            Handle<Value> args[2] = {
                wrap_eio_file(handler, isolate),
                compatibility_new<Integer>(isolate, error)
            };

            Function::Cast(*o)->Call(o->ToObject(), 2, args);

            delete[] (persistent - 1);
        };

        auto ret = eio_file_open(*String::Utf8Value(args[0]),
                                 args[1]->BooleanValue(), open_cb, error_cb,
                                 cb_data.release());

        return compatibility_return(wrap_eio_file(ret, isolate), args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

EAPI
void register_monitor_file_created(v8::Isolate *isolate,
                                   v8::Handle<v8::Object> global,
                                   v8::Handle<v8::String> name)
{
    using v8::Integer;
    global->Set(name,
                compatibility_new<Integer>(isolate, EIO_MONITOR_FILE_CREATED));
}

EAPI
void register_monitor_file_deleted(v8::Isolate *isolate,
                                   v8::Handle<v8::Object> global,
                                   v8::Handle<v8::String> name)
{
    using v8::Integer;
    global->Set(name,
                compatibility_new<Integer>(isolate, EIO_MONITOR_FILE_DELETED));
}

EAPI
void register_monitor_file_modified(v8::Isolate *isolate,
                                    v8::Handle<v8::Object> global,
                                    v8::Handle<v8::String> name)
{
    using v8::Integer;
    global->Set(name,
                compatibility_new<Integer>(isolate, EIO_MONITOR_FILE_MODIFIED));
}

EAPI
void register_monitor_file_closed(v8::Isolate *isolate,
                                  v8::Handle<v8::Object> global,
                                  v8::Handle<v8::String> name)
{
    using v8::Integer;
    global->Set(name,
                compatibility_new<Integer>(isolate, EIO_MONITOR_FILE_CLOSED));
}

EAPI
void register_monitor_directory_created(v8::Isolate *isolate,
                                        v8::Handle<v8::Object> global,
                                        v8::Handle<v8::String> name)
{
    using v8::Integer;
    global->Set(name,
                compatibility_new<Integer>(isolate,
                                           EIO_MONITOR_DIRECTORY_CREATED));
}

EAPI
void register_monitor_directory_deleted(v8::Isolate *isolate,
                                        v8::Handle<v8::Object> global,
                                        v8::Handle<v8::String> name)
{
    using v8::Integer;
    global->Set(name,
                compatibility_new<Integer>(isolate,
                                           EIO_MONITOR_DIRECTORY_DELETED));
}

EAPI
void register_monitor_directory_modified(v8::Isolate *isolate,
                                         v8::Handle<v8::Object> global,
                                         v8::Handle<v8::String> name)
{
    using v8::Integer;
    global->Set(name,
                compatibility_new<Integer>(isolate,
                                           EIO_MONITOR_DIRECTORY_MODIFIED));
}

EAPI
void register_monitor_directory_closed(v8::Isolate *isolate,
                                       v8::Handle<v8::Object> global,
                                       v8::Handle<v8::String> name)
{
    using v8::Integer;
    global->Set(name,
                compatibility_new<Integer>(isolate,
                                           EIO_MONITOR_DIRECTORY_CLOSED));
}

EAPI
void register_monitor_self_rename(v8::Isolate *isolate,
                                  v8::Handle<v8::Object> global,
                                  v8::Handle<v8::String> name)
{
    using v8::Integer;
    global->Set(name,
                compatibility_new<Integer>(isolate, EIO_MONITOR_SELF_RENAME));
}

EAPI
void register_monitor_self_deleted(v8::Isolate *isolate,
                                   v8::Handle<v8::Object> global,
                                   v8::Handle<v8::String> name)
{
    using v8::Integer;
    global->Set(name,
                compatibility_new<Integer>(isolate, EIO_MONITOR_SELF_DELETED));
}

EAPI
void register_monitor_error(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                            v8::Handle<v8::String> name)
{
    using v8::Integer;
    global->Set(name,
                compatibility_new<Integer>(isolate, EIO_MONITOR_ERROR));
}

EAPI
void register_monitor_add(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                          v8::Handle<v8::String> name)
{
    using v8::String;
    using v8::Integer;
    using v8::FunctionTemplate;
    using v8::Handle;
    using v8::Value;
    using v8::Function;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 1 || !args[0]->IsString())
            return compatibility_return();

        auto isolate = args.GetIsolate();
        auto ret = eio_monitor_add(*String::Utf8Value(args[0]));
        return compatibility_return(wrap_monitor(ret, isolate), args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

template<int &wanted_event>
static
void register_monitor_event_handler_add(v8::Isolate *isolate,
                                        v8::Handle<v8::Object> global,
                                        v8::Handle<v8::String> name)
{
    using v8::Integer;
    using v8::String;
    using v8::Value;
    using v8::Object;
    using v8::Function;
    using v8::Handle;
    using v8::Local;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 1 || !args[0]->IsFunction())
            return compatibility_return();

        auto isolate = args.GetIsolate();

        auto p = new efl::eina::js::global_ref<Value>(isolate, args[0]);

        auto cb = [](void *d, int type, void *event) -> Eina_Bool {
            auto p = reinterpret_cast<efl::eina::js::global_ref<Value>*>(d);
            auto e = reinterpret_cast<Eio_Monitor_Event*>(event);

            auto isolate = v8::Isolate::GetCurrent();

	    auto o = p->handle();
            auto wrapped_event = compatibility_new<Object>(isolate);

            wrapped_event->Set(compatibility_new<String>(isolate, "monitor"),
                               wrap_monitor(e->monitor, isolate));
            wrapped_event->Set(compatibility_new<String>(isolate, "filename"),
                               compatibility_new<String>(isolate, e->filename));

            Handle<Value> args[2]{
                compatibility_new<Integer>(isolate, type),
                wrapped_event
            };

            auto ret = Function::Cast(*o)->Call(o->ToObject(), 2, args);
            auto bret = ret->IsBoolean() && ret->BooleanValue();

            return bret ? EINA_TRUE : EINA_FALSE;
        };

        auto ret = ecore_event_handler_add(wanted_event, cb, p);
        return compatibility_return(wrap_event_handler(ret, isolate), args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

EAPI
void
register_event_monitor_file_created_handler_add(v8::Isolate *isolate,
                                                v8::Handle<v8::Object> global,
                                                v8::Handle<v8::String> name)
{
    return register_monitor_event_handler_add<EIO_MONITOR_FILE_CREATED>
        (isolate, global, name);
}

EAPI
void
register_event_monitor_file_deleted_handler_add(v8::Isolate *isolate,
                                                v8::Handle<v8::Object> global,
                                                v8::Handle<v8::String> name)
{
    return register_monitor_event_handler_add<EIO_MONITOR_FILE_DELETED>
        (isolate, global, name);
}

EAPI
void
register_event_monitor_file_modified_handler_add(v8::Isolate *isolate,
                                                 v8::Handle<v8::Object> global,
                                                 v8::Handle<v8::String> name)
{
    return register_monitor_event_handler_add<EIO_MONITOR_FILE_MODIFIED>
        (isolate, global, name);
}

EAPI
void
register_event_monitor_file_closed_handler_add(v8::Isolate *isolate,
                                               v8::Handle<v8::Object> global,
                                               v8::Handle<v8::String> name)
{
    return register_monitor_event_handler_add<EIO_MONITOR_FILE_CLOSED>
        (isolate, global, name);
}

EAPI
void register_event_monitor_directory_created_handler_add
(v8::Isolate *isolate, v8::Handle<v8::Object> global,
 v8::Handle<v8::String> name)
{
    return register_monitor_event_handler_add<EIO_MONITOR_DIRECTORY_CREATED>
        (isolate, global, name);
}

EAPI
void register_event_monitor_directory_deleted_handler_add
(v8::Isolate *isolate, v8::Handle<v8::Object> global,
 v8::Handle<v8::String> name)
{
    return register_monitor_event_handler_add<EIO_MONITOR_DIRECTORY_DELETED>
        (isolate, global, name);
}

EAPI
void register_event_monitor_directory_modified_handler_add
(v8::Isolate *isolate, v8::Handle<v8::Object> global,
 v8::Handle<v8::String> name)
{
    return register_monitor_event_handler_add<EIO_MONITOR_DIRECTORY_MODIFIED>
        (isolate, global, name);
}

EAPI
void register_event_monitor_directory_closed_handler_add
(v8::Isolate *isolate, v8::Handle<v8::Object> global,
 v8::Handle<v8::String> name)
{
    return register_monitor_event_handler_add<EIO_MONITOR_DIRECTORY_CLOSED>
        (isolate, global, name);
}

EAPI
void
register_event_monitor_self_rename_handler_add(v8::Isolate *isolate,
                                               v8::Handle<v8::Object> global,
                                               v8::Handle<v8::String> name)
{
    return register_monitor_event_handler_add<EIO_MONITOR_SELF_RENAME>
        (isolate, global, name);
}

EAPI
void
register_event_monitor_self_deleted_handler_add(v8::Isolate *isolate,
                                                v8::Handle<v8::Object> global,
                                                v8::Handle<v8::String> name)
{
    return register_monitor_event_handler_add<EIO_MONITOR_SELF_DELETED>
        (isolate, global, name);
}

EAPI
void
register_event_monitor_error_handler_add(v8::Isolate *isolate,
                                         v8::Handle<v8::Object> global,
                                         v8::Handle<v8::String> name)
{
    return register_monitor_event_handler_add<EIO_MONITOR_ERROR>
        (isolate, global, name);
}

EAPI
void register_file_ls(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                      v8::Handle<v8::String> name)
{
    using v8::String;
    using v8::Integer;
    using v8::FunctionTemplate;
    using v8::Handle;
    using v8::Value;
    using v8::Function;
    using std::unique_ptr;

    typedef efl::eina::js::global_ref<Value> persistent_t;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 5 || !args[0]->IsString() || !args[1]->IsFunction()
            || !args[2]->IsFunction() || !args[3]->IsFunction()
            || !args[4]->IsFunction()) {
            return compatibility_return();
        }

        auto isolate = args.GetIsolate();

        auto cb_data = unique_ptr<persistent_t[]>(new persistent_t[4]);
        cb_data[0] = persistent_t(isolate, args[1]);
        cb_data[1] = persistent_t(isolate, args[2]);
        cb_data[2] = persistent_t(isolate, args[3]);
        cb_data[3] = persistent_t(isolate, args[4]);

        auto filter_cb = [](void *data, Eio_File *handler, const char *file)
            -> Eina_Bool {
            auto persistent = reinterpret_cast<persistent_t*>(data);
            auto o = persistent->handle();

            auto isolate = v8::Isolate::GetCurrent();

            Handle<Value> args[2] = {
                wrap_eio_file(handler, isolate),
                compatibility_new<String>(isolate, file)
            };

            auto ret = Function::Cast(*o)->Call(o->ToObject(), 2, args);
            auto bret = ret->IsBoolean() && ret->BooleanValue();
            return bret ? EINA_TRUE : EINA_FALSE;
        };
        auto main_cb = [](void *data, Eio_File *handler, const char *file) {
            auto persistent = reinterpret_cast<persistent_t*>(data) + 1;
            auto o = persistent->handle();

            auto isolate = v8::Isolate::GetCurrent();

            Handle<Value> args[2] = {
                wrap_eio_file(handler, isolate),
                compatibility_new<String>(isolate, file)
            };

            Function::Cast(*o)->Call(o->ToObject(), 2, args);
        };
        auto done_cb = [](void *data, Eio_File *handler) {
            auto persistent = reinterpret_cast<persistent_t*>(data) + 2;
            auto o = persistent->handle();

            auto isolate = v8::Isolate::GetCurrent();

            Handle<Value> args = wrap_eio_file(handler, isolate);

            Function::Cast(*o)->Call(o->ToObject(), 1, &args);

            delete[] (persistent - 2);
        };
        auto error_cb = [](void *data, Eio_File *handler, int error) {
            auto persistent = reinterpret_cast<persistent_t*>(data) + 3;
            auto o = persistent->handle();

            auto isolate = v8::Isolate::GetCurrent();

            Handle<Value> args[2] = {
                wrap_eio_file(handler, isolate),
                compatibility_new<Integer>(isolate, error)
            };

            Function::Cast(*o)->Call(o->ToObject(), 2, args);

            delete[] (persistent - 3);
        };

        auto ret = eio_file_ls(*String::Utf8Value(args[0]), filter_cb, main_cb,
                               done_cb, error_cb, cb_data.release());
        return compatibility_return(wrap_eio_file(ret, isolate), args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

EAPI
void register_file_chmod(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                         v8::Handle<v8::String> name)
{
    using v8::String;
    using v8::Integer;
    using v8::FunctionTemplate;
    using v8::Handle;
    using v8::Value;
    using v8::Function;
    using std::unique_ptr;

    typedef efl::eina::js::global_ref<Value> persistent_t;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 4 || !args[0]->IsString() || !args[1]->IsNumber()
            || !args[2]->IsFunction() || !args[3]->IsFunction()) {
            return compatibility_return();
        }

        auto isolate = args.GetIsolate();

        auto cb_data = unique_ptr<persistent_t[]>(new persistent_t[2]);
        cb_data[0] = persistent_t(isolate, args[2]);
        cb_data[1] = persistent_t(isolate, args[3]);
        auto done_cb = [](void *data, Eio_File *handler) {
            auto persistent = reinterpret_cast<persistent_t*>(data);
            auto o = persistent->handle();

            auto isolate = v8::Isolate::GetCurrent();

            Handle<Value> args = wrap_eio_file(handler, isolate);

            Function::Cast(*o)->Call(o->ToObject(), 1, &args);

            delete[] persistent;
        };
        auto error_cb = [](void *data, Eio_File *handler, int error) {
            auto persistent = reinterpret_cast<persistent_t*>(data) + 1;
            auto o = persistent->handle();

            auto isolate = v8::Isolate::GetCurrent();

            Handle<Value> args[2] = {
                wrap_eio_file(handler, isolate),
                compatibility_new<Integer>(isolate, error)
            };

            Function::Cast(*o)->Call(o->ToObject(), 2, args);

            delete[] (persistent - 1);
        };

        auto ret = eio_file_chmod(*String::Utf8Value(args[0]),
                                  args[1]->IntegerValue(), done_cb, error_cb,
                                  cb_data.release());
        return compatibility_return(wrap_eio_file(ret, isolate), args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

EAPI
void register_file_chown(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                         v8::Handle<v8::String> name)
{
    using v8::String;
    using v8::Integer;
    using v8::FunctionTemplate;
    using v8::Handle;
    using v8::Value;
    using v8::Function;
    using std::unique_ptr;

    typedef efl::eina::js::global_ref<Value> persistent_t;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 5 || !args[0]->IsString() || !args[1]->IsString()
            || !args[2]->IsString() || !args[3]->IsFunction()
            || !args[4]->IsFunction()) {
            return compatibility_return();
        }

        auto isolate = args.GetIsolate();

        auto cb_data = unique_ptr<persistent_t[]>(new persistent_t[2]);
        cb_data[0] = persistent_t(isolate, args[3]);
        cb_data[1] = persistent_t(isolate, args[4]);
        auto done_cb = [](void *data, Eio_File *handler) {
            auto persistent = reinterpret_cast<persistent_t*>(data);
            auto o = persistent->handle();

            auto isolate = v8::Isolate::GetCurrent();

            Handle<Value> args = wrap_eio_file(handler, isolate);

            Function::Cast(*o)->Call(o->ToObject(), 1, &args);

            delete[] persistent;
        };
        auto error_cb = [](void *data, Eio_File *handler, int error) {
            auto persistent = reinterpret_cast<persistent_t*>(data) + 1;
            auto o = persistent->handle();

            auto isolate = v8::Isolate::GetCurrent();

            Handle<Value> args[2] = {
                wrap_eio_file(handler, isolate),
                compatibility_new<Integer>(isolate, error)
            };

            Function::Cast(*o)->Call(o->ToObject(), 2, args);

            delete[] (persistent - 1);
        };

        auto ret = eio_file_chown(*String::Utf8Value(args[0]),
                                  *String::Utf8Value(args[1]),
                                  *String::Utf8Value(args[2]), done_cb,
                                  error_cb,
                                  cb_data.release());
        return compatibility_return(wrap_eio_file(ret, isolate), args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

EAPI
void register_file_unlink(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                          v8::Handle<v8::String> name)
{
    using v8::String;
    using v8::Integer;
    using v8::FunctionTemplate;
    using v8::Handle;
    using v8::Value;
    using v8::Function;
    using std::unique_ptr;

    typedef efl::eina::js::global_ref<Value> persistent_t;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 3 || !args[0]->IsString() || !args[1]->IsFunction()
            || !args[2]->IsFunction()) {
            return compatibility_return();
        }

        auto isolate = args.GetIsolate();

        auto cb_data = unique_ptr<persistent_t[]>(new persistent_t[2]);
        cb_data[0] = persistent_t(isolate, args[1]);
        cb_data[1] = persistent_t(isolate, args[2]);
        auto done_cb = [](void *data, Eio_File *handler) {
            auto persistent = reinterpret_cast<persistent_t*>(data);
            auto o = persistent->handle();

            auto isolate = v8::Isolate::GetCurrent();

            Handle<Value> args = wrap_eio_file(handler, isolate);

            Function::Cast(*o)->Call(o->ToObject(), 1, &args);

            delete[] persistent;
        };
        auto error_cb = [](void *data, Eio_File *handler, int error) {
            auto persistent = reinterpret_cast<persistent_t*>(data) + 1;
            auto o = persistent->handle();

            auto isolate = v8::Isolate::GetCurrent();

            Handle<Value> args[2] = {
                wrap_eio_file(handler, isolate),
                compatibility_new<Integer>(isolate, error)
            };

            Function::Cast(*o)->Call(o->ToObject(), 2, args);

            delete[] (persistent - 1);
        };

        auto ret = eio_file_unlink(*String::Utf8Value(args[0]), done_cb,
                                   error_cb, cb_data.release());
        return compatibility_return(wrap_eio_file(ret, isolate), args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

EAPI
void register_file_mkdir(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                         v8::Handle<v8::String> name)
{
    using v8::String;
    using v8::Integer;
    using v8::FunctionTemplate;
    using v8::Handle;
    using v8::Value;
    using v8::Function;
    using std::unique_ptr;

    typedef efl::eina::js::global_ref<Value> persistent_t;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 4 || !args[0]->IsString() || !args[1]->IsNumber()
            || !args[2]->IsString() || !args[3]->IsFunction()) {
            return compatibility_return();
        }

        auto isolate = args.GetIsolate();

        auto cb_data = unique_ptr<persistent_t[]>(new persistent_t[2]);
        cb_data[0] = persistent_t(isolate, args[2]);
        cb_data[1] = persistent_t(isolate, args[3]);
        auto done_cb = [](void *data, Eio_File *handler) {
            auto persistent = reinterpret_cast<persistent_t*>(data);
            auto o = persistent->handle();

            auto isolate = v8::Isolate::GetCurrent();

            Handle<Value> args = wrap_eio_file(handler, isolate);

            Function::Cast(*o)->Call(o->ToObject(), 1, &args);

            delete[] persistent;
        };
        auto error_cb = [](void *data, Eio_File *handler, int error) {
            auto persistent = reinterpret_cast<persistent_t*>(data) + 1;
            auto o = persistent->handle();

            auto isolate = v8::Isolate::GetCurrent();

            Handle<Value> args[2] = {
                wrap_eio_file(handler, isolate),
                compatibility_new<Integer>(isolate, error)
            };

            Function::Cast(*o)->Call(o->ToObject(), 2, args);

            delete[] (persistent - 1);
        };

        auto ret = eio_file_mkdir(*String::Utf8Value(args[0]),
                                  args[1]->IntegerValue(), done_cb, error_cb,
                                  cb_data.release());
        return compatibility_return(wrap_eio_file(ret, isolate), args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

EAPI
void register_file_move(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                        v8::Handle<v8::String> name)
{
    using v8::String;
    using v8::Integer;
    using v8::Number;
    using v8::Object;
    using v8::FunctionTemplate;
    using v8::Handle;
    using v8::Value;
    using v8::Function;
    using std::unique_ptr;

    typedef efl::eina::js::global_ref<Value> persistent_t;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 5 || !args[0]->IsString() || !args[1]->IsString()
            || !args[2]->IsFunction() || !args[3]->IsFunction()
            || !args[4]->IsFunction()) {
            return compatibility_return();
        }

        auto isolate = args.GetIsolate();

        auto cb_data = unique_ptr<persistent_t[]>(new persistent_t[3]);
        cb_data[0] = persistent_t(isolate, args[2]);
        cb_data[1] = persistent_t(isolate, args[3]);
        cb_data[2] = persistent_t(isolate, args[4]);
        auto progress_cb = [](void *data, Eio_File *handler,
                              const Eio_Progress *info) {
            auto persistent = reinterpret_cast<persistent_t*>(data);
            auto o = persistent->handle();

            auto isolate = v8::Isolate::GetCurrent();

            auto wrapped_info = compatibility_new<Object>(isolate);

            wrapped_info->Set(compatibility_new<String>(isolate, "op"),
                              compatibility_new<Integer>(isolate, info->op));
            wrapped_info->Set(compatibility_new<String>(isolate, "current"),
                              compatibility_new<Integer>(isolate,
                                                         info->current));
            wrapped_info->Set(compatibility_new<String>(isolate, "max"),
                              compatibility_new<Integer>(isolate, info->max));
            wrapped_info->Set(compatibility_new<String>(isolate, "percent"),
                              compatibility_new<Number>(isolate,
                                                        info->percent));
            wrapped_info->Set(compatibility_new<String>(isolate, "source"),
                              compatibility_new<String>(isolate, info->source));
            wrapped_info->Set(compatibility_new<String>(isolate, "dest"),
                              compatibility_new<String>(isolate, info->dest));

            Handle<Value> args[2] = {
                wrap_eio_file(handler, isolate),
                wrapped_info
            };

            Function::Cast(*o)->Call(o->ToObject(), 2, args);
        };
        auto done_cb = [](void *data, Eio_File *handler) {
            auto persistent = reinterpret_cast<persistent_t*>(data) + 1;
            auto o = persistent->handle();

            auto isolate = v8::Isolate::GetCurrent();

            Handle<Value> args = wrap_eio_file(handler, isolate);

            Function::Cast(*o)->Call(o->ToObject(), 1, &args);

            delete[] (persistent - 1);
        };
        auto error_cb = [](void *data, Eio_File *handler, int error) {
            auto persistent = reinterpret_cast<persistent_t*>(data) + 2;
            auto o = persistent->handle();

            auto isolate = v8::Isolate::GetCurrent();

            Handle<Value> args[2] = {
                wrap_eio_file(handler, isolate),
                compatibility_new<Integer>(isolate, error)
            };

            Function::Cast(*o)->Call(o->ToObject(), 2, args);

            delete[] (persistent - 2);
        };

        auto ret = eio_file_move(*String::Utf8Value(args[0]),
                                 *String::Utf8Value(args[1]),
                                 progress_cb, done_cb, error_cb,
                                 cb_data.release());
        return compatibility_return(wrap_eio_file(ret, isolate), args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

EAPI
void register_file_copy(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                        v8::Handle<v8::String> name)
{
    using v8::String;
    using v8::Integer;
    using v8::Number;
    using v8::Object;
    using v8::FunctionTemplate;
    using v8::Handle;
    using v8::Value;
    using v8::Function;
    using std::unique_ptr;

    typedef efl::eina::js::global_ref<Value> persistent_t;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 5 || !args[0]->IsString() || !args[1]->IsString()
            || !args[2]->IsFunction() || !args[3]->IsFunction()
            || !args[4]->IsFunction()) {
            return compatibility_return();
        }

        auto isolate = args.GetIsolate();

        auto cb_data = unique_ptr<persistent_t[]>(new persistent_t[3]);
        cb_data[0] = persistent_t(isolate, args[2]);
        cb_data[1] = persistent_t(isolate, args[3]);
        cb_data[2] = persistent_t(isolate, args[4]);
        auto progress_cb = [](void *data, Eio_File *handler,
                              const Eio_Progress *info) {
            auto persistent = reinterpret_cast<persistent_t*>(data);
            auto o = persistent->handle();

            auto isolate = v8::Isolate::GetCurrent();

            auto wrapped_info = compatibility_new<Object>(isolate);

            wrapped_info->Set(compatibility_new<String>(isolate, "op"),
                              compatibility_new<Integer>(isolate, info->op));
            wrapped_info->Set(compatibility_new<String>(isolate, "current"),
                              compatibility_new<Integer>(isolate,
                                                         info->current));
            wrapped_info->Set(compatibility_new<String>(isolate, "max"),
                              compatibility_new<Integer>(isolate, info->max));
            wrapped_info->Set(compatibility_new<String>(isolate, "percent"),
                              compatibility_new<Number>(isolate,
                                                        info->percent));
            wrapped_info->Set(compatibility_new<String>(isolate, "source"),
                              compatibility_new<String>(isolate, info->source));
            wrapped_info->Set(compatibility_new<String>(isolate, "dest"),
                              compatibility_new<String>(isolate, info->dest));

            Handle<Value> args[2] = {
                wrap_eio_file(handler, isolate),
                wrapped_info
            };

            Function::Cast(*o)->Call(o->ToObject(), 2, args);
        };
        auto done_cb = [](void *data, Eio_File *handler) {
            auto persistent = reinterpret_cast<persistent_t*>(data) + 1;
            auto o = persistent->handle();

            auto isolate = v8::Isolate::GetCurrent();

            Handle<Value> args = wrap_eio_file(handler, isolate);

            Function::Cast(*o)->Call(o->ToObject(), 1, &args);

            delete[] (persistent - 1);
        };
        auto error_cb = [](void *data, Eio_File *handler, int error) {
            auto persistent = reinterpret_cast<persistent_t*>(data) + 2;
            auto o = persistent->handle();

            auto isolate = v8::Isolate::GetCurrent();

            Handle<Value> args[2] = {
                wrap_eio_file(handler, isolate),
                compatibility_new<Integer>(isolate, error)
            };

            Function::Cast(*o)->Call(o->ToObject(), 2, args);

            delete[] (persistent - 2);
        };

        auto ret = eio_file_copy(*String::Utf8Value(args[0]),
                                 *String::Utf8Value(args[1]),
                                 progress_cb, done_cb, error_cb,
                                 cb_data.release());
        return compatibility_return(wrap_eio_file(ret, isolate), args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

EAPI
void register_dir_move(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                       v8::Handle<v8::String> name)
{
    using v8::String;
    using v8::Integer;
    using v8::Number;
    using v8::Object;
    using v8::FunctionTemplate;
    using v8::Handle;
    using v8::Value;
    using v8::Function;
    using std::unique_ptr;

    typedef efl::eina::js::global_ref<Value> persistent_t;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 6 || !args[0]->IsString() || !args[1]->IsString()
            || !args[2]->IsFunction() || !args[3]->IsFunction()
            || !args[4]->IsFunction() || !args[5]->IsFunction()) {
            return compatibility_return();
        }

        auto isolate = args.GetIsolate();

        auto cb_data = unique_ptr<persistent_t[]>(new persistent_t[4]);
        cb_data[0] = persistent_t(isolate, args[2]);
        cb_data[1] = persistent_t(isolate, args[3]);
        cb_data[2] = persistent_t(isolate, args[4]);
        cb_data[3] = persistent_t(isolate, args[5]);
        auto filter_cb = [](void *data, Eio_File *handler,
                            const Eina_File_Direct_Info *info) -> Eina_Bool {
            auto persistent = reinterpret_cast<persistent_t*>(data);
            auto o = persistent->handle();

            auto isolate = v8::Isolate::GetCurrent();

            Handle<Value> args[2] = {
                wrap_eio_file(handler, isolate),
                wrap_eina_file_direct_info(info, isolate)
            };

            auto ret = Function::Cast(*o)->Call(o->ToObject(), 2, args);
            auto bret = ret->IsBoolean() && ret->BooleanValue();
            return bret ? EINA_TRUE : EINA_FALSE;
        };
        auto progress_cb = [](void *data, Eio_File *handler,
                              const Eio_Progress *info) {
            auto persistent = reinterpret_cast<persistent_t*>(data) + 1;
            auto o = persistent->handle();

            auto isolate = v8::Isolate::GetCurrent();

            auto wrapped_info = compatibility_new<Object>(isolate);

            wrapped_info->Set(compatibility_new<String>(isolate, "op"),
                              compatibility_new<Integer>(isolate, info->op));
            wrapped_info->Set(compatibility_new<String>(isolate, "current"),
                              compatibility_new<Integer>(isolate,
                                                         info->current));
            wrapped_info->Set(compatibility_new<String>(isolate, "max"),
                              compatibility_new<Integer>(isolate, info->max));
            wrapped_info->Set(compatibility_new<String>(isolate, "percent"),
                              compatibility_new<Number>(isolate,
                                                        info->percent));
            wrapped_info->Set(compatibility_new<String>(isolate, "source"),
                              compatibility_new<String>(isolate, info->source));
            wrapped_info->Set(compatibility_new<String>(isolate, "dest"),
                              compatibility_new<String>(isolate, info->dest));

            Handle<Value> args[2] = {
                wrap_eio_file(handler, isolate),
                wrapped_info
            };

            Function::Cast(*o)->Call(o->ToObject(), 2, args);
        };
        auto done_cb = [](void *data, Eio_File *handler) {
            auto persistent = reinterpret_cast<persistent_t*>(data) + 2;
            auto o = persistent->handle();

            auto isolate = v8::Isolate::GetCurrent();

            Handle<Value> args = wrap_eio_file(handler, isolate);

            Function::Cast(*o)->Call(o->ToObject(), 1, &args);

            delete[] (persistent - 2);
        };
        auto error_cb = [](void *data, Eio_File *handler, int error) {
            auto persistent = reinterpret_cast<persistent_t*>(data) + 3;
            auto o = persistent->handle();

            auto isolate = v8::Isolate::GetCurrent();

            Handle<Value> args[2] = {
                wrap_eio_file(handler, isolate),
                compatibility_new<Integer>(isolate, error)
            };

            Function::Cast(*o)->Call(o->ToObject(), 2, args);

            delete[] (persistent - 3);
        };

        auto ret = eio_dir_move(*String::Utf8Value(args[0]),
                                *String::Utf8Value(args[1]), filter_cb,
                                progress_cb, done_cb, error_cb,
                                cb_data.release());
        return compatibility_return(wrap_eio_file(ret, isolate), args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

EAPI
void register_dir_copy(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                       v8::Handle<v8::String> name)
{
    using v8::String;
    using v8::Integer;
    using v8::Number;
    using v8::Object;
    using v8::FunctionTemplate;
    using v8::Handle;
    using v8::Value;
    using v8::Function;
    using std::unique_ptr;

    typedef efl::eina::js::global_ref<Value> persistent_t;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 6 || !args[0]->IsString() || !args[1]->IsString()
            || !args[2]->IsFunction() || !args[3]->IsFunction()
            || !args[4]->IsFunction() || !args[5]->IsFunction()) {
            return compatibility_return();
        }

        auto isolate = args.GetIsolate();

        auto cb_data = unique_ptr<persistent_t[]>(new persistent_t[4]);
        cb_data[0] = persistent_t(isolate, args[2]);
        cb_data[1] = persistent_t(isolate, args[3]);
        cb_data[2] = persistent_t(isolate, args[4]);
        cb_data[3] = persistent_t(isolate, args[5]);
        auto filter_cb = [](void *data, Eio_File *handler,
                            const Eina_File_Direct_Info *info) -> Eina_Bool {
            auto persistent = reinterpret_cast<persistent_t*>(data);
            auto o = persistent->handle();

            auto isolate = v8::Isolate::GetCurrent();

            Handle<Value> args[2] = {
                wrap_eio_file(handler, isolate),
                wrap_eina_file_direct_info(info, isolate)
            };

            auto ret = Function::Cast(*o)->Call(o->ToObject(), 2, args);
            auto bret = ret->IsBoolean() && ret->BooleanValue();
            return bret ? EINA_TRUE : EINA_FALSE;
        };
        auto progress_cb = [](void *data, Eio_File *handler,
                              const Eio_Progress *info) {
            auto persistent = reinterpret_cast<persistent_t*>(data) + 1;
            auto o = persistent->handle();

            auto isolate = v8::Isolate::GetCurrent();

            auto wrapped_info = compatibility_new<Object>(isolate);

            wrapped_info->Set(compatibility_new<String>(isolate, "op"),
                              compatibility_new<Integer>(isolate, info->op));
            wrapped_info->Set(compatibility_new<String>(isolate, "current"),
                              compatibility_new<Integer>(isolate,
                                                         info->current));
            wrapped_info->Set(compatibility_new<String>(isolate, "max"),
                              compatibility_new<Integer>(isolate, info->max));
            wrapped_info->Set(compatibility_new<String>(isolate, "percent"),
                              compatibility_new<Number>(isolate,
                                                        info->percent));
            wrapped_info->Set(compatibility_new<String>(isolate, "source"),
                              compatibility_new<String>(isolate, info->source));
            wrapped_info->Set(compatibility_new<String>(isolate, "dest"),
                              compatibility_new<String>(isolate, info->dest));

            Handle<Value> args[2] = {
                wrap_eio_file(handler, isolate),
                wrapped_info
            };

            Function::Cast(*o)->Call(o->ToObject(), 2, args);
        };
        auto done_cb = [](void *data, Eio_File *handler) {
            auto persistent = reinterpret_cast<persistent_t*>(data) + 2;
            auto o = persistent->handle();

            auto isolate = v8::Isolate::GetCurrent();

            Handle<Value> args = wrap_eio_file(handler, isolate);

            Function::Cast(*o)->Call(o->ToObject(), 1, &args);

            delete[] (persistent - 2);
        };
        auto error_cb = [](void *data, Eio_File *handler, int error) {
            auto persistent = reinterpret_cast<persistent_t*>(data) + 3;
            auto o = persistent->handle();

            auto isolate = v8::Isolate::GetCurrent();

            Handle<Value> args[2] = {
                wrap_eio_file(handler, isolate),
                compatibility_new<Integer>(isolate, error)
            };

            Function::Cast(*o)->Call(o->ToObject(), 2, args);

            delete[] (persistent - 3);
        };

        auto ret = eio_dir_copy(*String::Utf8Value(args[0]),
                                *String::Utf8Value(args[1]), filter_cb,
                                progress_cb, done_cb, error_cb,
                                cb_data.release());
        return compatibility_return(wrap_eio_file(ret, isolate), args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

EAPI
void register_dir_unlink(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                         v8::Handle<v8::String> name)
{
    using v8::String;
    using v8::Integer;
    using v8::Number;
    using v8::Object;
    using v8::FunctionTemplate;
    using v8::Handle;
    using v8::Value;
    using v8::Function;
    using std::unique_ptr;

    typedef efl::eina::js::global_ref<Value> persistent_t;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 5 || !args[0]->IsString() || !args[1]->IsFunction()
            || !args[2]->IsFunction() || !args[3]->IsFunction()
            || !args[4]->IsFunction()) {
            return compatibility_return();
        }

        auto isolate = args.GetIsolate();

        auto cb_data = unique_ptr<persistent_t[]>(new persistent_t[4]);
        cb_data[0] = persistent_t(isolate, args[1]);
        cb_data[1] = persistent_t(isolate, args[2]);
        cb_data[2] = persistent_t(isolate, args[3]);
        cb_data[3] = persistent_t(isolate, args[4]);
        auto filter_cb = [](void *data, Eio_File *handler,
                            const Eina_File_Direct_Info *info) -> Eina_Bool {
            auto persistent = reinterpret_cast<persistent_t*>(data);
            auto o = persistent->handle();

            auto isolate = v8::Isolate::GetCurrent();

            Handle<Value> args[2] = {
                wrap_eio_file(handler, isolate),
                wrap_eina_file_direct_info(info, isolate)
            };

            auto ret = Function::Cast(*o)->Call(o->ToObject(), 2, args);
            auto bret = ret->IsBoolean() && ret->BooleanValue();
            return bret ? EINA_TRUE : EINA_FALSE;
        };
        auto progress_cb = [](void *data, Eio_File *handler,
                              const Eio_Progress *info) {
            auto persistent = reinterpret_cast<persistent_t*>(data) + 1;
            auto o = persistent->handle();

            auto isolate = v8::Isolate::GetCurrent();

            auto wrapped_info = compatibility_new<Object>(isolate);

            wrapped_info->Set(compatibility_new<String>(isolate, "op"),
                              compatibility_new<Integer>(isolate, info->op));
            wrapped_info->Set(compatibility_new<String>(isolate, "current"),
                              compatibility_new<Integer>(isolate,
                                                         info->current));
            wrapped_info->Set(compatibility_new<String>(isolate, "max"),
                              compatibility_new<Integer>(isolate, info->max));
            wrapped_info->Set(compatibility_new<String>(isolate, "percent"),
                              compatibility_new<Number>(isolate,
                                                        info->percent));
            wrapped_info->Set(compatibility_new<String>(isolate, "source"),
                              compatibility_new<String>(isolate, info->source));
            wrapped_info->Set(compatibility_new<String>(isolate, "dest"),
                              compatibility_new<String>(isolate, info->dest));

            Handle<Value> args[2] = {
                wrap_eio_file(handler, isolate),
                wrapped_info
            };

            Function::Cast(*o)->Call(o->ToObject(), 2, args);
        };
        auto done_cb = [](void *data, Eio_File *handler) {
            auto persistent = reinterpret_cast<persistent_t*>(data) + 2;
            auto o = persistent->handle();

            auto isolate = v8::Isolate::GetCurrent();

            Handle<Value> args = wrap_eio_file(handler, isolate);

            Function::Cast(*o)->Call(o->ToObject(), 1, &args);

            delete[] (persistent - 2);
        };
        auto error_cb = [](void *data, Eio_File *handler, int error) {
            auto persistent = reinterpret_cast<persistent_t*>(data) + 3;
            auto o = persistent->handle();

            auto isolate = v8::Isolate::GetCurrent();

            Handle<Value> args[2] = {
                wrap_eio_file(handler, isolate),
                compatibility_new<Integer>(isolate, error)
            };

            Function::Cast(*o)->Call(o->ToObject(), 2, args);

            delete[] (persistent - 3);
        };

        auto ret = eio_dir_unlink(*String::Utf8Value(args[0]), filter_cb,
                                  progress_cb, done_cb, error_cb,
                                  cb_data.release());
        return compatibility_return(wrap_eio_file(ret, isolate), args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

}

EAPI
void register_eio(v8::Isolate* isolate, v8::Handle<v8::Object> exports)
{
   using v8::String;
   auto eioNamespace = efl::eo::js::get_namespace({"Eio"}, isolate, exports);
   register_init(isolate, eioNamespace,
                 compatibility_new<String>(isolate, "init"));
   register_shutdown(isolate, eioNamespace,
                     compatibility_new<String>(isolate, "shutdown"));
   register_op_file_copy(isolate, eioNamespace,
                         compatibility_new<String>(isolate, "FILE_COPY"));
   register_op_file_move(isolate, eioNamespace,
                         compatibility_new<String>(isolate, "FILE_MOVE"));
   register_op_dir_copy(isolate, eioNamespace,
                        compatibility_new<String>(isolate, "DIR_COPY"));
   register_op_dir_move(isolate, eioNamespace,
                        compatibility_new<String>(isolate, "DIR_MOVE"));
   register_op_unlink(isolate, eioNamespace,
                      compatibility_new<String>(isolate, "UNLINK"));
   register_op_file_getpwnam(isolate, eioNamespace,
                             compatibility_new<String>(isolate,
                                                       "FILE_GETPWNAM"));
   register_op_file_getgrnam(isolate, eioNamespace,
                             compatibility_new<String>(isolate,
                                                       "FILE_GETGRNAM"));
   register_file_open(isolate, eioNamespace,
                      compatibility_new<String>(isolate, "openFile"));
   register_monitor_file_created(isolate, eioNamespace,
                                 compatibility_new<String>(isolate,
                                                           "MONITOR_FILE"
                                                           "_CREATED"));
   register_monitor_file_deleted(isolate, eioNamespace,
                                 compatibility_new<String>(isolate,
                                                           "MONITOR_FILE"
                                                           "_DELETED"));
   register_monitor_file_modified(isolate, eioNamespace,
                                  compatibility_new<String>(isolate,
                                                            "MONITOR_FILE"
                                                            "_MODIFIED"));
   register_monitor_file_closed(isolate, eioNamespace,
                                compatibility_new<String>(isolate,
                                                          "MONITOR_FILE"
                                                          "_CLOSED"));
   register_monitor_directory_created(isolate, eioNamespace,
                                      compatibility_new<String>
                                      (isolate,
                                       "MONITOR_DIRECTORY_CREATED"));
   register_monitor_directory_deleted(isolate, eioNamespace,
                                      compatibility_new<String>
                                      (isolate,
                                       "MONITOR_DIRECTORY_DELETED"));
   register_monitor_directory_modified(isolate, eioNamespace,
                                       compatibility_new<String>
                                       (isolate,
                                        "MONITOR_DIRECTORY_MODIFIED"));
   register_monitor_directory_closed(isolate, eioNamespace,
                                     compatibility_new<String>
                                     (isolate, "MONITOR_DIRECTORY_CLOSED"));
   register_monitor_self_rename(isolate, eioNamespace,
                                compatibility_new<String>
                                (isolate, "MONITOR_SELF_RENAME"));
   register_monitor_self_deleted(isolate, eioNamespace,
                                 compatibility_new<String>
                                 (isolate, "MONITOR_SELF_DELETED"));
   register_monitor_error(isolate, eioNamespace,
                          compatibility_new<String>(isolate,
                                                    "MONITOR_ERROR"));
   register_monitor_add(isolate, eioNamespace,
                        compatibility_new<String>(isolate, "addMonitor"));
   register_event_monitor_file_created_handler_add
     (isolate, eioNamespace,
      compatibility_new<String>(isolate,
                                "addEventMonitorFileCreatedHandler"));
   register_event_monitor_file_deleted_handler_add
     (isolate, eioNamespace,
      compatibility_new<String>(isolate,
                                "addEventMonitorFileDeletedHandler"));
   register_event_monitor_file_modified_handler_add
     (isolate, eioNamespace,
      compatibility_new<String>
      (isolate, "addEventMonitorFileModifiedHandler"));
   register_event_monitor_file_closed_handler_add
     (isolate, eioNamespace,
      compatibility_new<String>(isolate,
                                "addEventMonitorFileClosedHandler"));
   register_event_monitor_directory_created_handler_add
     (isolate, eioNamespace,
      compatibility_new<String>
      (isolate, "addEventMonitorDirectoryCreatedHandler"));
   register_event_monitor_directory_deleted_handler_add
     (isolate, eioNamespace,
      compatibility_new<String>
      (isolate, "addEventMonitorDirectoryDeletedHandler"));
   register_event_monitor_directory_modified_handler_add
     (isolate, eioNamespace,
      compatibility_new<String>
      (isolate, "addEventMonitorDirectoryModifiedHandler"));
   register_event_monitor_directory_closed_handler_add
     (isolate, eioNamespace,
      compatibility_new<String>
      (isolate, "addEventMonitorDirectoryClosedHandler"));
   register_event_monitor_self_rename_handler_add
     (isolate, eioNamespace,
      compatibility_new<String>(isolate,
                                "addEventMonitorSelfRenameHandler"));
   register_event_monitor_self_deleted_handler_add
     (isolate, eioNamespace,
      compatibility_new<String>(isolate,
                                "addEventMonitorSelfDeletedHandler"));
   register_event_monitor_error_handler_add
     (isolate, eioNamespace,
      compatibility_new<String>(isolate,
                                "addEventMonitorErrorHandler"));
   register_file_ls(isolate, eioNamespace,
                    compatibility_new<String>(isolate, "lsFile"));
   register_file_chmod(isolate, eioNamespace,
                       compatibility_new<String>(isolate, "chmodFile"));
   register_file_chown(isolate, eioNamespace,
                       compatibility_new<String>(isolate, "chownFile"));
   register_file_unlink(isolate, eioNamespace,
                        compatibility_new<String>(isolate, "unlinkFile"));
   register_file_mkdir(isolate, eioNamespace,
                       compatibility_new<String>(isolate, "mkdirFile"));
   register_file_move(isolate, eioNamespace,
                      compatibility_new<String>(isolate, "moveFile"));
   register_file_copy(isolate, eioNamespace,
                      compatibility_new<String>(isolate, "copyFile"));
   register_dir_move(isolate, eioNamespace,
                     compatibility_new<String>(isolate, "moveDir"));
   register_dir_copy(isolate, eioNamespace,
                     compatibility_new<String>(isolate, "copyDir"));
   register_dir_unlink(isolate, eioNamespace,
                       compatibility_new<String>(isolate, "unlinkDir"));
}
      
} } } // namespace efl { namespace eio { namespace js {
