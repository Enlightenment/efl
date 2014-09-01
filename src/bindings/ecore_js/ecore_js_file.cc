#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Ecore_Js.hh>

#include <memory>
#include <cstdlib>

#include <unistd.h>

namespace efl { namespace ecore { namespace js {

namespace {

Ecore_File_Monitor* extract_monitor(v8::Local<v8::Object> object)
{
    return compatibility_get_pointer_internal_field<Ecore_File_Monitor*>(object,
                                                                         0);
}

v8::Local<v8::Object> wrap_monitor(Ecore_File_Monitor *monitor,
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

        ecore_file_monitor_del(extract_monitor(info.This()));
        return compatibility_return();
    };

    auto path_get = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 0)
            return compatibility_return();

        auto ret = ecore_file_monitor_path_get(extract_monitor(info.This()));
        return compatibility_return(compatibility_new<String>(info.GetIsolate(),
                                                              ret),
                                    info);
    };

    ret->Set(compatibility_new<String>(isolate, "del"),
             compatibility_new<FunctionTemplate>(isolate, del)->GetFunction());
    ret->Set(compatibility_new<String>(isolate, "path_get"),
             compatibility_new<FunctionTemplate>(isolate, path_get)
             ->GetFunction());

    compatibility_set_pointer_internal_field(ret, 0, monitor);

    return ret;
}

static Ecore_File_Download_Job *extract_download_job(v8::Local<v8::Object> o)
{
    return compatibility_get_pointer_internal_field<Ecore_File_Download_Job*>
        (o, 0);
}

static
v8::Local<v8::Object> wrap_download_job(Ecore_File_Download_Job *download_job,
                                        v8::Isolate *isolate)
{
    using v8::String;
    using v8::ObjectTemplate;
    using v8::FunctionTemplate;

    auto obj_tpl = compatibility_new<ObjectTemplate>(isolate);
    obj_tpl->SetInternalFieldCount(1);
    auto ret = obj_tpl->NewInstance();

    auto abort = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 0)
            return compatibility_return();

        ecore_file_download_abort(extract_download_job(info.This()));
        return compatibility_return();
    };

    ret->Set(compatibility_new<String>(isolate, "abort"),
             compatibility_new<FunctionTemplate>(isolate, abort)
             ->GetFunction());

    compatibility_set_pointer_internal_field(ret, 0, download_job);

    return ret;
}

void register_file_event_none(v8::Isolate *isolate,
                              v8::Handle<v8::Object> exports,
                              v8::Handle<v8::String> name)
{
    using v8::Integer;
    exports->Set(name,
                compatibility_new<Integer>(isolate, ECORE_FILE_EVENT_NONE));
}

void register_file_event_created_file(v8::Isolate *isolate,
                                      v8::Handle<v8::Object> exports,
                                      v8::Handle<v8::String> name)
{
    using v8::Integer;
    exports->Set(name,
                compatibility_new<Integer>(isolate,
                                           ECORE_FILE_EVENT_CREATED_FILE));
}

void register_file_event_created_directory(v8::Isolate *isolate,
                                           v8::Handle<v8::Object> exports,
                                           v8::Handle<v8::String> name)
{
    using v8::Integer;
    exports->Set(name,
                compatibility_new<Integer>(isolate,
                                           ECORE_FILE_EVENT_CREATED_DIRECTORY));
}

void register_file_event_deleted_file(v8::Isolate *isolate,
                                      v8::Handle<v8::Object> exports,
                                      v8::Handle<v8::String> name)
{
    using v8::Integer;
    exports->Set(name,
                compatibility_new<Integer>(isolate,
                                           ECORE_FILE_EVENT_DELETED_FILE));
}

void register_file_event_deleted_directory(v8::Isolate *isolate,
                                           v8::Handle<v8::Object> exports,
                                           v8::Handle<v8::String> name)
{
    using v8::Integer;
    exports->Set(name,
                compatibility_new<Integer>(isolate,
                                           ECORE_FILE_EVENT_DELETED_DIRECTORY));
}

void register_file_event_deleted_self(v8::Isolate *isolate,
                                      v8::Handle<v8::Object> exports,
                                      v8::Handle<v8::String> name)
{
    using v8::Integer;
    exports->Set(name,
                compatibility_new<Integer>(isolate,
                                           ECORE_FILE_EVENT_DELETED_SELF));
}

void register_file_event_modified(v8::Isolate *isolate,
                                  v8::Handle<v8::Object> exports,
                                  v8::Handle<v8::String> name)
{
    using v8::Integer;
    exports->Set(name,
                compatibility_new<Integer>(isolate, ECORE_FILE_EVENT_MODIFIED));
}

void register_file_event_closed(v8::Isolate *isolate,
                                v8::Handle<v8::Object> exports,
                                v8::Handle<v8::String> name)
{
    using v8::Integer;
    exports->Set(name,
                compatibility_new<Integer>(isolate, ECORE_FILE_EVENT_CLOSED));
}

void register_file_progress_continue(v8::Isolate *isolate,
                                     v8::Handle<v8::Object> exports,
                                     v8::Handle<v8::String> name)
{
    using v8::Integer;
    exports->Set(name,
                compatibility_new<Integer>(isolate,
                                           ECORE_FILE_PROGRESS_CONTINUE));
}

