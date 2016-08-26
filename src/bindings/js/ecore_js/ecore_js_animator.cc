#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Ecore_Js.hh>

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

void register_pos_map_linear(v8::Isolate *isolate,
                             v8::Handle<v8::Object> global,
                             v8::Handle<v8::String> name)
{
    using v8::Integer;

    global->Set(name,
                compatibility_new<Integer>(isolate, ECORE_POS_MAP_LINEAR));
}

void register_pos_map_accelerate(v8::Isolate *isolate,
                                 v8::Handle<v8::Object> global,
                                 v8::Handle<v8::String> name)
{
    using v8::Integer;

    global->Set(name,
                compatibility_new<Integer>(isolate, ECORE_POS_MAP_ACCELERATE));
}

void register_pos_map_decelerate(v8::Isolate *isolate,
                                 v8::Handle<v8::Object> global,
                                 v8::Handle<v8::String> name)
{
    using v8::Integer;

    global->Set(name,
                compatibility_new<Integer>(isolate, ECORE_POS_MAP_DECELERATE));
}

void register_pos_map_sinusoidal(v8::Isolate *isolate,
                                 v8::Handle<v8::Object> global,
                                 v8::Handle<v8::String> name)
{
    using v8::Integer;

    global->Set(name,
                compatibility_new<Integer>(isolate, ECORE_POS_MAP_SINUSOIDAL));
}

void register_pos_map_accelerate_factor(v8::Isolate *isolate,
                                        v8::Handle<v8::Object> global,
                                        v8::Handle<v8::String> name)
{
    using v8::Integer;

    global->Set(name,
                compatibility_new<Integer>(isolate,
                                           ECORE_POS_MAP_ACCELERATE_FACTOR));
}

void register_pos_map_decelerate_factor(v8::Isolate *isolate,
                                        v8::Handle<v8::Object> global,
                                        v8::Handle<v8::String> name)
{
    using v8::Integer;

    global->Set(name,
                compatibility_new<Integer>(isolate,
                                           ECORE_POS_MAP_DECELERATE_FACTOR));
}

void register_pos_map_sinusoidal_factor(v8::Isolate *isolate,
                                        v8::Handle<v8::Object> global,
                                        v8::Handle<v8::String> name)
{
    using v8::Integer;

    global->Set(name,
                compatibility_new<Integer>(isolate,
                                           ECORE_POS_MAP_SINUSOIDAL_FACTOR));
}

void register_pos_map_divisor_interp(v8::Isolate *isolate,
                                     v8::Handle<v8::Object> global,
                                     v8::Handle<v8::String> name)
{
    using v8::Integer;

    global->Set(name,
                compatibility_new<Integer>(isolate,
                                           ECORE_POS_MAP_DIVISOR_INTERP));
}

void register_pos_map_bounce(v8::Isolate *isolate,
                             v8::Handle<v8::Object> global,
                             v8::Handle<v8::String> name)
{
    using v8::Integer;

    global->Set(name,
                compatibility_new<Integer>(isolate, ECORE_POS_MAP_BOUNCE));
}

void register_pos_map_spring(v8::Isolate *isolate,
                             v8::Handle<v8::Object> global,
                             v8::Handle<v8::String> name)
{
    using v8::Integer;

    global->Set(name,
                compatibility_new<Integer>(isolate, ECORE_POS_MAP_SPRING));
}

void register_pos_map_cubic_bezier(v8::Isolate *isolate,
                                   v8::Handle<v8::Object> global,
                                   v8::Handle<v8::String> name)
{
    using v8::Integer;

    global->Set(name,
                compatibility_new<Integer>(isolate,
                                           ECORE_POS_MAP_CUBIC_BEZIER));
}

