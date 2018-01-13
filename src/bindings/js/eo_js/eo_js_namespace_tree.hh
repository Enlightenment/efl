#ifndef EFL_EO_JS_NAMESPACE_TREE_HH
#define EFL_EO_JS_NAMESPACE_TREE_HH

#include <eina_string_view.hh>

#include <eina_tpl_char_switch.hh>

#include <eo_js_list.hh>

namespace efl { namespace eo { namespace js {

struct empty
{
  typedef list<> type;
};
      
template <char...Chars>
struct name
{
  static constexpr const int size = sizeof...(Chars);
};

template <typename T>
struct name_size;

template <char...Chars>
struct name_size<name<Chars...>>
  : std::integral_constant<std::size_t, sizeof...(Chars)>
{};
      
template <std::size_t, typename Name>
struct name_at_c;

template <char Char, char...Chars>
struct name_at_c<0ul, name<Char, Chars...>>
  : std::integral_constant<char, Char>
{
};
      
template <std::size_t Index, char Char, char...Chars>
struct name_at_c<Index, name<Char, Chars...>>
  : name_at_c<Index-1, name<Chars...>>
{
};

// template <std::size_t Index, char...Chars>
// struct name_at_c<Index, name<Chars...>
// {
  
// };

// template <std::size_t Index, char...CharsBefore, char Char, char...CharsAfter>
// struct name_at_c_impl<Index, name<Chars...>>
// {
  
// };
      
typedef void(*object_registration_type)(v8::Handle<v8::Object>, v8::Isolate*);
      
template <typename Name
          , typename Classes
          , typename InnerNamespaces>
struct namespace_object
{
  typedef Name name;
  typedef Classes classes;
  typedef InnerNamespaces inner_namespaces;
};

template <typename Name, typename Registration>
struct class_object
{
  typedef Name name;
  typedef Registration registration;
};
      
template <typename NameLhs, typename NameRhs>
struct name_less;

template <>
struct name_less<name<>, name<>>
{
  static constexpr const bool value = false;
};

template <char...Chars>
struct name_less<name<Chars...>, name<>>
{
  static constexpr const bool value = false;
};

template <char...Chars>
struct name_less<name<>, name<Chars...>>
{
  static constexpr const bool value = true;
};

template <char CharacterLhs, char...NameLhs, char CharacterRhs, char...NameRhs>
struct name_less<name<CharacterLhs, NameLhs...>, name<CharacterRhs, NameRhs...>>
{
  static constexpr const bool value =
    CharacterLhs < CharacterRhs
    || (CharacterLhs == CharacterRhs && name_less<name<NameLhs...>, name<NameRhs...>>::value);
};

template <typename T>
struct identity { typedef T type; };

struct name_less_predicate
{
  template <typename A0, typename A1>
  struct apply;

  template <typename Name1, typename Name2, typename...Others1, typename...Others2>
  struct apply<namespace_object<Name1, Others1...>, namespace_object<Name2, Others2...>>
    : name_less<Name1, Name2>
  {};
  template <typename Name1, typename Name2, typename...Others1, typename...Others2>
  struct apply<class_object<Name1, Others1...>, class_object<Name2, Others2...>>
    : name_less<Name1, Name2>
  {};
  template <typename Name1, typename Name2, typename...Others1, typename...Others2>
  struct apply<namespace_object<Name1, Others1...>, class_object<Name2, Others2...>>
    : name_less<Name1, Name2>
  {};
  template <typename Name1, typename Name2, typename...Others1, typename...Others2>
  struct apply<class_object<Name1, Others1...>, namespace_object<Name2, Others2...>>
    : name_less<Name1, Name2>
  {};
};

template <typename A0, typename A1>
struct pair
{
  typedef A0 first;
  typedef A1 second;
};

template <std::size_t Index>
struct map_op
{
  template <typename I, typename A0, typename Enable = void>
  struct apply;

  template <typename A0>
  struct apply<list<>, A0>
  {
    typedef list<pair<name_at_c<Index, typename A0::name>
                     , list<A0>>> type;
  };