void register_file_progress_abort(v8::Isolate *isolate,
                                  v8::Handle<v8::Object> exports,
                                  v8::Handle<v8::String> name)
{
    using v8::Integer;
    exports->Set(name,
                compatibility_new<Integer>(isolate, ECORE_FILE_PROGRESS_ABORT));
}

void register_file_init(v8::Isolate *isolate, v8::Handle<v8::Object> exports,
                        v8::Handle<v8::String> name)
{
    using v8::Integer;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 0)
            return compatibility_return();

        auto isolate = args.GetIsolate();
        auto ret = ecore_file_init();
        return compatibility_return(compatibility_new<Integer>(isolate, ret),
                                    args);
    };

    exports->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

void register_file_shutdown(v8::Isolate *isolate, v8::Handle<v8::Object> exports,
                            v8::Handle<v8::String> name)
{
    using v8::Integer;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 0)
            return compatibility_return();

        auto isolate = args.GetIsolate();
        auto ret = ecore_file_shutdown();
        return compatibility_return(compatibility_new<Integer>(isolate, ret),
                                    args);
    };

    exports->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

void register_file_mod_time(v8::Isolate *isolate, v8::Handle<v8::Object> exports,
                            v8::Handle<v8::String> name)
{
    using v8::Date;
    using v8::String;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 1 || !args[0]->IsString())
            return compatibility_return();

        auto isolate = args.GetIsolate();

        /* TODO: be less ofensive on the comment below once the documentation of
           `ecore_file_mod_time` is fixed. I'm planning to submit a patch. */
        /* `ecore_file_mod_time` returns "the time of the last data
           modification", which is one of the most useless descriptions of a
           function I ever found. The return type is `long long`, but looking at
           the implementation, I see the value is the number of seconds since
           the Epoch.

           v8's `Date` constructor takes the number of milliseconds since the
           Epoch represented as a `double`. */
        double ret = ecore_file_mod_time(*String::Utf8Value(args[0]));
        ret *= 1000;

        return compatibility_return(compatibility_new<Date>(isolate, ret),
                                    args);
    };

    exports->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

void register_file_size(v8::Isolate *isolate, v8::Handle<v8::Object> exports,
                        v8::Handle<v8::String> name)
{
    using v8::Integer;
    using v8::String;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 1 || !args[0]->IsString())
            return compatibility_return();

        auto isolate = args.GetIsolate();
        auto ret = ecore_file_size(*String::Utf8Value(args[0]));
        return compatibility_return(compatibility_new<Integer>(isolate, ret),
                                    args);
    };

    exports->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

void register_file_exists(v8::Isolate *isolate, v8::Handle<v8::Object> exports,
                          v8::Handle<v8::String> name)
{
    using v8::Boolean;
    using v8::String;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 1 || !args[0]->IsString())
            return compatibility_return();

        auto isolate = args.GetIsolate();
        auto ret = ecore_file_exists(*String::Utf8Value(args[0]));
        return compatibility_return(compatibility_new<Boolean>(isolate, ret),
                                    args);
    };

    exports->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

void register_file_is_dir(v8::Isolate *isolate, v8::Handle<v8::Object> exports,
                          v8::Handle<v8::String> name)
{
    using v8::Boolean;
    using v8::String;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 1 || !args[0]->IsString())
            return compatibility_return();

        auto isolate = args.GetIsolate();
        auto ret = ecore_file_is_dir(*String::Utf8Value(args[0]));
        return compatibility_return(compatibility_new<Boolean>(isolate, ret),
                                    args);
    };

    exports->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

void register_file_mkdir(v8::Isolate *isolate, v8::Handle<v8::Object> exports,
                         v8::Handle<v8::String> name)
{
    using v8::Boolean;
    using v8::String;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 1 || !args[0]->IsString())
            return compatibility_return();

        auto isolate = args.GetIsolate();
        auto ret = ecore_file_mkdir(*String::Utf8Value(args[0]));
        return compatibility_return(compatibility_new<Boolean>(isolate, ret),
                                    args);
    };

    exports->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

void register_file_mkdirs(v8::Isolate *isolate, v8::Handle<v8::Object> exports,
                          v8::Handle<v8::String> name)
{
    using v8::Integer;
    using v8::String;
    using v8::Array;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 1 || !args[0]->IsArray())
            return compatibility_return();

        auto isolate = args.GetIsolate();
        std::vector<std::string> dirs_data;
        std::vector<const char*> dirs;
        {
            auto array = Array::Cast(*args[0]);
            auto s = array->Length();
            dirs_data.reserve(s);
            dirs.reserve(s + 1);
            for (decltype(s) i = 0;i != s;++i) {
                auto e = array->Get(i);
                if (!e->IsString())
                    return compatibility_return();

                dirs_data.push_back(*String::Utf8Value(e));
                dirs.push_back(dirs_data.back().data());
            }
        }
        dirs.push_back(NULL);
        auto ret = ecore_file_mkdirs(dirs.data());
        return compatibility_return(compatibility_new<Integer>(isolate, ret),
                                    args);
    };

    exports->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

