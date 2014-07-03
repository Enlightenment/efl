#ifndef EET_HH_
#define EET_HH_

#include <Eet.h>

#include <eet_type.hh>
#include <eet_register.hh>

#include <eina_fold.hh>

#include <type_traits>
#include <cassert>
#include <stdexcept>

#include <iostream>
#include <array>

namespace efl { namespace eet { namespace _detail {

template <typename T>
void* _allocate( ::size_t size )
{
  assert(size == sizeof(T));
  (void)size;
  return new T();
}

template <typename T>
void _deallocate( void* p )
{
  delete static_cast<T*>(p);
}

template <typename T, typename... Args>
struct descriptor_type
{
  struct push_back
  {
    template <typename A, typename B>
    struct apply : eina::_mpl::push_back<A, typename _detail::member_type<typename B::member_type>::type> {};
  };

  typedef typename eina::_mpl::fold< std::tuple<Args...>, push_back
    , descriptor<T> >::type type;
};

}

#define EET_CXX_MEMBER(C, I) ::efl::eet::type(#I, &C::I)

template <typename F>
_detail::member_info<F, void> type(const char* name, F f)
{
  typedef typename _detail::member_type<F>::type member_type;
  static_assert(is_eet_primitive<member_type>::value, "");
  static_assert(std::is_member_pointer<F>::value, "");
  return _detail::member_info<F, void>{name, f};
}

template <typename F, typename U, typename... Args>
_detail::member_info<F, U, Args...> type(const char* name, F f, descriptor<U, Args...> const& descriptor)
{
  typedef typename _detail::member_type<F>::type member_type;
  static_assert(!is_eet_primitive<member_type>::value, "");
  static_assert(std::is_member_pointer<F>::value, "");
  return _detail::member_info<F, U, Args...>{name, f, &descriptor};
}

struct eet_init
{
  eet_init()
  {
    ::eet_init();
  }
  ~eet_init()
  {
    ::eet_shutdown();
  }
};

template <typename T, typename... Args>
struct descriptor
{
  typedef T object_type;

  descriptor() : _descriptor(nullptr) {}
  descriptor( ::Eet_Data_Descriptor* descriptor
              , std::array<_detail::member_desc_info, sizeof...(Args)> member_info)
    : _descriptor(descriptor), _member_info(member_info)
  {
  }
  descriptor(descriptor&& other)
    : _descriptor(other._descriptor)
  {
    other._descriptor = 0;
  }
  descriptor& operator=(descriptor&& other)
  {
    if(_descriptor)
      eet_data_descriptor_free(_descriptor);
    _descriptor = other._descriptor;
    other._descriptor = 0;
    return *this;
  }
  ~descriptor()
  {
    if(_descriptor)
      eet_data_descriptor_free(_descriptor);
  }
  typedef ::Eet_Data_Descriptor const* const_native_handle_type;
  typedef ::Eet_Data_Descriptor* native_handle_type;
  const_native_handle_type native_handle() const
  {
    return _descriptor;
  }
  native_handle_type native_handle()
  {
    return _descriptor;
  }
  typedef std::integral_constant<std::size_t, sizeof...(Args)> members;

  std::array<_detail::member_desc_info, sizeof...(Args)> get_member_info() const { return _member_info; }
private:
  ::Eet_Data_Descriptor* _descriptor;
  typedef descriptor<T, Args...> _self_type;
  descriptor(descriptor const&) = delete;
  descriptor& operator=(descriptor const&) = delete;
  std::array<_detail::member_desc_info, sizeof...(Args)> _member_info;
};

template <typename T, typename... Args>
std::unique_ptr<T> read_by_ptr(Eet_File* file, const char* name, descriptor<T, Args...> const& d)
{
  void* p = eet_data_read(file, const_cast<descriptor<T, Args...>&>(d).native_handle(), name);
  return std::unique_ptr<T>(static_cast<T*>(p));
}

template <typename T, typename... Args>
T read(Eet_File* file, const char* name, descriptor<T, Args...> const& d)
{
  typename std::aligned_storage<sizeof(T), alignof(T)>::type buffer;
  void * p =
    ::eet_data_read_cipher_buffer
    (file
     , const_cast<descriptor<T, Args...>&>(d).native_handle()
     , name, 0
     , static_cast<char*>(static_cast<void*>(&buffer))
     , sizeof(buffer));
  if(p)
    {
      assert(p == &buffer);
      return *static_cast<T*>(p);
    }
  else
    throw std::runtime_error("");
}

namespace _detail {

template <typename O>
inline void _item_fill(O*, ::Eet_Data_Descriptor*, member_desc_info*) {}

template <typename O, typename F, typename D, typename... Args, typename... FArgs>
inline void _item_fill(O* obj, ::Eet_Data_Descriptor* cls, member_desc_info* offset
                       , _detail::member_info<F, D, Args...> arg0, FArgs... args)
{
  static_assert(std::is_member_object_pointer<F>::value, "");
  offset->offset = static_cast<char*>( static_cast<void*>( &(obj ->* arg0.member) ))
    - static_cast<char*>( static_cast<void*>( obj ) );
  offset->name = arg0.name;
  _detail::_item_fill(obj, cls, ++offset, args...);
}

}

template <typename F, typename D, typename... OArgs, typename... Args>
typename _detail::descriptor_type
<typename _detail::object_type<F>::type
 , _detail::member_info<F, D, OArgs...>, Args...
>::type make_descriptor(const char* name, _detail::member_info<F, D, OArgs...> a0, Args... args)
{
  typedef F member_pointer;
  static_assert(std::is_member_object_pointer<member_pointer>::value, "");
  typedef typename _detail::object_type<member_pointer>::type object_type;

  typedef typename _detail::descriptor_type
    <object_type, _detail::member_info<F, D, OArgs...>, Args...>::type descriptor_type;

  ::Eet_Data_Descriptor_Class cls;
  eet_eina_file_data_descriptor_class_set(&cls, sizeof(cls), name, sizeof(object_type));
  cls.func.mem_alloc = & _detail::_allocate<object_type>;
  cls.func.mem_free = & _detail::_deallocate<object_type>;
  ::Eet_Data_Descriptor* native_handle = eet_data_descriptor_stream_new(&cls);
  if(!native_handle)
    throw std::runtime_error("");

  typename std::aligned_storage<sizeof(object_type), alignof(object_type)>::type buffer;
  object_type* p = static_cast<object_type*>(static_cast<void*>(&buffer));

  std::array<_detail::member_desc_info, sizeof...(Args)+1> offsets;
  _detail::_item_fill(p, native_handle, &offsets[0], a0, args...);
  _detail::descriptor_type_register(native_handle, &offsets[0], a0, args...);

  return descriptor_type(native_handle, offsets);
}

} }

#endif
