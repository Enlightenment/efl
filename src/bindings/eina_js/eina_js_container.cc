#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <v8.h>
#include <Eina.h>
#include <cstdlib>

#include <Eo.h>

#include <eina_integer_sequence.hh>
#include <eina_tuple.hh>
#include <eina_ptrlist.hh>
#include <eina_js_list.hh>

#include <tuple>

#include <iostream>

namespace efl { namespace js {

template <typename T>
struct tag { typedef T type; };

namespace {

v8::UniquePersistent<v8::ObjectTemplate> instance_persistents[container_type_size];
v8::Handle<v8::FunctionTemplate> instance_templates[container_type_size];

v8::Local<v8::Object> concat(eina_container_base& lhs, v8::Isolate* isolate, v8::Local<v8::Value> other)
{
  if(other->IsObject())
    {
      v8::Local<v8::Object> obj = v8::Local<v8::Object>::Cast(other);
      v8::String::Utf8Value constructor_name (obj->GetConstructorName());
      std::cout << "constructor " << *constructor_name << std::endl;
      if(obj->GetConstructorName()->Equals(v8::String::NewFromUtf8(isolate, "eina_list")))
        {
          eina_container_base& rhs = *static_cast<eina_container_base*>(obj->GetAlignedPointerFromInternalField(0));
          std::type_info const& typeinfo_lhs = typeid(lhs)
            , &typeinfo_rhs = typeid(rhs);
          if(!typeinfo_lhs.before(typeinfo_rhs) && !typeinfo_rhs.before(typeinfo_lhs))
            {
              v8::Handle<v8::Value> a[] = {v8::External::New(isolate, rhs.concat(lhs))};
              v8::Local<v8::Object> result = instance_templates[lhs.get_container_type()]->GetFunction()->NewInstance(1, a);
              return result;
            }
          else
            std::cout << "not same implementation type" << std::endl;
        }
      else
        std::cout << "Not a list" << std::endl;
    }
  else
    std::cout << "Not an object" << std::endl;
  std::cout << "Some test failed" << std::endl;
  std::abort();
}

v8::Local<v8::Object> slice(eina_container_base& self, v8::Isolate* isolate, v8::Local<v8::Value> iv
                            , v8::Local<v8::Value> jv)
{
  if((iv->IsUint32() || iv->IsInt32()) && (jv->IsUint32() || jv->IsInt32()))
    {
      std::int64_t i = iv->IntegerValue(), j = jv->IntegerValue();
      v8::Handle<v8::Value> a[] = {v8::External::New(isolate, self.slice(i, j))};
      v8::Local<v8::Object> result = instance_templates[self.get_container_type()]
        ->GetFunction()->NewInstance(1, a);
      return result;
    }
  else
    std::cout << "parameters are not integral" << std::endl;
  std::cout << "Some test failed" << std::endl;
  std::abort();
}

void length(v8::Local<v8::String>, v8::PropertyCallbackInfo<v8::Value> const& info)
{
  eina_container_base* self = static_cast<eina_container_base*>(info.This()->GetAlignedPointerFromInternalField(0));
  info.GetReturnValue().Set((uint32_t)self->size());
}

void index_get(uint32_t index, v8::PropertyCallbackInfo<v8::Value>const& info)
{
  std::cout << "index_get " << index << std::endl;
  eina_container_base* self = static_cast<eina_container_base*>(info.This()->GetAlignedPointerFromInternalField(0));
  info.GetReturnValue().Set(self->get(info.GetIsolate(), index));
}

void new_eina_list(v8::FunctionCallbackInfo<v8::Value> const& args)
{
  if(args.IsConstructCall())
    {
      if(args.Length() == 0)
        {
          eina_container_base* p = new range_eina_list<int>;
          std::cerr << "called eina list constructor p = " << p << std::endl;
          args.This()->SetAlignedPointerInInternalField(0, dynamic_cast<void*>(p));
        }
      else
        {
          std::cout << "more than one parameter" << std::endl;
          if(args[0]->IsExternal())
            {
              std::cout << "Is external" << std::endl;
              eina_container_base* base = reinterpret_cast<eina_container_base*>
                (v8::External::Cast(*args[0])->Value());
              std::cout << "base " << base << std::endl;
              args.This()->SetAlignedPointerInInternalField(0, dynamic_cast<void*>(base));
            }
          else
            std::abort();
        }
    }
  else
    std::abort();
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

template <typename T, typename U>
struct is_persistent<v8::Persistent<T, U> > : std::true_type {};
      
template <std::size_t I, typename Sig>
typename std::tuple_element<I, Sig>::type
get_element(v8::Isolate* isolate
            , v8::FunctionCallbackInfo<v8::Value> const& args
            , typename std::enable_if
            <is_persistent<typename std::tuple_element<I, Sig>::type>::value>::type* = 0)
{
  std::cout << "get args " << I << std::endl;
  return typename std::tuple_element<I, Sig>::type(isolate, args[I]);
}

template <std::size_t I, typename Sig>
typename std::tuple_element<I, Sig>::type
get_element(v8::Isolate* /*isolate*/
            , v8::FunctionCallbackInfo<v8::Value> const& args
            , typename std::enable_if
            <!is_persistent<typename std::tuple_element<I, Sig>::type>::value>::type* = 0)
{
  std::cout << "get args " << I << std::endl;
  return args[I];
}
      
template <typename Sig, typename R, typename T, typename F, std::size_t... N>
R call_impl(v8::Isolate* isolate
            , v8::FunctionCallbackInfo<v8::Value> const& args
            , T* self, F* f
            , eina::index_sequence<N...>)
{
  struct print
  {
    ~print()
    {
      std::cout << "was called" << std::endl;
    }
  } print_;
  std::cout << "self " << self << " pointer " << (void*)f << std::endl;
  assert(self != 0);
  return (*f)(*self, isolate, js::get_element<N, Sig>(isolate, args)...);
}

template <typename Sig, typename T, typename F, typename R>
void call_generic_impl(v8::FunctionCallbackInfo<v8::Value> const& args, tag<R>)
{
  T* self = static_cast<T*>(args.This()->GetAlignedPointerFromInternalField(0));
  F* f = reinterpret_cast<F*>(v8::External::Cast(*args.Data())->Value());
  args.GetReturnValue().Set
    (js::call_impl<Sig, R>(args.GetIsolate(), args, self, f
                           , eina::make_index_sequence<std::tuple_size<Sig>::value>()));
}

template <typename Sig, typename T, typename F>
void call_generic_impl(v8::FunctionCallbackInfo<v8::Value> const& args, tag<void>)
{
  T* self = static_cast<T*>(args.This()->GetAlignedPointerFromInternalField(0));
  F* f = reinterpret_cast<F*>(v8::External::Cast(*args.Data())->Value());
  js::call_impl<Sig, void>(args.GetIsolate(), args, self, f
                           , eina::make_index_sequence<std::tuple_size<Sig>::value>());
}
  
template <typename Sig, typename R, typename T, typename F>
void call_generic(v8::FunctionCallbackInfo<v8::Value> const& args)
{
  efl::js::call_generic_impl<Sig, T, F>(args, tag<R>());
}

template <typename Sig, typename T, typename F, typename R>
void call_function_impl(v8::FunctionCallbackInfo<v8::Value> const& args, tag<R>)
{
  std::cout << "return type " << typeid(R).name() << std::endl;
  T* self = static_cast<T*>(args.This()->GetAlignedPointerFromInternalField(0));
  F f = reinterpret_cast<F>(v8::External::Cast(*args.Data())->Value());
  args.GetReturnValue().Set
    (js::call_impl<Sig, R>(args.GetIsolate(), args, self, f
                           , eina::make_index_sequence<std::tuple_size<Sig>::value>()));
}

template <typename Sig, typename T, typename F>
void call_function_impl(v8::FunctionCallbackInfo<v8::Value> const& args, tag<void>)
{
  T* self = static_cast<T*>(args.This()->GetAlignedPointerFromInternalField(0));
  F f = reinterpret_cast<F>(v8::External::Cast(*args.Data())->Value());
  js::call_impl<Sig, void>(args.GetIsolate(), args, self, f
                           , eina::make_index_sequence<std::tuple_size<Sig>::value>());
}

template <typename Sig, typename R, typename T, typename F>
void call_function(v8::FunctionCallbackInfo<v8::Value> const& args)
{
  std::cout << "R: " << typeid(R).name() << std::endl;
  efl::js::call_function_impl<Sig, T, F>(args, tag<R>());
}

template <typename T, typename F>
void register_(v8::Isolate* isolate, const char* name, F f, v8::Handle<v8::ObjectTemplate> template_
               , typename std::enable_if<std::is_function<typename std::remove_pointer<F>::type>::value>::type* = 0)
{
  std::cout << "registering " << name << " with pointer " << reinterpret_cast<void*>(f) << std::endl;
  template_->Set(v8::String::NewFromUtf8(isolate, name)
                 , v8::FunctionTemplate::New
                 (isolate, &efl::js::call_function
                  <typename eina::_mpl::pop_front<typename function_params<F>::type, 2u>::type
                  , typename function_result<F>::type, T, F>
                  , v8::External::New
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
  template_->Set(v8::String::NewFromUtf8(isolate, name)
                 , v8::FunctionTemplate::New
                 (isolate
                  , &efl::js::call_generic<std::tuple<Sig...>, result_type, T, F>
                  , v8::External::New
                  (isolate, new F(std::forward<F>(f)))));
}

v8::Local<v8::ObjectTemplate> register_template(v8::Isolate* isolate, v8::Handle<v8::FunctionTemplate> constructor)
{
  v8::Handle<v8::ObjectTemplate> instance_t = constructor->InstanceTemplate();
  instance_t->SetInternalFieldCount(1);

  instance_t->SetIndexedPropertyHandler(& efl::js::index_get);

  v8::Local<v8::ObjectTemplate> prototype = constructor->PrototypeTemplate();
  prototype->SetAccessor(v8::String::NewFromUtf8(isolate, "length"), &efl::js::length);

  using namespace std::placeholders;
  js::register_<js::eina_container_base>
    (isolate, "concat", &js::concat, prototype);
  js::register_<js::eina_container_base>
    (isolate, "slice", &js::slice, prototype);
  js::register_<js::eina_container_base>
    (isolate, "toString", std::bind(&js::eina_container_base::to_string, _1, _2), prototype);
  js::register_<js::eina_container_base>
    (isolate, "join", std::bind(&js::eina_container_base::to_string, _1, _2), prototype);
  js::register_<js::eina_container_base, v8::Local<v8::Value> >
    (isolate, "indexOf", std::bind(&js::eina_container_base::index_of, _1, _2, _3), prototype);
  js::register_<js::eina_container_base, v8::Local<v8::Value> >
    (isolate, "lastIndexOf", std::bind(&js::eina_container_base::last_index_of, _1, _2, _3), prototype);

  return instance_t;
}

void register_class(v8::Isolate* isolate, container_type type, const char* class_name
                    , v8::FunctionCallback callback)
{
  v8::Handle<v8::FunctionTemplate> constructor = v8::FunctionTemplate::New(isolate, callback);
  constructor->SetClassName(v8::String::NewFromUtf8(isolate, class_name));

  v8::Local<v8::ObjectTemplate> instance_t = efl::js::register_template(isolate, constructor);
  
  efl::js::instance_persistents[type] = v8::UniquePersistent<v8::ObjectTemplate> (isolate, instance_t);
  efl::js::instance_templates[type] = constructor;
}
  
} } }

EAPI void eina_container_register(v8::Handle<v8::ObjectTemplate>, v8::Isolate* isolate)
{
  efl::js::register_class(isolate, efl::js::list_container_type, "eina_list", &efl::js::new_eina_list);
}

EAPI v8::Handle<v8::FunctionTemplate> get_list_instance_template()
{
  return efl::js::instance_templates[efl::js::list_container_type];
}