void register_file_mksubdirs(v8::Isolate *isolate,
                             v8::Handle<v8::Object> exports,
                             v8::Handle<v8::String> name)
{
    using v8::Integer;
    using v8::String;
    using v8::Array;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 2 || !args[0]->IsString() || !args[1]->IsArray())
            return compatibility_return();

        auto isolate = args.GetIsolate();
        std::vector<std::string> subdirs_data;
        std::vector<const char*> subdirs;
        {
            auto array = Array::Cast(*args[1]);
            auto s = array->Length();
            subdirs_data.reserve(s);
            subdirs.reserve(s + 1);
            for (decltype(s) i = 0;i != s;++i) {
                auto e = array->Get(i);
                if (!e->IsString())
                    return compatibility_return();

                subdirs_data.push_back(*String::Utf8Value(e));
                subdirs.push_back(subdirs_data.back().data());
            }
        }
        subdirs.push_back(NULL);
        auto ret = ecore_file_mksubdirs(*String::Utf8Value(args[0]),
                                        subdirs.data());
        return compatibility_return(compatibility_new<Integer>(isolate, ret),
                                    args);
    };

    exports->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

void register_file_rmdir(v8::Isolate *isolate, v8::Handle<v8::Object> exports,
                         v8::Handle<v8::String> name)
{
    using v8::Boolean;
    using v8::String;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 1 || !args[0]->IsString())
            return compatibility_return();

        auto isolate = args.GetIsolate();
        auto ret = ecore_file_rmdir(*String::Utf8Value(args[0]));
        return compatibility_return(compatibility_new<Boolean>(isolate, ret),
                                    args);
    };

    exports->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

void register_file_unlink(v8::Isolate *isolate, v8::Handle<v8::Object> exports,
                          v8::Handle<v8::String> name)
{
    using v8::Boolean;
    using v8::String;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 1 || !args[0]->IsString())
            return compatibility_return();

        auto isolate = args.GetIsolate();
        auto ret = ecore_file_unlink(*String::Utf8Value(args[0]));
        return compatibility_return(compatibility_new<Boolean>(isolate, ret),
                                    args);
    };

    exports->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

void register_file_remove(v8::Isolate *isolate, v8::Handle<v8::Object> exports,
                          v8::Handle<v8::String> name)
{
    using v8::Boolean;
    using v8::String;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 1 || !args[0]->IsString())
            return compatibility_return();

        auto isolate = args.GetIsolate();
        auto ret = ecore_file_remove(*String::Utf8Value(args[0]));
        return compatibility_return(compatibility_new<Boolean>(isolate, ret),
                                    args);
    };

    exports->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

void register_file_recursive_rm(v8::Isolate *isolate,
                                v8::Handle<v8::Object> exports,
                                v8::Handle<v8::String> name)
{
    using v8::Boolean;
    using v8::String;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 1 || !args[0]->IsString())
            return compatibility_return();

        auto isolate = args.GetIsolate();
        auto ret = ecore_file_recursive_rm(*String::Utf8Value(args[0]));
        return compatibility_return(compatibility_new<Boolean>(isolate, ret),
                                    args);
    };

    exports->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

void register_file_mkpath(v8::Isolate *isolate, v8::Handle<v8::Object> exports,
                          v8::Handle<v8::String> name)
{
    using v8::Boolean;
    using v8::String;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 1 || !args[0]->IsString())
            return compatibility_return();

        auto isolate = args.GetIsolate();
        auto ret = ecore_file_mkpath(*String::Utf8Value(args[0]));
        return compatibility_return(compatibility_new<Boolean>(isolate, ret),
                                    args);
    };

    exports->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

void register_file_mkpaths(v8::Isolate *isolate, v8::Handle<v8::Object> exports,
                           v8::Handle<v8::String> name)
{
    using v8::Integer;
    using v8::String;
    using v8::Array;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 1 || !args[0]->IsArray())
            return compatibility_return();

        auto isolate = args.GetIsolate();
        std::vector<std::string> paths_data;
        std::vector<const char*> paths;
        {
            auto array = Array::Cast(*args[0]);
            auto s = array->Length();
            paths_data.reserve(s);
            paths.reserve(s + 1);
            for (decltype(s) i = 0;i != s;++i) {
                auto e = array->Get(i);
                if (!e->IsString())
                    return compatibility_return();

                paths_data.push_back(*String::Utf8Value(e));
                paths.push_back(paths_data.back().data());
            }
        }
        paths.push_back(NULL);
        auto ret = ecore_file_mkpaths(paths.data());
        return compatibility_return(compatibility_new<Integer>(isolate, ret),
                                    args);
    };

    exports->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

void register_file_cp(v8::Isolate *isolate, v8::Handle<v8::Object> exports,
                      v8::Handle<v8::String> name)
{
    using v8::Boolean;
    using v8::String;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 2 || !args[0]->IsString() || !args[1]->IsString())
            return compatibility_return();

        auto isolate = args.GetIsolate();
        auto ret = ecore_file_cp(*String::Utf8Value(args[0]),
                                 *String::Utf8Value(args[1]));
        return compatibility_return(compatibility_new<Boolean>(isolate, ret),
                                    args);
    };

    exports->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

