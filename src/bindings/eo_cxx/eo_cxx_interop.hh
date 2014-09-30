
#ifndef EFL_EOLIAN_INTEROP_HH
#define EFL_EOLIAN_INTEROP_HH

#include <string>
#include <tuple>
#include <type_traits>

#include <Eina.hh>
#include <Eo.hh>

namespace efl { namespace eolian {

//// From C++ to C

inline Eo*
to_c(efl::eo::base const& x)
{
   return ::eo_ref(x._eo_ptr());
}

inline const char*
to_c(std::string const& x)
{
   return x.c_str();
}

inline const char*
to_c(efl::eina::stringshare const& x)
{
   return x.c_str();
}

inline Eina_Bool
to_c(bool x)
{
   return x ? EINA_TRUE : EINA_FALSE;
}

inline Eina_Bool*
to_c(bool* x)
{
   return static_cast<Eina_Bool*>(x);
}

template <typename T>
T to_c(T const& v, typename std::enable_if<!std::is_base_of<efl::eo::base, T>::value>::type* = 0)
{
   return v;
}
    
//// From C to C++

template <typename T>
struct tag {};

template <typename T, typename U, typename O>
T to_cxx(U object, O o);

// XXX
inline void*
to_cxx(void *x, std::tuple<std::false_type>, tag<void*>)
{
   return x;
}

// XXX
inline const void*
to_cxx(const void *x, std::tuple<std::false_type>, tag<const void*>)
{
   return x;
}

template <typename T>
inline T
to_cxx(Eo* x, std::tuple<std::true_type>, tag<T>)
{
   return T(x);
}

template <typename T>
inline T
to_cxx(Eo* x, std::tuple<std::false_type>, tag<T>)
{
   return T(::eo_ref(x));
}

#ifdef _EVAS_H
template <typename T>
Evas_Object_Textblock_Node_Format *
to_cxx(Evas_Object_Textblock_Node_Format* x, std::tuple<std::false_type>, tag<T>)
{
   return x; // XXX
}
#endif

inline bool
to_cxx(Eina_Bool x, std::tuple<std::false_type>, tag<bool>)
{
   return !!x;
}

inline std::string
to_cxx(const char* x, std::tuple<std::false_type>, tag<std::string>)
{
   return std::string(x);
}

template <typename T, typename Enable = void>
struct traits
{
   typedef T type;
};

template <typename T>
struct traits
 <T, typename std::enable_if<std::is_base_of<efl::eo::base, T>::value>::type>
{
   typedef Eo* type;
};

template <typename T>
struct traits
 <T, typename std::enable_if<std::is_base_of<std::basic_string<char>, T>::value>::type>
{
   typedef const char* type;
};

template <typename T, typename ...Args>
inline efl::eina::range_list<T>
to_cxx(const Eina_List* x, std::tuple<std::false_type, Args...>, tag< efl::eina::range_list<T> >)
{
   return efl::eina::list<T> {x};
}

template <typename T, typename ...Args>
inline efl::eina::range_list<T>
to_cxx(Eina_List* x, std::tuple<std::false_type, Args...>, tag< efl::eina::range_list<T> >)
{
   return efl::eina::range_list<T>{x};
}

template <typename T, typename ...Args>
inline efl::eina::list<T>
to_cxx(Eina_List* x, std::tuple<std::true_type, Args...>, tag< efl::eina::list<T> >)
{
   return efl::eina::list<T> {x};
}

inline eina::stringshare
to_cxx(Eina_Stringshare const* x, const std::false_type, tag<eina::stringshare>)
{
   return ::eina_stringshare_ref(x);
}

template <typename T, typename ...Args>
inline efl::eina::accessor<T>
to_cxx(Eina_Accessor* x, std::tuple<std::false_type, Args...>, tag< efl::eina::accessor<T> >)
{
   return efl::eina::accessor<T>(x);
}

template <typename T, typename ...Args>
inline efl::eina::iterator<T>
to_cxx(Eina_Iterator* x, std::tuple<std::false_type, Args...>, tag< efl::eina::iterator<T> >)
{
   return efl::eina::iterator<T>(x);
}

template <typename T, typename ...Args>
T
to_cxx(Eo const* x, std::tuple<std::false_type, Args...>, tag< T >
       , typename std::enable_if<std::is_base_of<efl::eo::base, T>::value>* = 0)
{
   // Workaround for erroneous constness
   return T{ ::eo_ref(const_cast<Eo*>(x))};
}

template <typename T, typename ...Args>
T
to_cxx(Eo const* x, std::tuple<std::true_type, Args...>, tag< T >
       , typename std::enable_if<std::is_base_of<efl::eo::base, T>::value>* = 0)
{
   // Workaround for erroneous constness
   return T{const_cast<Eo*>(x)};
}

template <typename T, typename U, typename O>
T to_cxx(U object, O o)
{
   return to_cxx(object, o, tag<T>());
}

//// Callbacks

template <typename F, typename R, typename V, typename... Args>
R funcall(V* data, Args... args)
{
   F const* f = static_cast<F const*>(data);
   return (*f)(args...);
}

template <typename T>
struct callback_result_type;

template <typename R, typename... Args>
struct callback_result_type<R(*)(Args...)>
{
   typedef R type;
};

template <typename R>
struct callback_args_type;

template <typename R, typename... Args>
struct callback_args_type<R(*)(Args...)>
{
   typedef std::tuple<Args...> type;
};

template <typename C, typename F, typename R, typename V, typename... Args>
C get_callback_impl(tag<std::tuple<V*, Args...> >)
{
   static_assert(std::is_same<void, typename std::remove_cv<V>::type>::value,
                 "First argument of callback should be void* or const void*");
   return static_cast<C>(&eolian::funcall<F, R, V, Args...>);
}

template <typename C, typename F>
C get_callback()
{
   return get_callback_impl<C, F, typename callback_result_type<C>::type>
     (tag<typename callback_args_type<C>::type>());
}

} } // namespace efl { namespace eolian {

#endif // EFL_EOLIAN_INTEROP_HH
