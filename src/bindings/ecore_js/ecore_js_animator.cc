#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <ecore_js_animator.hh>
#include <Ecore.h>

namespace efl { namespace ecore { namespace js {

static Ecore_Animator* extract_animator(v8::Local<v8::Object> object)
{
    auto ptr = v8::External::Cast(*object->GetInternalField(0))->Value();
    return reinterpret_cast<Ecore_Animator*>(ptr);
}

static v8::Local<v8::Object> wrap_animator(Ecore_Animator *animator,
                                           v8::Isolate *isolate)
{
    using v8::Boolean;
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

        ecore_animator_del(extract_animator(info.This()));
        return compatibility_return();
    };

    auto freeze = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 0)
            return compatibility_return();

        ecore_animator_freeze(extract_animator(info.This()));
        return compatibility_return();
    };

    auto thaw = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 0)
            return compatibility_return();

        ecore_animator_thaw(extract_animator(info.This()));
        return compatibility_return();
    };

    ret->Set(compatibility_new<String>(isolate, "del"),
             compatibility_new<FunctionTemplate>(isolate, del)->GetFunction());
    ret->Set(compatibility_new<String>(isolate, "freeze"),
             compatibility_new<FunctionTemplate>(isolate, freeze)
             ->GetFunction());
    ret->Set(compatibility_new<String>(isolate, "thaw"),
             compatibility_new<FunctionTemplate>(isolate, thaw)->GetFunction());

    ret->SetInternalField(0, compatibility_new<v8::External>(isolate,
                                                             animator));

    return ret;
}

EAPI
void register_pos_map_linear(v8::Isolate *isolate,
                             v8::Handle<v8::Object> global,
                             v8::Handle<v8::String> name)
{
    using v8::Integer;

    global->Set(name,
                compatibility_new<Integer>(isolate, ECORE_POS_MAP_LINEAR));
}

EAPI
void register_pos_map_accelerate(v8::Isolate *isolate,
                                 v8::Handle<v8::Object> global,
                                 v8::Handle<v8::String> name)
{
    using v8::Integer;

    global->Set(name,
                compatibility_new<Integer>(isolate, ECORE_POS_MAP_ACCELERATE));
}

EAPI
void register_pos_map_decelerate(v8::Isolate *isolate,
                                 v8::Handle<v8::Object> global,
                                 v8::Handle<v8::String> name)
{
    using v8::Integer;

    global->Set(name,
                compatibility_new<Integer>(isolate, ECORE_POS_MAP_DECELERATE));
}

EAPI
void register_pos_map_sinusoidal(v8::Isolate *isolate,
                                 v8::Handle<v8::Object> global,
                                 v8::Handle<v8::String> name)
{
    using v8::Integer;

    global->Set(name,
                compatibility_new<Integer>(isolate, ECORE_POS_MAP_SINUSOIDAL));
}

EAPI
void register_pos_map_accelerate_factor(v8::Isolate *isolate,
                                        v8::Handle<v8::Object> global,
                                        v8::Handle<v8::String> name)
{
    using v8::Integer;

    global->Set(name,
                compatibility_new<Integer>(isolate,
                                           ECORE_POS_MAP_ACCELERATE_FACTOR));
}

EAPI
void register_pos_map_decelerate_factor(v8::Isolate *isolate,
                                        v8::Handle<v8::Object> global,
                                        v8::Handle<v8::String> name)
{
    using v8::Integer;

    global->Set(name,
                compatibility_new<Integer>(isolate,
                                           ECORE_POS_MAP_DECELERATE_FACTOR));
}

EAPI
void register_pos_map_sinusoidal_factor(v8::Isolate *isolate,
                                        v8::Handle<v8::Object> global,
                                        v8::Handle<v8::String> name)
{
    using v8::Integer;

    global->Set(name,
                compatibility_new<Integer>(isolate,
                                           ECORE_POS_MAP_SINUSOIDAL_FACTOR));
}

EAPI
void register_pos_map_divisor_interp(v8::Isolate *isolate,
                                     v8::Handle<v8::Object> global,
                                     v8::Handle<v8::String> name)
{
    using v8::Integer;

    global->Set(name,
                compatibility_new<Integer>(isolate,
                                           ECORE_POS_MAP_DIVISOR_INTERP));
}