  template <typename C, typename...Items, typename...Pairs, typename A0>
  struct apply<list<pair<C, list<Items...>>, Pairs...>, A0
               , typename std::enable_if<(name_at_c<Index, typename A0::name>::value == C::value)>::type>
  {
    typedef list<pair<C, list<A0, Items...>>, Pairs...> type;
  };

  template <typename C, typename List, typename...Pairs, typename A0>
  struct apply<list<pair<C, List>, Pairs...>, A0
               , typename std::enable_if<!(name_at_c<Index, typename A0::name>::value == C::value)>::type>
  {
    typedef list<pair<name_at_c<Index, typename A0::name>, list<A0>>, pair<C, List>, Pairs...> type;
  };
};

template <std::size_t Index, typename List>
struct map_chars
{
  typedef typename list_fold<List, list<>, map_op<Index>>::type type;
};

struct create_switch_fold
{
  template <typename I, typename A0>
  struct apply;

  template <char...Chars, typename A0, typename A1>
  struct apply<eina::_mpl::tpl_char_switch<Chars...>, pair<A0, A1>>
  {
    typedef eina::_mpl::tpl_char_switch<Chars..., A0::value> type;
  };
};

template <typename Map>
struct create_switch
{
  typedef typename list_fold<Map, eina::_mpl::tpl_char_switch<>, create_switch_fold>::type type;
};

template <std::size_t Index, typename...Objects>
void search_separate_switch_case(eina::string_view string, list<Objects...>
                                 , std::false_type);
template <std::size_t Index, typename...Objects>
void search_separate_switch_case(eina::string_view string, list<Objects...>
                                 , std::true_type);

template <int Index, typename Map>
struct switch_case_call
{
  template <char C>
  struct apply
  {
    struct find_char_pair
    {
      template <typename T>
      struct apply;
      template <typename P1, typename P2>
      struct apply<pair<P1, P2>>
        : std::integral_constant<bool, (P1::value == C)>
      {
      };
    };
    
    struct type
    {
      template <typename F>
      constexpr void operator()(eina::string_view string, F const f) const
      {
        std::cout << "Found character " << C << std::endl;
        typedef typename list_find_if<Map, find_char_pair>::type type;
        typedef typename type::second objects;
        typedef typename list_max<name_less_predicate, objects>::type max_type;

        search_separate_switch_case<Index+1>(string, objects{}
          , f
          , std::integral_constant<bool, (name_size<typename max_type::name>::value == Index+1)>{});
      }
    };
  };
};

template <std::size_t Index, typename F, typename Object>
void search_separate_switch_case(eina::string_view string, list<Object>
                                 , F const f
                                 , std::true_type)
{
  if(string.size() == Index)
  {
    std::cout << "Found " << typeid(Object).name() << std::endl;
    f(Object{});
  }
  else
  {
    std::cout << "Not found" << std::endl;
  }  
}

template <std::size_t Index, typename F, typename...Objects>
void search_separate_switch_case(eina::string_view string, list<Objects...>
                                 , F const f
                                 , std::false_type)
{
  typedef typename map_chars<Index, list<Objects...>>::type map;
  typedef typename create_switch<map>::type switch_type;

  switch_type{}
  .template operator()<switch_case_call<Index, map>>
  (string[Index]
   , [] (char c)
    {
      std::cout << "Not found character " << c << " in Index " << Index << " possible characters should be " << typeid(switch_type).name() << std::endl;
    }, string, f);
}

template <typename F, /*typename Registration, */typename Name, typename NamespaceList, typename ClassList>
void search_separate(eina::string_view string, namespace_object<Name
                     , ClassList, NamespaceList>
                     , F const f)
{
  // typedef typename eina::_mpl::tuple_sort<std::tuple<Items...>, item_less_pred>::type ordered_by_tuple_sizes;
  typedef typename list_sort<name_less_predicate, typename ClassList::type, typename NamespaceList::type
                             >::type objects;
  typedef typename list_max<name_less_predicate, objects>::type max_type;

  search_separate_switch_case<0ul>(string, objects{}
     , f
     , std::integral_constant<bool, (name_size<typename max_type::name>::value == 0)>{});
  // search_separate_by_length_case(string, identity<ordered_by_tuple_sizes>{});
}

      
// template <typename...Items>
// void search(identity<std::tuple<Items...>> x)
// {
  
// }
      
} } }

#endif
