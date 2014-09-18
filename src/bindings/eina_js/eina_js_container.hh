#ifndef EINA_JS_CONTAINER_HH
#define EINA_JS_CONTAINER_HH

#include <algorithm>

namespace efl { namespace js {

enum container_type
{
  list_container_type
  , array_container_type
  , container_type_size
};
    
struct eina_container_base
{
  virtual ~eina_container_base() {}

  virtual std::size_t size() const = 0;
  virtual eina_container_base* concat(eina_container_base const& rhs) const = 0;
  virtual eina_container_base* slice(std::int64_t i, std::int64_t j) const = 0;
  virtual int index_of(v8::Isolate* isolate, v8::Local<v8::Value> v) const = 0;
  virtual int last_index_of(v8::Isolate* isolate, v8::Local<v8::Value> v) const = 0;
  virtual v8::Local<v8::Value> get(v8::Isolate*, std::size_t) const = 0;
  virtual v8::Local<v8::String> to_string(v8::Isolate*) const = 0;
  virtual container_type get_container_type() const = 0;
};

template <typename C>
struct eina_container_common;
    
template <typename C, typename T, typename Enable = void>
struct eina_container_type_specific;

template <typename C, typename T>
struct eina_container_type_specific
<C, T, typename std::enable_if<std::is_integral<T>::value
                               && !std::is_same<T, bool>::value
                               && !std::is_same<T, char>::value
                               && !std::is_same<T, char16_t>::value
                               && !std::is_same<T, char32_t>::value
                               && !std::is_same<T, wchar_t>::value>::type>
  : eina_container_base
{
  v8::Local<v8::Value> get(v8::Isolate* isolate, std::size_t index) const
  {
    return v8::Number::New(isolate, *std::next(container_get().begin(), index));
  }
  int index_of(v8::Isolate*, v8::Local<v8::Value> v) const
  {
    std::cout << "index_of" << std::endl;
    if(v->IsInt32() || v->IsUint32())
      {
        int64_t value = v->IntegerValue();
        typedef typename C::const_iterator iterator;
        iterator first = container_get().begin()
          , last = container_get().end()
          , found = std::find(first, last, value);
        if(found == last)
          return -1;
        else
          return std::distance(first, found);
      }
    else
      return -1;
  }
  int last_index_of(v8::Isolate*, v8::Local<v8::Value> v) const
  {
    std::cout << "last_index_of" << std::endl;
    if(v->IsInt32() || v->IsUint32())
      {
        int64_t value = v->IntegerValue();
        auto last = container_get().rend()
          , found = std::find(container_get().rbegin(), last, value);
        if(found == last)
          return -1;
        else
          return std::distance(container_get().begin(), found.base()) -1;
      }
    else
      return -1;
  }
  C& container_get() { return static_cast<eina_container_common<C>&>(*this)._container; }
  C const& container_get() const { return static_cast<eina_container_common<C>const&>(*this)._container; }
};
    
} }

#endif