EAPI
void register_pos_map_bounce(v8::Isolate *isolate,
                             v8::Handle<v8::Object> global,
                             v8::Handle<v8::String> name)
{
    using v8::Integer;

    global->Set(name,
                compatibility_new<Integer>(isolate, ECORE_POS_MAP_BOUNCE));
}

EAPI
void register_pos_map_spring(v8::Isolate *isolate,
                             v8::Handle<v8::Object> global,
                             v8::Handle<v8::String> name)
{
    using v8::Integer;

    global->Set(name,
                compatibility_new<Integer>(isolate, ECORE_POS_MAP_SPRING));
}

EAPI
void register_pos_map_cubic_bezier(v8::Isolate *isolate,
                                   v8::Handle<v8::Object> global,
                                   v8::Handle<v8::String> name)
{
    using v8::Integer;

    global->Set(name,
                compatibility_new<Integer>(isolate,
                                           ECORE_POS_MAP_CUBIC_BEZIER));
}

EAPI
void register_animator_source_timer(v8::Isolate *isolate,
                                    v8::Handle<v8::Object> global,
                                    v8::Handle<v8::String> name)
{
    using v8::Integer;

    global->Set(name,
                compatibility_new<Integer>(isolate,
                                           ECORE_ANIMATOR_SOURCE_TIMER));
}

EAPI
void register_animator_source_custom(v8::Isolate *isolate,
                                     v8::Handle<v8::Object> global,
                                     v8::Handle<v8::String> name)
{
    using v8::Integer;

    global->Set(name,
                compatibility_new<Integer>(isolate,
                                           ECORE_ANIMATOR_SOURCE_CUSTOM));
}

EAPI
void register_animator_frametime_set(v8::Isolate *isolate,
                                     v8::Handle<v8::Object> global,
                                     v8::Handle<v8::String> name)
{
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 1 || !args[0]->IsNumber())
            return compatibility_return();

        ecore_animator_frametime_set(args[0]->NumberValue());
        return compatibility_return();
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

