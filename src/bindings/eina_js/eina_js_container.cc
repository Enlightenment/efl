#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Eina.hh>
#include EINA_STRINGIZE(V8_INCLUDE_HEADER)
#include <Eina.h>
#include <cstdlib>

#include <Eo.h>

#include <eina_integer_sequence.hh>
#include <eina_tuple.hh>
#include <eina_ptrlist.hh>
#include <eina_js_list.hh>
#include <eina_js_array.hh>
#include <eina_js_get_value_from_c.hh>

#include <tuple>

#include <iostream>

namespace efl { namespace eina { namespace js {

template <typename T>
struct tag { typedef T type; };

namespace {

compatibility_persistent<v8::ObjectTemplate> instance_persistents[container_type_size];
v8::Handle<v8::Function> instance_templates[container_type_size];

v8::Local<v8::Object> concat(eina_container_base& lhs, v8::Isolate* isolate, v8::Local<v8::Value> other)
{
  if(other->IsObject())
    {
      v8::Local<v8::Object> obj = v8::Local<v8::Object>::Cast(other);
      v8::String::Utf8Value constructor_name (obj->GetConstructorName());
      std::cout << "constructor " << *constructor_name << std::endl;
      if(obj->GetConstructorName()->Equals(efl::eina::js::compatibility_new<v8::String>(isolate, "eina_list")))
        {
          eina_container_base& rhs = *static_cast<eina_container_base*>
            (efl::eina::js::compatibility_get_pointer_internal_field(obj, 0));
          std::cout << "me " << &lhs << " other " << &rhs << std::endl;
          std::type_info const& typeinfo_lhs = typeid(lhs)
            , &typeinfo_rhs = typeid(rhs);
          if(!typeinfo_lhs.before(typeinfo_rhs) && !typeinfo_rhs.before(typeinfo_lhs))
            {
              v8::Handle<v8::Value> a[] =
                {efl::eina::js::compatibility_new<v8::External>(isolate, rhs.concat(lhs))};
              std::cerr << __func__ << ":" << __LINE__<< " " << lhs.get_container_type() << std::endl;
              v8::Local<v8::Object> result =
                instance_templates[lhs.get_container_type()]->NewInstance(1, a);
              std::cerr << __func__ << ":" << __LINE__<< std::endl;
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
      v8::Handle<v8::Value> a[] = {efl::eina::js::compatibility_new<v8::External>(isolate, self.slice(i, j))};
      v8::Local<v8::Object> result = instance_templates[self.get_container_type()]
        ->NewInstance(1, a);
      return result;
    }
  else
    std::cout << "parameters are not integral" << std::endl;
  std::cout << "Some test failed" << std::endl;
  std::abort();
}

compatibility_accessor_getter_return_type length
  (v8::Local<v8::String>, compatibility_accessor_callback_info_type info)
{
  v8::Local<v8::Object> self_obj = compatibility_cast<v8::Object>(info.This());
  eina_container_base* self = static_cast<eina_container_base*>
    (compatibility_get_pointer_internal_field(self_obj, 0));
  std::cout << "size " << self->size() << std::endl;
  return compatibility_return(js::get_value_from_c(self->size(), info.GetIsolate()), info);
}

compatibility_accessor_getter_return_type index_get
  (uint32_t index, compatibility_accessor_callback_info_type info)
{
  std::cout << "index_get " << index << std::endl;
  v8::Local<v8::Object> self_obj = v8::Local<v8::Object>::Cast(info.This());
  eina_container_base* self = static_cast<eina_container_base*>
    (compatibility_get_pointer_internal_field(self_obj, 0));
  return compatibility_return(self->get(info.GetIsolate(), index), info);
}

compatibility_return_type new_eina_list(compatibility_callback_info_type args)
{
  std::cerr << "new_eina_list" << std::endl;
  if(args.IsConstructCall())
    {
      if(args.Length() == 0)
        {
          eina_container_base* p = new range_eina_list<int>;
          std::cerr << "called eina list constructor p = " << p << std::endl;
          compatibility_set_pointer_internal_field
            (args.This(), 0, dynamic_cast<void*>(p));
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
  std::cerr << "new_eina_array" << std::endl;
  if(args.IsConstructCall())
    {
      if(args.Length() == 0)
        {
          eina_container_base* p = new eina_array<int>;
          std::cerr << "called eina array constructor p = " << p << std::endl;
          compatibility_set_pointer_internal_field
            (args.This(), 0, dynamic_cast<void*>(p));
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
  std::cout << "get args " << I << std::endl;
  return typename std::tuple_element<I, Sig>::type(isolate, args[I]);
}

template <std::size_t I, typename Sig>
typename std::tuple_element<I, Sig>::type
get_element(v8::Isolate* /*isolate*/
            , compatibility_callback_info_type args
            , typename std::enable_if
            <!is_persistent<typename std::tuple_element<I, Sig>::type>::value>::type* = 0)
{
  std::cout << "get args " << I << std::endl;
  return args[I];
}
      
template <typename Sig, typename R, typename T, typename F, std::size_t... N>
R call_impl(v8::Isolate* isolate
            , compatibility_callback_info_type args
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
  std::cout << "self " << self << " pointer " << (void*)f
            << " F " << typeid(F).name() << std::endl;
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
      , args.GetIsolate())
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
  std::cout << "return type " << typeid(R).name() << std::endl;
  T* self = static_cast<T*>
    (compatibility_get_pointer_internal_field(args.This(), 0));
  F f = reinterpret_cast<F>(v8::External::Cast(*args.Data())->Value());
  return compatibility_return
    (/*js::get_value_from_c*/
     (js::call_impl<Sig, R>(args.GetIsolate(), args, self, f
                            , eina::make_index_sequence<std::tuple_size<Sig>::value>())
      /*, args.GetIsolate()*/)
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
  std::cout << "R: " << typeid(R).name() << std::endl;
  return efl::eina::js::call_function_impl<Sig, T, F>(args, tag<R>());
}

template <typename T, typename F>
void register_(v8::Isolate* isolate, const char* name, F f, v8::Handle<v8::ObjectTemplate> template_
               , typename std::enable_if<std::is_function<typename std::remove_pointer<F>::type>::value>::type* = 0)
{
  std::cout << "registering " << name << " with pointer " << reinterpret_cast<void*>(f) << std::endl;
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

  instance_t->SetIndexedPropertyHandler(& efl::eina::js::index_get);

  v8::Local<v8::ObjectTemplate> prototype = constructor->PrototypeTemplate();
  prototype->SetAccessor(compatibility_new<v8::String>(isolate, "length"), &efl::eina::js::length);

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
                    , compatibility_function_callback callback)
{
  v8::Handle<v8::FunctionTemplate> constructor
    = compatibility_new<v8::FunctionTemplate>(isolate, callback);
  constructor->SetClassName(compatibility_new<v8::String>(isolate, class_name));

  v8::Local<v8::ObjectTemplate> instance_t = efl::eina::js::register_template(isolate, constructor);
  
  efl::eina::js::instance_persistents[type] = {isolate, instance_t};
  efl::eina::js::instance_templates[type] = constructor->GetFunction();
}
  
} } } }

EAPI void eina_container_register(v8::Handle<v8::Object>, v8::Isolate* isolate)
{
  efl::eina::js::register_class(isolate, efl::eina::js::list_container_type
                                , "eina_list", &efl::eina::js::new_eina_list);
  efl::eina::js::register_class(isolate, efl::eina::js::array_container_type
                                , "eina_array", &efl::eina::js::new_eina_array);
}

EAPI v8::Handle<v8::Function> get_list_instance_template()
{
  return efl::eina::js::instance_templates[efl::eina::js::list_container_type];
}

