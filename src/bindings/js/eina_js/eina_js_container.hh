#ifndef EINA_JS_CONTAINER_HH
#define EINA_JS_CONTAINER_HH

#include <algorithm>

#include <eina_js_compatibility.hh>
#include <eina_js_get_value.hh>
#include <eina_js_get_value_from_c.hh>

struct _Elm_Calendar_Mark;
struct Elm_Gen_Item;
struct _Eina_Rectangle;
struct _Evas_Textblock_Rectangle;
struct _Elm_Map_Overlay;

namespace efl { namespace eina { namespace js {

namespace detail {

template <typename T>
eina_container_base* concat(T const& self, eina_container_base const& other)
{
  T const& rhs = static_cast<T const&>(other);
  typedef typename T::container_type container_type;
  container_type container(self._container.begin(), self._container.end());
  container.insert(container.end(), rhs._container.begin(), rhs._container.end());
  return new T(container.release_native_handle());
}

template <typename T>
eina_container_base* slice(T const& self, std::int64_t i, std::int64_t j)
{
  typedef typename T::container_type container_type;
  container_type container(std::next(self._container.begin(), i), std::next(self._container.begin(), j));
  return new T(container.release_native_handle());
}

// T, W from the container instantiation
template<typename T, typename W, typename C>
int push(v8::Isolate* isolate, C& self, v8::Local<v8::Value> v)
{
  try
      {
         W value = container_wrap(get_value_from_javascript(v, isolate, "", value_tag<T>()));
         self._container.push_back(value);
      }
    catch (std::logic_error const&)
      {
         return -1;
      }
    return self.size();
}

template<typename T, typename W, typename K, typename C>
v8::Local<v8::Value> pop(v8::Isolate* isolate, C& self)
{

  if (self._container.empty())
    return v8::Undefined(isolate);

  auto value = eina::js::get_value_from_c(
       eina::js::wrap_value<T>(eina::js::container_unwrap(self._container.back()), eina::js::value_tag<T>{})
       , isolate
       , K::class_name());
  self._container.pop_back();
  return value;
}
template<typename T, typename W, typename C>
v8::Local<v8::Value> set(v8::Isolate* isolate, C& self, std::size_t index, v8::Local<v8::Value> v)
  {
    using notag_type = typename remove_tag<T>::type;
    try
      {
         W value = container_wrap(get_value_from_javascript(v, isolate, "", value_tag<T>()));

         if (index >= self.size())
           {
               std::size_t items_to_add = index - self.size() + 1;
               for (int i = items_to_add; i; i--)
                 {
                    self._container.push_back(container_wrap(notag_type{}));
                 }
           }

         auto v2 = std::next(self._container.begin(), index);
         *v2 = value;
      }
    catch (std::logic_error const&)
      {
         return v8::Undefined(isolate);
      }

    return v;
  }

}

template <typename InputIterator, typename T>
inline InputIterator find_element(InputIterator first, InputIterator last, T const& value, typename std::enable_if<is_handable_by_value<T>::value>::type* = 0)
{
  return std::find(first, last, value);
}

template <typename InputIterator, typename T>
inline InputIterator find_element(InputIterator first, InputIterator last, T const& value, typename std::enable_if<!is_handable_by_value<T>::value>::type* = 0)
{
  return std::find_if(first, last, [&](T const& e){ return &e == &value; });
}

template <typename InputIterator>
inline InputIterator find_element(InputIterator first, InputIterator last, char * value)
{
  return std::find_if(first, last, [=](char* e){ return strcmp(e, value) == 0; });
}

template <typename CharT, typename T>
inline void stream_element(std::basic_ostream<CharT>& s, T const& value, typename std::enable_if<is_handable_by_value<T>::value>::type* = 0)
{
  s << value;
}

template <typename CharT, typename T>
inline void stream_element(std::basic_ostream<CharT>& s, T const& value, typename std::enable_if<!is_handable_by_value<T>::value>::type* = 0)
{
  s << &value;
}

template <typename CharT>
inline void stream_element(std::basic_ostream<CharT>& s, Eina_Bool value)
{
  s << (value ? "true" : "false");
}

template <typename C, typename T, typename K>
struct eina_container_common;


template <typename C, typename V, typename T, typename K, typename Enable = void>
struct eina_container_type_specific
  : eina_container_base
{
  v8::Local<v8::Value> get(v8::Isolate* isolate, std::size_t index) const
  {
     if(index >= this->size())
       return v8::Undefined(isolate);
     return eina::js::get_value_from_c(
       eina::js::wrap_value<T>(eina::js::container_unwrap(*std::next(container_get().begin(), index)), eina::js::value_tag<T>{})
       , isolate
       , K::class_name());
  }
  int index_of(v8::Isolate* isolate, v8::Local<v8::Value> v) const
  {
    try
      {
        V value = container_wrap(get_value_from_javascript(
           v, isolate, K::class_name(), eina::js::value_tag<T>{}, false));
        typedef typename C::const_iterator iterator;
        iterator first = container_get().cbegin()
          , last = container_get().cend()
          , found = find_element(first, last, value);
        if(found == last)
          return -1;
        else
          return std::distance(first, found);
      }
    catch (std::logic_error const&)
      {
         return -1;
      }
  }
  int last_index_of(v8::Isolate* isolate, v8::Local<v8::Value> v) const
  {
    try
      {
        V value = container_wrap(get_value_from_javascript(
          v, isolate, K::class_name(), eina::js::value_tag<T>{}, false));
        auto last = container_get().crend()
          , found = find_element(container_get().crbegin(), last, value);
        if(found == last)
          return -1;
        else
          return std::distance(container_get().cbegin(), found.base()) -1;
      }
    catch (std::logic_error const&)
      {
         return -1;
      }
  }
  void* get_container_native_handle()
  {
    void const* h = container_get().native_handle();
    return const_cast<void*>(h);
  }
  void const* get_container_native_handle() const
  {
    return container_get().native_handle();
  }
  C& container_get() { return static_cast<eina_container_common<C, T, K>&>(*this)._container; }
  C const& container_get() const { return static_cast<eina_container_common<C, T, K>const&>(*this)._container; }
};

template <typename C, typename T, typename K>
struct eina_container_common : eina_container_type_specific<C, typename C::value_type, T, K>
{
  eina_container_common() : _container() {}
  eina_container_common(typename C::native_handle_type raw) : _container(raw) {}