void register_file_mv(v8::Isolate *isolate, v8::Handle<v8::Object> exports,
                      v8::Handle<v8::String> name)
{
    using v8::Boolean;
    using v8::String;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 2 || !args[0]->IsString() || !args[1]->IsString())
            return compatibility_return();

        auto isolate = args.GetIsolate();
        auto ret = ecore_file_mv(*String::Utf8Value(args[0]),
                                 *String::Utf8Value(args[1]));
        return compatibility_return(compatibility_new<Boolean>(isolate, ret),
                                    args);
    };

    exports->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

void register_file_symlink(v8::Isolate *isolate, v8::Handle<v8::Object> exports,
                           v8::Handle<v8::String> name)
{
    using v8::Boolean;
    using v8::String;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 2 || !args[0]->IsString() || !args[1]->IsString())
            return compatibility_return();

        auto isolate = args.GetIsolate();
        auto ret = ecore_file_symlink(*String::Utf8Value(args[0]),
                                      *String::Utf8Value(args[1]));
        return compatibility_return(compatibility_new<Boolean>(isolate, ret),
                                    args);
    };

    exports->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

void register_file_realpath(v8::Isolate *isolate, v8::Handle<v8::Object> exports,
                            v8::Handle<v8::String> name)
{
    using v8::String;
    using v8::FunctionTemplate;
    using std::unique_ptr;
    using std::free;

    typedef unique_ptr<char, void(*)(char*)> guard_t;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 1 || !args[0]->IsString())
            return compatibility_return();

        auto isolate = args.GetIsolate();
        auto rp = guard_t(ecore_file_realpath(*String::Utf8Value(args[0])),
                          [](char *str) { free(str); });
        auto ret = compatibility_new<String>(isolate, rp ? rp.get() : "");
        return compatibility_return(ret, args);
    };

    exports->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

void register_file_file_get(v8::Isolate *isolate, v8::Handle<v8::Object> exports,
                            v8::Handle<v8::String> name)
{
    using v8::String;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 1 || !args[0]->IsString())
            return compatibility_return();

        auto isolate = args.GetIsolate();
        String::Utf8Value str(args[0]);
        auto ret = ecore_file_file_get(*str);
        return compatibility_return(compatibility_new<String>(isolate, ret),
                                    args);
    };

    exports->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

void register_file_dir_get(v8::Isolate *isolate, v8::Handle<v8::Object> exports,
                           v8::Handle<v8::String> name)
{
    using v8::String;
    using v8::FunctionTemplate;
    using std::unique_ptr;
    using std::free;

    typedef unique_ptr<char, void(*)(char*)> guard_t;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 1 || !args[0]->IsString())
            return compatibility_return();

        auto isolate = args.GetIsolate();
        auto d = guard_t(ecore_file_dir_get(*String::Utf8Value(args[0])),
                         [](char *str) { free(str); });
        auto ret = compatibility_new<String>(isolate, d ? d.get() : "");
        return compatibility_return(ret, args);
    };

    exports->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

void register_file_can_read(v8::Isolate *isolate, v8::Handle<v8::Object> exports,
                            v8::Handle<v8::String> name)
{
    using v8::Boolean;
    using v8::String;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 1 || !args[0]->IsString())
            return compatibility_return();

        auto isolate = args.GetIsolate();
        auto ret = ecore_file_can_read(*String::Utf8Value(args[0]));
        return compatibility_return(compatibility_new<Boolean>(isolate, ret),
                                    args);
    };

    exports->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

void register_file_can_write(v8::Isolate *isolate, v8::Handle<v8::Object> exports,
                             v8::Handle<v8::String> name)
{
    using v8::Boolean;
    using v8::String;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 1 || !args[0]->IsString())
            return compatibility_return();

        auto isolate = args.GetIsolate();
        auto ret = ecore_file_can_write(*String::Utf8Value(args[0]));
        return compatibility_return(compatibility_new<Boolean>(isolate, ret),
                                    args);
    };

    exports->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

void register_file_can_exec(v8::Isolate *isolate, v8::Handle<v8::Object> exports,
                            v8::Handle<v8::String> name)
{
    using v8::Boolean;
    using v8::String;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 1 || !args[0]->IsString())
            return compatibility_return();

        auto isolate = args.GetIsolate();
        auto ret = ecore_file_can_exec(*String::Utf8Value(args[0]));
        return compatibility_return(compatibility_new<Boolean>(isolate, ret),
                                    args);
    };

    exports->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

void register_file_readlink(v8::Isolate *isolate, v8::Handle<v8::Object> exports,
                            v8::Handle<v8::String> name)
{
    using v8::String;
    using v8::FunctionTemplate;
    using std::unique_ptr;
    using std::free;

    typedef unique_ptr<char, void(*)(char*)> guard_t;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 1 || !args[0]->IsString())
            return compatibility_return();

        auto isolate = args.GetIsolate();
        auto l = guard_t(ecore_file_readlink(*String::Utf8Value(args[0])),
                         [](char *str) { free(str); });
        auto ret = compatibility_new<String>(isolate, l ? l.get() : "");
        return compatibility_return(ret, args);
    };

    exports->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

