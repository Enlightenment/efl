#ifndef EINA_TPL_SWITCH_HH_
#define EINA_TPL_SWITCH_HH_

namespace efl { namespace eina { namespace _mpl {

template <char...Chars>
struct tpl_char_switch;
    
template <>
struct tpl_char_switch<>
{
  template <typename F, typename Fail>
  constexpr void operator()(char c, Fail const fail) const
  {
    fail(c);
  }
};

template <char C0>
struct tpl_char_switch<C0>
{
  template <typename F, typename Fail, typename...Args>
  constexpr void operator()(char c, Fail const fail, Args&&...args) const
  {
    if(c == C0)
      typename F::template apply<C0>::type{}(args...);
    else
      fail(c);
  }
};

template <char C0, char C1>
struct tpl_char_switch<C0, C1>
{
  template <typename F, typename Fail, typename...Args>
  constexpr void operator()(char c, Fail const fail, Args&&...args) const
  {
    switch(c)
    {
    case C0:
      typename F::template apply<C0>::type{}(args...);
      break;
    case C1:
      typename F::template apply<C1>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2>
struct tpl_char_switch<C0, C1, C2>
{
  template <typename F, typename Fail, typename...Args>
  constexpr void operator()(char c, Fail const fail, Args&&...args) const
  {
    switch(c)
    {
    case C0:
      typename F::template apply<C0>::type{}(args...);
      break;
    case C1:
      typename F::template apply<C1>::type{}(args...);
      break;
    case C2:
      typename F::template apply<C2>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3>
struct tpl_char_switch<C0, C1, C2, C3>
{
  template <typename F, typename Fail, typename...Args>
  constexpr void operator()(char c, Fail const fail, Args&&...args) const
  {
    switch(c)
    {
    case C0:
      typename F::template apply<C0>::type{}(args...);
      break;
    case C1:
      typename F::template apply<C1>::type{}(args...);
      break;
    case C2:
      typename F::template apply<C2>::type{}(args...);
      break;
    case C3:
      typename F::template apply<C3>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4>
struct tpl_char_switch<C0, C1, C2, C3, C4>
{
  template <typename F, typename Fail, typename...Args>
  constexpr void operator()(char c, Fail const fail, Args&&...args) const
  {
    switch(c)
    {
    case C0:
      typename F::template apply<C0>::type{}(args...);
      break;
    case C1:
      typename F::template apply<C1>::type{}(args...);
      break;
    case C2:
      typename F::template apply<C2>::type{}(args...);
      break;
    case C3:
      typename F::template apply<C3>::type{}(args...);
      break;
    case C4:
      typename F::template apply<C4>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5>
{
  template <typename F, typename Fail, typename...Args>
  constexpr void operator()(char c, Fail const fail, Args&&...args) const
  {
    switch(c)
    {
    case C0:
      typename F::template apply<C0>::type{}(args...);
      break;
    case C1:
      typename F::template apply<C1>::type{}(args...);
      break;
    case C2:
      typename F::template apply<C2>::type{}(args...);
      break;
    case C3:
      typename F::template apply<C3>::type{}(args...);
      break;
    case C4:
      typename F::template apply<C4>::type{}(args...);
      break;
    case C5:
      typename F::template apply<C5>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6>
{
  template <typename F, typename Fail, typename...Args>
  constexpr void operator()(char c, Fail const fail, Args&&...args) const
  {
    switch(c)
    {
    case C0:
      typename F::template apply<C0>::type{}(args...);
      break;
    case C1:
      typename F::template apply<C1>::type{}(args...);
      break;
    case C2:
      typename F::template apply<C2>::type{}(args...);
      break;
    case C3:
      typename F::template apply<C3>::type{}(args...);
      break;
    case C4:
      typename F::template apply<C4>::type{}(args...);
      break;
    case C5:
      typename F::template apply<C5>::type{}(args...);
      break;
    case C6:
      typename F::template apply<C6>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7>
{
  template <typename F, typename Fail, typename...Args>
  constexpr void operator()(char c, Fail const fail, Args&&...args) const
  {
    switch(c)
    {
    case C0:
      typename F::template apply<C0>::type{}(args...);
      break;
    case C1:
      typename F::template apply<C1>::type{}(args...);
      break;
    case C2:
      typename F::template apply<C2>::type{}(args...);
      break;
    case C3:
      typename F::template apply<C3>::type{}(args...);
      break;
    case C4:
      typename F::template apply<C4>::type{}(args...);
      break;
    case C5:
      typename F::template apply<C5>::type{}(args...);
      break;
    case C6:
      typename F::template apply<C6>::type{}(args...);
      break;
    case C7:
      typename F::template apply<C7>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8>
{
  template <typename F, typename Fail, typename...Args>
  constexpr void operator()(char c, Fail const fail, Args&&...args) const
  {
    switch(c)
    {
    case C0:
      typename F::template apply<C0>::type{}(args...);
      break;
    case C1:
      typename F::template apply<C1>::type{}(args...);
      break;
    case C2:
      typename F::template apply<C2>::type{}(args...);
      break;
    case C3:
      typename F::template apply<C3>::type{}(args...);
      break;
    case C4:
      typename F::template apply<C4>::type{}(args...);
      break;
    case C5:
      typename F::template apply<C5>::type{}(args...);
      break;
    case C6:
      typename F::template apply<C6>::type{}(args...);
      break;
    case C7:
      typename F::template apply<C7>::type{}(args...);
      break;
    case C8:
      typename F::template apply<C8>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9>
{
  template <typename F, typename Fail, typename...Args>
  constexpr void operator()(char c, Fail const fail, Args&&...args) const
  {
    switch(c)
    {
    case C0:
      typename F::template apply<C0>::type{}(args...);
      break;
    case C1:
      typename F::template apply<C1>::type{}(args...);
      break;
    case C2:
      typename F::template apply<C2>::type{}(args...);
      break;
    case C3:
      typename F::template apply<C3>::type{}(args...);
      break;
    case C4:
      typename F::template apply<C4>::type{}(args...);
      break;
    case C5:
      typename F::template apply<C5>::type{}(args...);
      break;
    case C6:
      typename F::template apply<C6>::type{}(args...);
      break;
    case C7:
      typename F::template apply<C7>::type{}(args...);
      break;
    case C8:
      typename F::template apply<C8>::type{}(args...);
      break;
    case C9:
      typename F::template apply<C9>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10>
{
  template <typename F, typename Fail, typename...Args>
  constexpr void operator()(char c, Fail const fail, Args&&...args) const
  {
    switch(c)
    {
    case C0:
      typename F::template apply<C0>::type{}(args...);
      break;
    case C1:
      typename F::template apply<C1>::type{}(args...);
      break;
    case C2:
      typename F::template apply<C2>::type{}(args...);
      break;
    case C3:
      typename F::template apply<C3>::type{}(args...);
      break;
    case C4:
      typename F::template apply<C4>::type{}(args...);
      break;
    case C5:
      typename F::template apply<C5>::type{}(args...);
      break;
    case C6:
      typename F::template apply<C6>::type{}(args...);
      break;
    case C7:
      typename F::template apply<C7>::type{}(args...);
      break;
    case C8:
      typename F::template apply<C8>::type{}(args...);
      break;
    case C9:
      typename F::template apply<C9>::type{}(args...);
      break;
    case C10:
      typename F::template apply<C10>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11>
{
  template <typename F, typename Fail, typename...Args>
  constexpr void operator()(char c, Fail const fail, Args&&...args) const
  {
    switch(c)
    {
    case C0:
      typename F::template apply<C0>::type{}(args...);
      break;
    case C1:
      typename F::template apply<C1>::type{}(args...);
      break;
    case C2:
      typename F::template apply<C2>::type{}(args...);
      break;
    case C3:
      typename F::template apply<C3>::type{}(args...);
      break;
    case C4:
      typename F::template apply<C4>::type{}(args...);
      break;
    case C5:
      typename F::template apply<C5>::type{}(args...);
      break;
    case C6:
      typename F::template apply<C6>::type{}(args...);
      break;
    case C7:
      typename F::template apply<C7>::type{}(args...);
      break;
    case C8:
      typename F::template apply<C8>::type{}(args...);
      break;
    case C9:
      typename F::template apply<C9>::type{}(args...);
      break;
    case C10:
      typename F::template apply<C10>::type{}(args...);
      break;
    case C11:
      typename F::template apply<C11>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12>
{
  template <typename F, typename Fail, typename...Args>
  constexpr void operator()(char c, Fail const fail, Args&&...args) const
  {
    switch(c)
    {
    case C0:
      typename F::template apply<C0>::type{}(args...);
      break;
    case C1:
      typename F::template apply<C1>::type{}(args...);
      break;
    case C2:
      typename F::template apply<C2>::type{}(args...);
      break;
    case C3:
      typename F::template apply<C3>::type{}(args...);
      break;
    case C4:
      typename F::template apply<C4>::type{}(args...);
      break;
    case C5:
      typename F::template apply<C5>::type{}(args...);
      break;
    case C6:
      typename F::template apply<C6>::type{}(args...);
      break;
    case C7:
      typename F::template apply<C7>::type{}(args...);
      break;
    case C8:
      typename F::template apply<C8>::type{}(args...);
      break;
    case C9:
      typename F::template apply<C9>::type{}(args...);
      break;
    case C10:
      typename F::template apply<C10>::type{}(args...);
      break;
    case C11:
      typename F::template apply<C11>::type{}(args...);
      break;
    case C12:
      typename F::template apply<C12>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13>
{
  template <typename F, typename Fail, typename...Args>
  constexpr void operator()(char c, Fail const fail, Args&&...args) const
  {
    switch(c)
    {
    case C0:
      typename F::template apply<C0>::type{}(args...);
      break;
    case C1:
      typename F::template apply<C1>::type{}(args...);
      break;
    case C2:
      typename F::template apply<C2>::type{}(args...);
      break;
    case C3:
      typename F::template apply<C3>::type{}(args...);
      break;
    case C4:
      typename F::template apply<C4>::type{}(args...);
      break;
    case C5:
      typename F::template apply<C5>::type{}(args...);
      break;
    case C6:
      typename F::template apply<C6>::type{}(args...);
      break;
    case C7:
      typename F::template apply<C7>::type{}(args...);
      break;
    case C8:
      typename F::template apply<C8>::type{}(args...);
      break;
    case C9:
      typename F::template apply<C9>::type{}(args...);
      break;
    case C10:
      typename F::template apply<C10>::type{}(args...);
      break;
    case C11:
      typename F::template apply<C11>::type{}(args...);
      break;
    case C12:
      typename F::template apply<C12>::type{}(args...);
      break;
    case C13:
      typename F::template apply<C13>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14>
{
  template <typename F, typename Fail, typename...Args>
  constexpr void operator()(char c, Fail const fail, Args&&...args) const
  {
    switch(c)
    {
    case C0:
      typename F::template apply<C0>::type{}(args...);
      break;
    case C1:
      typename F::template apply<C1>::type{}(args...);
      break;
    case C2:
      typename F::template apply<C2>::type{}(args...);
      break;
    case C3:
      typename F::template apply<C3>::type{}(args...);
      break;
    case C4:
      typename F::template apply<C4>::type{}(args...);
      break;
    case C5:
      typename F::template apply<C5>::type{}(args...);
      break;
    case C6:
      typename F::template apply<C6>::type{}(args...);
      break;
    case C7:
      typename F::template apply<C7>::type{}(args...);
      break;
    case C8:
      typename F::template apply<C8>::type{}(args...);
      break;
    case C9:
      typename F::template apply<C9>::type{}(args...);
      break;
    case C10:
      typename F::template apply<C10>::type{}(args...);
      break;
    case C11:
      typename F::template apply<C11>::type{}(args...);
      break;
    case C12:
      typename F::template apply<C12>::type{}(args...);
      break;
    case C13:
      typename F::template apply<C13>::type{}(args...);
      break;
    case C14:
      typename F::template apply<C14>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15>
{
  template <typename F, typename Fail, typename...Args>
  constexpr void operator()(char c, Fail const fail, Args&&...args) const
  {
    switch(c)
    {
    case C0:
      typename F::template apply<C0>::type{}(args...);
      break;
    case C1:
      typename F::template apply<C1>::type{}(args...);
      break;
    case C2:
      typename F::template apply<C2>::type{}(args...);
      break;
    case C3:
      typename F::template apply<C3>::type{}(args...);
      break;
    case C4:
      typename F::template apply<C4>::type{}(args...);
      break;
    case C5:
      typename F::template apply<C5>::type{}(args...);
      break;
    case C6:
      typename F::template apply<C6>::type{}(args...);
      break;
    case C7:
      typename F::template apply<C7>::type{}(args...);
      break;
    case C8:
      typename F::template apply<C8>::type{}(args...);
      break;
    case C9:
      typename F::template apply<C9>::type{}(args...);
      break;
    case C10:
      typename F::template apply<C10>::type{}(args...);
      break;
    case C11:
      typename F::template apply<C11>::type{}(args...);
      break;
    case C12:
      typename F::template apply<C12>::type{}(args...);
      break;
    case C13:
      typename F::template apply<C13>::type{}(args...);
      break;
    case C14:
      typename F::template apply<C14>::type{}(args...);
      break;
    case C15:
      typename F::template apply<C15>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16>
{
  template <typename F, typename Fail, typename...Args>
  constexpr void operator()(char c, Fail const fail, Args&&...args) const
  {
    switch(c)
    {
    case C0:
      typename F::template apply<C0>::type{}(args...);
      break;
    case C1:
      typename F::template apply<C1>::type{}(args...);
      break;
    case C2:
      typename F::template apply<C2>::type{}(args...);
      break;
    case C3:
      typename F::template apply<C3>::type{}(args...);
      break;
    case C4:
      typename F::template apply<C4>::type{}(args...);
      break;
    case C5:
      typename F::template apply<C5>::type{}(args...);
      break;
    case C6:
      typename F::template apply<C6>::type{}(args...);
      break;
    case C7:
      typename F::template apply<C7>::type{}(args...);
      break;
    case C8:
      typename F::template apply<C8>::type{}(args...);
      break;
    case C9:
      typename F::template apply<C9>::type{}(args...);
      break;
    case C10:
      typename F::template apply<C10>::type{}(args...);
      break;
    case C11:
      typename F::template apply<C11>::type{}(args...);
      break;
    case C12:
      typename F::template apply<C12>::type{}(args...);
      break;
    case C13:
      typename F::template apply<C13>::type{}(args...);
      break;
    case C14:
      typename F::template apply<C14>::type{}(args...);
      break;
    case C15:
      typename F::template apply<C15>::type{}(args...);
      break;
    case C16:
      typename F::template apply<C16>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17>
{
  template <typename F, typename Fail, typename...Args>
  constexpr void operator()(char c, Fail const fail, Args&&...args) const
  {
    switch(c)
    {
    case C0:
      typename F::template apply<C0>::type{}(args...);
      break;
    case C1:
      typename F::template apply<C1>::type{}(args...);
      break;
    case C2:
      typename F::template apply<C2>::type{}(args...);
      break;
    case C3:
      typename F::template apply<C3>::type{}(args...);
      break;
    case C4:
      typename F::template apply<C4>::type{}(args...);
      break;
    case C5:
      typename F::template apply<C5>::type{}(args...);
      break;
    case C6:
      typename F::template apply<C6>::type{}(args...);
      break;
    case C7:
      typename F::template apply<C7>::type{}(args...);
      break;
    case C8:
      typename F::template apply<C8>::type{}(args...);
      break;
    case C9:
      typename F::template apply<C9>::type{}(args...);
      break;
    case C10:
      typename F::template apply<C10>::type{}(args...);
      break;
    case C11:
      typename F::template apply<C11>::type{}(args...);
      break;
    case C12:
      typename F::template apply<C12>::type{}(args...);
      break;
    case C13:
      typename F::template apply<C13>::type{}(args...);
      break;
    case C14:
      typename F::template apply<C14>::type{}(args...);
      break;
    case C15:
      typename F::template apply<C15>::type{}(args...);
      break;
    case C16:
      typename F::template apply<C16>::type{}(args...);
      break;
    case C17:
      typename F::template apply<C17>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18>
{
  template <typename F, typename Fail, typename...Args>
  constexpr void operator()(char c, Fail const fail, Args&&...args) const
  {
    switch(c)
    {
    case C0:
      typename F::template apply<C0>::type{}(args...);
      break;
    case C1:
      typename F::template apply<C1>::type{}(args...);
      break;
    case C2:
      typename F::template apply<C2>::type{}(args...);
      break;
    case C3:
      typename F::template apply<C3>::type{}(args...);
      break;
    case C4:
      typename F::template apply<C4>::type{}(args...);
      break;
    case C5:
      typename F::template apply<C5>::type{}(args...);
      break;
    case C6:
      typename F::template apply<C6>::type{}(args...);
      break;
    case C7:
      typename F::template apply<C7>::type{}(args...);
      break;
    case C8:
      typename F::template apply<C8>::type{}(args...);
      break;
    case C9:
      typename F::template apply<C9>::type{}(args...);
      break;
    case C10:
      typename F::template apply<C10>::type{}(args...);
      break;
    case C11:
      typename F::template apply<C11>::type{}(args...);
      break;
    case C12:
      typename F::template apply<C12>::type{}(args...);
      break;
    case C13:
      typename F::template apply<C13>::type{}(args...);
      break;
    case C14:
      typename F::template apply<C14>::type{}(args...);
      break;
    case C15:
      typename F::template apply<C15>::type{}(args...);
      break;
    case C16:
      typename F::template apply<C16>::type{}(args...);
      break;
    case C17:
      typename F::template apply<C17>::type{}(args...);
      break;
    case C18:
      typename F::template apply<C18>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19>
{
  template <typename F, typename Fail, typename...Args>
  constexpr void operator()(char c, Fail const fail, Args&&...args) const
  {
    switch(c)
    {
    case C0:
      typename F::template apply<C0>::type{}(args...);
      break;
    case C1:
      typename F::template apply<C1>::type{}(args...);
      break;
    case C2:
      typename F::template apply<C2>::type{}(args...);
      break;
    case C3:
      typename F::template apply<C3>::type{}(args...);
      break;
    case C4:
      typename F::template apply<C4>::type{}(args...);
      break;
    case C5:
      typename F::template apply<C5>::type{}(args...);
      break;
    case C6:
      typename F::template apply<C6>::type{}(args...);
      break;
    case C7:
      typename F::template apply<C7>::type{}(args...);
      break;
    case C8:
      typename F::template apply<C8>::type{}(args...);
      break;
    case C9:
      typename F::template apply<C9>::type{}(args...);
      break;
    case C10:
      typename F::template apply<C10>::type{}(args...);
      break;
    case C11:
      typename F::template apply<C11>::type{}(args...);
      break;
    case C12:
      typename F::template apply<C12>::type{}(args...);
      break;
    case C13:
      typename F::template apply<C13>::type{}(args...);
      break;
    case C14:
      typename F::template apply<C14>::type{}(args...);
      break;
    case C15:
      typename F::template apply<C15>::type{}(args...);
      break;
    case C16:
      typename F::template apply<C16>::type{}(args...);
      break;
    case C17:
      typename F::template apply<C17>::type{}(args...);
      break;
    case C18:
      typename F::template apply<C18>::type{}(args...);
      break;
    case C19:
      typename F::template apply<C19>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20>
{
  template <typename F, typename Fail, typename...Args>
  constexpr void operator()(char c, Fail const fail, Args&&...args) const
  {
    switch(c)
    {
    case C0:
      typename F::template apply<C0>::type{}(args...);
      break;
    case C1:
      typename F::template apply<C1>::type{}(args...);
      break;
    case C2:
      typename F::template apply<C2>::type{}(args...);
      break;
    case C3:
      typename F::template apply<C3>::type{}(args...);
      break;
    case C4:
      typename F::template apply<C4>::type{}(args...);
      break;
    case C5:
      typename F::template apply<C5>::type{}(args...);
      break;
    case C6:
      typename F::template apply<C6>::type{}(args...);
      break;
    case C7:
      typename F::template apply<C7>::type{}(args...);
      break;
    case C8:
      typename F::template apply<C8>::type{}(args...);
      break;
    case C9:
      typename F::template apply<C9>::type{}(args...);
      break;
    case C10:
      typename F::template apply<C10>::type{}(args...);
      break;
    case C11:
      typename F::template apply<C11>::type{}(args...);
      break;
    case C12:
      typename F::template apply<C12>::type{}(args...);
      break;
    case C13:
      typename F::template apply<C13>::type{}(args...);
      break;
    case C14:
      typename F::template apply<C14>::type{}(args...);
      break;
    case C15:
      typename F::template apply<C15>::type{}(args...);
      break;
    case C16:
      typename F::template apply<C16>::type{}(args...);
      break;
    case C17:
      typename F::template apply<C17>::type{}(args...);
      break;
    case C18:
      typename F::template apply<C18>::type{}(args...);
      break;
    case C19:
      typename F::template apply<C19>::type{}(args...);
      break;
    case C20:
      typename F::template apply<C20>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

    
} } }

#endif