  std::size_t size() const { return _container.size(); }
  v8::Local<v8::String> to_string(v8::Isolate* isolate) const
  {
    return join(isolate, compatibility_new<v8::String>(isolate, ","));
  }

  v8::Local<v8::String> join(v8::Isolate* isolate, v8::Local<v8::Value> separator_js) const
  {
    std::string separator = ",";
    typedef typename container_type::const_iterator iterator;
    std::stringstream s;

    if (separator_js->IsString())
      {
        v8::String::Utf8Value str(separator_js);
        if (*str)
          {
            separator = *str;
          }
        else
          {
            eina::js::compatibility_throw(
              isolate, v8::Exception::TypeError(
                eina::js::compatibility_new<v8::String>(isolate, "Invalid separator.")));
            return compatibility_new<v8::String>(isolate, "");
          }
      }
    else if (!separator_js->IsUndefined()) // Called join without arguments.
      {
        eina::js::compatibility_throw(
          isolate, v8::Exception::TypeError(
            eina::js::compatibility_new<v8::String>(isolate, "Separator must be a string.")));
        return compatibility_new<v8::String>(isolate, "");
      }

    for(iterator first = _container.begin()
          , last = _container.end()
          , last_elem = std::next(last, -1); first != last; ++first)
      {
        stream_element(s, *first);
        if(first !=  last_elem)
          s << separator;
      }
    return compatibility_new<v8::String>(isolate, s.str().c_str());
  }

  // Default implementation of some methods
  int push(v8::Isolate* isolate, v8::Local<v8::Value>)
  {
    eina::js::compatibility_throw
      (isolate, v8::Exception::TypeError
       (eina::js::compatibility_new<v8::String>(isolate, "Push method was not implemented.")));
    return -1;
  }
  v8::Local<v8::Value> pop(v8::Isolate* isolate)
  {
    eina::js::compatibility_throw
      (isolate, v8::Exception::TypeError
       (eina::js::compatibility_new<v8::String>(isolate, "Pop method was not implemented.")));
    return v8::Undefined(isolate);
  }
  v8::Local<v8::Value> set(v8::Isolate* isolate, std::size_t, v8::Local<v8::Value>)
  {
    eina::js::compatibility_throw
      (isolate, v8::Exception::TypeError
       (eina::js::compatibility_new<v8::String>(isolate, "Indexed attribution was not implemented.")));
    return v8::Undefined(isolate);
  }

  typename C::native_handle_type release_native_handle()
  {
    return _container.release_native_handle();
  }
  C _container;
  typedef C container_type;
};


// Specialization for problematic types
struct empty_container_base
  : eina_container_base
{
  empty_container_base() {}
  virtual ~empty_container_base() {}

  std::size_t size() const
  {
    return 0;
  }
  eina_container_base* concat(eina_container_base const& ) const
  {
    return new empty_container_base;
  }
  eina_container_base* slice(std::int64_t , std::int64_t ) const
  {
    return new empty_container_base;
  }
  int index_of(v8::Isolate*, v8::Local<v8::Value>) const
  {
    return -1;
  }
  int last_index_of(v8::Isolate*, v8::Local<v8::Value>) const
  {
    return -1;
  }
  v8::Local<v8::Value> get(v8::Isolate* isolate, std::size_t) const
  {
    return v8::Undefined(isolate);
  }
  v8::Local<v8::Value> set(v8::Isolate* isolate, std::size_t, v8::Local<v8::Value>)
  {
    return v8::Undefined(isolate);
  }
  int push(v8::Isolate*, v8::Local<v8::Value>)
  {
    return -1;
  }
  v8::Local<v8::Value> pop(v8::Isolate* isolate)
  {
    return v8::Undefined(isolate);
  }
  v8::Local<v8::String> to_string(v8::Isolate* isolate) const
  {
    return compatibility_new<v8::String>(isolate, "");
  }
  v8::Local<v8::String> join(v8::Isolate* isolate, v8::Local<v8::Value>) const
  {
    return compatibility_new<v8::String>(isolate, "");
  }
  container_type get_container_type() const
  {
    throw std::runtime_error("get_container_type of container with unmanagable type");
    return container_type_size;
  }
  void* get_container_native_handle()
  {
    return nullptr;
  }
  void const* get_container_native_handle() const
  {
    return nullptr;
  }
};

} } }

EAPI void eina_container_register(v8::Handle<v8::Object> exports, v8::Isolate* isolate);

#endif