void register_file_ls(v8::Isolate *isolate, v8::Handle<v8::Object> exports,
                      v8::Handle<v8::String> name)
{
    using v8::Array;
    using v8::String;
    using v8::FunctionTemplate;
    using std::unique_ptr;
    using std::free;

    typedef unique_ptr<Eina_List, Eina_List*(*)(Eina_List*)> guard_t;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 1 || !args[0]->IsString())
            return compatibility_return();

        auto isolate = args.GetIsolate();
        auto list = guard_t(ecore_file_ls(*String::Utf8Value(args[0])),
                            eina_list_free);
        auto ret = compatibility_new<Array>(isolate);
        {
            uint32_t idx = 0;
            for (Eina_List *l = list.get() ; l ; l = eina_list_next(l)) {
                /* Not using idiomatic RAII here because it'd be a fake safety,
                   given that remaining objects would leak if an exception is
                   throw. It shouldn't be a problem because v8 doesn't use
                   exceptions (nor idiomatic C++). */
                auto data = reinterpret_cast<char*>(eina_list_data_get(l));
                ret->Set(idx++, compatibility_new<String>(isolate, data));
                free(data);
            }
        }
        return compatibility_return(ret, args);
    };

    exports->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

void register_file_app_exe_get(v8::Isolate *isolate,
                               v8::Handle<v8::Object> exports,
                               v8::Handle<v8::String> name)
{
    using v8::String;
    using v8::FunctionTemplate;
    using std::unique_ptr;
    using std::free;

    typedef unique_ptr<char, void(*)(char*)> guard_t;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 1 || !args[0]->IsString())
            return compatibility_return();

        auto isolate = args.GetIsolate();
        auto e = guard_t(ecore_file_app_exe_get(*String::Utf8Value(args[0])),
                         [](char *str) { free(str); });
        auto ret = compatibility_new<String>(isolate, e ? e.get() : "");
        return compatibility_return(ret, args);
    };

    exports->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

void register_file_escape_name(v8::Isolate *isolate,
                               v8::Handle<v8::Object> exports,
                               v8::Handle<v8::String> name)
{
    using v8::String;
    using v8::FunctionTemplate;
    using std::unique_ptr;
    using std::free;

    typedef unique_ptr<char, void(*)(char*)> guard_t;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 1 || !args[0]->IsString())
            return compatibility_return();

        auto isolate = args.GetIsolate();
        auto n = guard_t(ecore_file_escape_name(*String::Utf8Value(args[0])),
                         [](char *str) { free(str); });
        auto ret = compatibility_new<String>(isolate, n ? n.get() : "");
        return compatibility_return(ret, args);
    };

    exports->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

void register_file_strip_ext(v8::Isolate *isolate,
                             v8::Handle<v8::Object> exports,
                             v8::Handle<v8::String> name)
{
    using v8::String;
    using v8::FunctionTemplate;
    using std::unique_ptr;
    using std::free;

    typedef unique_ptr<char, void(*)(char*)> guard_t;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 1 || !args[0]->IsString())
            return compatibility_return();

        auto isolate = args.GetIsolate();
        auto n = guard_t(ecore_file_strip_ext(*String::Utf8Value(args[0])),
                         [](char *str) { free(str); });
        auto ret = compatibility_new<String>(isolate, n ? n.get() : "");
        return compatibility_return(ret, args);
    };

    exports->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

void register_file_dir_is_empty(v8::Isolate *isolate,
                                v8::Handle<v8::Object> exports,
                                v8::Handle<v8::String> name)
{
    using v8::Integer;
    using v8::String;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 1 || !args[0]->IsString())
            return compatibility_return();

        auto isolate = args.GetIsolate();
        auto ret = ecore_file_dir_is_empty(*String::Utf8Value(args[0]));
        return compatibility_return(compatibility_new<Integer>(isolate, ret),
                                    args);
    };

    exports->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

void register_file_monitor_add(v8::Isolate *isolate,
                               v8::Handle<v8::Object> exports,
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
        if (args.Length() != 2 || !args[0]->IsString()
            || !args[1]->IsFunction()) {
            return compatibility_return();
        }

        auto f = new efl::eina::js::global_ref<Value>(args.GetIsolate(), args[1]);

        auto cb = [](void *data, Ecore_File_Monitor *em, Ecore_File_Event event,
                     const char *path) {
            auto persistent = static_cast<efl::eina::js::global_ref<Value>*>(data);
            auto o = persistent->handle();

            auto isolate = v8::Isolate::GetCurrent();

            Handle<Value> args[3] = {
                wrap_monitor(em, isolate),
                compatibility_new<Integer>(isolate, event),
                compatibility_new<String>(isolate, path)
            };

            Function::Cast(*o)->Call(o->ToObject(), 3, args);

            persistent->dispose();
            delete persistent;
        };

        auto isolate = args.GetIsolate();
        auto ret = ecore_file_monitor_add(*String::Utf8Value(args[0]), cb, f);
        return compatibility_return(wrap_monitor(ret, isolate), args);
    };

    exports->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

