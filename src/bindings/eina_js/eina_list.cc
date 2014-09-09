
#include <v8.h>
#include <Eina.h>
#include <cstdlib>

#include <eina_integer_sequence.hh>
#include <eina_tuple.hh>

#include <tuple>

#include <iostream>

namespace efl { namespace js { namespace {

struct eina_list
{
  eina_list() : _list(0) {}
  
  Eina_List* _list;
};

void push_back(eina_list& list, v8::Persistent<v8::Value, v8::CopyablePersistentTraits<v8::Value> > object)
{
  std::cout << "eina_list push_back" << std::endl;
}

void index_get(uint32_t index, v8::PropertyCallbackInfo<v8::Value>const& info)
{
  std::cout << "index_get " << index << std::endl;
  if(
  info.GetReturnValue().Set(5);
}
      
void new_eina_list(v8::FunctionCallbackInfo<v8::Value> const& args)
{
  void* p = new eina_list;
  std::cerr << "called eina list constructor p = " << p << std::endl;
  args.This()->SetAlignedPointerInInternalField(0, p);
}

template <typename F>
struct function_params;

template <typename R, typename... Sig>
struct function_params<R(*)(Sig...)>
{
  typedef std::tuple<Sig...> type;
};

// template <std::size_t I, typename Sig>
// typename std::tuple_element<I, Sig>::type
// get_element(v8::FunctionCallbackInfo<v8::Value> const& args)
// {
//   std::cout << "get args " << I << std::endl;
//   return args[I];
// }

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
      
template <typename Sig, typename R, typename T, typename F, std::size_t... N>
R call_impl(v8::Isolate* isolate
            , v8::FunctionCallbackInfo<v8::Value> const& args
            , T* self, F* f
            , eina::index_sequence<N...>)

{
  std::cout << "self " << self << std::endl;
  (*f)(*self, js::get_element<N, Sig>(isolate, args)...);
}

template <typename Sig, typename R, typename T, typename F>
R call_generic(v8::FunctionCallbackInfo<v8::Value> const& args)
{
  T* self = static_cast<T*>(args.This()->GetAlignedPointerFromInternalField(0));
  F* f = reinterpret_cast<F*>(v8::External::Cast(*args.Data())->Value());
  js::call_impl<Sig, R>(args.GetIsolate(), args, self, f
                        , eina::make_index_sequence<std::tuple_size<Sig>::value>());
}

template <typename Sig, typename R, typename T, typename F>
R call_function(v8::FunctionCallbackInfo<v8::Value> const& args)
{
  T* self = static_cast<T*>(args.This()->GetAlignedPointerFromInternalField(0));
  F f = reinterpret_cast<F>(v8::External::Cast(*args.Data())->Value());
  js::call_impl<Sig, R>(args.GetIsolate(), args, self, f
                        , eina::make_index_sequence<std::tuple_size<Sig>::value>());
}

template <typename T, typename F>
void register_(v8::Isolate* isolate, const char* name, F f, v8::Handle<v8::ObjectTemplate> template_
               , typename std::enable_if<std::is_function<typename std::remove_pointer<F>::type>::value>::type* = 0)
{
  template_->Set(v8::String::NewFromUtf8(isolate, name)
                 , v8::FunctionTemplate::New
                 (isolate, &efl::js::call_function
                  <typename eina::_mpl::pop_front<typename function_params<F>::type>::type
                  , void, T, F>
                  , v8::External::New
                  (isolate, reinterpret_cast<void*>(f))));
}

template <typename T, typename...Sig, typename F>
void register_(v8::Isolate* isolate, const char* name, F&& f, v8::Handle<v8::ObjectTemplate> template_
               , typename std::enable_if<!std::is_function<typename std::remove_pointer<F>::type>::value>::type* = 0)
{
  template_->Set(v8::String::NewFromUtf8(isolate, name)
                 , v8::FunctionTemplate::New
                 (isolate, &efl::js::call_generic<std::tuple<Sig...>, void, T, F>
                  , v8::External::New
                  (isolate, new F(std::forward<F>(f)))));
}

} } }

EAPI void eina_list_register(v8::Handle<v8::ObjectTemplate> global, v8::Isolate* isolate)
{
  v8::Handle<v8::FunctionTemplate> constructor = v8::FunctionTemplate::New(isolate, &efl::js::new_eina_list );
  v8::Local<v8::ObjectTemplate> instance_t = constructor->InstanceTemplate();
  instance_t->SetInternalFieldCount(1);

  instance_t->SetIndexedPropertyHandler(& efl::js::index_get);
  
  global->Set(v8::String::NewFromUtf8(isolate, "List"), constructor);
  // efl::js::register_<efl::js::eina_list>
  //   (isolate, "push_back", &efl::js::push_back, instance_t);
}

