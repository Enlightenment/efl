#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Eina.hh>
#include <Eina.h>
#include <cstdlib>

#include <Eo.h>

#include <eina_integer_sequence.hh>
#include <eina_tuple.hh>
#include <eina_ptrlist.hh>

#include <Eina_Js.hh>

#include <tuple>

#include <iostream>

namespace efl { namespace eina { namespace js {

template <typename T>
struct tag { typedef T type; };

namespace {

global_ref<v8::ObjectTemplate> instance_persistents[container_type_size];
global_ref<v8::Function> instance_templates[container_type_size];

v8::Local<v8::Value> push(eina_container_base& self, v8::Isolate* isolate, v8::Local<v8::Value> value)
{
  return v8::Integer::New(isolate, self.push(isolate, value));
}

v8::Local<v8::Value> pop(eina_container_base& self, v8::Isolate* isolate, v8::Local<v8::Value>)
{
  return self.pop(isolate);
}

v8::Local<v8::Value> concat(eina_container_base& lhs, v8::Isolate* isolate, v8::Local<v8::Value> other)
{
  const char* error_message = 0;
  if(other->IsObject())
    {
      v8::Local<v8::Object> obj = v8::Local<v8::Object>::Cast(other);
      v8::String::Utf8Value constructor_name (obj->GetConstructorName());
      if(obj->GetConstructorName()->Equals(efl::eina::js::compatibility_new<v8::String>(isolate, "eina_list"))
         || obj->GetConstructorName()->Equals(efl::eina::js::compatibility_new<v8::String>(isolate, "eina_array")))
        {
          eina_container_base& rhs = *static_cast<eina_container_base*>
            (efl::eina::js::compatibility_get_pointer_internal_field(obj, 0));
          std::type_info const& typeinfo_lhs = typeid(lhs)
            , &typeinfo_rhs = typeid(rhs);
          if(!typeinfo_lhs.before(typeinfo_rhs) && !typeinfo_rhs.before(typeinfo_lhs))
            {
              auto ptr = rhs.concat(lhs);
              v8::Handle<v8::Value> a[] =
                {efl::eina::js::compatibility_new<v8::External>(isolate, ptr)};
              assert(!!*instance_templates[lhs.get_container_type()].handle());
              v8::Local<v8::Object> result =
                instance_templates[lhs.get_container_type()].handle()->NewInstance(1, a);
              if (ptr)
                efl::eina::js::make_weak(isolate, result, [ptr]{ delete ptr; });
              return result;
            }
          else
            error_message = "Containers are not of the same type.";
        }
      else
        error_message = "Object to be concatenated is not a container.";
    }
  else
    error_message = "Concatenation argument is not an container";

  eina::js::compatibility_throw
    (isolate, v8::Exception::TypeError(eina::js::compatibility_new<v8::String>(isolate, error_message)));

  return v8::Undefined(isolate);
}

v8::Local<v8::Value> slice(eina_container_base& self, v8::Isolate* isolate, v8::Local<v8::Value> begin
                            , v8::Local<v8::Value> end)
{
  std::size_t i, j;

  if(begin->IsUint32() || begin->IsInt32())
    i = begin->IntegerValue();
  else if (begin->IsUndefined())
    i = 0;
  else
    return v8::Undefined(isolate);

  if(end->IsUint32() || end->IsInt32())
    j = end->IntegerValue();
  else if (end->IsUndefined())
    j = self.size();
  else
    return v8::Undefined(isolate);

  auto ptr = self.slice(i, j);
  v8::Handle<v8::Value> a[] = {efl::eina::js::compatibility_new<v8::External>(isolate, ptr)};
  v8::Local<v8::Object> result = instance_templates[self.get_container_type()].handle()
    ->NewInstance(1, a);
  if (ptr)
    efl::eina::js::make_weak(isolate, result, [ptr]{ delete ptr; });
  return result;
}

compatibility_accessor_getter_return_type length
  (v8::Local<v8::String>, compatibility_accessor_getter_callback_info_type info)
{
  v8::Local<v8::Object> self_obj = compatibility_cast<v8::Object>(info.This());
  eina_container_base* self = static_cast<eina_container_base*>
    (compatibility_get_pointer_internal_field(self_obj, 0));
  return compatibility_return(js::get_value_from_c(self->size(), info.GetIsolate(), ""), info);
}

compatibility_indexed_property_getset_return_type index_get
  (uint32_t index, compatibility_indexed_property_callback_info_type info)
{
  v8::Local<v8::Object> self_obj = v8::Local<v8::Object>::Cast(info.This());
  eina_container_base* self = static_cast<eina_container_base*>
    (compatibility_get_pointer_internal_field(self_obj, 0));
  return compatibility_return(self->get(info.GetIsolate(), index), info);
}

compatibility_indexed_property_getset_return_type index_set
  (uint32_t index, v8::Local<v8::Value> value, compatibility_indexed_property_callback_info_type info)
{
  v8::Local<v8::Object> self_obj = v8::Local<v8::Object>::Cast(info.This());
  eina_container_base* self = static_cast<eina_container_base*>
    (compatibility_get_pointer_internal_field(self_obj, 0));

  return compatibility_return(self->set(info.GetIsolate(), index, value), info);
}


#define GENERATE_CONTAINER_CONSTRUCT_TYPE_IF(x) GENERATE_CONTAINER_CONSTRUCT_TYPE2_IF(x, x)
#define GENERATE_CONTAINER_CONSTRUCT_TYPE2_IF(x, y) if (!strcmp(class_name, #x)) \
    return new Container<y, nonclass_cls_name_getter, typename container_wrapper<y>::type>();

template< template<typename, typename, typename> class Container>
eina_container_base* construct_container(const char* class_name)
{
  GENERATE_CONTAINER_CONSTRUCT_TYPE_IF(int);
  GENERATE_CONTAINER_CONSTRUCT_TYPE_IF(float);
  GENERATE_CONTAINER_CONSTRUCT_TYPE2_IF(bool, Eina_Bool);
  GENERATE_CONTAINER_CONSTRUCT_TYPE2_IF(string, char*);

  return 0;
}

compatibility_return_type new_eina_list_internal(compatibility_callback_info_type args)
{
  if(args.IsConstructCall())
    {
      if(args.Length() == 0)
        {
          eina_container_base* p = new eina_list<int>;
          compatibility_set_pointer_internal_field
            (args.This(), 0, dynamic_cast<void*>(p));
          efl::eina::js::make_weak(args.GetIsolate(), args.This(), [p]{ delete p; });
        }
      else
        {
          if(args[0]->IsExternal())
            {
              eina_container_base* base = reinterpret_cast<eina_container_base*>
                (v8::External::Cast(*args[0])->Value());
              compatibility_set_pointer_internal_field
                (args.This(), 0, dynamic_cast<void*>(base));
            }
          else
            std::abort();
        }
    }
  else
    std::abort();
  return compatibility_return();
}

compatibility_return_type new_eina_list(compatibility_callback_info_type args)
{
  if(args.IsConstructCall())
    {
      if(args.Length() == 0) // Default constructor, list of ints. Or should be list of Eo's?
        {
          eina_container_base* p = new eina_list<int>;
          compatibility_set_pointer_internal_field
            (args.This(), 0, dynamic_cast<void*>(p));
          efl::eina::js::make_weak(args.GetIsolate(), args.This(), [p]{ delete p; });
          return compatibility_return();
        }
        else if (args.Length() == 1 && args[0]->IsString())
        {
          v8::String::Utf8Value string(args[0]);
          eina_container_base* p = construct_container<efl::eina::js::eina_list>(*string);
          if (!p) {
             return eina::js::compatibility_throw
              (args.GetIsolate(), v8::Exception::TypeError
                (eina::js::compatibility_new<v8::String>(args.GetIsolate(), "Invalid type for container.")));
          }
          compatibility_set_pointer_internal_field
            (args.This(), 0, dynamic_cast<void*>(p));
          efl::eina::js::make_weak(args.GetIsolate(), args.This(), [p]{ delete p; });
          return compatibility_return();
        }
    }
  return eina::js::compatibility_throw
    (args.GetIsolate(), v8::Exception::TypeError
     (eina::js::compatibility_new<v8::String>(args.GetIsolate(), "Wrong number of arguments for constructor call")));
}

compatibility_return_type new_eina_array_internal(compatibility_callback_info_type args)
{
  if(args.IsConstructCall())
    {
      if(args.Length() == 0)
        {
          eina_container_base* p = new eina_array<int>;
          compatibility_set_pointer_internal_field
            (args.This(), 0, dynamic_cast<void*>(p));
          efl::eina::js::make_weak(args.GetIsolate(), args.This(), [p]{ delete p; });
        }
      else
        {
          if(args[0]->IsExternal())
            {
              eina_container_base* base = reinterpret_cast<eina_container_base*>
                (v8::External::Cast(*args[0])->Value());
              compatibility_set_pointer_internal_field
                (args.This(), 0, dynamic_cast<void*>(base));
            }
          else
            std::abort();
        }
    }
  else
    std::abort();
  return compatibility_return();
}

compatibility_return_type new_eina_array(compatibility_callback_info_type args)
{
  if(args.IsConstructCall())
    {
      if(args.Length() == 0)
        {
          eina_container_base* p = new eina_array<int>;
          compatibility_set_pointer_internal_field
            (args.This(), 0, dynamic_cast<void*>(p));
          efl::eina::js::make_weak(args.GetIsolate(), args.This(), [p]{ delete p; });
          return compatibility_return();
        }
        else if (args.Length() == 1 && args[0]->IsString())
        {
          v8::String::Utf8Value string(args[0]);
          eina_container_base* p = construct_container<efl::eina::js::eina_array>(*string);
          if (!p) {
             return eina::js::compatibility_throw
              (args.GetIsolate(), v8::Exception::TypeError
                (eina::js::compatibility_new<v8::String>(args.GetIsolate(), "Invalid type for container.")));
          }
          compatibility_set_pointer_internal_field
            (args.This(), 0, dynamic_cast<void*>(p));
          efl::eina::js::make_weak(args.GetIsolate(), args.This(), [p]{ delete p; });
          return compatibility_return();
        }
    }
  return eina::js::compatibility_throw
    (args.GetIsolate(), v8::Exception::TypeError
     (eina::js::compatibility_new<v8::String>(args.GetIsolate(), "Wrong number of arguments for constructor call")));
}

template <typename F>
struct function_params;

template <typename R, typename... Sig>
struct function_params<R(*)(Sig...)>
{
  typedef std::tuple<Sig...> type;
};

template <typename F>
struct function_result;

template <typename R, typename... Sig>
struct function_result<R(*)(Sig...)>
{
  typedef R type;
};

template <typename T>
struct is_persistent : std::false_type {};

template <typename...A>
struct is_persistent<v8::Persistent<A...> > : std::true_type {};

template <std::size_t I, typename Sig>
typename std::tuple_element<I, Sig>::type
get_element(v8::Isolate* isolate
            , compatibility_callback_info_type args
            , typename std::enable_if
            <is_persistent<typename std::tuple_element<I, Sig>::type>::value>::type* = 0)
{
  return typename std::tuple_element<I, Sig>::type(isolate, args[I]);
}

template <std::size_t I, typename Sig>
typename std::tuple_element<I, Sig>::type
get_element(v8::Isolate* /*isolate*/
            , compatibility_callback_info_type args
            , typename std::enable_if
            <!is_persistent<typename std::tuple_element<I, Sig>::type>::value>::type* = 0)
{
  return args[I];
}

template <typename Sig, typename R, typename T, typename F, std::size_t... N>
R call_impl(v8::Isolate* isolate
            , compatibility_callback_info_type args
            , T* self, F* f
            , eina::index_sequence<N...>)
{
  assert(self != 0);
  return (*f)(*self, isolate, js::get_element<N, Sig>(isolate, args)...);
}

template <typename Sig, typename T, typename F, typename R>
compatibility_return_type call_generic_impl(compatibility_callback_info_type args, tag<R>)
{
  T* self = static_cast<T*>
    (compatibility_get_pointer_internal_field<>(args.This(), 0));
  F* f = reinterpret_cast<F*>(v8::External::Cast(*args.Data())->Value());
  return compatibility_return
    (js::get_value_from_c
     (js::call_impl<Sig, R>(args.GetIsolate(), args, self, f
                            , eina::make_index_sequence<std::tuple_size<Sig>::value>())
      , args.GetIsolate(), "")
     , args);
}

template <typename Sig, typename T, typename F>
compatibility_return_type call_generic_impl(compatibility_callback_info_type args, tag<void>)
{
  T* self = static_cast<T*>
    (compatibility_get_pointer_internal_field(args.This(), 0));
  F* f = reinterpret_cast<F*>(v8::External::Cast(*args.Data())->Value());
  js::call_impl<Sig, void>(args.GetIsolate(), args, self, f
                           , eina::make_index_sequence<std::tuple_size<Sig>::value>());
  return compatibility_return();
}

template <typename Sig, typename R, typename T, typename F>
compatibility_return_type call_generic(compatibility_callback_info_type args)
{
  return efl::eina::js::call_generic_impl<Sig, T, F>(args, tag<R>());
}

template <typename Sig, typename T, typename F, typename R>
compatibility_return_type call_function_impl(compatibility_callback_info_type args, tag<R>)
{
  T* self = static_cast<T*>
    (compatibility_get_pointer_internal_field(args.This(), 0));
  F f = reinterpret_cast<F>(v8::External::Cast(*args.Data())->Value());
  return compatibility_return
    (/*js::get_value_from_c*/
     (js::call_impl<Sig, R>(args.GetIsolate(), args, self, f
                            , eina::make_index_sequence<std::tuple_size<Sig>::value>())
      /*, args.GetIsolate(), ""*/)
     , args);
}

template <typename Sig, typename T, typename F>
compatibility_return_type call_function_impl(compatibility_callback_info_type args, tag<void>)
{
  T* self = static_cast<T*>
    (compatibility_get_pointer_internal_field(args.This(), 0));
  F f = reinterpret_cast<F>(v8::External::Cast(*args.Data())->Value());
  js::call_impl<Sig, void>(args.GetIsolate(), args, self, f
                           , eina::make_index_sequence<std::tuple_size<Sig>::value>());
  return compatibility_return();
}

template <typename Sig, typename R, typename T, typename F>
compatibility_return_type call_function(compatibility_callback_info_type args)
{
  return efl::eina::js::call_function_impl<Sig, T, F>(args, tag<R>());
}

template <typename T, typename F>
void register_(v8::Isolate* isolate, const char* name, F f, v8::Handle<v8::ObjectTemplate> template_
               , typename std::enable_if<std::is_function<typename std::remove_pointer<F>::type>::value>::type* = 0)
{
  template_->Set(compatibility_new<v8::String>(isolate, name)
                 , compatibility_new<v8::FunctionTemplate>
                 (isolate, &efl::eina::js::call_function
                  <typename eina::_mpl::pop_front<typename function_params<F>::type, 2u>::type
                  , typename function_result<F>::type, T, F>
                  , compatibility_new<v8::External>
                  (isolate, reinterpret_cast<void*>(f))));
}

template <typename T, typename...Sig, typename F>
void register_(v8::Isolate* isolate, const char* name, F&& f, v8::Handle<v8::ObjectTemplate> template_
               , typename std::enable_if<!std::is_function<typename std::remove_pointer<F>::type>::value>::type* = 0)
{
  using result_type = decltype
                        (std::declval<F>()
                         (std::declval<T&>(), std::declval<v8::Isolate*>()
                          , std::declval<Sig>()...)
                        );
  template_->Set(compatibility_new<v8::String>(isolate, name)
                 , compatibility_new<v8::FunctionTemplate>
                 (isolate
                  , &efl::eina::js::call_generic<std::tuple<Sig...>, result_type, T, F>
                  , compatibility_new<v8::External>
                  (isolate, new F(std::forward<F>(f)))));
}

v8::Local<v8::ObjectTemplate> register_template(v8::Isolate* isolate, v8::Handle<v8::FunctionTemplate> constructor)
{
  v8::Local<v8::ObjectTemplate> instance_t = constructor->InstanceTemplate();
  instance_t->SetInternalFieldCount(1);

  instance_t->SetIndexedPropertyHandler(& efl::eina::js::index_get, & efl::eina::js::index_set);

  v8::Local<v8::ObjectTemplate> prototype = constructor->PrototypeTemplate();
  prototype->SetAccessor(compatibility_new<v8::String>(isolate, "length"), &efl::eina::js::length);

  using namespace std::placeholders;
  js::register_<js::eina_container_base>
    (isolate, "push", &js::push, prototype);
  js::register_<js::eina_container_base>
    (isolate, "pop", &js::pop, prototype);
  js::register_<js::eina_container_base>
    (isolate, "concat", &js::concat, prototype);
  js::register_<js::eina_container_base>
    (isolate, "slice", &js::slice, prototype);
  js::register_<js::eina_container_base>
    (isolate, "toString", std::bind(&js::eina_container_base::to_string, _1, _2), prototype);
  js::register_<js::eina_container_base, v8::Local<v8::Value> >
    (isolate, "join", std::bind(&js::eina_container_base::join, _1, _2, _3), prototype);
  js::register_<js::eina_container_base, v8::Local<v8::Value> >
    (isolate, "indexOf", std::bind(&js::eina_container_base::index_of, _1, _2, _3), prototype);
  js::register_<js::eina_container_base, v8::Local<v8::Value> >
    (isolate, "lastIndexOf", std::bind(&js::eina_container_base::last_index_of, _1, _2, _3), prototype);

  return instance_t;
}

void register_class(v8::Isolate* isolate, container_type type, const char* class_name
                    , compatibility_function_callback callback)
{
  v8::Handle<v8::FunctionTemplate> constructor
    = compatibility_new<v8::FunctionTemplate>(isolate, callback);
  constructor->SetClassName(compatibility_new<v8::String>(isolate, class_name));

  v8::Local<v8::ObjectTemplate> instance_t = efl::eina::js::register_template(isolate, constructor);

  efl::eina::js::instance_persistents[type] = global_ref<v8::ObjectTemplate>{isolate, instance_t};
  efl::eina::js::instance_templates[type] = global_ref<v8::Function>{isolate, constructor->GetFunction()};
}
void register_class(v8::Isolate* isolate, container_type, const char* class_name_
                    , const char* constructor_name_
                    , compatibility_function_callback callback
                    , v8::Handle<v8::Object> exports)
{
  v8::Handle<v8::FunctionTemplate> constructor
    = compatibility_new<v8::FunctionTemplate>(isolate, callback);
  auto class_name = compatibility_new<v8::String>(isolate, class_name_);
  auto constructor_name = compatibility_new<v8::String>(isolate, constructor_name_);
  constructor->SetClassName(class_name);

  v8::Local<v8::ObjectTemplate> instance_t = efl::eina::js::register_template(isolate, constructor);
  (void)instance_t;

  exports->Set(constructor_name, constructor->GetFunction());
}

}

EAPI v8::Handle<v8::Function> get_list_instance_template()
{
  return efl::eina::js::instance_templates[efl::eina::js::list_container_type].handle();
}

EAPI v8::Handle<v8::Function> get_array_instance_template()
{
  return efl::eina::js::instance_templates[efl::eina::js::array_container_type].handle();
}

} } }

EAPI void eina_container_register(v8::Handle<v8::Object> exports, v8::Isolate* isolate)
{
  efl::eina::js::register_class(isolate, efl::eina::js::list_container_type
                                , "eina_list", &efl::eina::js::new_eina_list_internal);
  efl::eina::js::register_class(isolate, efl::eina::js::list_container_type
                                , "eina_list", "List", &efl::eina::js::new_eina_list, exports);
  efl::eina::js::register_class(isolate, efl::eina::js::array_container_type
                                , "eina_array", &efl::eina::js::new_eina_array_internal);
  efl::eina::js::register_class(isolate, efl::eina::js::array_container_type
                                , "eina_array", "Array", &efl::eina::js::new_eina_array, exports);
}