void register_file_path_dir_exists(v8::Isolate *isolate,
                                   v8::Handle<v8::Object> exports,
                                   v8::Handle<v8::String> name)
{
    using v8::Boolean;
    using v8::String;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 1 || !args[0]->IsString())
            return compatibility_return();

        auto isolate = args.GetIsolate();
        Eina_Bool ret = ecore_file_path_dir_exists(*String::Utf8Value(args[0]));
        return compatibility_return(compatibility_new<Boolean>(isolate, ret == EINA_TRUE ? true : false),
                                    args);
    };

    exports->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

void register_file_app_installed(v8::Isolate *isolate,
                                 v8::Handle<v8::Object> exports,
                                 v8::Handle<v8::String> name)
{
    using v8::Boolean;
    using v8::String;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 1 || !args[0]->IsString())
            return compatibility_return();

        auto isolate = args.GetIsolate();
        auto ret = ecore_file_app_installed(*String::Utf8Value(args[0]));
        return compatibility_return(compatibility_new<Boolean>(isolate, ret),
                                    args);
    };

    exports->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

void register_file_app_list(v8::Isolate *isolate, v8::Handle<v8::Object> exports,
                            v8::Handle<v8::String> name)
{
    using v8::Array;
    using v8::String;
    using v8::FunctionTemplate;
    using std::unique_ptr;
    using std::free;

    typedef unique_ptr<Eina_List, Eina_List*(*)(Eina_List*)> guard_t;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 0)
            return compatibility_return();

        auto isolate = args.GetIsolate();
        auto list = guard_t(ecore_file_app_list(), eina_list_free);
        auto ret = compatibility_new<Array>(isolate);
        {
            uint32_t idx = 0;
            for (Eina_List *l = list.get() ; l ; l = eina_list_next(l)) {
                /* Not using idiomatic RAII here because it'd be a fake safety,
                   given that remaining objects would leak if an exception is
                   throw. It shouldn't be a problem because v8 doesn't use
                   exceptions (nor idiomatic C++). */
                auto data = reinterpret_cast<char*>(eina_list_data_get(l));
                ret->Set(idx++, compatibility_new<String>(isolate, data));
                free(data);
            }
        }
        return compatibility_return(ret, args);
    };

    exports->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

void register_file_download(v8::Isolate *isolate, v8::Handle<v8::Object> exports,
                            v8::Handle<v8::String> name)
{
    using v8::Boolean;
    using v8::Integer;
    using v8::String;
    using v8::Object;
    using v8::Function;
    using v8::Value;
    using v8::Handle;
    using v8::FunctionTemplate;
    using std::unique_ptr;
    using std::free;

    typedef unique_ptr<Eina_Hash, void(*)(Eina_Hash*)> guard_t;
    typedef efl::eina::js::global_ref<Value> persistent_t;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if ((args.Length() != 4 && args.Length() != 5) || !args[0]->IsString()
            || !args[1]->IsString() || !args[2]->IsFunction()
            || !args[3]->IsFunction()
            || (args.Length() == 5 && !args[4]->IsObject()))
            return compatibility_return();

        auto isolate = args.GetIsolate();

        auto cb_data = unique_ptr<persistent_t[]>(new persistent_t[2]);
        cb_data[0] = persistent_t(isolate, args[2]);
        cb_data[1] = persistent_t(isolate, args[3]);

        String::Utf8Value url(args[0]);
        String::Utf8Value dst(args[1]);
        auto completion_cb = [](void *data, const char *file, int status) {
            auto persistent = reinterpret_cast<persistent_t*>(data);
            auto o = persistent->handle();

            auto isolate = v8::Isolate::GetCurrent();

            Handle<Value> args[2] = {
                compatibility_new<String>(isolate, file),
                compatibility_new<Integer>(isolate, status)
            };

            Function::Cast(*o)->Call(o->ToObject(), 2, args);

            delete[] persistent;
        };
        auto progress_cb = [](void *data, const char *file, long int dltotal,
                              long int dlnow, long int ultotal,
                              long int ulnow) -> int {
            auto persistent = reinterpret_cast<persistent_t*>(data) + 1;
            auto o = persistent->handle();

            auto isolate = v8::Isolate::GetCurrent();

            Handle<Value> args[5] = {
                compatibility_new<String>(isolate, file),
                compatibility_new<Integer>(isolate, dltotal),
                compatibility_new<Integer>(isolate, dlnow),
                compatibility_new<Integer>(isolate, ultotal),
                compatibility_new<Integer>(isolate, ulnow)
            };

            auto ret = Function::Cast(*o)->Call(o->ToObject(), 5, args);
            auto iret = ret->IsNumber() ? int(ret->NumberValue()) : 0;
            if (iret != ECORE_FILE_PROGRESS_CONTINUE)
                delete[] (persistent - 1);

            return iret;
        };
        Ecore_File_Download_Job *job_ret = NULL;
        auto ret = compatibility_new<Object>(isolate);
        bool bret;

        if (args.Length() == 4) {
            bret = ecore_file_download(*url, *dst, completion_cb, progress_cb,
                                       cb_data.get(), &job_ret);
        } else {
            auto headers = guard_t(eina_hash_string_djb2_new(free),
                                   eina_hash_free);
            auto js_headers = Object::Cast(*args[4]);
            auto keys = js_headers->GetOwnPropertyNames();
            for (uint32_t i = 0;i != keys->Length();++i) {
                auto key = keys->CloneElementAt(i);
                if (!key->IsString())
                    return compatibility_return();

                auto value = js_headers->Get(key);
                if (!value->IsString())
                    return compatibility_return();

                eina_hash_add(headers.get(), *String::Utf8Value(key),
                              strdup(*String::Utf8Value(value)));
            }
            bret = ecore_file_download_full(*url, *dst, completion_cb,
                                            progress_cb, cb_data.get(),
                                            &job_ret, headers.get());
        }
        if (bret)
            cb_data.release();

        ret->Set(compatibility_new<String>(isolate, "ok"),
                 compatibility_new<Boolean>(isolate, bret));

        if (job_ret) {
            ret->Set(compatibility_new<String>(isolate, "job"),
                     wrap_download_job(job_ret, isolate));
        }

        return compatibility_return(ret, args);
    };

    exports->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