void register_animator_source_timer(v8::Isolate *isolate,
                                    v8::Handle<v8::Object> global,
                                    v8::Handle<v8::String> name)
{
    using v8::Integer;

    global->Set(name,
                compatibility_new<Integer>(isolate,
                                           ECORE_ANIMATOR_SOURCE_TIMER));
}

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
        auto f = new efl::eina::js::global_ref<Value>(isolate, args[0]);

        auto cb = [](void *data) -> Eina_Bool {
            auto persistent = static_cast<efl::eina::js::global_ref<Value>*>(data);
            auto o = persistent->handle();

            auto ret = Function::Cast(*o)->Call(o->ToObject(), 0, NULL);
            auto bret = ret->IsBoolean() && ret->BooleanValue();

            if (!bret)
            {
              persistent->dispose();
              delete persistent;
            }

            return bret ? EINA_TRUE : EINA_FALSE;
        };

        auto ret = ecore_animator_add(cb, f);
        return compatibility_return(wrap_animator(ret, isolate), args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

void register_animator_timeline_add(v8::Isolate *isolate,
                                    v8::Handle<v8::Object> global,
                                    v8::Handle<v8::String> name)
{
    using v8::Handle;
    using v8::HandleScope;
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

        auto f = new efl::eina::js::global_ref<Value>(args.GetIsolate(), args[1]);

        auto cb = [](void *data, double pos) -> Eina_Bool {
            auto persistent
              = reinterpret_cast<efl::eina::js::global_ref<Value>*>(data);
            auto o = persistent->handle();
            auto isolate = v8::Isolate::GetCurrent();
            HandleScope handle_scope(isolate);

            Handle<Value> args = compatibility_new<Number>(isolate, pos);

            auto ret = Function::Cast(*o)->Call(o->ToObject(), 1, &args);
            auto bret = ret->IsBoolean() && ret->BooleanValue();

            if (!bret)
              {
                 persistent->dispose();
                 delete persistent;
              }

            return bret ? EINA_TRUE : EINA_FALSE;
        };

        auto ret = ecore_animator_timeline_add(args[0]->NumberValue(), cb, f);
        return compatibility_return(wrap_animator(ret, args.GetIsolate()),
                                    args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

void register_ecore_animator(v8::Isolate* isolate, v8::Handle<v8::Object> exports)
{
   register_pos_map_linear(isolate, exports,
                           compatibility_new<v8::String>(isolate,
                                                     "POS_MAP_LINEAR"));
   register_pos_map_accelerate(isolate, exports,
                               compatibility_new<v8::String>(isolate,
                                                         "POS_MAP"
                                                         "_ACCELERATE"));
   register_pos_map_decelerate(isolate, exports,
                               compatibility_new<v8::String>(isolate,
                                                         "POS_MAP"
                                                         "_DECELERATE"));
   register_pos_map_sinusoidal(isolate, exports,
                               compatibility_new<v8::String>(isolate,
                                                         "POS_MAP"
                                                         "_SINUSOIDAL"));
   register_pos_map_accelerate_factor(isolate, exports,
                                      compatibility_new<v8::String>(isolate,
                                                                "POS_MAP"
                                                                "_ACCELERATE"
                                                                "_FACTOR"));
   register_pos_map_decelerate_factor(isolate, exports,
                                      compatibility_new<v8::String>(isolate,
                                                                "POS_MAP"
                                                                "_DECELERATE"
                                                                "_FACTOR"));
   register_pos_map_sinusoidal_factor(isolate, exports,
                                      compatibility_new<v8::String>(isolate,
                                                                "POS_MAP"
                                                                "_SINUSOIDAL"
                                                                "_FACTOR"));
   register_pos_map_divisor_interp(isolate, exports,
                                   compatibility_new<v8::String>(isolate,
                                                             "POS_MAP"
                                                             "_DIVISOR_INTERP"));
   register_pos_map_bounce(isolate, exports,
                           compatibility_new<v8::String>(isolate,
                                                     "POS_MAP_BOUNCE"));
   register_pos_map_spring(isolate, exports,
                           compatibility_new<v8::String>(isolate,
                                                     "POS_MAP_SPRING"));
   register_pos_map_cubic_bezier(isolate, exports,
                                 compatibility_new<v8::String>(isolate,
                                                           "POS_MAP_CUBIC"
                                                           "_BEZIER"));
   register_animator_source_timer(isolate, exports,
                                  compatibility_new<v8::String>(isolate,
                                                            "SOURCE_TIMER"));
   register_animator_frametime_set(isolate, exports,
                                   compatibility_new<v8::String>(isolate,
                                                             "setFrametime"));
   register_animator_frametime_get(isolate, exports,
                                   compatibility_new<v8::String>(isolate,
                                                             "getFrametime"));
   register_animator_pos_map(isolate, exports,
                             compatibility_new<v8::String>(isolate,
                                                       "posMap"));
   register_animator_pos_map_n(isolate, exports,
                               compatibility_new<v8::String>(isolate,
                                                         "posMapN"));
   register_animator_source_set(isolate, exports,
                                compatibility_new<v8::String>(isolate,
                                                          "setSource"));
   register_animator_source_get(isolate, exports,
                                compatibility_new<v8::String>(isolate,
                                                          "getSource"));
   register_animator_add(isolate, exports,
                         compatibility_new<v8::String>(isolate,
                                                   "add"));
   register_animator_timeline_add(isolate, exports,
                                  compatibility_new<v8::String>(isolate,
                                                            "addTimeline"));
}   
      
} } } // namespace efl { namespace js {