EAPI
void register_animator_frametime_get(v8::Isolate *isolate,
                                     v8::Handle<v8::Object> global,
                                     v8::Handle<v8::String> name)
{
    using v8::Number;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 0)
            return compatibility_return();

        auto isolate = args.GetIsolate();
        auto ret = ecore_animator_frametime_get();
        return compatibility_return(compatibility_new<Number>(isolate, ret),
                                    args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

EAPI
void register_animator_pos_map(v8::Isolate *isolate,
                               v8::Handle<v8::Object> global,
                               v8::Handle<v8::String> name)
{
    using v8::Number;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 4 || !args[0]->IsNumber() || !args[1]->IsNumber()
            || !args[2]->IsNumber() || !args[3]->IsNumber())
            return compatibility_return();

        Ecore_Pos_Map map;

        switch ((int)(args[1]->NumberValue())) {
        case ECORE_POS_MAP_LINEAR:
            map = ECORE_POS_MAP_LINEAR;
            break;
        case ECORE_POS_MAP_ACCELERATE:
            map = ECORE_POS_MAP_ACCELERATE;
            break;
        case ECORE_POS_MAP_DECELERATE:
            map = ECORE_POS_MAP_DECELERATE;
            break;
        case ECORE_POS_MAP_SINUSOIDAL:
            map = ECORE_POS_MAP_SINUSOIDAL;
            break;
        case ECORE_POS_MAP_ACCELERATE_FACTOR:
            map = ECORE_POS_MAP_ACCELERATE_FACTOR;
            break;
        case ECORE_POS_MAP_DECELERATE_FACTOR:
            map = ECORE_POS_MAP_DECELERATE_FACTOR;
            break;
        case ECORE_POS_MAP_SINUSOIDAL_FACTOR:
            map = ECORE_POS_MAP_SINUSOIDAL_FACTOR;
            break;
        case ECORE_POS_MAP_DIVISOR_INTERP:
            map = ECORE_POS_MAP_DIVISOR_INTERP;
            break;
        case ECORE_POS_MAP_BOUNCE:
            map = ECORE_POS_MAP_BOUNCE;
            break;
        case ECORE_POS_MAP_SPRING:
            map = ECORE_POS_MAP_SPRING;
            break;
        case ECORE_POS_MAP_CUBIC_BEZIER:
            map = ECORE_POS_MAP_CUBIC_BEZIER;
            break;
        default:
            return compatibility_return();
        }

        auto isolate = args.GetIsolate();
        auto ret = ecore_animator_pos_map(args[0]->NumberValue(), map,
                                          args[2]->NumberValue(),
                                          args[3]->NumberValue());
        return compatibility_return(compatibility_new<Number>(isolate, ret),
                                    args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

EAPI
void register_animator_pos_map_n(v8::Isolate *isolate,
                                 v8::Handle<v8::Object> global,
                                 v8::Handle<v8::String> name)
{
    using v8::Number;
    using v8::NumberObject;
    using v8::FunctionTemplate;
    using v8::Array;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 3 || !args[0]->IsNumber() || !args[1]->IsNumber()
            || !args[2]->IsArray())
            return compatibility_return();

        Ecore_Pos_Map map;

        switch ((int)(args[1]->NumberValue())) {
        case ECORE_POS_MAP_LINEAR:
            map = ECORE_POS_MAP_LINEAR;
            break;
        case ECORE_POS_MAP_ACCELERATE:
            map = ECORE_POS_MAP_ACCELERATE;
            break;
        case ECORE_POS_MAP_DECELERATE:
            map = ECORE_POS_MAP_DECELERATE;
            break;
        case ECORE_POS_MAP_SINUSOIDAL:
            map = ECORE_POS_MAP_SINUSOIDAL;
            break;
        case ECORE_POS_MAP_ACCELERATE_FACTOR:
            map = ECORE_POS_MAP_ACCELERATE_FACTOR;
            break;
        case ECORE_POS_MAP_DECELERATE_FACTOR:
            map = ECORE_POS_MAP_DECELERATE_FACTOR;
            break;
        case ECORE_POS_MAP_SINUSOIDAL_FACTOR:
            map = ECORE_POS_MAP_SINUSOIDAL_FACTOR;
            break;
        case ECORE_POS_MAP_DIVISOR_INTERP:
            map = ECORE_POS_MAP_DIVISOR_INTERP;
            break;
        case ECORE_POS_MAP_BOUNCE:
            map = ECORE_POS_MAP_BOUNCE;
            break;
        case ECORE_POS_MAP_SPRING:
            map = ECORE_POS_MAP_SPRING;
            break;
        case ECORE_POS_MAP_CUBIC_BEZIER:
            map = ECORE_POS_MAP_CUBIC_BEZIER;
            break;
        default:
            return compatibility_return();
        }

        std::vector<double> v;
        {
            auto array = Array::Cast(*args[2]);
            auto s = array->Length();
            v.reserve(s);
            for (decltype(s) i = 0;i != s;++i) {
                auto e = array->Get(i);
                if (!e->IsNumber())
                    return compatibility_return();

                v.push_back(e->NumberValue());
            }
        }

        auto isolate = args.GetIsolate();
        auto ret = ecore_animator_pos_map_n(args[0]->NumberValue(), map,
                                            v.size(), v.data());
        return compatibility_return(compatibility_new<Number>(isolate, ret),
                                    args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

EAPI
void register_animator_source_set(v8::Isolate *isolate,
                                  v8::Handle<v8::Object> global,
                                  v8::Handle<v8::String> name)
{
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 1 || !args[0]->IsNumber())
            return compatibility_return();

        Ecore_Animator_Source source;

        switch ((int)(args[0]->NumberValue())) {
        case ECORE_ANIMATOR_SOURCE_TIMER:
            source = ECORE_ANIMATOR_SOURCE_TIMER;
            break;
        case ECORE_ANIMATOR_SOURCE_CUSTOM:
            source = ECORE_ANIMATOR_SOURCE_CUSTOM;
            break;
        default:
            return compatibility_return();
        }

        ecore_animator_source_set(source);
        return compatibility_return();
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

EAPI
void register_animator_source_get(v8::Isolate *isolate,
                                  v8::Handle<v8::Object> global,
                                  v8::Handle<v8::String> name)
{
    using v8::Integer;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 0)
            return compatibility_return();

        auto isolate = args.GetIsolate();
        auto ret = ecore_animator_source_get();
        return compatibility_return(compatibility_new<Integer>(isolate, ret),
                                    args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

static
compatibility_persistent<v8::Value> animator_custom_source_tick_begin_cb_data;
static
compatibility_persistent<v8::Value> animator_custom_source_tick_end_cb_data;

EAPI void
register_animator_custom_source_tick_begin_callback_set(v8::Isolate *isolate,
                                                        v8::Handle<v8::Object>
                                                        global,
                                                        v8::Handle<v8::String>
                                                        name)
{
    using v8::FunctionCallbackInfo;
    using v8::Value;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args) {
        if (args.Length() != 1 || !args[0]->IsFunction())
            return compatibility_return();

        animator_custom_source_tick_begin_cb_data
            = compatibility_persistent<Value>(args.GetIsolate(), args[0]);
        ecore_animator_custom_source_tick_begin_callback_set([](void*) {
                using v8::Function;
                using v8::Undefined;
                using v8::Isolate;

		auto o = animator_custom_source_tick_begin_cb_data.handle();
                Function::Cast(*o)->Call(o->ToObject(), 0, NULL);
            }, NULL);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

EAPI void
register_animator_custom_source_tick_end_callback_set(v8::Isolate *isolate,
                                                      v8::Handle<v8::Object>
                                                      global,
                                                      v8::Handle<v8::String>
                                                      name)
{
    using v8::FunctionCallbackInfo;
    using v8::Value;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args) {
        if (args.Length() != 1 || !args[0]->IsFunction())
            return compatibility_return();

        animator_custom_source_tick_end_cb_data
            = compatibility_persistent<Value>(args.GetIsolate(), args[0]);
        ecore_animator_custom_source_tick_end_callback_set([](void*) {
                using v8::Function;
                using v8::Undefined;
                using v8::Isolate;

		auto o = animator_custom_source_tick_end_cb_data.handle();
                Function::Cast(*o)->Call(o->ToObject(), 0, NULL);
            }, NULL);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

EAPI void
register_animator_custom_tick(v8::Isolate *isolate,
                              v8::Handle<v8::Object> global,
                              v8::Handle<v8::String> name)
{
    using v8::FunctionCallbackInfo;
    using v8::Value;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args) {
        if (args.Length() != 0)
            return compatibility_return();

        ecore_animator_custom_tick();
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

EAPI
void register_animator_add(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                           v8::Handle<v8::String> name)
{
    using v8::Local;
    using v8::Value;
    using v8::Undefined;
    using v8::Function;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 1 || !args[0]->IsFunction())
            return compatibility_return();

        auto isolate = args.GetIsolate();
        auto f = new compatibility_persistent<Value>(isolate, args[0]);

        auto cb = [](void *data) -> Eina_Bool {
            auto persistent
                = reinterpret_cast<compatibility_persistent<Value>*>(data);
	    auto o = persistent->handle();

            auto ret = Function::Cast(*o)->Call(o->ToObject(), 0, NULL);
            auto bret = ret->IsBoolean() && ret->BooleanValue();

            if (!bret)
                delete persistent;

            return bret ? EINA_TRUE : EINA_FALSE;
        };

        auto ret = ecore_animator_add(cb, f);
        return compatibility_return(wrap_animator(ret, isolate), args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

EAPI
void register_animator_timeline_add(v8::Isolate *isolate,
                                    v8::Handle<v8::Object> global,
                                    v8::Handle<v8::String> name)
{
    using v8::Handle;
    using v8::Local;
    using v8::Value;
    using v8::Undefined;
    using v8::Function;
    using v8::FunctionTemplate;
    using v8::Number;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 2 || !args[0]->IsNumber()
            || !args[1]->IsFunction()) {
            return compatibility_return();
        }

        compatibility_persistent<Value> *f
            = new compatibility_persistent<Value>(args.GetIsolate(), args[1]);

        auto cb = [](void *data, double pos) -> Eina_Bool {
            auto persistent
                = reinterpret_cast<compatibility_persistent<Value>*>(data);
	    auto o = persistent->handle();
            auto isolate = v8::Isolate::GetCurrent();

            Handle<Value> args = compatibility_new<Number>(isolate, pos);

            auto ret = Function::Cast(*o)->Call(o->ToObject(), 1, &args);
            auto bret = ret->IsBoolean() && ret->BooleanValue();

            if (!bret)
                delete persistent;

            return bret ? EINA_TRUE : EINA_FALSE;
        };

        auto ret = ecore_animator_timeline_add(args[0]->NumberValue(), cb, f);
        return compatibility_return(wrap_animator(ret, args.GetIsolate()),
                                    args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

} } } // namespace efl { namespace js {