void register_file_download_abort_all(v8::Isolate *isolate,
                                      v8::Handle<v8::Object> exports,
                                      v8::Handle<v8::String> name)
{
    using v8::Integer;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 0)
            return compatibility_return();

        ecore_file_download_abort_all();
        return compatibility_return();
    };

    exports->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

void register_file_download_protocol_available(v8::Isolate *isolate,
                                               v8::Handle<v8::Object> exports,
                                               v8::Handle<v8::String> name)
{
    using v8::Boolean;
    using v8::String;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 1 || !args[0]->IsString())
            return compatibility_return();

        auto isolate = args.GetIsolate();
        String::Utf8Value protocol(args[0]);
        auto ret = ecore_file_download_protocol_available(*protocol);
        return compatibility_return(compatibility_new<Boolean>(isolate, ret),
                                    args);
    };

    exports->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

void register_file_mkstemp(v8::Isolate *isolate,
                           v8::Handle<v8::Object> exports,
                           v8::Handle<v8::String> name)
{
  auto f = [](compatibility_callback_info_type args)
    -> compatibility_return_type {
    if (args.Length() != 1 || !args[0]->IsString())
      return compatibility_return();

    v8::String::Utf8Value buffer(args[0]);
    char* buf = (char*)malloc(std::strlen(*buffer)+1);
    struct free_buf
    {
      free_buf(char* p) : p(p) {}
      ~free_buf() { free(p); }
      char* p;
    } free_buf_(buf);
    std::strcpy(buf, *buffer);
    int fd = mkstemp(buf);

    if(fd > 0)
      {
         close(fd);

         return compatibility_return(compatibility_new<v8::String>(nullptr, buf), args);
      }
    else
      return compatibility_return();
  };
  exports->Set(name, compatibility_new<v8::FunctionTemplate>(isolate, f)->GetFunction());
}

void register_file_environment_tmp(v8::Isolate *isolate,
                                   v8::Handle<v8::Object> exports,
                                   v8::Handle<v8::String> name)
{
  auto f = [](compatibility_callback_info_type args)
    -> compatibility_return_type {

    return compatibility_return(compatibility_new<v8::String>(nullptr, eina_environment_tmp_get()), args);
  };
  exports->Set(name, compatibility_new<v8::FunctionTemplate>(isolate, f)->GetFunction());
}

}

