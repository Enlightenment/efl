#ifndef EFL_EO_JS_CONSTRUCTOR_HH
#define EFL_EO_JS_CONSTRUCTOR_HH

#include <Eina.hh>

#include <eina_tuple.hh>
#include <eina_function.hh>
#include <Eo.h>

#include <cstdlib>
#include <functional>
#include <iostream>
#include <cassert>
#include <vector>

namespace efl { namespace eo { namespace js {

template <typename T>
struct eo_function_params;

template <typename R, typename... P>
struct eo_function_params<R(*)(Eo*, P...)>
{
  typedef std::tuple<P...> type;
};

template <typename R, typename... P>
struct eo_function_params<R(*)(Eo const*, P...)>
{
  typedef std::tuple<P...> type;
};

inline eina::js::compatibility_return_type constructor(eina::js::compatibility_callback_info_type args)
{
  if(args.IsConstructCall())
    {
      void* data = v8::External::Cast(*args.Data())->Value();
      std::function<eina::js::compatibility_return_type(eina::js::compatibility_callback_info_type)>*
        f = static_cast<std::function<eina::js::compatibility_return_type(eina::js::compatibility_callback_info_type)>*>
        (data);
      return (*f)(args);
    }
  else
    {
      std::size_t argc = args.Length();
      std::vector<v8::Local<v8::Value> > argv (argc ? argc : 1 );
      for(int i = 0; i != args.Length(); ++i)
        argv[i] = args[i];
      args.Callee()->NewInstance(argc, &argv[0]);
      return eina::js::compatibility_return();
    }
}
      
template <typename...F>
struct constructor_caller
{
  struct call
  {
    template <typename T>
    void operator()(T function) const
    {
      int const parameters
        = std::tuple_size<typename eo::js::eo_function_params<T>::type>::value;
      if(*current + parameters <= args->Length())
        {
          aux(function, eina::make_index_sequence<parameters>());
          *current += parameters;
        }
      else
        {
          throw std::logic_error("Expected more arguments for this call");
        }
    }

    template <typename U, std::size_t I>
    static
    typename std::tuple_element<I, typename eo::js::eo_function_params<U>::type>::type
    get_value(v8::Local<v8::Value> v, v8::Isolate* isolate)
    {
      typename std::tuple_element<I, typename eo::js::eo_function_params<U>::type>::type
        tmp = 
        eina::js::get_value_from_javascript
        (v, isolate, ""
         , eina::js::value_tag<typename std::tuple_element
         <I, typename eo::js::eo_function_params<U>::type>::type>());
      return tmp;
    }
    
    template <typename T, std::size_t... I>
    void aux(T function, eina::index_sequence<I...>) const
    {
      function(obj_efl_self, get_value<T, I>((*args)[I + *current], args->GetIsolate())...);
    }

    Eo* obj_efl_self;
    int* current;
    eina::js::compatibility_callback_info_pointer args;
  };

  eina::js::compatibility_return_type operator()(eina::js::compatibility_callback_info_type args) const
  {
    int current_index = 1;
    if(args.Length() != 0)
      {
        try
          {
            Eo* parent = eina::js::get_value_from_javascript
              (args[0], args.GetIsolate(), "", eina::js::value_tag<Eo*>());
            Eo* eo = efl_add_ref
              (klass
               , parent
               , eina::_mpl::for_each(constructors, call{efl_added, &current_index, &args})
               );
            if (!eo)
              throw std::logic_error("Failed to create object.");
            v8::Local<v8::Object> self = args.This();
            self->SetInternalField(0, eina::js::compatibility_new<v8::External>(args.GetIsolate(), eo));
            efl::eina::js::make_weak(args.GetIsolate(), self
                                     , [eo]
                                     {
                                       efl_unref(eo);
                                     });
          }
        catch(std::logic_error const& error)
          {
             eina::js::compatibility_throw
               (v8::Exception::TypeError
                (eina::js::compatibility_new<v8::String>(args.GetIsolate(), error.what())));
          }
      }
    else
      {
        eina::js::compatibility_throw
          (v8::Exception::TypeError
           (eina::js::compatibility_new<v8::String>(args.GetIsolate(), "Expected at least one argument for this call")));
      }
    return eina::js::compatibility_return();
  }
  
  Efl_Class const* klass;
  std::tuple<F...> constructors;
};

template <typename... F>
v8::Handle<v8::Value> constructor_data(v8::Isolate* isolate, Efl_Class const* klass, F... f)
{
  return eina::js::compatibility_new<v8::External>
    (isolate
     , new std::function<eina::js::compatibility_return_type(eina::js::compatibility_callback_info_type)>
     (constructor_caller<F...>{klass, std::tuple<F...>{f...}}));
}

} } }

#endif