EAPI
void register_ecore_file(v8::Isolate* isolate, v8::Handle<v8::Object> exports)
{
   register_file_event_none(isolate, exports,
                            compatibility_new<v8::String>(isolate,
                                                      "ECORE_FILE_EVENT_NONE"));
   register_file_event_created_file(isolate, exports,
                                    compatibility_new<v8::String>(isolate,
                                                              "ECORE_FILE_EVENT"
                                                              "_CREATED_FILE"));
   register_file_event_created_directory(isolate, exports,
                                         compatibility_new<v8::String>
                                         (isolate,
                                          "ECORE_FILE_EVENT_CREATED_DIRECTORY"));
   register_file_event_deleted_file(isolate, exports,
                                    compatibility_new<v8::String>(isolate,
                                                              "ECORE_FILE_EVENT"
                                                              "_DELETED_FILE"));
   register_file_event_deleted_directory(isolate, exports,
                                         compatibility_new<v8::String>
                                         (isolate,
                                          "ECORE_FILE_EVENT_DELETED_DIRECTORY"));
   register_file_event_deleted_self(isolate, exports,
                                    compatibility_new<v8::String>(isolate,
                                                              "ECORE_FILE_EVENT"
                                                              "_DELETED_SELF"));
   register_file_event_modified(isolate, exports,
                                compatibility_new<v8::String>(isolate,
                                                          "ECORE_FILE_EVENT"
                                                          "_MODIFIED"));
   register_file_event_closed(isolate, exports,
                              compatibility_new<v8::String>(isolate,
                                                        "ECORE_FILE_EVENT"
                                                        "_CLOSED"));
   register_file_progress_continue(isolate, exports,
                                   compatibility_new<v8::String>
                                   (isolate, "ECORE_FILE_PROGRESS_CONTINUE"));
   register_file_progress_abort(isolate, exports,
                                compatibility_new<v8::String>
                                (isolate, "ECORE_FILE_PROGRESS_ABORT"));
   register_file_init(isolate, exports,
                      compatibility_new<v8::String>(isolate, "ecore_file_init"));
   register_file_shutdown(isolate, exports,
                          compatibility_new<v8::String>(isolate,
                                                    "ecore_file_shutdown"));
   register_file_mod_time(isolate, exports,
                          compatibility_new<v8::String>(isolate,
                                                    "ecore_file_mod_time"));
   register_file_size(isolate, exports,
                      compatibility_new<v8::String>(isolate, "ecore_file_size"));
   register_file_exists(isolate, exports,
                        compatibility_new<v8::String>(isolate, "ecore_file_exists"));
   register_file_is_dir(isolate, exports,
                        compatibility_new<v8::String>(isolate, "ecore_file_is_dir"));
   register_file_mkdir(isolate, exports,
                       compatibility_new<v8::String>(isolate, "ecore_file_mkdir"));
   register_file_mkdirs(isolate, exports,
                        compatibility_new<v8::String>(isolate, "ecore_file_mkdirs"));
   register_file_mksubdirs(isolate, exports,
                           compatibility_new<v8::String>(isolate,
                                                     "ecore_file_mksubdirs"));
   register_file_rmdir(isolate, exports,
                       compatibility_new<v8::String>(isolate, "ecore_file_rmdir"));
   register_file_unlink(isolate, exports,
                        compatibility_new<v8::String>(isolate, "ecore_file_unlink"));
   register_file_remove(isolate, exports,
                        compatibility_new<v8::String>(isolate, "ecore_file_remove"));
   register_file_recursive_rm(isolate, exports,
                              compatibility_new<v8::String>
                              (isolate, "ecore_file_recursive_rm"));
   register_file_mkpath(isolate, exports,
                        compatibility_new<v8::String>(isolate, "ecore_file_mkpath"));
   register_file_mkpaths(isolate, exports,
                         compatibility_new<v8::String>(isolate,
                                                   "ecore_file_mkpaths"));
   register_file_cp(isolate, exports,
                    compatibility_new<v8::String>(isolate, "ecore_file_cp"));
   register_file_mv(isolate, exports,
                    compatibility_new<v8::String>(isolate, "ecore_file_mv"));
   register_file_symlink(isolate, exports,
                         compatibility_new<v8::String>(isolate,
                                                   "ecore_file_symlink"));
   register_file_realpath(isolate, exports,
                          compatibility_new<v8::String>(isolate,
                                                    "ecore_file_realpath"));
   register_file_file_get(isolate, exports,
                          compatibility_new<v8::String>(isolate,
                                                    "ecore_file_file_get"));
   register_file_dir_get(isolate, exports,
                         compatibility_new<v8::String>(isolate,
                                                   "ecore_file_dir_get"));
   register_file_can_read(isolate, exports,
                          compatibility_new<v8::String>(isolate,
                                                    "ecore_file_can_read"));
   register_file_can_write(isolate, exports,
                           compatibility_new<v8::String>(isolate,
                                                     "ecore_file_can_write"));
   register_file_can_exec(isolate, exports,
                          compatibility_new<v8::String>(isolate,
                                                    "ecore_file_can_exec"));
   register_file_readlink(isolate, exports,
                          compatibility_new<v8::String>(isolate,
                                                    "ecore_file_readlink"));
   register_file_ls(isolate, exports,
                    compatibility_new<v8::String>(isolate, "ecore_file_ls"));
   register_file_app_exe_get(isolate, exports,
                             compatibility_new<v8::String>
                             (isolate, "ecore_file_app_exe_get"));
   register_file_escape_name(isolate, exports,
                             compatibility_new<v8::String>
                             (isolate, "ecore_file_escape_name"));
   register_file_strip_ext(isolate, exports,
                           compatibility_new<v8::String>(isolate,
                                                     "ecore_file_strip_ext"));
   register_file_dir_is_empty(isolate, exports,
                              compatibility_new<v8::String>
                              (isolate, "ecore_file_dir_is_empty"));
   register_file_monitor_add(isolate, exports,
                             compatibility_new<v8::String>
                             (isolate, "ecore_file_monitor_add"));
   register_file_path_dir_exists(isolate, exports,
                                 compatibility_new<v8::String>
                                 (isolate, "ecore_file_path_dir_exists"));
   register_file_app_installed(isolate, exports,
                               compatibility_new<v8::String>
                               (isolate, "ecore_file_app_installed"));
   register_file_app_list(isolate, exports,
                          compatibility_new<v8::String>(isolate,
                                                    "ecore_file_app_list"));
   register_file_download(isolate, exports,
                          compatibility_new<v8::String>(isolate,
                                                    "ecore_file_download"));
   register_file_download_abort_all(isolate, exports,
                                    compatibility_new<v8::String>
                                    (isolate, "ecore_file_download_abort_all"));
   register_file_download_protocol_available(isolate, exports,
                                             compatibility_new<v8::String>
                                             (isolate,
                                              "ecore_file_download_protocol"
                                              "_available"));
   register_file_mkstemp(isolate, exports,
                         compatibility_new<v8::String>(isolate, "mkstemp"));
   register_file_environment_tmp(isolate, exports,
                                 compatibility_new<v8::String>(isolate, "environment_tmp"));
}
      
} } } // namespace efl { namespace ecore { namespace js {
