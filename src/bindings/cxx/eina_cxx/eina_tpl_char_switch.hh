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

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27, char C28>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    case C28:
      typename F::template apply<C28>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27, char C28, char C29>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    case C28:
      typename F::template apply<C28>::type{}(args...);
      break;
    case C29:
      typename F::template apply<C29>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27, char C28, char C29, char C30>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    case C28:
      typename F::template apply<C28>::type{}(args...);
      break;
    case C29:
      typename F::template apply<C29>::type{}(args...);
      break;
    case C30:
      typename F::template apply<C30>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27, char C28, char C29, char C30, char C31>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    case C28:
      typename F::template apply<C28>::type{}(args...);
      break;
    case C29:
      typename F::template apply<C29>::type{}(args...);
      break;
    case C30:
      typename F::template apply<C30>::type{}(args...);
      break;
    case C31:
      typename F::template apply<C31>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27, char C28, char C29, char C30, char C31, char C32>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31, C32>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    case C28:
      typename F::template apply<C28>::type{}(args...);
      break;
    case C29:
      typename F::template apply<C29>::type{}(args...);
      break;
    case C30:
      typename F::template apply<C30>::type{}(args...);
      break;
    case C31:
      typename F::template apply<C31>::type{}(args...);
      break;
    case C32:
      typename F::template apply<C32>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27, char C28, char C29, char C30, char C31, char C32, char C33>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31, C32, C33>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    case C28:
      typename F::template apply<C28>::type{}(args...);
      break;
    case C29:
      typename F::template apply<C29>::type{}(args...);
      break;
    case C30:
      typename F::template apply<C30>::type{}(args...);
      break;
    case C31:
      typename F::template apply<C31>::type{}(args...);
      break;
    case C32:
      typename F::template apply<C32>::type{}(args...);
      break;
    case C33:
      typename F::template apply<C33>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27, char C28, char C29, char C30, char C31, char C32, char C33, char C34>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31, C32, C33, C34>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    case C28:
      typename F::template apply<C28>::type{}(args...);
      break;
    case C29:
      typename F::template apply<C29>::type{}(args...);
      break;
    case C30:
      typename F::template apply<C30>::type{}(args...);
      break;
    case C31:
      typename F::template apply<C31>::type{}(args...);
      break;
    case C32:
      typename F::template apply<C32>::type{}(args...);
      break;
    case C33:
      typename F::template apply<C33>::type{}(args...);
      break;
    case C34:
      typename F::template apply<C34>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27, char C28, char C29, char C30, char C31, char C32, char C33, char C34, char C35>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31, C32, C33, C34, C35>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    case C28:
      typename F::template apply<C28>::type{}(args...);
      break;
    case C29:
      typename F::template apply<C29>::type{}(args...);
      break;
    case C30:
      typename F::template apply<C30>::type{}(args...);
      break;
    case C31:
      typename F::template apply<C31>::type{}(args...);
      break;
    case C32:
      typename F::template apply<C32>::type{}(args...);
      break;
    case C33:
      typename F::template apply<C33>::type{}(args...);
      break;
    case C34:
      typename F::template apply<C34>::type{}(args...);
      break;
    case C35:
      typename F::template apply<C35>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27, char C28, char C29, char C30, char C31, char C32, char C33, char C34, char C35, char C36>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31, C32, C33, C34, C35, C36>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    case C28:
      typename F::template apply<C28>::type{}(args...);
      break;
    case C29:
      typename F::template apply<C29>::type{}(args...);
      break;
    case C30:
      typename F::template apply<C30>::type{}(args...);
      break;
    case C31:
      typename F::template apply<C31>::type{}(args...);
      break;
    case C32:
      typename F::template apply<C32>::type{}(args...);
      break;
    case C33:
      typename F::template apply<C33>::type{}(args...);
      break;
    case C34:
      typename F::template apply<C34>::type{}(args...);
      break;
    case C35:
      typename F::template apply<C35>::type{}(args...);
      break;
    case C36:
      typename F::template apply<C36>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27, char C28, char C29, char C30, char C31, char C32, char C33, char C34, char C35, char C36, char C37>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31, C32, C33, C34, C35, C36, C37>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    case C28:
      typename F::template apply<C28>::type{}(args...);
      break;
    case C29:
      typename F::template apply<C29>::type{}(args...);
      break;
    case C30:
      typename F::template apply<C30>::type{}(args...);
      break;
    case C31:
      typename F::template apply<C31>::type{}(args...);
      break;
    case C32:
      typename F::template apply<C32>::type{}(args...);
      break;
    case C33:
      typename F::template apply<C33>::type{}(args...);
      break;
    case C34:
      typename F::template apply<C34>::type{}(args...);
      break;
    case C35:
      typename F::template apply<C35>::type{}(args...);
      break;
    case C36:
      typename F::template apply<C36>::type{}(args...);
      break;
    case C37:
      typename F::template apply<C37>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27, char C28, char C29, char C30, char C31, char C32, char C33, char C34, char C35, char C36, char C37, char C38>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31, C32, C33, C34, C35, C36, C37, C38>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    case C28:
      typename F::template apply<C28>::type{}(args...);
      break;
    case C29:
      typename F::template apply<C29>::type{}(args...);
      break;
    case C30:
      typename F::template apply<C30>::type{}(args...);
      break;
    case C31:
      typename F::template apply<C31>::type{}(args...);
      break;
    case C32:
      typename F::template apply<C32>::type{}(args...);
      break;
    case C33:
      typename F::template apply<C33>::type{}(args...);
      break;
    case C34:
      typename F::template apply<C34>::type{}(args...);
      break;
    case C35:
      typename F::template apply<C35>::type{}(args...);
      break;
    case C36:
      typename F::template apply<C36>::type{}(args...);
      break;
    case C37:
      typename F::template apply<C37>::type{}(args...);
      break;
    case C38:
      typename F::template apply<C38>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27, char C28, char C29, char C30, char C31, char C32, char C33, char C34, char C35, char C36, char C37, char C38, char C39>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31, C32, C33, C34, C35, C36, C37, C38, C39>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    case C28:
      typename F::template apply<C28>::type{}(args...);
      break;
    case C29:
      typename F::template apply<C29>::type{}(args...);
      break;
    case C30:
      typename F::template apply<C30>::type{}(args...);
      break;
    case C31:
      typename F::template apply<C31>::type{}(args...);
      break;
    case C32:
      typename F::template apply<C32>::type{}(args...);
      break;
    case C33:
      typename F::template apply<C33>::type{}(args...);
      break;
    case C34:
      typename F::template apply<C34>::type{}(args...);
      break;
    case C35:
      typename F::template apply<C35>::type{}(args...);
      break;
    case C36:
      typename F::template apply<C36>::type{}(args...);
      break;
    case C37:
      typename F::template apply<C37>::type{}(args...);
      break;
    case C38:
      typename F::template apply<C38>::type{}(args...);
      break;
    case C39:
      typename F::template apply<C39>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27, char C28, char C29, char C30, char C31, char C32, char C33, char C34, char C35, char C36, char C37, char C38, char C39, char C40>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31, C32, C33, C34, C35, C36, C37, C38, C39, C40>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    case C28:
      typename F::template apply<C28>::type{}(args...);
      break;
    case C29:
      typename F::template apply<C29>::type{}(args...);
      break;
    case C30:
      typename F::template apply<C30>::type{}(args...);
      break;
    case C31:
      typename F::template apply<C31>::type{}(args...);
      break;
    case C32:
      typename F::template apply<C32>::type{}(args...);
      break;
    case C33:
      typename F::template apply<C33>::type{}(args...);
      break;
    case C34:
      typename F::template apply<C34>::type{}(args...);
      break;
    case C35:
      typename F::template apply<C35>::type{}(args...);
      break;
    case C36:
      typename F::template apply<C36>::type{}(args...);
      break;
    case C37:
      typename F::template apply<C37>::type{}(args...);
      break;
    case C38:
      typename F::template apply<C38>::type{}(args...);
      break;
    case C39:
      typename F::template apply<C39>::type{}(args...);
      break;
    case C40:
      typename F::template apply<C40>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27, char C28, char C29, char C30, char C31, char C32, char C33, char C34, char C35, char C36, char C37, char C38, char C39, char C40, char C41>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31, C32, C33, C34, C35, C36, C37, C38, C39, C40, C41>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    case C28:
      typename F::template apply<C28>::type{}(args...);
      break;
    case C29:
      typename F::template apply<C29>::type{}(args...);
      break;
    case C30:
      typename F::template apply<C30>::type{}(args...);
      break;
    case C31:
      typename F::template apply<C31>::type{}(args...);
      break;
    case C32:
      typename F::template apply<C32>::type{}(args...);
      break;
    case C33:
      typename F::template apply<C33>::type{}(args...);
      break;
    case C34:
      typename F::template apply<C34>::type{}(args...);
      break;
    case C35:
      typename F::template apply<C35>::type{}(args...);
      break;
    case C36:
      typename F::template apply<C36>::type{}(args...);
      break;
    case C37:
      typename F::template apply<C37>::type{}(args...);
      break;
    case C38:
      typename F::template apply<C38>::type{}(args...);
      break;
    case C39:
      typename F::template apply<C39>::type{}(args...);
      break;
    case C40:
      typename F::template apply<C40>::type{}(args...);
      break;
    case C41:
      typename F::template apply<C41>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27, char C28, char C29, char C30, char C31, char C32, char C33, char C34, char C35, char C36, char C37, char C38, char C39, char C40, char C41, char C42>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31, C32, C33, C34, C35, C36, C37, C38, C39, C40, C41, C42>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    case C28:
      typename F::template apply<C28>::type{}(args...);
      break;
    case C29:
      typename F::template apply<C29>::type{}(args...);
      break;
    case C30:
      typename F::template apply<C30>::type{}(args...);
      break;
    case C31:
      typename F::template apply<C31>::type{}(args...);
      break;
    case C32:
      typename F::template apply<C32>::type{}(args...);
      break;
    case C33:
      typename F::template apply<C33>::type{}(args...);
      break;
    case C34:
      typename F::template apply<C34>::type{}(args...);
      break;
    case C35:
      typename F::template apply<C35>::type{}(args...);
      break;
    case C36:
      typename F::template apply<C36>::type{}(args...);
      break;
    case C37:
      typename F::template apply<C37>::type{}(args...);
      break;
    case C38:
      typename F::template apply<C38>::type{}(args...);
      break;
    case C39:
      typename F::template apply<C39>::type{}(args...);
      break;
    case C40:
      typename F::template apply<C40>::type{}(args...);
      break;
    case C41:
      typename F::template apply<C41>::type{}(args...);
      break;
    case C42:
      typename F::template apply<C42>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27, char C28, char C29, char C30, char C31, char C32, char C33, char C34, char C35, char C36, char C37, char C38, char C39, char C40, char C41, char C42, char C43>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31, C32, C33, C34, C35, C36, C37, C38, C39, C40, C41, C42, C43>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    case C28:
      typename F::template apply<C28>::type{}(args...);
      break;
    case C29:
      typename F::template apply<C29>::type{}(args...);
      break;
    case C30:
      typename F::template apply<C30>::type{}(args...);
      break;
    case C31:
      typename F::template apply<C31>::type{}(args...);
      break;
    case C32:
      typename F::template apply<C32>::type{}(args...);
      break;
    case C33:
      typename F::template apply<C33>::type{}(args...);
      break;
    case C34:
      typename F::template apply<C34>::type{}(args...);
      break;
    case C35:
      typename F::template apply<C35>::type{}(args...);
      break;
    case C36:
      typename F::template apply<C36>::type{}(args...);
      break;
    case C37:
      typename F::template apply<C37>::type{}(args...);
      break;
    case C38:
      typename F::template apply<C38>::type{}(args...);
      break;
    case C39:
      typename F::template apply<C39>::type{}(args...);
      break;
    case C40:
      typename F::template apply<C40>::type{}(args...);
      break;
    case C41:
      typename F::template apply<C41>::type{}(args...);
      break;
    case C42:
      typename F::template apply<C42>::type{}(args...);
      break;
    case C43:
      typename F::template apply<C43>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27, char C28, char C29, char C30, char C31, char C32, char C33, char C34, char C35, char C36, char C37, char C38, char C39, char C40, char C41, char C42, char C43, char C44>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31, C32, C33, C34, C35, C36, C37, C38, C39, C40, C41, C42, C43, C44>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    case C28:
      typename F::template apply<C28>::type{}(args...);
      break;
    case C29:
      typename F::template apply<C29>::type{}(args...);
      break;
    case C30:
      typename F::template apply<C30>::type{}(args...);
      break;
    case C31:
      typename F::template apply<C31>::type{}(args...);
      break;
    case C32:
      typename F::template apply<C32>::type{}(args...);
      break;
    case C33:
      typename F::template apply<C33>::type{}(args...);
      break;
    case C34:
      typename F::template apply<C34>::type{}(args...);
      break;
    case C35:
      typename F::template apply<C35>::type{}(args...);
      break;
    case C36:
      typename F::template apply<C36>::type{}(args...);
      break;
    case C37:
      typename F::template apply<C37>::type{}(args...);
      break;
    case C38:
      typename F::template apply<C38>::type{}(args...);
      break;
    case C39:
      typename F::template apply<C39>::type{}(args...);
      break;
    case C40:
      typename F::template apply<C40>::type{}(args...);
      break;
    case C41:
      typename F::template apply<C41>::type{}(args...);
      break;
    case C42:
      typename F::template apply<C42>::type{}(args...);
      break;
    case C43:
      typename F::template apply<C43>::type{}(args...);
      break;
    case C44:
      typename F::template apply<C44>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27, char C28, char C29, char C30, char C31, char C32, char C33, char C34, char C35, char C36, char C37, char C38, char C39, char C40, char C41, char C42, char C43, char C44, char C45>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31, C32, C33, C34, C35, C36, C37, C38, C39, C40, C41, C42, C43, C44, C45>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    case C28:
      typename F::template apply<C28>::type{}(args...);
      break;
    case C29:
      typename F::template apply<C29>::type{}(args...);
      break;
    case C30:
      typename F::template apply<C30>::type{}(args...);
      break;
    case C31:
      typename F::template apply<C31>::type{}(args...);
      break;
    case C32:
      typename F::template apply<C32>::type{}(args...);
      break;
    case C33:
      typename F::template apply<C33>::type{}(args...);
      break;
    case C34:
      typename F::template apply<C34>::type{}(args...);
      break;
    case C35:
      typename F::template apply<C35>::type{}(args...);
      break;
    case C36:
      typename F::template apply<C36>::type{}(args...);
      break;
    case C37:
      typename F::template apply<C37>::type{}(args...);
      break;
    case C38:
      typename F::template apply<C38>::type{}(args...);
      break;
    case C39:
      typename F::template apply<C39>::type{}(args...);
      break;
    case C40:
      typename F::template apply<C40>::type{}(args...);
      break;
    case C41:
      typename F::template apply<C41>::type{}(args...);
      break;
    case C42:
      typename F::template apply<C42>::type{}(args...);
      break;
    case C43:
      typename F::template apply<C43>::type{}(args...);
      break;
    case C44:
      typename F::template apply<C44>::type{}(args...);
      break;
    case C45:
      typename F::template apply<C45>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27, char C28, char C29, char C30, char C31, char C32, char C33, char C34, char C35, char C36, char C37, char C38, char C39, char C40, char C41, char C42, char C43, char C44, char C45, char C46>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31, C32, C33, C34, C35, C36, C37, C38, C39, C40, C41, C42, C43, C44, C45, C46>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    case C28:
      typename F::template apply<C28>::type{}(args...);
      break;
    case C29:
      typename F::template apply<C29>::type{}(args...);
      break;
    case C30:
      typename F::template apply<C30>::type{}(args...);
      break;
    case C31:
      typename F::template apply<C31>::type{}(args...);
      break;
    case C32:
      typename F::template apply<C32>::type{}(args...);
      break;
    case C33:
      typename F::template apply<C33>::type{}(args...);
      break;
    case C34:
      typename F::template apply<C34>::type{}(args...);
      break;
    case C35:
      typename F::template apply<C35>::type{}(args...);
      break;
    case C36:
      typename F::template apply<C36>::type{}(args...);
      break;
    case C37:
      typename F::template apply<C37>::type{}(args...);
      break;
    case C38:
      typename F::template apply<C38>::type{}(args...);
      break;
    case C39:
      typename F::template apply<C39>::type{}(args...);
      break;
    case C40:
      typename F::template apply<C40>::type{}(args...);
      break;
    case C41:
      typename F::template apply<C41>::type{}(args...);
      break;
    case C42:
      typename F::template apply<C42>::type{}(args...);
      break;
    case C43:
      typename F::template apply<C43>::type{}(args...);
      break;
    case C44:
      typename F::template apply<C44>::type{}(args...);
      break;
    case C45:
      typename F::template apply<C45>::type{}(args...);
      break;
    case C46:
      typename F::template apply<C46>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27, char C28, char C29, char C30, char C31, char C32, char C33, char C34, char C35, char C36, char C37, char C38, char C39, char C40, char C41, char C42, char C43, char C44, char C45, char C46, char C47>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31, C32, C33, C34, C35, C36, C37, C38, C39, C40, C41, C42, C43, C44, C45, C46, C47>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    case C28:
      typename F::template apply<C28>::type{}(args...);
      break;
    case C29:
      typename F::template apply<C29>::type{}(args...);
      break;
    case C30:
      typename F::template apply<C30>::type{}(args...);
      break;
    case C31:
      typename F::template apply<C31>::type{}(args...);
      break;
    case C32:
      typename F::template apply<C32>::type{}(args...);
      break;
    case C33:
      typename F::template apply<C33>::type{}(args...);
      break;
    case C34:
      typename F::template apply<C34>::type{}(args...);
      break;
    case C35:
      typename F::template apply<C35>::type{}(args...);
      break;
    case C36:
      typename F::template apply<C36>::type{}(args...);
      break;
    case C37:
      typename F::template apply<C37>::type{}(args...);
      break;
    case C38:
      typename F::template apply<C38>::type{}(args...);
      break;
    case C39:
      typename F::template apply<C39>::type{}(args...);
      break;
    case C40:
      typename F::template apply<C40>::type{}(args...);
      break;
    case C41:
      typename F::template apply<C41>::type{}(args...);
      break;
    case C42:
      typename F::template apply<C42>::type{}(args...);
      break;
    case C43:
      typename F::template apply<C43>::type{}(args...);
      break;
    case C44:
      typename F::template apply<C44>::type{}(args...);
      break;
    case C45:
      typename F::template apply<C45>::type{}(args...);
      break;
    case C46:
      typename F::template apply<C46>::type{}(args...);
      break;
    case C47:
      typename F::template apply<C47>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27, char C28, char C29, char C30, char C31, char C32, char C33, char C34, char C35, char C36, char C37, char C38, char C39, char C40, char C41, char C42, char C43, char C44, char C45, char C46, char C47, char C48>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31, C32, C33, C34, C35, C36, C37, C38, C39, C40, C41, C42, C43, C44, C45, C46, C47, C48>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    case C28:
      typename F::template apply<C28>::type{}(args...);
      break;
    case C29:
      typename F::template apply<C29>::type{}(args...);
      break;
    case C30:
      typename F::template apply<C30>::type{}(args...);
      break;
    case C31:
      typename F::template apply<C31>::type{}(args...);
      break;
    case C32:
      typename F::template apply<C32>::type{}(args...);
      break;
    case C33:
      typename F::template apply<C33>::type{}(args...);
      break;
    case C34:
      typename F::template apply<C34>::type{}(args...);
      break;
    case C35:
      typename F::template apply<C35>::type{}(args...);
      break;
    case C36:
      typename F::template apply<C36>::type{}(args...);
      break;
    case C37:
      typename F::template apply<C37>::type{}(args...);
      break;
    case C38:
      typename F::template apply<C38>::type{}(args...);
      break;
    case C39:
      typename F::template apply<C39>::type{}(args...);
      break;
    case C40:
      typename F::template apply<C40>::type{}(args...);
      break;
    case C41:
      typename F::template apply<C41>::type{}(args...);
      break;
    case C42:
      typename F::template apply<C42>::type{}(args...);
      break;
    case C43:
      typename F::template apply<C43>::type{}(args...);
      break;
    case C44:
      typename F::template apply<C44>::type{}(args...);
      break;
    case C45:
      typename F::template apply<C45>::type{}(args...);
      break;
    case C46:
      typename F::template apply<C46>::type{}(args...);
      break;
    case C47:
      typename F::template apply<C47>::type{}(args...);
      break;
    case C48:
      typename F::template apply<C48>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27, char C28, char C29, char C30, char C31, char C32, char C33, char C34, char C35, char C36, char C37, char C38, char C39, char C40, char C41, char C42, char C43, char C44, char C45, char C46, char C47, char C48, char C49>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31, C32, C33, C34, C35, C36, C37, C38, C39, C40, C41, C42, C43, C44, C45, C46, C47, C48, C49>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    case C28:
      typename F::template apply<C28>::type{}(args...);
      break;
    case C29:
      typename F::template apply<C29>::type{}(args...);
      break;
    case C30:
      typename F::template apply<C30>::type{}(args...);
      break;
    case C31:
      typename F::template apply<C31>::type{}(args...);
      break;
    case C32:
      typename F::template apply<C32>::type{}(args...);
      break;
    case C33:
      typename F::template apply<C33>::type{}(args...);
      break;
    case C34:
      typename F::template apply<C34>::type{}(args...);
      break;
    case C35:
      typename F::template apply<C35>::type{}(args...);
      break;
    case C36:
      typename F::template apply<C36>::type{}(args...);
      break;
    case C37:
      typename F::template apply<C37>::type{}(args...);
      break;
    case C38:
      typename F::template apply<C38>::type{}(args...);
      break;
    case C39:
      typename F::template apply<C39>::type{}(args...);
      break;
    case C40:
      typename F::template apply<C40>::type{}(args...);
      break;
    case C41:
      typename F::template apply<C41>::type{}(args...);
      break;
    case C42:
      typename F::template apply<C42>::type{}(args...);
      break;
    case C43:
      typename F::template apply<C43>::type{}(args...);
      break;
    case C44:
      typename F::template apply<C44>::type{}(args...);
      break;
    case C45:
      typename F::template apply<C45>::type{}(args...);
      break;
    case C46:
      typename F::template apply<C46>::type{}(args...);
      break;
    case C47:
      typename F::template apply<C47>::type{}(args...);
      break;
    case C48:
      typename F::template apply<C48>::type{}(args...);
      break;
    case C49:
      typename F::template apply<C49>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27, char C28, char C29, char C30, char C31, char C32, char C33, char C34, char C35, char C36, char C37, char C38, char C39, char C40, char C41, char C42, char C43, char C44, char C45, char C46, char C47, char C48, char C49, char C50>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31, C32, C33, C34, C35, C36, C37, C38, C39, C40, C41, C42, C43, C44, C45, C46, C47, C48, C49, C50>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    case C28:
      typename F::template apply<C28>::type{}(args...);
      break;
    case C29:
      typename F::template apply<C29>::type{}(args...);
      break;
    case C30:
      typename F::template apply<C30>::type{}(args...);
      break;
    case C31:
      typename F::template apply<C31>::type{}(args...);
      break;
    case C32:
      typename F::template apply<C32>::type{}(args...);
      break;
    case C33:
      typename F::template apply<C33>::type{}(args...);
      break;
    case C34:
      typename F::template apply<C34>::type{}(args...);
      break;
    case C35:
      typename F::template apply<C35>::type{}(args...);
      break;
    case C36:
      typename F::template apply<C36>::type{}(args...);
      break;
    case C37:
      typename F::template apply<C37>::type{}(args...);
      break;
    case C38:
      typename F::template apply<C38>::type{}(args...);
      break;
    case C39:
      typename F::template apply<C39>::type{}(args...);
      break;
    case C40:
      typename F::template apply<C40>::type{}(args...);
      break;
    case C41:
      typename F::template apply<C41>::type{}(args...);
      break;
    case C42:
      typename F::template apply<C42>::type{}(args...);
      break;
    case C43:
      typename F::template apply<C43>::type{}(args...);
      break;
    case C44:
      typename F::template apply<C44>::type{}(args...);
      break;
    case C45:
      typename F::template apply<C45>::type{}(args...);
      break;
    case C46:
      typename F::template apply<C46>::type{}(args...);
      break;
    case C47:
      typename F::template apply<C47>::type{}(args...);
      break;
    case C48:
      typename F::template apply<C48>::type{}(args...);
      break;
    case C49:
      typename F::template apply<C49>::type{}(args...);
      break;
    case C50:
      typename F::template apply<C50>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27, char C28, char C29, char C30, char C31, char C32, char C33, char C34, char C35, char C36, char C37, char C38, char C39, char C40, char C41, char C42, char C43, char C44, char C45, char C46, char C47, char C48, char C49, char C50, char C51>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31, C32, C33, C34, C35, C36, C37, C38, C39, C40, C41, C42, C43, C44, C45, C46, C47, C48, C49, C50, C51>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    case C28:
      typename F::template apply<C28>::type{}(args...);
      break;
    case C29:
      typename F::template apply<C29>::type{}(args...);
      break;
    case C30:
      typename F::template apply<C30>::type{}(args...);
      break;
    case C31:
      typename F::template apply<C31>::type{}(args...);
      break;
    case C32:
      typename F::template apply<C32>::type{}(args...);
      break;
    case C33:
      typename F::template apply<C33>::type{}(args...);
      break;
    case C34:
      typename F::template apply<C34>::type{}(args...);
      break;
    case C35:
      typename F::template apply<C35>::type{}(args...);
      break;
    case C36:
      typename F::template apply<C36>::type{}(args...);
      break;
    case C37:
      typename F::template apply<C37>::type{}(args...);
      break;
    case C38:
      typename F::template apply<C38>::type{}(args...);
      break;
    case C39:
      typename F::template apply<C39>::type{}(args...);
      break;
    case C40:
      typename F::template apply<C40>::type{}(args...);
      break;
    case C41:
      typename F::template apply<C41>::type{}(args...);
      break;
    case C42:
      typename F::template apply<C42>::type{}(args...);
      break;
    case C43:
      typename F::template apply<C43>::type{}(args...);
      break;
    case C44:
      typename F::template apply<C44>::type{}(args...);
      break;
    case C45:
      typename F::template apply<C45>::type{}(args...);
      break;
    case C46:
      typename F::template apply<C46>::type{}(args...);
      break;
    case C47:
      typename F::template apply<C47>::type{}(args...);
      break;
    case C48:
      typename F::template apply<C48>::type{}(args...);
      break;
    case C49:
      typename F::template apply<C49>::type{}(args...);
      break;
    case C50:
      typename F::template apply<C50>::type{}(args...);
      break;
    case C51:
      typename F::template apply<C51>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27, char C28, char C29, char C30, char C31, char C32, char C33, char C34, char C35, char C36, char C37, char C38, char C39, char C40, char C41, char C42, char C43, char C44, char C45, char C46, char C47, char C48, char C49, char C50, char C51, char C52>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31, C32, C33, C34, C35, C36, C37, C38, C39, C40, C41, C42, C43, C44, C45, C46, C47, C48, C49, C50, C51, C52>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    case C28:
      typename F::template apply<C28>::type{}(args...);
      break;
    case C29:
      typename F::template apply<C29>::type{}(args...);
      break;
    case C30:
      typename F::template apply<C30>::type{}(args...);
      break;
    case C31:
      typename F::template apply<C31>::type{}(args...);
      break;
    case C32:
      typename F::template apply<C32>::type{}(args...);
      break;
    case C33:
      typename F::template apply<C33>::type{}(args...);
      break;
    case C34:
      typename F::template apply<C34>::type{}(args...);
      break;
    case C35:
      typename F::template apply<C35>::type{}(args...);
      break;
    case C36:
      typename F::template apply<C36>::type{}(args...);
      break;
    case C37:
      typename F::template apply<C37>::type{}(args...);
      break;
    case C38:
      typename F::template apply<C38>::type{}(args...);
      break;
    case C39:
      typename F::template apply<C39>::type{}(args...);
      break;
    case C40:
      typename F::template apply<C40>::type{}(args...);
      break;
    case C41:
      typename F::template apply<C41>::type{}(args...);
      break;
    case C42:
      typename F::template apply<C42>::type{}(args...);
      break;
    case C43:
      typename F::template apply<C43>::type{}(args...);
      break;
    case C44:
      typename F::template apply<C44>::type{}(args...);
      break;
    case C45:
      typename F::template apply<C45>::type{}(args...);
      break;
    case C46:
      typename F::template apply<C46>::type{}(args...);
      break;
    case C47:
      typename F::template apply<C47>::type{}(args...);
      break;
    case C48:
      typename F::template apply<C48>::type{}(args...);
      break;
    case C49:
      typename F::template apply<C49>::type{}(args...);
      break;
    case C50:
      typename F::template apply<C50>::type{}(args...);
      break;
    case C51:
      typename F::template apply<C51>::type{}(args...);
      break;
    case C52:
      typename F::template apply<C52>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27, char C28, char C29, char C30, char C31, char C32, char C33, char C34, char C35, char C36, char C37, char C38, char C39, char C40, char C41, char C42, char C43, char C44, char C45, char C46, char C47, char C48, char C49, char C50, char C51, char C52, char C53>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31, C32, C33, C34, C35, C36, C37, C38, C39, C40, C41, C42, C43, C44, C45, C46, C47, C48, C49, C50, C51, C52, C53>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    case C28:
      typename F::template apply<C28>::type{}(args...);
      break;
    case C29:
      typename F::template apply<C29>::type{}(args...);
      break;
    case C30:
      typename F::template apply<C30>::type{}(args...);
      break;
    case C31:
      typename F::template apply<C31>::type{}(args...);
      break;
    case C32:
      typename F::template apply<C32>::type{}(args...);
      break;
    case C33:
      typename F::template apply<C33>::type{}(args...);
      break;
    case C34:
      typename F::template apply<C34>::type{}(args...);
      break;
    case C35:
      typename F::template apply<C35>::type{}(args...);
      break;
    case C36:
      typename F::template apply<C36>::type{}(args...);
      break;
    case C37:
      typename F::template apply<C37>::type{}(args...);
      break;
    case C38:
      typename F::template apply<C38>::type{}(args...);
      break;
    case C39:
      typename F::template apply<C39>::type{}(args...);
      break;
    case C40:
      typename F::template apply<C40>::type{}(args...);
      break;
    case C41:
      typename F::template apply<C41>::type{}(args...);
      break;
    case C42:
      typename F::template apply<C42>::type{}(args...);
      break;
    case C43:
      typename F::template apply<C43>::type{}(args...);
      break;
    case C44:
      typename F::template apply<C44>::type{}(args...);
      break;
    case C45:
      typename F::template apply<C45>::type{}(args...);
      break;
    case C46:
      typename F::template apply<C46>::type{}(args...);
      break;
    case C47:
      typename F::template apply<C47>::type{}(args...);
      break;
    case C48:
      typename F::template apply<C48>::type{}(args...);
      break;
    case C49:
      typename F::template apply<C49>::type{}(args...);
      break;
    case C50:
      typename F::template apply<C50>::type{}(args...);
      break;
    case C51:
      typename F::template apply<C51>::type{}(args...);
      break;
    case C52:
      typename F::template apply<C52>::type{}(args...);
      break;
    case C53:
      typename F::template apply<C53>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27, char C28, char C29, char C30, char C31, char C32, char C33, char C34, char C35, char C36, char C37, char C38, char C39, char C40, char C41, char C42, char C43, char C44, char C45, char C46, char C47, char C48, char C49, char C50, char C51, char C52, char C53, char C54>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31, C32, C33, C34, C35, C36, C37, C38, C39, C40, C41, C42, C43, C44, C45, C46, C47, C48, C49, C50, C51, C52, C53, C54>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    case C28:
      typename F::template apply<C28>::type{}(args...);
      break;
    case C29:
      typename F::template apply<C29>::type{}(args...);
      break;
    case C30:
      typename F::template apply<C30>::type{}(args...);
      break;
    case C31:
      typename F::template apply<C31>::type{}(args...);
      break;
    case C32:
      typename F::template apply<C32>::type{}(args...);
      break;
    case C33:
      typename F::template apply<C33>::type{}(args...);
      break;
    case C34:
      typename F::template apply<C34>::type{}(args...);
      break;
    case C35:
      typename F::template apply<C35>::type{}(args...);
      break;
    case C36:
      typename F::template apply<C36>::type{}(args...);
      break;
    case C37:
      typename F::template apply<C37>::type{}(args...);
      break;
    case C38:
      typename F::template apply<C38>::type{}(args...);
      break;
    case C39:
      typename F::template apply<C39>::type{}(args...);
      break;
    case C40:
      typename F::template apply<C40>::type{}(args...);
      break;
    case C41:
      typename F::template apply<C41>::type{}(args...);
      break;
    case C42:
      typename F::template apply<C42>::type{}(args...);
      break;
    case C43:
      typename F::template apply<C43>::type{}(args...);
      break;
    case C44:
      typename F::template apply<C44>::type{}(args...);
      break;
    case C45:
      typename F::template apply<C45>::type{}(args...);
      break;
    case C46:
      typename F::template apply<C46>::type{}(args...);
      break;
    case C47:
      typename F::template apply<C47>::type{}(args...);
      break;
    case C48:
      typename F::template apply<C48>::type{}(args...);
      break;
    case C49:
      typename F::template apply<C49>::type{}(args...);
      break;
    case C50:
      typename F::template apply<C50>::type{}(args...);
      break;
    case C51:
      typename F::template apply<C51>::type{}(args...);
      break;
    case C52:
      typename F::template apply<C52>::type{}(args...);
      break;
    case C53:
      typename F::template apply<C53>::type{}(args...);
      break;
    case C54:
      typename F::template apply<C54>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27, char C28, char C29, char C30, char C31, char C32, char C33, char C34, char C35, char C36, char C37, char C38, char C39, char C40, char C41, char C42, char C43, char C44, char C45, char C46, char C47, char C48, char C49, char C50, char C51, char C52, char C53, char C54, char C55>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31, C32, C33, C34, C35, C36, C37, C38, C39, C40, C41, C42, C43, C44, C45, C46, C47, C48, C49, C50, C51, C52, C53, C54, C55>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    case C28:
      typename F::template apply<C28>::type{}(args...);
      break;
    case C29:
      typename F::template apply<C29>::type{}(args...);
      break;
    case C30:
      typename F::template apply<C30>::type{}(args...);
      break;
    case C31:
      typename F::template apply<C31>::type{}(args...);
      break;
    case C32:
      typename F::template apply<C32>::type{}(args...);
      break;
    case C33:
      typename F::template apply<C33>::type{}(args...);
      break;
    case C34:
      typename F::template apply<C34>::type{}(args...);
      break;
    case C35:
      typename F::template apply<C35>::type{}(args...);
      break;
    case C36:
      typename F::template apply<C36>::type{}(args...);
      break;
    case C37:
      typename F::template apply<C37>::type{}(args...);
      break;
    case C38:
      typename F::template apply<C38>::type{}(args...);
      break;
    case C39:
      typename F::template apply<C39>::type{}(args...);
      break;
    case C40:
      typename F::template apply<C40>::type{}(args...);
      break;
    case C41:
      typename F::template apply<C41>::type{}(args...);
      break;
    case C42:
      typename F::template apply<C42>::type{}(args...);
      break;
    case C43:
      typename F::template apply<C43>::type{}(args...);
      break;
    case C44:
      typename F::template apply<C44>::type{}(args...);
      break;
    case C45:
      typename F::template apply<C45>::type{}(args...);
      break;
    case C46:
      typename F::template apply<C46>::type{}(args...);
      break;
    case C47:
      typename F::template apply<C47>::type{}(args...);
      break;
    case C48:
      typename F::template apply<C48>::type{}(args...);
      break;
    case C49:
      typename F::template apply<C49>::type{}(args...);
      break;
    case C50:
      typename F::template apply<C50>::type{}(args...);
      break;
    case C51:
      typename F::template apply<C51>::type{}(args...);
      break;
    case C52:
      typename F::template apply<C52>::type{}(args...);
      break;
    case C53:
      typename F::template apply<C53>::type{}(args...);
      break;
    case C54:
      typename F::template apply<C54>::type{}(args...);
      break;
    case C55:
      typename F::template apply<C55>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27, char C28, char C29, char C30, char C31, char C32, char C33, char C34, char C35, char C36, char C37, char C38, char C39, char C40, char C41, char C42, char C43, char C44, char C45, char C46, char C47, char C48, char C49, char C50, char C51, char C52, char C53, char C54, char C55, char C56>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31, C32, C33, C34, C35, C36, C37, C38, C39, C40, C41, C42, C43, C44, C45, C46, C47, C48, C49, C50, C51, C52, C53, C54, C55, C56>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    case C28:
      typename F::template apply<C28>::type{}(args...);
      break;
    case C29:
      typename F::template apply<C29>::type{}(args...);
      break;
    case C30:
      typename F::template apply<C30>::type{}(args...);
      break;
    case C31:
      typename F::template apply<C31>::type{}(args...);
      break;
    case C32:
      typename F::template apply<C32>::type{}(args...);
      break;
    case C33:
      typename F::template apply<C33>::type{}(args...);
      break;
    case C34:
      typename F::template apply<C34>::type{}(args...);
      break;
    case C35:
      typename F::template apply<C35>::type{}(args...);
      break;
    case C36:
      typename F::template apply<C36>::type{}(args...);
      break;
    case C37:
      typename F::template apply<C37>::type{}(args...);
      break;
    case C38:
      typename F::template apply<C38>::type{}(args...);
      break;
    case C39:
      typename F::template apply<C39>::type{}(args...);
      break;
    case C40:
      typename F::template apply<C40>::type{}(args...);
      break;
    case C41:
      typename F::template apply<C41>::type{}(args...);
      break;
    case C42:
      typename F::template apply<C42>::type{}(args...);
      break;
    case C43:
      typename F::template apply<C43>::type{}(args...);
      break;
    case C44:
      typename F::template apply<C44>::type{}(args...);
      break;
    case C45:
      typename F::template apply<C45>::type{}(args...);
      break;
    case C46:
      typename F::template apply<C46>::type{}(args...);
      break;
    case C47:
      typename F::template apply<C47>::type{}(args...);
      break;
    case C48:
      typename F::template apply<C48>::type{}(args...);
      break;
    case C49:
      typename F::template apply<C49>::type{}(args...);
      break;
    case C50:
      typename F::template apply<C50>::type{}(args...);
      break;
    case C51:
      typename F::template apply<C51>::type{}(args...);
      break;
    case C52:
      typename F::template apply<C52>::type{}(args...);
      break;
    case C53:
      typename F::template apply<C53>::type{}(args...);
      break;
    case C54:
      typename F::template apply<C54>::type{}(args...);
      break;
    case C55:
      typename F::template apply<C55>::type{}(args...);
      break;
    case C56:
      typename F::template apply<C56>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27, char C28, char C29, char C30, char C31, char C32, char C33, char C34, char C35, char C36, char C37, char C38, char C39, char C40, char C41, char C42, char C43, char C44, char C45, char C46, char C47, char C48, char C49, char C50, char C51, char C52, char C53, char C54, char C55, char C56, char C57>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31, C32, C33, C34, C35, C36, C37, C38, C39, C40, C41, C42, C43, C44, C45, C46, C47, C48, C49, C50, C51, C52, C53, C54, C55, C56, C57>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    case C28:
      typename F::template apply<C28>::type{}(args...);
      break;
    case C29:
      typename F::template apply<C29>::type{}(args...);
      break;
    case C30:
      typename F::template apply<C30>::type{}(args...);
      break;
    case C31:
      typename F::template apply<C31>::type{}(args...);
      break;
    case C32:
      typename F::template apply<C32>::type{}(args...);
      break;
    case C33:
      typename F::template apply<C33>::type{}(args...);
      break;
    case C34:
      typename F::template apply<C34>::type{}(args...);
      break;
    case C35:
      typename F::template apply<C35>::type{}(args...);
      break;
    case C36:
      typename F::template apply<C36>::type{}(args...);
      break;
    case C37:
      typename F::template apply<C37>::type{}(args...);
      break;
    case C38:
      typename F::template apply<C38>::type{}(args...);
      break;
    case C39:
      typename F::template apply<C39>::type{}(args...);
      break;
    case C40:
      typename F::template apply<C40>::type{}(args...);
      break;
    case C41:
      typename F::template apply<C41>::type{}(args...);
      break;
    case C42:
      typename F::template apply<C42>::type{}(args...);
      break;
    case C43:
      typename F::template apply<C43>::type{}(args...);
      break;
    case C44:
      typename F::template apply<C44>::type{}(args...);
      break;
    case C45:
      typename F::template apply<C45>::type{}(args...);
      break;
    case C46:
      typename F::template apply<C46>::type{}(args...);
      break;
    case C47:
      typename F::template apply<C47>::type{}(args...);
      break;
    case C48:
      typename F::template apply<C48>::type{}(args...);
      break;
    case C49:
      typename F::template apply<C49>::type{}(args...);
      break;
    case C50:
      typename F::template apply<C50>::type{}(args...);
      break;
    case C51:
      typename F::template apply<C51>::type{}(args...);
      break;
    case C52:
      typename F::template apply<C52>::type{}(args...);
      break;
    case C53:
      typename F::template apply<C53>::type{}(args...);
      break;
    case C54:
      typename F::template apply<C54>::type{}(args...);
      break;
    case C55:
      typename F::template apply<C55>::type{}(args...);
      break;
    case C56:
      typename F::template apply<C56>::type{}(args...);
      break;
    case C57:
      typename F::template apply<C57>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27, char C28, char C29, char C30, char C31, char C32, char C33, char C34, char C35, char C36, char C37, char C38, char C39, char C40, char C41, char C42, char C43, char C44, char C45, char C46, char C47, char C48, char C49, char C50, char C51, char C52, char C53, char C54, char C55, char C56, char C57, char C58>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31, C32, C33, C34, C35, C36, C37, C38, C39, C40, C41, C42, C43, C44, C45, C46, C47, C48, C49, C50, C51, C52, C53, C54, C55, C56, C57, C58>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    case C28:
      typename F::template apply<C28>::type{}(args...);
      break;
    case C29:
      typename F::template apply<C29>::type{}(args...);
      break;
    case C30:
      typename F::template apply<C30>::type{}(args...);
      break;
    case C31:
      typename F::template apply<C31>::type{}(args...);
      break;
    case C32:
      typename F::template apply<C32>::type{}(args...);
      break;
    case C33:
      typename F::template apply<C33>::type{}(args...);
      break;
    case C34:
      typename F::template apply<C34>::type{}(args...);
      break;
    case C35:
      typename F::template apply<C35>::type{}(args...);
      break;
    case C36:
      typename F::template apply<C36>::type{}(args...);
      break;
    case C37:
      typename F::template apply<C37>::type{}(args...);
      break;
    case C38:
      typename F::template apply<C38>::type{}(args...);
      break;
    case C39:
      typename F::template apply<C39>::type{}(args...);
      break;
    case C40:
      typename F::template apply<C40>::type{}(args...);
      break;
    case C41:
      typename F::template apply<C41>::type{}(args...);
      break;
    case C42:
      typename F::template apply<C42>::type{}(args...);
      break;
    case C43:
      typename F::template apply<C43>::type{}(args...);
      break;
    case C44:
      typename F::template apply<C44>::type{}(args...);
      break;
    case C45:
      typename F::template apply<C45>::type{}(args...);
      break;
    case C46:
      typename F::template apply<C46>::type{}(args...);
      break;
    case C47:
      typename F::template apply<C47>::type{}(args...);
      break;
    case C48:
      typename F::template apply<C48>::type{}(args...);
      break;
    case C49:
      typename F::template apply<C49>::type{}(args...);
      break;
    case C50:
      typename F::template apply<C50>::type{}(args...);
      break;
    case C51:
      typename F::template apply<C51>::type{}(args...);
      break;
    case C52:
      typename F::template apply<C52>::type{}(args...);
      break;
    case C53:
      typename F::template apply<C53>::type{}(args...);
      break;
    case C54:
      typename F::template apply<C54>::type{}(args...);
      break;
    case C55:
      typename F::template apply<C55>::type{}(args...);
      break;
    case C56:
      typename F::template apply<C56>::type{}(args...);
      break;
    case C57:
      typename F::template apply<C57>::type{}(args...);
      break;
    case C58:
      typename F::template apply<C58>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27, char C28, char C29, char C30, char C31, char C32, char C33, char C34, char C35, char C36, char C37, char C38, char C39, char C40, char C41, char C42, char C43, char C44, char C45, char C46, char C47, char C48, char C49, char C50, char C51, char C52, char C53, char C54, char C55, char C56, char C57, char C58, char C59>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31, C32, C33, C34, C35, C36, C37, C38, C39, C40, C41, C42, C43, C44, C45, C46, C47, C48, C49, C50, C51, C52, C53, C54, C55, C56, C57, C58, C59>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    case C28:
      typename F::template apply<C28>::type{}(args...);
      break;
    case C29:
      typename F::template apply<C29>::type{}(args...);
      break;
    case C30:
      typename F::template apply<C30>::type{}(args...);
      break;
    case C31:
      typename F::template apply<C31>::type{}(args...);
      break;
    case C32:
      typename F::template apply<C32>::type{}(args...);
      break;
    case C33:
      typename F::template apply<C33>::type{}(args...);
      break;
    case C34:
      typename F::template apply<C34>::type{}(args...);
      break;
    case C35:
      typename F::template apply<C35>::type{}(args...);
      break;
    case C36:
      typename F::template apply<C36>::type{}(args...);
      break;
    case C37:
      typename F::template apply<C37>::type{}(args...);
      break;
    case C38:
      typename F::template apply<C38>::type{}(args...);
      break;
    case C39:
      typename F::template apply<C39>::type{}(args...);
      break;
    case C40:
      typename F::template apply<C40>::type{}(args...);
      break;
    case C41:
      typename F::template apply<C41>::type{}(args...);
      break;
    case C42:
      typename F::template apply<C42>::type{}(args...);
      break;
    case C43:
      typename F::template apply<C43>::type{}(args...);
      break;
    case C44:
      typename F::template apply<C44>::type{}(args...);
      break;
    case C45:
      typename F::template apply<C45>::type{}(args...);
      break;
    case C46:
      typename F::template apply<C46>::type{}(args...);
      break;
    case C47:
      typename F::template apply<C47>::type{}(args...);
      break;
    case C48:
      typename F::template apply<C48>::type{}(args...);
      break;
    case C49:
      typename F::template apply<C49>::type{}(args...);
      break;
    case C50:
      typename F::template apply<C50>::type{}(args...);
      break;
    case C51:
      typename F::template apply<C51>::type{}(args...);
      break;
    case C52:
      typename F::template apply<C52>::type{}(args...);
      break;
    case C53:
      typename F::template apply<C53>::type{}(args...);
      break;
    case C54:
      typename F::template apply<C54>::type{}(args...);
      break;
    case C55:
      typename F::template apply<C55>::type{}(args...);
      break;
    case C56:
      typename F::template apply<C56>::type{}(args...);
      break;
    case C57:
      typename F::template apply<C57>::type{}(args...);
      break;
    case C58:
      typename F::template apply<C58>::type{}(args...);
      break;
    case C59:
      typename F::template apply<C59>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27, char C28, char C29, char C30, char C31, char C32, char C33, char C34, char C35, char C36, char C37, char C38, char C39, char C40, char C41, char C42, char C43, char C44, char C45, char C46, char C47, char C48, char C49, char C50, char C51, char C52, char C53, char C54, char C55, char C56, char C57, char C58, char C59, char C60>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31, C32, C33, C34, C35, C36, C37, C38, C39, C40, C41, C42, C43, C44, C45, C46, C47, C48, C49, C50, C51, C52, C53, C54, C55, C56, C57, C58, C59, C60>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    case C28:
      typename F::template apply<C28>::type{}(args...);
      break;
    case C29:
      typename F::template apply<C29>::type{}(args...);
      break;
    case C30:
      typename F::template apply<C30>::type{}(args...);
      break;
    case C31:
      typename F::template apply<C31>::type{}(args...);
      break;
    case C32:
      typename F::template apply<C32>::type{}(args...);
      break;
    case C33:
      typename F::template apply<C33>::type{}(args...);
      break;
    case C34:
      typename F::template apply<C34>::type{}(args...);
      break;
    case C35:
      typename F::template apply<C35>::type{}(args...);
      break;
    case C36:
      typename F::template apply<C36>::type{}(args...);
      break;
    case C37:
      typename F::template apply<C37>::type{}(args...);
      break;
    case C38:
      typename F::template apply<C38>::type{}(args...);
      break;
    case C39:
      typename F::template apply<C39>::type{}(args...);
      break;
    case C40:
      typename F::template apply<C40>::type{}(args...);
      break;
    case C41:
      typename F::template apply<C41>::type{}(args...);
      break;
    case C42:
      typename F::template apply<C42>::type{}(args...);
      break;
    case C43:
      typename F::template apply<C43>::type{}(args...);
      break;
    case C44:
      typename F::template apply<C44>::type{}(args...);
      break;
    case C45:
      typename F::template apply<C45>::type{}(args...);
      break;
    case C46:
      typename F::template apply<C46>::type{}(args...);
      break;
    case C47:
      typename F::template apply<C47>::type{}(args...);
      break;
    case C48:
      typename F::template apply<C48>::type{}(args...);
      break;
    case C49:
      typename F::template apply<C49>::type{}(args...);
      break;
    case C50:
      typename F::template apply<C50>::type{}(args...);
      break;
    case C51:
      typename F::template apply<C51>::type{}(args...);
      break;
    case C52:
      typename F::template apply<C52>::type{}(args...);
      break;
    case C53:
      typename F::template apply<C53>::type{}(args...);
      break;
    case C54:
      typename F::template apply<C54>::type{}(args...);
      break;
    case C55:
      typename F::template apply<C55>::type{}(args...);
      break;
    case C56:
      typename F::template apply<C56>::type{}(args...);
      break;
    case C57:
      typename F::template apply<C57>::type{}(args...);
      break;
    case C58:
      typename F::template apply<C58>::type{}(args...);
      break;
    case C59:
      typename F::template apply<C59>::type{}(args...);
      break;
    case C60:
      typename F::template apply<C60>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27, char C28, char C29, char C30, char C31, char C32, char C33, char C34, char C35, char C36, char C37, char C38, char C39, char C40, char C41, char C42, char C43, char C44, char C45, char C46, char C47, char C48, char C49, char C50, char C51, char C52, char C53, char C54, char C55, char C56, char C57, char C58, char C59, char C60, char C61>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31, C32, C33, C34, C35, C36, C37, C38, C39, C40, C41, C42, C43, C44, C45, C46, C47, C48, C49, C50, C51, C52, C53, C54, C55, C56, C57, C58, C59, C60, C61>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    case C28:
      typename F::template apply<C28>::type{}(args...);
      break;
    case C29:
      typename F::template apply<C29>::type{}(args...);
      break;
    case C30:
      typename F::template apply<C30>::type{}(args...);
      break;
    case C31:
      typename F::template apply<C31>::type{}(args...);
      break;
    case C32:
      typename F::template apply<C32>::type{}(args...);
      break;
    case C33:
      typename F::template apply<C33>::type{}(args...);
      break;
    case C34:
      typename F::template apply<C34>::type{}(args...);
      break;
    case C35:
      typename F::template apply<C35>::type{}(args...);
      break;
    case C36:
      typename F::template apply<C36>::type{}(args...);
      break;
    case C37:
      typename F::template apply<C37>::type{}(args...);
      break;
    case C38:
      typename F::template apply<C38>::type{}(args...);
      break;
    case C39:
      typename F::template apply<C39>::type{}(args...);
      break;
    case C40:
      typename F::template apply<C40>::type{}(args...);
      break;
    case C41:
      typename F::template apply<C41>::type{}(args...);
      break;
    case C42:
      typename F::template apply<C42>::type{}(args...);
      break;
    case C43:
      typename F::template apply<C43>::type{}(args...);
      break;
    case C44:
      typename F::template apply<C44>::type{}(args...);
      break;
    case C45:
      typename F::template apply<C45>::type{}(args...);
      break;
    case C46:
      typename F::template apply<C46>::type{}(args...);
      break;
    case C47:
      typename F::template apply<C47>::type{}(args...);
      break;
    case C48:
      typename F::template apply<C48>::type{}(args...);
      break;
    case C49:
      typename F::template apply<C49>::type{}(args...);
      break;
    case C50:
      typename F::template apply<C50>::type{}(args...);
      break;
    case C51:
      typename F::template apply<C51>::type{}(args...);
      break;
    case C52:
      typename F::template apply<C52>::type{}(args...);
      break;
    case C53:
      typename F::template apply<C53>::type{}(args...);
      break;
    case C54:
      typename F::template apply<C54>::type{}(args...);
      break;
    case C55:
      typename F::template apply<C55>::type{}(args...);
      break;
    case C56:
      typename F::template apply<C56>::type{}(args...);
      break;
    case C57:
      typename F::template apply<C57>::type{}(args...);
      break;
    case C58:
      typename F::template apply<C58>::type{}(args...);
      break;
    case C59:
      typename F::template apply<C59>::type{}(args...);
      break;
    case C60:
      typename F::template apply<C60>::type{}(args...);
      break;
    case C61:
      typename F::template apply<C61>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27, char C28, char C29, char C30, char C31, char C32, char C33, char C34, char C35, char C36, char C37, char C38, char C39, char C40, char C41, char C42, char C43, char C44, char C45, char C46, char C47, char C48, char C49, char C50, char C51, char C52, char C53, char C54, char C55, char C56, char C57, char C58, char C59, char C60, char C61, char C62>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31, C32, C33, C34, C35, C36, C37, C38, C39, C40, C41, C42, C43, C44, C45, C46, C47, C48, C49, C50, C51, C52, C53, C54, C55, C56, C57, C58, C59, C60, C61, C62>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    case C28:
      typename F::template apply<C28>::type{}(args...);
      break;
    case C29:
      typename F::template apply<C29>::type{}(args...);
      break;
    case C30:
      typename F::template apply<C30>::type{}(args...);
      break;
    case C31:
      typename F::template apply<C31>::type{}(args...);
      break;
    case C32:
      typename F::template apply<C32>::type{}(args...);
      break;
    case C33:
      typename F::template apply<C33>::type{}(args...);
      break;
    case C34:
      typename F::template apply<C34>::type{}(args...);
      break;
    case C35:
      typename F::template apply<C35>::type{}(args...);
      break;
    case C36:
      typename F::template apply<C36>::type{}(args...);
      break;
    case C37:
      typename F::template apply<C37>::type{}(args...);
      break;
    case C38:
      typename F::template apply<C38>::type{}(args...);
      break;
    case C39:
      typename F::template apply<C39>::type{}(args...);
      break;
    case C40:
      typename F::template apply<C40>::type{}(args...);
      break;
    case C41:
      typename F::template apply<C41>::type{}(args...);
      break;
    case C42:
      typename F::template apply<C42>::type{}(args...);
      break;
    case C43:
      typename F::template apply<C43>::type{}(args...);
      break;
    case C44:
      typename F::template apply<C44>::type{}(args...);
      break;
    case C45:
      typename F::template apply<C45>::type{}(args...);
      break;
    case C46:
      typename F::template apply<C46>::type{}(args...);
      break;
    case C47:
      typename F::template apply<C47>::type{}(args...);
      break;
    case C48:
      typename F::template apply<C48>::type{}(args...);
      break;
    case C49:
      typename F::template apply<C49>::type{}(args...);
      break;
    case C50:
      typename F::template apply<C50>::type{}(args...);
      break;
    case C51:
      typename F::template apply<C51>::type{}(args...);
      break;
    case C52:
      typename F::template apply<C52>::type{}(args...);
      break;
    case C53:
      typename F::template apply<C53>::type{}(args...);
      break;
    case C54:
      typename F::template apply<C54>::type{}(args...);
      break;
    case C55:
      typename F::template apply<C55>::type{}(args...);
      break;
    case C56:
      typename F::template apply<C56>::type{}(args...);
      break;
    case C57:
      typename F::template apply<C57>::type{}(args...);
      break;
    case C58:
      typename F::template apply<C58>::type{}(args...);
      break;
    case C59:
      typename F::template apply<C59>::type{}(args...);
      break;
    case C60:
      typename F::template apply<C60>::type{}(args...);
      break;
    case C61:
      typename F::template apply<C61>::type{}(args...);
      break;
    case C62:
      typename F::template apply<C62>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27, char C28, char C29, char C30, char C31, char C32, char C33, char C34, char C35, char C36, char C37, char C38, char C39, char C40, char C41, char C42, char C43, char C44, char C45, char C46, char C47, char C48, char C49, char C50, char C51, char C52, char C53, char C54, char C55, char C56, char C57, char C58, char C59, char C60, char C61, char C62, char C63>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31, C32, C33, C34, C35, C36, C37, C38, C39, C40, C41, C42, C43, C44, C45, C46, C47, C48, C49, C50, C51, C52, C53, C54, C55, C56, C57, C58, C59, C60, C61, C62, C63>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    case C28:
      typename F::template apply<C28>::type{}(args...);
      break;
    case C29:
      typename F::template apply<C29>::type{}(args...);
      break;
    case C30:
      typename F::template apply<C30>::type{}(args...);
      break;
    case C31:
      typename F::template apply<C31>::type{}(args...);
      break;
    case C32:
      typename F::template apply<C32>::type{}(args...);
      break;
    case C33:
      typename F::template apply<C33>::type{}(args...);
      break;
    case C34:
      typename F::template apply<C34>::type{}(args...);
      break;
    case C35:
      typename F::template apply<C35>::type{}(args...);
      break;
    case C36:
      typename F::template apply<C36>::type{}(args...);
      break;
    case C37:
      typename F::template apply<C37>::type{}(args...);
      break;
    case C38:
      typename F::template apply<C38>::type{}(args...);
      break;
    case C39:
      typename F::template apply<C39>::type{}(args...);
      break;
    case C40:
      typename F::template apply<C40>::type{}(args...);
      break;
    case C41:
      typename F::template apply<C41>::type{}(args...);
      break;
    case C42:
      typename F::template apply<C42>::type{}(args...);
      break;
    case C43:
      typename F::template apply<C43>::type{}(args...);
      break;
    case C44:
      typename F::template apply<C44>::type{}(args...);
      break;
    case C45:
      typename F::template apply<C45>::type{}(args...);
      break;
    case C46:
      typename F::template apply<C46>::type{}(args...);
      break;
    case C47:
      typename F::template apply<C47>::type{}(args...);
      break;
    case C48:
      typename F::template apply<C48>::type{}(args...);
      break;
    case C49:
      typename F::template apply<C49>::type{}(args...);
      break;
    case C50:
      typename F::template apply<C50>::type{}(args...);
      break;
    case C51:
      typename F::template apply<C51>::type{}(args...);
      break;
    case C52:
      typename F::template apply<C52>::type{}(args...);
      break;
    case C53:
      typename F::template apply<C53>::type{}(args...);
      break;
    case C54:
      typename F::template apply<C54>::type{}(args...);
      break;
    case C55:
      typename F::template apply<C55>::type{}(args...);
      break;
    case C56:
      typename F::template apply<C56>::type{}(args...);
      break;
    case C57:
      typename F::template apply<C57>::type{}(args...);
      break;
    case C58:
      typename F::template apply<C58>::type{}(args...);
      break;
    case C59:
      typename F::template apply<C59>::type{}(args...);
      break;
    case C60:
      typename F::template apply<C60>::type{}(args...);
      break;
    case C61:
      typename F::template apply<C61>::type{}(args...);
      break;
    case C62:
      typename F::template apply<C62>::type{}(args...);
      break;
    case C63:
      typename F::template apply<C63>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27, char C28, char C29, char C30, char C31, char C32, char C33, char C34, char C35, char C36, char C37, char C38, char C39, char C40, char C41, char C42, char C43, char C44, char C45, char C46, char C47, char C48, char C49, char C50, char C51, char C52, char C53, char C54, char C55, char C56, char C57, char C58, char C59, char C60, char C61, char C62, char C63, char C64>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31, C32, C33, C34, C35, C36, C37, C38, C39, C40, C41, C42, C43, C44, C45, C46, C47, C48, C49, C50, C51, C52, C53, C54, C55, C56, C57, C58, C59, C60, C61, C62, C63, C64>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    case C28:
      typename F::template apply<C28>::type{}(args...);
      break;
    case C29:
      typename F::template apply<C29>::type{}(args...);
      break;
    case C30:
      typename F::template apply<C30>::type{}(args...);
      break;
    case C31:
      typename F::template apply<C31>::type{}(args...);
      break;
    case C32:
      typename F::template apply<C32>::type{}(args...);
      break;
    case C33:
      typename F::template apply<C33>::type{}(args...);
      break;
    case C34:
      typename F::template apply<C34>::type{}(args...);
      break;
    case C35:
      typename F::template apply<C35>::type{}(args...);
      break;
    case C36:
      typename F::template apply<C36>::type{}(args...);
      break;
    case C37:
      typename F::template apply<C37>::type{}(args...);
      break;
    case C38:
      typename F::template apply<C38>::type{}(args...);
      break;
    case C39:
      typename F::template apply<C39>::type{}(args...);
      break;
    case C40:
      typename F::template apply<C40>::type{}(args...);
      break;
    case C41:
      typename F::template apply<C41>::type{}(args...);
      break;
    case C42:
      typename F::template apply<C42>::type{}(args...);
      break;
    case C43:
      typename F::template apply<C43>::type{}(args...);
      break;
    case C44:
      typename F::template apply<C44>::type{}(args...);
      break;
    case C45:
      typename F::template apply<C45>::type{}(args...);
      break;
    case C46:
      typename F::template apply<C46>::type{}(args...);
      break;
    case C47:
      typename F::template apply<C47>::type{}(args...);
      break;
    case C48:
      typename F::template apply<C48>::type{}(args...);
      break;
    case C49:
      typename F::template apply<C49>::type{}(args...);
      break;
    case C50:
      typename F::template apply<C50>::type{}(args...);
      break;
    case C51:
      typename F::template apply<C51>::type{}(args...);
      break;
    case C52:
      typename F::template apply<C52>::type{}(args...);
      break;
    case C53:
      typename F::template apply<C53>::type{}(args...);
      break;
    case C54:
      typename F::template apply<C54>::type{}(args...);
      break;
    case C55:
      typename F::template apply<C55>::type{}(args...);
      break;
    case C56:
      typename F::template apply<C56>::type{}(args...);
      break;
    case C57:
      typename F::template apply<C57>::type{}(args...);
      break;
    case C58:
      typename F::template apply<C58>::type{}(args...);
      break;
    case C59:
      typename F::template apply<C59>::type{}(args...);
      break;
    case C60:
      typename F::template apply<C60>::type{}(args...);
      break;
    case C61:
      typename F::template apply<C61>::type{}(args...);
      break;
    case C62:
      typename F::template apply<C62>::type{}(args...);
      break;
    case C63:
      typename F::template apply<C63>::type{}(args...);
      break;
    case C64:
      typename F::template apply<C64>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27, char C28, char C29, char C30, char C31, char C32, char C33, char C34, char C35, char C36, char C37, char C38, char C39, char C40, char C41, char C42, char C43, char C44, char C45, char C46, char C47, char C48, char C49, char C50, char C51, char C52, char C53, char C54, char C55, char C56, char C57, char C58, char C59, char C60, char C61, char C62, char C63, char C64, char C65>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31, C32, C33, C34, C35, C36, C37, C38, C39, C40, C41, C42, C43, C44, C45, C46, C47, C48, C49, C50, C51, C52, C53, C54, C55, C56, C57, C58, C59, C60, C61, C62, C63, C64, C65>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    case C28:
      typename F::template apply<C28>::type{}(args...);
      break;
    case C29:
      typename F::template apply<C29>::type{}(args...);
      break;
    case C30:
      typename F::template apply<C30>::type{}(args...);
      break;
    case C31:
      typename F::template apply<C31>::type{}(args...);
      break;
    case C32:
      typename F::template apply<C32>::type{}(args...);
      break;
    case C33:
      typename F::template apply<C33>::type{}(args...);
      break;
    case C34:
      typename F::template apply<C34>::type{}(args...);
      break;
    case C35:
      typename F::template apply<C35>::type{}(args...);
      break;
    case C36:
      typename F::template apply<C36>::type{}(args...);
      break;
    case C37:
      typename F::template apply<C37>::type{}(args...);
      break;
    case C38:
      typename F::template apply<C38>::type{}(args...);
      break;
    case C39:
      typename F::template apply<C39>::type{}(args...);
      break;
    case C40:
      typename F::template apply<C40>::type{}(args...);
      break;
    case C41:
      typename F::template apply<C41>::type{}(args...);
      break;
    case C42:
      typename F::template apply<C42>::type{}(args...);
      break;
    case C43:
      typename F::template apply<C43>::type{}(args...);
      break;
    case C44:
      typename F::template apply<C44>::type{}(args...);
      break;
    case C45:
      typename F::template apply<C45>::type{}(args...);
      break;
    case C46:
      typename F::template apply<C46>::type{}(args...);
      break;
    case C47:
      typename F::template apply<C47>::type{}(args...);
      break;
    case C48:
      typename F::template apply<C48>::type{}(args...);
      break;
    case C49:
      typename F::template apply<C49>::type{}(args...);
      break;
    case C50:
      typename F::template apply<C50>::type{}(args...);
      break;
    case C51:
      typename F::template apply<C51>::type{}(args...);
      break;
    case C52:
      typename F::template apply<C52>::type{}(args...);
      break;
    case C53:
      typename F::template apply<C53>::type{}(args...);
      break;
    case C54:
      typename F::template apply<C54>::type{}(args...);
      break;
    case C55:
      typename F::template apply<C55>::type{}(args...);
      break;
    case C56:
      typename F::template apply<C56>::type{}(args...);
      break;
    case C57:
      typename F::template apply<C57>::type{}(args...);
      break;
    case C58:
      typename F::template apply<C58>::type{}(args...);
      break;
    case C59:
      typename F::template apply<C59>::type{}(args...);
      break;
    case C60:
      typename F::template apply<C60>::type{}(args...);
      break;
    case C61:
      typename F::template apply<C61>::type{}(args...);
      break;
    case C62:
      typename F::template apply<C62>::type{}(args...);
      break;
    case C63:
      typename F::template apply<C63>::type{}(args...);
      break;
    case C64:
      typename F::template apply<C64>::type{}(args...);
      break;
    case C65:
      typename F::template apply<C65>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27, char C28, char C29, char C30, char C31, char C32, char C33, char C34, char C35, char C36, char C37, char C38, char C39, char C40, char C41, char C42, char C43, char C44, char C45, char C46, char C47, char C48, char C49, char C50, char C51, char C52, char C53, char C54, char C55, char C56, char C57, char C58, char C59, char C60, char C61, char C62, char C63, char C64, char C65, char C66>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31, C32, C33, C34, C35, C36, C37, C38, C39, C40, C41, C42, C43, C44, C45, C46, C47, C48, C49, C50, C51, C52, C53, C54, C55, C56, C57, C58, C59, C60, C61, C62, C63, C64, C65, C66>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    case C28:
      typename F::template apply<C28>::type{}(args...);
      break;
    case C29:
      typename F::template apply<C29>::type{}(args...);
      break;
    case C30:
      typename F::template apply<C30>::type{}(args...);
      break;
    case C31:
      typename F::template apply<C31>::type{}(args...);
      break;
    case C32:
      typename F::template apply<C32>::type{}(args...);
      break;
    case C33:
      typename F::template apply<C33>::type{}(args...);
      break;
    case C34:
      typename F::template apply<C34>::type{}(args...);
      break;
    case C35:
      typename F::template apply<C35>::type{}(args...);
      break;
    case C36:
      typename F::template apply<C36>::type{}(args...);
      break;
    case C37:
      typename F::template apply<C37>::type{}(args...);
      break;
    case C38:
      typename F::template apply<C38>::type{}(args...);
      break;
    case C39:
      typename F::template apply<C39>::type{}(args...);
      break;
    case C40:
      typename F::template apply<C40>::type{}(args...);
      break;
    case C41:
      typename F::template apply<C41>::type{}(args...);
      break;
    case C42:
      typename F::template apply<C42>::type{}(args...);
      break;
    case C43:
      typename F::template apply<C43>::type{}(args...);
      break;
    case C44:
      typename F::template apply<C44>::type{}(args...);
      break;
    case C45:
      typename F::template apply<C45>::type{}(args...);
      break;
    case C46:
      typename F::template apply<C46>::type{}(args...);
      break;
    case C47:
      typename F::template apply<C47>::type{}(args...);
      break;
    case C48:
      typename F::template apply<C48>::type{}(args...);
      break;
    case C49:
      typename F::template apply<C49>::type{}(args...);
      break;
    case C50:
      typename F::template apply<C50>::type{}(args...);
      break;
    case C51:
      typename F::template apply<C51>::type{}(args...);
      break;
    case C52:
      typename F::template apply<C52>::type{}(args...);
      break;
    case C53:
      typename F::template apply<C53>::type{}(args...);
      break;
    case C54:
      typename F::template apply<C54>::type{}(args...);
      break;
    case C55:
      typename F::template apply<C55>::type{}(args...);
      break;
    case C56:
      typename F::template apply<C56>::type{}(args...);
      break;
    case C57:
      typename F::template apply<C57>::type{}(args...);
      break;
    case C58:
      typename F::template apply<C58>::type{}(args...);
      break;
    case C59:
      typename F::template apply<C59>::type{}(args...);
      break;
    case C60:
      typename F::template apply<C60>::type{}(args...);
      break;
    case C61:
      typename F::template apply<C61>::type{}(args...);
      break;
    case C62:
      typename F::template apply<C62>::type{}(args...);
      break;
    case C63:
      typename F::template apply<C63>::type{}(args...);
      break;
    case C64:
      typename F::template apply<C64>::type{}(args...);
      break;
    case C65:
      typename F::template apply<C65>::type{}(args...);
      break;
    case C66:
      typename F::template apply<C66>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27, char C28, char C29, char C30, char C31, char C32, char C33, char C34, char C35, char C36, char C37, char C38, char C39, char C40, char C41, char C42, char C43, char C44, char C45, char C46, char C47, char C48, char C49, char C50, char C51, char C52, char C53, char C54, char C55, char C56, char C57, char C58, char C59, char C60, char C61, char C62, char C63, char C64, char C65, char C66, char C67>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31, C32, C33, C34, C35, C36, C37, C38, C39, C40, C41, C42, C43, C44, C45, C46, C47, C48, C49, C50, C51, C52, C53, C54, C55, C56, C57, C58, C59, C60, C61, C62, C63, C64, C65, C66, C67>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    case C28:
      typename F::template apply<C28>::type{}(args...);
      break;
    case C29:
      typename F::template apply<C29>::type{}(args...);
      break;
    case C30:
      typename F::template apply<C30>::type{}(args...);
      break;
    case C31:
      typename F::template apply<C31>::type{}(args...);
      break;
    case C32:
      typename F::template apply<C32>::type{}(args...);
      break;
    case C33:
      typename F::template apply<C33>::type{}(args...);
      break;
    case C34:
      typename F::template apply<C34>::type{}(args...);
      break;
    case C35:
      typename F::template apply<C35>::type{}(args...);
      break;
    case C36:
      typename F::template apply<C36>::type{}(args...);
      break;
    case C37:
      typename F::template apply<C37>::type{}(args...);
      break;
    case C38:
      typename F::template apply<C38>::type{}(args...);
      break;
    case C39:
      typename F::template apply<C39>::type{}(args...);
      break;
    case C40:
      typename F::template apply<C40>::type{}(args...);
      break;
    case C41:
      typename F::template apply<C41>::type{}(args...);
      break;
    case C42:
      typename F::template apply<C42>::type{}(args...);
      break;
    case C43:
      typename F::template apply<C43>::type{}(args...);
      break;
    case C44:
      typename F::template apply<C44>::type{}(args...);
      break;
    case C45:
      typename F::template apply<C45>::type{}(args...);
      break;
    case C46:
      typename F::template apply<C46>::type{}(args...);
      break;
    case C47:
      typename F::template apply<C47>::type{}(args...);
      break;
    case C48:
      typename F::template apply<C48>::type{}(args...);
      break;
    case C49:
      typename F::template apply<C49>::type{}(args...);
      break;
    case C50:
      typename F::template apply<C50>::type{}(args...);
      break;
    case C51:
      typename F::template apply<C51>::type{}(args...);
      break;
    case C52:
      typename F::template apply<C52>::type{}(args...);
      break;
    case C53:
      typename F::template apply<C53>::type{}(args...);
      break;
    case C54:
      typename F::template apply<C54>::type{}(args...);
      break;
    case C55:
      typename F::template apply<C55>::type{}(args...);
      break;
    case C56:
      typename F::template apply<C56>::type{}(args...);
      break;
    case C57:
      typename F::template apply<C57>::type{}(args...);
      break;
    case C58:
      typename F::template apply<C58>::type{}(args...);
      break;
    case C59:
      typename F::template apply<C59>::type{}(args...);
      break;
    case C60:
      typename F::template apply<C60>::type{}(args...);
      break;
    case C61:
      typename F::template apply<C61>::type{}(args...);
      break;
    case C62:
      typename F::template apply<C62>::type{}(args...);
      break;
    case C63:
      typename F::template apply<C63>::type{}(args...);
      break;
    case C64:
      typename F::template apply<C64>::type{}(args...);
      break;
    case C65:
      typename F::template apply<C65>::type{}(args...);
      break;
    case C66:
      typename F::template apply<C66>::type{}(args...);
      break;
    case C67:
      typename F::template apply<C67>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27, char C28, char C29, char C30, char C31, char C32, char C33, char C34, char C35, char C36, char C37, char C38, char C39, char C40, char C41, char C42, char C43, char C44, char C45, char C46, char C47, char C48, char C49, char C50, char C51, char C52, char C53, char C54, char C55, char C56, char C57, char C58, char C59, char C60, char C61, char C62, char C63, char C64, char C65, char C66, char C67, char C68>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31, C32, C33, C34, C35, C36, C37, C38, C39, C40, C41, C42, C43, C44, C45, C46, C47, C48, C49, C50, C51, C52, C53, C54, C55, C56, C57, C58, C59, C60, C61, C62, C63, C64, C65, C66, C67, C68>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    case C28:
      typename F::template apply<C28>::type{}(args...);
      break;
    case C29:
      typename F::template apply<C29>::type{}(args...);
      break;
    case C30:
      typename F::template apply<C30>::type{}(args...);
      break;
    case C31:
      typename F::template apply<C31>::type{}(args...);
      break;
    case C32:
      typename F::template apply<C32>::type{}(args...);
      break;
    case C33:
      typename F::template apply<C33>::type{}(args...);
      break;
    case C34:
      typename F::template apply<C34>::type{}(args...);
      break;
    case C35:
      typename F::template apply<C35>::type{}(args...);
      break;
    case C36:
      typename F::template apply<C36>::type{}(args...);
      break;
    case C37:
      typename F::template apply<C37>::type{}(args...);
      break;
    case C38:
      typename F::template apply<C38>::type{}(args...);
      break;
    case C39:
      typename F::template apply<C39>::type{}(args...);
      break;
    case C40:
      typename F::template apply<C40>::type{}(args...);
      break;
    case C41:
      typename F::template apply<C41>::type{}(args...);
      break;
    case C42:
      typename F::template apply<C42>::type{}(args...);
      break;
    case C43:
      typename F::template apply<C43>::type{}(args...);
      break;
    case C44:
      typename F::template apply<C44>::type{}(args...);
      break;
    case C45:
      typename F::template apply<C45>::type{}(args...);
      break;
    case C46:
      typename F::template apply<C46>::type{}(args...);
      break;
    case C47:
      typename F::template apply<C47>::type{}(args...);
      break;
    case C48:
      typename F::template apply<C48>::type{}(args...);
      break;
    case C49:
      typename F::template apply<C49>::type{}(args...);
      break;
    case C50:
      typename F::template apply<C50>::type{}(args...);
      break;
    case C51:
      typename F::template apply<C51>::type{}(args...);
      break;
    case C52:
      typename F::template apply<C52>::type{}(args...);
      break;
    case C53:
      typename F::template apply<C53>::type{}(args...);
      break;
    case C54:
      typename F::template apply<C54>::type{}(args...);
      break;
    case C55:
      typename F::template apply<C55>::type{}(args...);
      break;
    case C56:
      typename F::template apply<C56>::type{}(args...);
      break;
    case C57:
      typename F::template apply<C57>::type{}(args...);
      break;
    case C58:
      typename F::template apply<C58>::type{}(args...);
      break;
    case C59:
      typename F::template apply<C59>::type{}(args...);
      break;
    case C60:
      typename F::template apply<C60>::type{}(args...);
      break;
    case C61:
      typename F::template apply<C61>::type{}(args...);
      break;
    case C62:
      typename F::template apply<C62>::type{}(args...);
      break;
    case C63:
      typename F::template apply<C63>::type{}(args...);
      break;
    case C64:
      typename F::template apply<C64>::type{}(args...);
      break;
    case C65:
      typename F::template apply<C65>::type{}(args...);
      break;
    case C66:
      typename F::template apply<C66>::type{}(args...);
      break;
    case C67:
      typename F::template apply<C67>::type{}(args...);
      break;
    case C68:
      typename F::template apply<C68>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27, char C28, char C29, char C30, char C31, char C32, char C33, char C34, char C35, char C36, char C37, char C38, char C39, char C40, char C41, char C42, char C43, char C44, char C45, char C46, char C47, char C48, char C49, char C50, char C51, char C52, char C53, char C54, char C55, char C56, char C57, char C58, char C59, char C60, char C61, char C62, char C63, char C64, char C65, char C66, char C67, char C68, char C69>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31, C32, C33, C34, C35, C36, C37, C38, C39, C40, C41, C42, C43, C44, C45, C46, C47, C48, C49, C50, C51, C52, C53, C54, C55, C56, C57, C58, C59, C60, C61, C62, C63, C64, C65, C66, C67, C68, C69>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    case C28:
      typename F::template apply<C28>::type{}(args...);
      break;
    case C29:
      typename F::template apply<C29>::type{}(args...);
      break;
    case C30:
      typename F::template apply<C30>::type{}(args...);
      break;
    case C31:
      typename F::template apply<C31>::type{}(args...);
      break;
    case C32:
      typename F::template apply<C32>::type{}(args...);
      break;
    case C33:
      typename F::template apply<C33>::type{}(args...);
      break;
    case C34:
      typename F::template apply<C34>::type{}(args...);
      break;
    case C35:
      typename F::template apply<C35>::type{}(args...);
      break;
    case C36:
      typename F::template apply<C36>::type{}(args...);
      break;
    case C37:
      typename F::template apply<C37>::type{}(args...);
      break;
    case C38:
      typename F::template apply<C38>::type{}(args...);
      break;
    case C39:
      typename F::template apply<C39>::type{}(args...);
      break;
    case C40:
      typename F::template apply<C40>::type{}(args...);
      break;
    case C41:
      typename F::template apply<C41>::type{}(args...);
      break;
    case C42:
      typename F::template apply<C42>::type{}(args...);
      break;
    case C43:
      typename F::template apply<C43>::type{}(args...);
      break;
    case C44:
      typename F::template apply<C44>::type{}(args...);
      break;
    case C45:
      typename F::template apply<C45>::type{}(args...);
      break;
    case C46:
      typename F::template apply<C46>::type{}(args...);
      break;
    case C47:
      typename F::template apply<C47>::type{}(args...);
      break;
    case C48:
      typename F::template apply<C48>::type{}(args...);
      break;
    case C49:
      typename F::template apply<C49>::type{}(args...);
      break;
    case C50:
      typename F::template apply<C50>::type{}(args...);
      break;
    case C51:
      typename F::template apply<C51>::type{}(args...);
      break;
    case C52:
      typename F::template apply<C52>::type{}(args...);
      break;
    case C53:
      typename F::template apply<C53>::type{}(args...);
      break;
    case C54:
      typename F::template apply<C54>::type{}(args...);
      break;
    case C55:
      typename F::template apply<C55>::type{}(args...);
      break;
    case C56:
      typename F::template apply<C56>::type{}(args...);
      break;
    case C57:
      typename F::template apply<C57>::type{}(args...);
      break;
    case C58:
      typename F::template apply<C58>::type{}(args...);
      break;
    case C59:
      typename F::template apply<C59>::type{}(args...);
      break;
    case C60:
      typename F::template apply<C60>::type{}(args...);
      break;
    case C61:
      typename F::template apply<C61>::type{}(args...);
      break;
    case C62:
      typename F::template apply<C62>::type{}(args...);
      break;
    case C63:
      typename F::template apply<C63>::type{}(args...);
      break;
    case C64:
      typename F::template apply<C64>::type{}(args...);
      break;
    case C65:
      typename F::template apply<C65>::type{}(args...);
      break;
    case C66:
      typename F::template apply<C66>::type{}(args...);
      break;
    case C67:
      typename F::template apply<C67>::type{}(args...);
      break;
    case C68:
      typename F::template apply<C68>::type{}(args...);
      break;
    case C69:
      typename F::template apply<C69>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27, char C28, char C29, char C30, char C31, char C32, char C33, char C34, char C35, char C36, char C37, char C38, char C39, char C40, char C41, char C42, char C43, char C44, char C45, char C46, char C47, char C48, char C49, char C50, char C51, char C52, char C53, char C54, char C55, char C56, char C57, char C58, char C59, char C60, char C61, char C62, char C63, char C64, char C65, char C66, char C67, char C68, char C69, char C70>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31, C32, C33, C34, C35, C36, C37, C38, C39, C40, C41, C42, C43, C44, C45, C46, C47, C48, C49, C50, C51, C52, C53, C54, C55, C56, C57, C58, C59, C60, C61, C62, C63, C64, C65, C66, C67, C68, C69, C70>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    case C28:
      typename F::template apply<C28>::type{}(args...);
      break;
    case C29:
      typename F::template apply<C29>::type{}(args...);
      break;
    case C30:
      typename F::template apply<C30>::type{}(args...);
      break;
    case C31:
      typename F::template apply<C31>::type{}(args...);
      break;
    case C32:
      typename F::template apply<C32>::type{}(args...);
      break;
    case C33:
      typename F::template apply<C33>::type{}(args...);
      break;
    case C34:
      typename F::template apply<C34>::type{}(args...);
      break;
    case C35:
      typename F::template apply<C35>::type{}(args...);
      break;
    case C36:
      typename F::template apply<C36>::type{}(args...);
      break;
    case C37:
      typename F::template apply<C37>::type{}(args...);
      break;
    case C38:
      typename F::template apply<C38>::type{}(args...);
      break;
    case C39:
      typename F::template apply<C39>::type{}(args...);
      break;
    case C40:
      typename F::template apply<C40>::type{}(args...);
      break;
    case C41:
      typename F::template apply<C41>::type{}(args...);
      break;
    case C42:
      typename F::template apply<C42>::type{}(args...);
      break;
    case C43:
      typename F::template apply<C43>::type{}(args...);
      break;
    case C44:
      typename F::template apply<C44>::type{}(args...);
      break;
    case C45:
      typename F::template apply<C45>::type{}(args...);
      break;
    case C46:
      typename F::template apply<C46>::type{}(args...);
      break;
    case C47:
      typename F::template apply<C47>::type{}(args...);
      break;
    case C48:
      typename F::template apply<C48>::type{}(args...);
      break;
    case C49:
      typename F::template apply<C49>::type{}(args...);
      break;
    case C50:
      typename F::template apply<C50>::type{}(args...);
      break;
    case C51:
      typename F::template apply<C51>::type{}(args...);
      break;
    case C52:
      typename F::template apply<C52>::type{}(args...);
      break;
    case C53:
      typename F::template apply<C53>::type{}(args...);
      break;
    case C54:
      typename F::template apply<C54>::type{}(args...);
      break;
    case C55:
      typename F::template apply<C55>::type{}(args...);
      break;
    case C56:
      typename F::template apply<C56>::type{}(args...);
      break;
    case C57:
      typename F::template apply<C57>::type{}(args...);
      break;
    case C58:
      typename F::template apply<C58>::type{}(args...);
      break;
    case C59:
      typename F::template apply<C59>::type{}(args...);
      break;
    case C60:
      typename F::template apply<C60>::type{}(args...);
      break;
    case C61:
      typename F::template apply<C61>::type{}(args...);
      break;
    case C62:
      typename F::template apply<C62>::type{}(args...);
      break;
    case C63:
      typename F::template apply<C63>::type{}(args...);
      break;
    case C64:
      typename F::template apply<C64>::type{}(args...);
      break;
    case C65:
      typename F::template apply<C65>::type{}(args...);
      break;
    case C66:
      typename F::template apply<C66>::type{}(args...);
      break;
    case C67:
      typename F::template apply<C67>::type{}(args...);
      break;
    case C68:
      typename F::template apply<C68>::type{}(args...);
      break;
    case C69:
      typename F::template apply<C69>::type{}(args...);
      break;
    case C70:
      typename F::template apply<C70>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27, char C28, char C29, char C30, char C31, char C32, char C33, char C34, char C35, char C36, char C37, char C38, char C39, char C40, char C41, char C42, char C43, char C44, char C45, char C46, char C47, char C48, char C49, char C50, char C51, char C52, char C53, char C54, char C55, char C56, char C57, char C58, char C59, char C60, char C61, char C62, char C63, char C64, char C65, char C66, char C67, char C68, char C69, char C70, char C71>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31, C32, C33, C34, C35, C36, C37, C38, C39, C40, C41, C42, C43, C44, C45, C46, C47, C48, C49, C50, C51, C52, C53, C54, C55, C56, C57, C58, C59, C60, C61, C62, C63, C64, C65, C66, C67, C68, C69, C70, C71>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    case C28:
      typename F::template apply<C28>::type{}(args...);
      break;
    case C29:
      typename F::template apply<C29>::type{}(args...);
      break;
    case C30:
      typename F::template apply<C30>::type{}(args...);
      break;
    case C31:
      typename F::template apply<C31>::type{}(args...);
      break;
    case C32:
      typename F::template apply<C32>::type{}(args...);
      break;
    case C33:
      typename F::template apply<C33>::type{}(args...);
      break;
    case C34:
      typename F::template apply<C34>::type{}(args...);
      break;
    case C35:
      typename F::template apply<C35>::type{}(args...);
      break;
    case C36:
      typename F::template apply<C36>::type{}(args...);
      break;
    case C37:
      typename F::template apply<C37>::type{}(args...);
      break;
    case C38:
      typename F::template apply<C38>::type{}(args...);
      break;
    case C39:
      typename F::template apply<C39>::type{}(args...);
      break;
    case C40:
      typename F::template apply<C40>::type{}(args...);
      break;
    case C41:
      typename F::template apply<C41>::type{}(args...);
      break;
    case C42:
      typename F::template apply<C42>::type{}(args...);
      break;
    case C43:
      typename F::template apply<C43>::type{}(args...);
      break;
    case C44:
      typename F::template apply<C44>::type{}(args...);
      break;
    case C45:
      typename F::template apply<C45>::type{}(args...);
      break;
    case C46:
      typename F::template apply<C46>::type{}(args...);
      break;
    case C47:
      typename F::template apply<C47>::type{}(args...);
      break;
    case C48:
      typename F::template apply<C48>::type{}(args...);
      break;
    case C49:
      typename F::template apply<C49>::type{}(args...);
      break;
    case C50:
      typename F::template apply<C50>::type{}(args...);
      break;
    case C51:
      typename F::template apply<C51>::type{}(args...);
      break;
    case C52:
      typename F::template apply<C52>::type{}(args...);
      break;
    case C53:
      typename F::template apply<C53>::type{}(args...);
      break;
    case C54:
      typename F::template apply<C54>::type{}(args...);
      break;
    case C55:
      typename F::template apply<C55>::type{}(args...);
      break;
    case C56:
      typename F::template apply<C56>::type{}(args...);
      break;
    case C57:
      typename F::template apply<C57>::type{}(args...);
      break;
    case C58:
      typename F::template apply<C58>::type{}(args...);
      break;
    case C59:
      typename F::template apply<C59>::type{}(args...);
      break;
    case C60:
      typename F::template apply<C60>::type{}(args...);
      break;
    case C61:
      typename F::template apply<C61>::type{}(args...);
      break;
    case C62:
      typename F::template apply<C62>::type{}(args...);
      break;
    case C63:
      typename F::template apply<C63>::type{}(args...);
      break;
    case C64:
      typename F::template apply<C64>::type{}(args...);
      break;
    case C65:
      typename F::template apply<C65>::type{}(args...);
      break;
    case C66:
      typename F::template apply<C66>::type{}(args...);
      break;
    case C67:
      typename F::template apply<C67>::type{}(args...);
      break;
    case C68:
      typename F::template apply<C68>::type{}(args...);
      break;
    case C69:
      typename F::template apply<C69>::type{}(args...);
      break;
    case C70:
      typename F::template apply<C70>::type{}(args...);
      break;
    case C71:
      typename F::template apply<C71>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27, char C28, char C29, char C30, char C31, char C32, char C33, char C34, char C35, char C36, char C37, char C38, char C39, char C40, char C41, char C42, char C43, char C44, char C45, char C46, char C47, char C48, char C49, char C50, char C51, char C52, char C53, char C54, char C55, char C56, char C57, char C58, char C59, char C60, char C61, char C62, char C63, char C64, char C65, char C66, char C67, char C68, char C69, char C70, char C71, char C72>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31, C32, C33, C34, C35, C36, C37, C38, C39, C40, C41, C42, C43, C44, C45, C46, C47, C48, C49, C50, C51, C52, C53, C54, C55, C56, C57, C58, C59, C60, C61, C62, C63, C64, C65, C66, C67, C68, C69, C70, C71, C72>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    case C28:
      typename F::template apply<C28>::type{}(args...);
      break;
    case C29:
      typename F::template apply<C29>::type{}(args...);
      break;
    case C30:
      typename F::template apply<C30>::type{}(args...);
      break;
    case C31:
      typename F::template apply<C31>::type{}(args...);
      break;
    case C32:
      typename F::template apply<C32>::type{}(args...);
      break;
    case C33:
      typename F::template apply<C33>::type{}(args...);
      break;
    case C34:
      typename F::template apply<C34>::type{}(args...);
      break;
    case C35:
      typename F::template apply<C35>::type{}(args...);
      break;
    case C36:
      typename F::template apply<C36>::type{}(args...);
      break;
    case C37:
      typename F::template apply<C37>::type{}(args...);
      break;
    case C38:
      typename F::template apply<C38>::type{}(args...);
      break;
    case C39:
      typename F::template apply<C39>::type{}(args...);
      break;
    case C40:
      typename F::template apply<C40>::type{}(args...);
      break;
    case C41:
      typename F::template apply<C41>::type{}(args...);
      break;
    case C42:
      typename F::template apply<C42>::type{}(args...);
      break;
    case C43:
      typename F::template apply<C43>::type{}(args...);
      break;
    case C44:
      typename F::template apply<C44>::type{}(args...);
      break;
    case C45:
      typename F::template apply<C45>::type{}(args...);
      break;
    case C46:
      typename F::template apply<C46>::type{}(args...);
      break;
    case C47:
      typename F::template apply<C47>::type{}(args...);
      break;
    case C48:
      typename F::template apply<C48>::type{}(args...);
      break;
    case C49:
      typename F::template apply<C49>::type{}(args...);
      break;
    case C50:
      typename F::template apply<C50>::type{}(args...);
      break;
    case C51:
      typename F::template apply<C51>::type{}(args...);
      break;
    case C52:
      typename F::template apply<C52>::type{}(args...);
      break;
    case C53:
      typename F::template apply<C53>::type{}(args...);
      break;
    case C54:
      typename F::template apply<C54>::type{}(args...);
      break;
    case C55:
      typename F::template apply<C55>::type{}(args...);
      break;
    case C56:
      typename F::template apply<C56>::type{}(args...);
      break;
    case C57:
      typename F::template apply<C57>::type{}(args...);
      break;
    case C58:
      typename F::template apply<C58>::type{}(args...);
      break;
    case C59:
      typename F::template apply<C59>::type{}(args...);
      break;
    case C60:
      typename F::template apply<C60>::type{}(args...);
      break;
    case C61:
      typename F::template apply<C61>::type{}(args...);
      break;
    case C62:
      typename F::template apply<C62>::type{}(args...);
      break;
    case C63:
      typename F::template apply<C63>::type{}(args...);
      break;
    case C64:
      typename F::template apply<C64>::type{}(args...);
      break;
    case C65:
      typename F::template apply<C65>::type{}(args...);
      break;
    case C66:
      typename F::template apply<C66>::type{}(args...);
      break;
    case C67:
      typename F::template apply<C67>::type{}(args...);
      break;
    case C68:
      typename F::template apply<C68>::type{}(args...);
      break;
    case C69:
      typename F::template apply<C69>::type{}(args...);
      break;
    case C70:
      typename F::template apply<C70>::type{}(args...);
      break;
    case C71:
      typename F::template apply<C71>::type{}(args...);
      break;
    case C72:
      typename F::template apply<C72>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27, char C28, char C29, char C30, char C31, char C32, char C33, char C34, char C35, char C36, char C37, char C38, char C39, char C40, char C41, char C42, char C43, char C44, char C45, char C46, char C47, char C48, char C49, char C50, char C51, char C52, char C53, char C54, char C55, char C56, char C57, char C58, char C59, char C60, char C61, char C62, char C63, char C64, char C65, char C66, char C67, char C68, char C69, char C70, char C71, char C72, char C73>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31, C32, C33, C34, C35, C36, C37, C38, C39, C40, C41, C42, C43, C44, C45, C46, C47, C48, C49, C50, C51, C52, C53, C54, C55, C56, C57, C58, C59, C60, C61, C62, C63, C64, C65, C66, C67, C68, C69, C70, C71, C72, C73>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    case C28:
      typename F::template apply<C28>::type{}(args...);
      break;
    case C29:
      typename F::template apply<C29>::type{}(args...);
      break;
    case C30:
      typename F::template apply<C30>::type{}(args...);
      break;
    case C31:
      typename F::template apply<C31>::type{}(args...);
      break;
    case C32:
      typename F::template apply<C32>::type{}(args...);
      break;
    case C33:
      typename F::template apply<C33>::type{}(args...);
      break;
    case C34:
      typename F::template apply<C34>::type{}(args...);
      break;
    case C35:
      typename F::template apply<C35>::type{}(args...);
      break;
    case C36:
      typename F::template apply<C36>::type{}(args...);
      break;
    case C37:
      typename F::template apply<C37>::type{}(args...);
      break;
    case C38:
      typename F::template apply<C38>::type{}(args...);
      break;
    case C39:
      typename F::template apply<C39>::type{}(args...);
      break;
    case C40:
      typename F::template apply<C40>::type{}(args...);
      break;
    case C41:
      typename F::template apply<C41>::type{}(args...);
      break;
    case C42:
      typename F::template apply<C42>::type{}(args...);
      break;
    case C43:
      typename F::template apply<C43>::type{}(args...);
      break;
    case C44:
      typename F::template apply<C44>::type{}(args...);
      break;
    case C45:
      typename F::template apply<C45>::type{}(args...);
      break;
    case C46:
      typename F::template apply<C46>::type{}(args...);
      break;
    case C47:
      typename F::template apply<C47>::type{}(args...);
      break;
    case C48:
      typename F::template apply<C48>::type{}(args...);
      break;
    case C49:
      typename F::template apply<C49>::type{}(args...);
      break;
    case C50:
      typename F::template apply<C50>::type{}(args...);
      break;
    case C51:
      typename F::template apply<C51>::type{}(args...);
      break;
    case C52:
      typename F::template apply<C52>::type{}(args...);
      break;
    case C53:
      typename F::template apply<C53>::type{}(args...);
      break;
    case C54:
      typename F::template apply<C54>::type{}(args...);
      break;
    case C55:
      typename F::template apply<C55>::type{}(args...);
      break;
    case C56:
      typename F::template apply<C56>::type{}(args...);
      break;
    case C57:
      typename F::template apply<C57>::type{}(args...);
      break;
    case C58:
      typename F::template apply<C58>::type{}(args...);
      break;
    case C59:
      typename F::template apply<C59>::type{}(args...);
      break;
    case C60:
      typename F::template apply<C60>::type{}(args...);
      break;
    case C61:
      typename F::template apply<C61>::type{}(args...);
      break;
    case C62:
      typename F::template apply<C62>::type{}(args...);
      break;
    case C63:
      typename F::template apply<C63>::type{}(args...);
      break;
    case C64:
      typename F::template apply<C64>::type{}(args...);
      break;
    case C65:
      typename F::template apply<C65>::type{}(args...);
      break;
    case C66:
      typename F::template apply<C66>::type{}(args...);
      break;
    case C67:
      typename F::template apply<C67>::type{}(args...);
      break;
    case C68:
      typename F::template apply<C68>::type{}(args...);
      break;
    case C69:
      typename F::template apply<C69>::type{}(args...);
      break;
    case C70:
      typename F::template apply<C70>::type{}(args...);
      break;
    case C71:
      typename F::template apply<C71>::type{}(args...);
      break;
    case C72:
      typename F::template apply<C72>::type{}(args...);
      break;
    case C73:
      typename F::template apply<C73>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27, char C28, char C29, char C30, char C31, char C32, char C33, char C34, char C35, char C36, char C37, char C38, char C39, char C40, char C41, char C42, char C43, char C44, char C45, char C46, char C47, char C48, char C49, char C50, char C51, char C52, char C53, char C54, char C55, char C56, char C57, char C58, char C59, char C60, char C61, char C62, char C63, char C64, char C65, char C66, char C67, char C68, char C69, char C70, char C71, char C72, char C73, char C74>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31, C32, C33, C34, C35, C36, C37, C38, C39, C40, C41, C42, C43, C44, C45, C46, C47, C48, C49, C50, C51, C52, C53, C54, C55, C56, C57, C58, C59, C60, C61, C62, C63, C64, C65, C66, C67, C68, C69, C70, C71, C72, C73, C74>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    case C28:
      typename F::template apply<C28>::type{}(args...);
      break;
    case C29:
      typename F::template apply<C29>::type{}(args...);
      break;
    case C30:
      typename F::template apply<C30>::type{}(args...);
      break;
    case C31:
      typename F::template apply<C31>::type{}(args...);
      break;
    case C32:
      typename F::template apply<C32>::type{}(args...);
      break;
    case C33:
      typename F::template apply<C33>::type{}(args...);
      break;
    case C34:
      typename F::template apply<C34>::type{}(args...);
      break;
    case C35:
      typename F::template apply<C35>::type{}(args...);
      break;
    case C36:
      typename F::template apply<C36>::type{}(args...);
      break;
    case C37:
      typename F::template apply<C37>::type{}(args...);
      break;
    case C38:
      typename F::template apply<C38>::type{}(args...);
      break;
    case C39:
      typename F::template apply<C39>::type{}(args...);
      break;
    case C40:
      typename F::template apply<C40>::type{}(args...);
      break;
    case C41:
      typename F::template apply<C41>::type{}(args...);
      break;
    case C42:
      typename F::template apply<C42>::type{}(args...);
      break;
    case C43:
      typename F::template apply<C43>::type{}(args...);
      break;
    case C44:
      typename F::template apply<C44>::type{}(args...);
      break;
    case C45:
      typename F::template apply<C45>::type{}(args...);
      break;
    case C46:
      typename F::template apply<C46>::type{}(args...);
      break;
    case C47:
      typename F::template apply<C47>::type{}(args...);
      break;
    case C48:
      typename F::template apply<C48>::type{}(args...);
      break;
    case C49:
      typename F::template apply<C49>::type{}(args...);
      break;
    case C50:
      typename F::template apply<C50>::type{}(args...);
      break;
    case C51:
      typename F::template apply<C51>::type{}(args...);
      break;
    case C52:
      typename F::template apply<C52>::type{}(args...);
      break;
    case C53:
      typename F::template apply<C53>::type{}(args...);
      break;
    case C54:
      typename F::template apply<C54>::type{}(args...);
      break;
    case C55:
      typename F::template apply<C55>::type{}(args...);
      break;
    case C56:
      typename F::template apply<C56>::type{}(args...);
      break;
    case C57:
      typename F::template apply<C57>::type{}(args...);
      break;
    case C58:
      typename F::template apply<C58>::type{}(args...);
      break;
    case C59:
      typename F::template apply<C59>::type{}(args...);
      break;
    case C60:
      typename F::template apply<C60>::type{}(args...);
      break;
    case C61:
      typename F::template apply<C61>::type{}(args...);
      break;
    case C62:
      typename F::template apply<C62>::type{}(args...);
      break;
    case C63:
      typename F::template apply<C63>::type{}(args...);
      break;
    case C64:
      typename F::template apply<C64>::type{}(args...);
      break;
    case C65:
      typename F::template apply<C65>::type{}(args...);
      break;
    case C66:
      typename F::template apply<C66>::type{}(args...);
      break;
    case C67:
      typename F::template apply<C67>::type{}(args...);
      break;
    case C68:
      typename F::template apply<C68>::type{}(args...);
      break;
    case C69:
      typename F::template apply<C69>::type{}(args...);
      break;
    case C70:
      typename F::template apply<C70>::type{}(args...);
      break;
    case C71:
      typename F::template apply<C71>::type{}(args...);
      break;
    case C72:
      typename F::template apply<C72>::type{}(args...);
      break;
    case C73:
      typename F::template apply<C73>::type{}(args...);
      break;
    case C74:
      typename F::template apply<C74>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27, char C28, char C29, char C30, char C31, char C32, char C33, char C34, char C35, char C36, char C37, char C38, char C39, char C40, char C41, char C42, char C43, char C44, char C45, char C46, char C47, char C48, char C49, char C50, char C51, char C52, char C53, char C54, char C55, char C56, char C57, char C58, char C59, char C60, char C61, char C62, char C63, char C64, char C65, char C66, char C67, char C68, char C69, char C70, char C71, char C72, char C73, char C74, char C75>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31, C32, C33, C34, C35, C36, C37, C38, C39, C40, C41, C42, C43, C44, C45, C46, C47, C48, C49, C50, C51, C52, C53, C54, C55, C56, C57, C58, C59, C60, C61, C62, C63, C64, C65, C66, C67, C68, C69, C70, C71, C72, C73, C74, C75>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    case C28:
      typename F::template apply<C28>::type{}(args...);
      break;
    case C29:
      typename F::template apply<C29>::type{}(args...);
      break;
    case C30:
      typename F::template apply<C30>::type{}(args...);
      break;
    case C31:
      typename F::template apply<C31>::type{}(args...);
      break;
    case C32:
      typename F::template apply<C32>::type{}(args...);
      break;
    case C33:
      typename F::template apply<C33>::type{}(args...);
      break;
    case C34:
      typename F::template apply<C34>::type{}(args...);
      break;
    case C35:
      typename F::template apply<C35>::type{}(args...);
      break;
    case C36:
      typename F::template apply<C36>::type{}(args...);
      break;
    case C37:
      typename F::template apply<C37>::type{}(args...);
      break;
    case C38:
      typename F::template apply<C38>::type{}(args...);
      break;
    case C39:
      typename F::template apply<C39>::type{}(args...);
      break;
    case C40:
      typename F::template apply<C40>::type{}(args...);
      break;
    case C41:
      typename F::template apply<C41>::type{}(args...);
      break;
    case C42:
      typename F::template apply<C42>::type{}(args...);
      break;
    case C43:
      typename F::template apply<C43>::type{}(args...);
      break;
    case C44:
      typename F::template apply<C44>::type{}(args...);
      break;
    case C45:
      typename F::template apply<C45>::type{}(args...);
      break;
    case C46:
      typename F::template apply<C46>::type{}(args...);
      break;
    case C47:
      typename F::template apply<C47>::type{}(args...);
      break;
    case C48:
      typename F::template apply<C48>::type{}(args...);
      break;
    case C49:
      typename F::template apply<C49>::type{}(args...);
      break;
    case C50:
      typename F::template apply<C50>::type{}(args...);
      break;
    case C51:
      typename F::template apply<C51>::type{}(args...);
      break;
    case C52:
      typename F::template apply<C52>::type{}(args...);
      break;
    case C53:
      typename F::template apply<C53>::type{}(args...);
      break;
    case C54:
      typename F::template apply<C54>::type{}(args...);
      break;
    case C55:
      typename F::template apply<C55>::type{}(args...);
      break;
    case C56:
      typename F::template apply<C56>::type{}(args...);
      break;
    case C57:
      typename F::template apply<C57>::type{}(args...);
      break;
    case C58:
      typename F::template apply<C58>::type{}(args...);
      break;
    case C59:
      typename F::template apply<C59>::type{}(args...);
      break;
    case C60:
      typename F::template apply<C60>::type{}(args...);
      break;
    case C61:
      typename F::template apply<C61>::type{}(args...);
      break;
    case C62:
      typename F::template apply<C62>::type{}(args...);
      break;
    case C63:
      typename F::template apply<C63>::type{}(args...);
      break;
    case C64:
      typename F::template apply<C64>::type{}(args...);
      break;
    case C65:
      typename F::template apply<C65>::type{}(args...);
      break;
    case C66:
      typename F::template apply<C66>::type{}(args...);
      break;
    case C67:
      typename F::template apply<C67>::type{}(args...);
      break;
    case C68:
      typename F::template apply<C68>::type{}(args...);
      break;
    case C69:
      typename F::template apply<C69>::type{}(args...);
      break;
    case C70:
      typename F::template apply<C70>::type{}(args...);
      break;
    case C71:
      typename F::template apply<C71>::type{}(args...);
      break;
    case C72:
      typename F::template apply<C72>::type{}(args...);
      break;
    case C73:
      typename F::template apply<C73>::type{}(args...);
      break;
    case C74:
      typename F::template apply<C74>::type{}(args...);
      break;
    case C75:
      typename F::template apply<C75>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27, char C28, char C29, char C30, char C31, char C32, char C33, char C34, char C35, char C36, char C37, char C38, char C39, char C40, char C41, char C42, char C43, char C44, char C45, char C46, char C47, char C48, char C49, char C50, char C51, char C52, char C53, char C54, char C55, char C56, char C57, char C58, char C59, char C60, char C61, char C62, char C63, char C64, char C65, char C66, char C67, char C68, char C69, char C70, char C71, char C72, char C73, char C74, char C75, char C76>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31, C32, C33, C34, C35, C36, C37, C38, C39, C40, C41, C42, C43, C44, C45, C46, C47, C48, C49, C50, C51, C52, C53, C54, C55, C56, C57, C58, C59, C60, C61, C62, C63, C64, C65, C66, C67, C68, C69, C70, C71, C72, C73, C74, C75, C76>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    case C28:
      typename F::template apply<C28>::type{}(args...);
      break;
    case C29:
      typename F::template apply<C29>::type{}(args...);
      break;
    case C30:
      typename F::template apply<C30>::type{}(args...);
      break;
    case C31:
      typename F::template apply<C31>::type{}(args...);
      break;
    case C32:
      typename F::template apply<C32>::type{}(args...);
      break;
    case C33:
      typename F::template apply<C33>::type{}(args...);
      break;
    case C34:
      typename F::template apply<C34>::type{}(args...);
      break;
    case C35:
      typename F::template apply<C35>::type{}(args...);
      break;
    case C36:
      typename F::template apply<C36>::type{}(args...);
      break;
    case C37:
      typename F::template apply<C37>::type{}(args...);
      break;
    case C38:
      typename F::template apply<C38>::type{}(args...);
      break;
    case C39:
      typename F::template apply<C39>::type{}(args...);
      break;
    case C40:
      typename F::template apply<C40>::type{}(args...);
      break;
    case C41:
      typename F::template apply<C41>::type{}(args...);
      break;
    case C42:
      typename F::template apply<C42>::type{}(args...);
      break;
    case C43:
      typename F::template apply<C43>::type{}(args...);
      break;
    case C44:
      typename F::template apply<C44>::type{}(args...);
      break;
    case C45:
      typename F::template apply<C45>::type{}(args...);
      break;
    case C46:
      typename F::template apply<C46>::type{}(args...);
      break;
    case C47:
      typename F::template apply<C47>::type{}(args...);
      break;
    case C48:
      typename F::template apply<C48>::type{}(args...);
      break;
    case C49:
      typename F::template apply<C49>::type{}(args...);
      break;
    case C50:
      typename F::template apply<C50>::type{}(args...);
      break;
    case C51:
      typename F::template apply<C51>::type{}(args...);
      break;
    case C52:
      typename F::template apply<C52>::type{}(args...);
      break;
    case C53:
      typename F::template apply<C53>::type{}(args...);
      break;
    case C54:
      typename F::template apply<C54>::type{}(args...);
      break;
    case C55:
      typename F::template apply<C55>::type{}(args...);
      break;
    case C56:
      typename F::template apply<C56>::type{}(args...);
      break;
    case C57:
      typename F::template apply<C57>::type{}(args...);
      break;
    case C58:
      typename F::template apply<C58>::type{}(args...);
      break;
    case C59:
      typename F::template apply<C59>::type{}(args...);
      break;
    case C60:
      typename F::template apply<C60>::type{}(args...);
      break;
    case C61:
      typename F::template apply<C61>::type{}(args...);
      break;
    case C62:
      typename F::template apply<C62>::type{}(args...);
      break;
    case C63:
      typename F::template apply<C63>::type{}(args...);
      break;
    case C64:
      typename F::template apply<C64>::type{}(args...);
      break;
    case C65:
      typename F::template apply<C65>::type{}(args...);
      break;
    case C66:
      typename F::template apply<C66>::type{}(args...);
      break;
    case C67:
      typename F::template apply<C67>::type{}(args...);
      break;
    case C68:
      typename F::template apply<C68>::type{}(args...);
      break;
    case C69:
      typename F::template apply<C69>::type{}(args...);
      break;
    case C70:
      typename F::template apply<C70>::type{}(args...);
      break;
    case C71:
      typename F::template apply<C71>::type{}(args...);
      break;
    case C72:
      typename F::template apply<C72>::type{}(args...);
      break;
    case C73:
      typename F::template apply<C73>::type{}(args...);
      break;
    case C74:
      typename F::template apply<C74>::type{}(args...);
      break;
    case C75:
      typename F::template apply<C75>::type{}(args...);
      break;
    case C76:
      typename F::template apply<C76>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27, char C28, char C29, char C30, char C31, char C32, char C33, char C34, char C35, char C36, char C37, char C38, char C39, char C40, char C41, char C42, char C43, char C44, char C45, char C46, char C47, char C48, char C49, char C50, char C51, char C52, char C53, char C54, char C55, char C56, char C57, char C58, char C59, char C60, char C61, char C62, char C63, char C64, char C65, char C66, char C67, char C68, char C69, char C70, char C71, char C72, char C73, char C74, char C75, char C76, char C77>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31, C32, C33, C34, C35, C36, C37, C38, C39, C40, C41, C42, C43, C44, C45, C46, C47, C48, C49, C50, C51, C52, C53, C54, C55, C56, C57, C58, C59, C60, C61, C62, C63, C64, C65, C66, C67, C68, C69, C70, C71, C72, C73, C74, C75, C76, C77>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    case C28:
      typename F::template apply<C28>::type{}(args...);
      break;
    case C29:
      typename F::template apply<C29>::type{}(args...);
      break;
    case C30:
      typename F::template apply<C30>::type{}(args...);
      break;
    case C31:
      typename F::template apply<C31>::type{}(args...);
      break;
    case C32:
      typename F::template apply<C32>::type{}(args...);
      break;
    case C33:
      typename F::template apply<C33>::type{}(args...);
      break;
    case C34:
      typename F::template apply<C34>::type{}(args...);
      break;
    case C35:
      typename F::template apply<C35>::type{}(args...);
      break;
    case C36:
      typename F::template apply<C36>::type{}(args...);
      break;
    case C37:
      typename F::template apply<C37>::type{}(args...);
      break;
    case C38:
      typename F::template apply<C38>::type{}(args...);
      break;
    case C39:
      typename F::template apply<C39>::type{}(args...);
      break;
    case C40:
      typename F::template apply<C40>::type{}(args...);
      break;
    case C41:
      typename F::template apply<C41>::type{}(args...);
      break;
    case C42:
      typename F::template apply<C42>::type{}(args...);
      break;
    case C43:
      typename F::template apply<C43>::type{}(args...);
      break;
    case C44:
      typename F::template apply<C44>::type{}(args...);
      break;
    case C45:
      typename F::template apply<C45>::type{}(args...);
      break;
    case C46:
      typename F::template apply<C46>::type{}(args...);
      break;
    case C47:
      typename F::template apply<C47>::type{}(args...);
      break;
    case C48:
      typename F::template apply<C48>::type{}(args...);
      break;
    case C49:
      typename F::template apply<C49>::type{}(args...);
      break;
    case C50:
      typename F::template apply<C50>::type{}(args...);
      break;
    case C51:
      typename F::template apply<C51>::type{}(args...);
      break;
    case C52:
      typename F::template apply<C52>::type{}(args...);
      break;
    case C53:
      typename F::template apply<C53>::type{}(args...);
      break;
    case C54:
      typename F::template apply<C54>::type{}(args...);
      break;
    case C55:
      typename F::template apply<C55>::type{}(args...);
      break;
    case C56:
      typename F::template apply<C56>::type{}(args...);
      break;
    case C57:
      typename F::template apply<C57>::type{}(args...);
      break;
    case C58:
      typename F::template apply<C58>::type{}(args...);
      break;
    case C59:
      typename F::template apply<C59>::type{}(args...);
      break;
    case C60:
      typename F::template apply<C60>::type{}(args...);
      break;
    case C61:
      typename F::template apply<C61>::type{}(args...);
      break;
    case C62:
      typename F::template apply<C62>::type{}(args...);
      break;
    case C63:
      typename F::template apply<C63>::type{}(args...);
      break;
    case C64:
      typename F::template apply<C64>::type{}(args...);
      break;
    case C65:
      typename F::template apply<C65>::type{}(args...);
      break;
    case C66:
      typename F::template apply<C66>::type{}(args...);
      break;
    case C67:
      typename F::template apply<C67>::type{}(args...);
      break;
    case C68:
      typename F::template apply<C68>::type{}(args...);
      break;
    case C69:
      typename F::template apply<C69>::type{}(args...);
      break;
    case C70:
      typename F::template apply<C70>::type{}(args...);
      break;
    case C71:
      typename F::template apply<C71>::type{}(args...);
      break;
    case C72:
      typename F::template apply<C72>::type{}(args...);
      break;
    case C73:
      typename F::template apply<C73>::type{}(args...);
      break;
    case C74:
      typename F::template apply<C74>::type{}(args...);
      break;
    case C75:
      typename F::template apply<C75>::type{}(args...);
      break;
    case C76:
      typename F::template apply<C76>::type{}(args...);
      break;
    case C77:
      typename F::template apply<C77>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27, char C28, char C29, char C30, char C31, char C32, char C33, char C34, char C35, char C36, char C37, char C38, char C39, char C40, char C41, char C42, char C43, char C44, char C45, char C46, char C47, char C48, char C49, char C50, char C51, char C52, char C53, char C54, char C55, char C56, char C57, char C58, char C59, char C60, char C61, char C62, char C63, char C64, char C65, char C66, char C67, char C68, char C69, char C70, char C71, char C72, char C73, char C74, char C75, char C76, char C77, char C78>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31, C32, C33, C34, C35, C36, C37, C38, C39, C40, C41, C42, C43, C44, C45, C46, C47, C48, C49, C50, C51, C52, C53, C54, C55, C56, C57, C58, C59, C60, C61, C62, C63, C64, C65, C66, C67, C68, C69, C70, C71, C72, C73, C74, C75, C76, C77, C78>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    case C28:
      typename F::template apply<C28>::type{}(args...);
      break;
    case C29:
      typename F::template apply<C29>::type{}(args...);
      break;
    case C30:
      typename F::template apply<C30>::type{}(args...);
      break;
    case C31:
      typename F::template apply<C31>::type{}(args...);
      break;
    case C32:
      typename F::template apply<C32>::type{}(args...);
      break;
    case C33:
      typename F::template apply<C33>::type{}(args...);
      break;
    case C34:
      typename F::template apply<C34>::type{}(args...);
      break;
    case C35:
      typename F::template apply<C35>::type{}(args...);
      break;
    case C36:
      typename F::template apply<C36>::type{}(args...);
      break;
    case C37:
      typename F::template apply<C37>::type{}(args...);
      break;
    case C38:
      typename F::template apply<C38>::type{}(args...);
      break;
    case C39:
      typename F::template apply<C39>::type{}(args...);
      break;
    case C40:
      typename F::template apply<C40>::type{}(args...);
      break;
    case C41:
      typename F::template apply<C41>::type{}(args...);
      break;
    case C42:
      typename F::template apply<C42>::type{}(args...);
      break;
    case C43:
      typename F::template apply<C43>::type{}(args...);
      break;
    case C44:
      typename F::template apply<C44>::type{}(args...);
      break;
    case C45:
      typename F::template apply<C45>::type{}(args...);
      break;
    case C46:
      typename F::template apply<C46>::type{}(args...);
      break;
    case C47:
      typename F::template apply<C47>::type{}(args...);
      break;
    case C48:
      typename F::template apply<C48>::type{}(args...);
      break;
    case C49:
      typename F::template apply<C49>::type{}(args...);
      break;
    case C50:
      typename F::template apply<C50>::type{}(args...);
      break;
    case C51:
      typename F::template apply<C51>::type{}(args...);
      break;
    case C52:
      typename F::template apply<C52>::type{}(args...);
      break;
    case C53:
      typename F::template apply<C53>::type{}(args...);
      break;
    case C54:
      typename F::template apply<C54>::type{}(args...);
      break;
    case C55:
      typename F::template apply<C55>::type{}(args...);
      break;
    case C56:
      typename F::template apply<C56>::type{}(args...);
      break;
    case C57:
      typename F::template apply<C57>::type{}(args...);
      break;
    case C58:
      typename F::template apply<C58>::type{}(args...);
      break;
    case C59:
      typename F::template apply<C59>::type{}(args...);
      break;
    case C60:
      typename F::template apply<C60>::type{}(args...);
      break;
    case C61:
      typename F::template apply<C61>::type{}(args...);
      break;
    case C62:
      typename F::template apply<C62>::type{}(args...);
      break;
    case C63:
      typename F::template apply<C63>::type{}(args...);
      break;
    case C64:
      typename F::template apply<C64>::type{}(args...);
      break;
    case C65:
      typename F::template apply<C65>::type{}(args...);
      break;
    case C66:
      typename F::template apply<C66>::type{}(args...);
      break;
    case C67:
      typename F::template apply<C67>::type{}(args...);
      break;
    case C68:
      typename F::template apply<C68>::type{}(args...);
      break;
    case C69:
      typename F::template apply<C69>::type{}(args...);
      break;
    case C70:
      typename F::template apply<C70>::type{}(args...);
      break;
    case C71:
      typename F::template apply<C71>::type{}(args...);
      break;
    case C72:
      typename F::template apply<C72>::type{}(args...);
      break;
    case C73:
      typename F::template apply<C73>::type{}(args...);
      break;
    case C74:
      typename F::template apply<C74>::type{}(args...);
      break;
    case C75:
      typename F::template apply<C75>::type{}(args...);
      break;
    case C76:
      typename F::template apply<C76>::type{}(args...);
      break;
    case C77:
      typename F::template apply<C77>::type{}(args...);
      break;
    case C78:
      typename F::template apply<C78>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27, char C28, char C29, char C30, char C31, char C32, char C33, char C34, char C35, char C36, char C37, char C38, char C39, char C40, char C41, char C42, char C43, char C44, char C45, char C46, char C47, char C48, char C49, char C50, char C51, char C52, char C53, char C54, char C55, char C56, char C57, char C58, char C59, char C60, char C61, char C62, char C63, char C64, char C65, char C66, char C67, char C68, char C69, char C70, char C71, char C72, char C73, char C74, char C75, char C76, char C77, char C78, char C79>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31, C32, C33, C34, C35, C36, C37, C38, C39, C40, C41, C42, C43, C44, C45, C46, C47, C48, C49, C50, C51, C52, C53, C54, C55, C56, C57, C58, C59, C60, C61, C62, C63, C64, C65, C66, C67, C68, C69, C70, C71, C72, C73, C74, C75, C76, C77, C78, C79>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    case C28:
      typename F::template apply<C28>::type{}(args...);
      break;
    case C29:
      typename F::template apply<C29>::type{}(args...);
      break;
    case C30:
      typename F::template apply<C30>::type{}(args...);
      break;
    case C31:
      typename F::template apply<C31>::type{}(args...);
      break;
    case C32:
      typename F::template apply<C32>::type{}(args...);
      break;
    case C33:
      typename F::template apply<C33>::type{}(args...);
      break;
    case C34:
      typename F::template apply<C34>::type{}(args...);
      break;
    case C35:
      typename F::template apply<C35>::type{}(args...);
      break;
    case C36:
      typename F::template apply<C36>::type{}(args...);
      break;
    case C37:
      typename F::template apply<C37>::type{}(args...);
      break;
    case C38:
      typename F::template apply<C38>::type{}(args...);
      break;
    case C39:
      typename F::template apply<C39>::type{}(args...);
      break;
    case C40:
      typename F::template apply<C40>::type{}(args...);
      break;
    case C41:
      typename F::template apply<C41>::type{}(args...);
      break;
    case C42:
      typename F::template apply<C42>::type{}(args...);
      break;
    case C43:
      typename F::template apply<C43>::type{}(args...);
      break;
    case C44:
      typename F::template apply<C44>::type{}(args...);
      break;
    case C45:
      typename F::template apply<C45>::type{}(args...);
      break;
    case C46:
      typename F::template apply<C46>::type{}(args...);
      break;
    case C47:
      typename F::template apply<C47>::type{}(args...);
      break;
    case C48:
      typename F::template apply<C48>::type{}(args...);
      break;
    case C49:
      typename F::template apply<C49>::type{}(args...);
      break;
    case C50:
      typename F::template apply<C50>::type{}(args...);
      break;
    case C51:
      typename F::template apply<C51>::type{}(args...);
      break;
    case C52:
      typename F::template apply<C52>::type{}(args...);
      break;
    case C53:
      typename F::template apply<C53>::type{}(args...);
      break;
    case C54:
      typename F::template apply<C54>::type{}(args...);
      break;
    case C55:
      typename F::template apply<C55>::type{}(args...);
      break;
    case C56:
      typename F::template apply<C56>::type{}(args...);
      break;
    case C57:
      typename F::template apply<C57>::type{}(args...);
      break;
    case C58:
      typename F::template apply<C58>::type{}(args...);
      break;
    case C59:
      typename F::template apply<C59>::type{}(args...);
      break;
    case C60:
      typename F::template apply<C60>::type{}(args...);
      break;
    case C61:
      typename F::template apply<C61>::type{}(args...);
      break;
    case C62:
      typename F::template apply<C62>::type{}(args...);
      break;
    case C63:
      typename F::template apply<C63>::type{}(args...);
      break;
    case C64:
      typename F::template apply<C64>::type{}(args...);
      break;
    case C65:
      typename F::template apply<C65>::type{}(args...);
      break;
    case C66:
      typename F::template apply<C66>::type{}(args...);
      break;
    case C67:
      typename F::template apply<C67>::type{}(args...);
      break;
    case C68:
      typename F::template apply<C68>::type{}(args...);
      break;
    case C69:
      typename F::template apply<C69>::type{}(args...);
      break;
    case C70:
      typename F::template apply<C70>::type{}(args...);
      break;
    case C71:
      typename F::template apply<C71>::type{}(args...);
      break;
    case C72:
      typename F::template apply<C72>::type{}(args...);
      break;
    case C73:
      typename F::template apply<C73>::type{}(args...);
      break;
    case C74:
      typename F::template apply<C74>::type{}(args...);
      break;
    case C75:
      typename F::template apply<C75>::type{}(args...);
      break;
    case C76:
      typename F::template apply<C76>::type{}(args...);
      break;
    case C77:
      typename F::template apply<C77>::type{}(args...);
      break;
    case C78:
      typename F::template apply<C78>::type{}(args...);
      break;
    case C79:
      typename F::template apply<C79>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27, char C28, char C29, char C30, char C31, char C32, char C33, char C34, char C35, char C36, char C37, char C38, char C39, char C40, char C41, char C42, char C43, char C44, char C45, char C46, char C47, char C48, char C49, char C50, char C51, char C52, char C53, char C54, char C55, char C56, char C57, char C58, char C59, char C60, char C61, char C62, char C63, char C64, char C65, char C66, char C67, char C68, char C69, char C70, char C71, char C72, char C73, char C74, char C75, char C76, char C77, char C78, char C79, char C80>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31, C32, C33, C34, C35, C36, C37, C38, C39, C40, C41, C42, C43, C44, C45, C46, C47, C48, C49, C50, C51, C52, C53, C54, C55, C56, C57, C58, C59, C60, C61, C62, C63, C64, C65, C66, C67, C68, C69, C70, C71, C72, C73, C74, C75, C76, C77, C78, C79, C80>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    case C28:
      typename F::template apply<C28>::type{}(args...);
      break;
    case C29:
      typename F::template apply<C29>::type{}(args...);
      break;
    case C30:
      typename F::template apply<C30>::type{}(args...);
      break;
    case C31:
      typename F::template apply<C31>::type{}(args...);
      break;
    case C32:
      typename F::template apply<C32>::type{}(args...);
      break;
    case C33:
      typename F::template apply<C33>::type{}(args...);
      break;
    case C34:
      typename F::template apply<C34>::type{}(args...);
      break;
    case C35:
      typename F::template apply<C35>::type{}(args...);
      break;
    case C36:
      typename F::template apply<C36>::type{}(args...);
      break;
    case C37:
      typename F::template apply<C37>::type{}(args...);
      break;
    case C38:
      typename F::template apply<C38>::type{}(args...);
      break;
    case C39:
      typename F::template apply<C39>::type{}(args...);
      break;
    case C40:
      typename F::template apply<C40>::type{}(args...);
      break;
    case C41:
      typename F::template apply<C41>::type{}(args...);
      break;
    case C42:
      typename F::template apply<C42>::type{}(args...);
      break;
    case C43:
      typename F::template apply<C43>::type{}(args...);
      break;
    case C44:
      typename F::template apply<C44>::type{}(args...);
      break;
    case C45:
      typename F::template apply<C45>::type{}(args...);
      break;
    case C46:
      typename F::template apply<C46>::type{}(args...);
      break;
    case C47:
      typename F::template apply<C47>::type{}(args...);
      break;
    case C48:
      typename F::template apply<C48>::type{}(args...);
      break;
    case C49:
      typename F::template apply<C49>::type{}(args...);
      break;
    case C50:
      typename F::template apply<C50>::type{}(args...);
      break;
    case C51:
      typename F::template apply<C51>::type{}(args...);
      break;
    case C52:
      typename F::template apply<C52>::type{}(args...);
      break;
    case C53:
      typename F::template apply<C53>::type{}(args...);
      break;
    case C54:
      typename F::template apply<C54>::type{}(args...);
      break;
    case C55:
      typename F::template apply<C55>::type{}(args...);
      break;
    case C56:
      typename F::template apply<C56>::type{}(args...);
      break;
    case C57:
      typename F::template apply<C57>::type{}(args...);
      break;
    case C58:
      typename F::template apply<C58>::type{}(args...);
      break;
    case C59:
      typename F::template apply<C59>::type{}(args...);
      break;
    case C60:
      typename F::template apply<C60>::type{}(args...);
      break;
    case C61:
      typename F::template apply<C61>::type{}(args...);
      break;
    case C62:
      typename F::template apply<C62>::type{}(args...);
      break;
    case C63:
      typename F::template apply<C63>::type{}(args...);
      break;
    case C64:
      typename F::template apply<C64>::type{}(args...);
      break;
    case C65:
      typename F::template apply<C65>::type{}(args...);
      break;
    case C66:
      typename F::template apply<C66>::type{}(args...);
      break;
    case C67:
      typename F::template apply<C67>::type{}(args...);
      break;
    case C68:
      typename F::template apply<C68>::type{}(args...);
      break;
    case C69:
      typename F::template apply<C69>::type{}(args...);
      break;
    case C70:
      typename F::template apply<C70>::type{}(args...);
      break;
    case C71:
      typename F::template apply<C71>::type{}(args...);
      break;
    case C72:
      typename F::template apply<C72>::type{}(args...);
      break;
    case C73:
      typename F::template apply<C73>::type{}(args...);
      break;
    case C74:
      typename F::template apply<C74>::type{}(args...);
      break;
    case C75:
      typename F::template apply<C75>::type{}(args...);
      break;
    case C76:
      typename F::template apply<C76>::type{}(args...);
      break;
    case C77:
      typename F::template apply<C77>::type{}(args...);
      break;
    case C78:
      typename F::template apply<C78>::type{}(args...);
      break;
    case C79:
      typename F::template apply<C79>::type{}(args...);
      break;
    case C80:
      typename F::template apply<C80>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27, char C28, char C29, char C30, char C31, char C32, char C33, char C34, char C35, char C36, char C37, char C38, char C39, char C40, char C41, char C42, char C43, char C44, char C45, char C46, char C47, char C48, char C49, char C50, char C51, char C52, char C53, char C54, char C55, char C56, char C57, char C58, char C59, char C60, char C61, char C62, char C63, char C64, char C65, char C66, char C67, char C68, char C69, char C70, char C71, char C72, char C73, char C74, char C75, char C76, char C77, char C78, char C79, char C80, char C81>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31, C32, C33, C34, C35, C36, C37, C38, C39, C40, C41, C42, C43, C44, C45, C46, C47, C48, C49, C50, C51, C52, C53, C54, C55, C56, C57, C58, C59, C60, C61, C62, C63, C64, C65, C66, C67, C68, C69, C70, C71, C72, C73, C74, C75, C76, C77, C78, C79, C80, C81>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    case C28:
      typename F::template apply<C28>::type{}(args...);
      break;
    case C29:
      typename F::template apply<C29>::type{}(args...);
      break;
    case C30:
      typename F::template apply<C30>::type{}(args...);
      break;
    case C31:
      typename F::template apply<C31>::type{}(args...);
      break;
    case C32:
      typename F::template apply<C32>::type{}(args...);
      break;
    case C33:
      typename F::template apply<C33>::type{}(args...);
      break;
    case C34:
      typename F::template apply<C34>::type{}(args...);
      break;
    case C35:
      typename F::template apply<C35>::type{}(args...);
      break;
    case C36:
      typename F::template apply<C36>::type{}(args...);
      break;
    case C37:
      typename F::template apply<C37>::type{}(args...);
      break;
    case C38:
      typename F::template apply<C38>::type{}(args...);
      break;
    case C39:
      typename F::template apply<C39>::type{}(args...);
      break;
    case C40:
      typename F::template apply<C40>::type{}(args...);
      break;
    case C41:
      typename F::template apply<C41>::type{}(args...);
      break;
    case C42:
      typename F::template apply<C42>::type{}(args...);
      break;
    case C43:
      typename F::template apply<C43>::type{}(args...);
      break;
    case C44:
      typename F::template apply<C44>::type{}(args...);
      break;
    case C45:
      typename F::template apply<C45>::type{}(args...);
      break;
    case C46:
      typename F::template apply<C46>::type{}(args...);
      break;
    case C47:
      typename F::template apply<C47>::type{}(args...);
      break;
    case C48:
      typename F::template apply<C48>::type{}(args...);
      break;
    case C49:
      typename F::template apply<C49>::type{}(args...);
      break;
    case C50:
      typename F::template apply<C50>::type{}(args...);
      break;
    case C51:
      typename F::template apply<C51>::type{}(args...);
      break;
    case C52:
      typename F::template apply<C52>::type{}(args...);
      break;
    case C53:
      typename F::template apply<C53>::type{}(args...);
      break;
    case C54:
      typename F::template apply<C54>::type{}(args...);
      break;
    case C55:
      typename F::template apply<C55>::type{}(args...);
      break;
    case C56:
      typename F::template apply<C56>::type{}(args...);
      break;
    case C57:
      typename F::template apply<C57>::type{}(args...);
      break;
    case C58:
      typename F::template apply<C58>::type{}(args...);
      break;
    case C59:
      typename F::template apply<C59>::type{}(args...);
      break;
    case C60:
      typename F::template apply<C60>::type{}(args...);
      break;
    case C61:
      typename F::template apply<C61>::type{}(args...);
      break;
    case C62:
      typename F::template apply<C62>::type{}(args...);
      break;
    case C63:
      typename F::template apply<C63>::type{}(args...);
      break;
    case C64:
      typename F::template apply<C64>::type{}(args...);
      break;
    case C65:
      typename F::template apply<C65>::type{}(args...);
      break;
    case C66:
      typename F::template apply<C66>::type{}(args...);
      break;
    case C67:
      typename F::template apply<C67>::type{}(args...);
      break;
    case C68:
      typename F::template apply<C68>::type{}(args...);
      break;
    case C69:
      typename F::template apply<C69>::type{}(args...);
      break;
    case C70:
      typename F::template apply<C70>::type{}(args...);
      break;
    case C71:
      typename F::template apply<C71>::type{}(args...);
      break;
    case C72:
      typename F::template apply<C72>::type{}(args...);
      break;
    case C73:
      typename F::template apply<C73>::type{}(args...);
      break;
    case C74:
      typename F::template apply<C74>::type{}(args...);
      break;
    case C75:
      typename F::template apply<C75>::type{}(args...);
      break;
    case C76:
      typename F::template apply<C76>::type{}(args...);
      break;
    case C77:
      typename F::template apply<C77>::type{}(args...);
      break;
    case C78:
      typename F::template apply<C78>::type{}(args...);
      break;
    case C79:
      typename F::template apply<C79>::type{}(args...);
      break;
    case C80:
      typename F::template apply<C80>::type{}(args...);
      break;
    case C81:
      typename F::template apply<C81>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27, char C28, char C29, char C30, char C31, char C32, char C33, char C34, char C35, char C36, char C37, char C38, char C39, char C40, char C41, char C42, char C43, char C44, char C45, char C46, char C47, char C48, char C49, char C50, char C51, char C52, char C53, char C54, char C55, char C56, char C57, char C58, char C59, char C60, char C61, char C62, char C63, char C64, char C65, char C66, char C67, char C68, char C69, char C70, char C71, char C72, char C73, char C74, char C75, char C76, char C77, char C78, char C79, char C80, char C81, char C82>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31, C32, C33, C34, C35, C36, C37, C38, C39, C40, C41, C42, C43, C44, C45, C46, C47, C48, C49, C50, C51, C52, C53, C54, C55, C56, C57, C58, C59, C60, C61, C62, C63, C64, C65, C66, C67, C68, C69, C70, C71, C72, C73, C74, C75, C76, C77, C78, C79, C80, C81, C82>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    case C28:
      typename F::template apply<C28>::type{}(args...);
      break;
    case C29:
      typename F::template apply<C29>::type{}(args...);
      break;
    case C30:
      typename F::template apply<C30>::type{}(args...);
      break;
    case C31:
      typename F::template apply<C31>::type{}(args...);
      break;
    case C32:
      typename F::template apply<C32>::type{}(args...);
      break;
    case C33:
      typename F::template apply<C33>::type{}(args...);
      break;
    case C34:
      typename F::template apply<C34>::type{}(args...);
      break;
    case C35:
      typename F::template apply<C35>::type{}(args...);
      break;
    case C36:
      typename F::template apply<C36>::type{}(args...);
      break;
    case C37:
      typename F::template apply<C37>::type{}(args...);
      break;
    case C38:
      typename F::template apply<C38>::type{}(args...);
      break;
    case C39:
      typename F::template apply<C39>::type{}(args...);
      break;
    case C40:
      typename F::template apply<C40>::type{}(args...);
      break;
    case C41:
      typename F::template apply<C41>::type{}(args...);
      break;
    case C42:
      typename F::template apply<C42>::type{}(args...);
      break;
    case C43:
      typename F::template apply<C43>::type{}(args...);
      break;
    case C44:
      typename F::template apply<C44>::type{}(args...);
      break;
    case C45:
      typename F::template apply<C45>::type{}(args...);
      break;
    case C46:
      typename F::template apply<C46>::type{}(args...);
      break;
    case C47:
      typename F::template apply<C47>::type{}(args...);
      break;
    case C48:
      typename F::template apply<C48>::type{}(args...);
      break;
    case C49:
      typename F::template apply<C49>::type{}(args...);
      break;
    case C50:
      typename F::template apply<C50>::type{}(args...);
      break;
    case C51:
      typename F::template apply<C51>::type{}(args...);
      break;
    case C52:
      typename F::template apply<C52>::type{}(args...);
      break;
    case C53:
      typename F::template apply<C53>::type{}(args...);
      break;
    case C54:
      typename F::template apply<C54>::type{}(args...);
      break;
    case C55:
      typename F::template apply<C55>::type{}(args...);
      break;
    case C56:
      typename F::template apply<C56>::type{}(args...);
      break;
    case C57:
      typename F::template apply<C57>::type{}(args...);
      break;
    case C58:
      typename F::template apply<C58>::type{}(args...);
      break;
    case C59:
      typename F::template apply<C59>::type{}(args...);
      break;
    case C60:
      typename F::template apply<C60>::type{}(args...);
      break;
    case C61:
      typename F::template apply<C61>::type{}(args...);
      break;
    case C62:
      typename F::template apply<C62>::type{}(args...);
      break;
    case C63:
      typename F::template apply<C63>::type{}(args...);
      break;
    case C64:
      typename F::template apply<C64>::type{}(args...);
      break;
    case C65:
      typename F::template apply<C65>::type{}(args...);
      break;
    case C66:
      typename F::template apply<C66>::type{}(args...);
      break;
    case C67:
      typename F::template apply<C67>::type{}(args...);
      break;
    case C68:
      typename F::template apply<C68>::type{}(args...);
      break;
    case C69:
      typename F::template apply<C69>::type{}(args...);
      break;
    case C70:
      typename F::template apply<C70>::type{}(args...);
      break;
    case C71:
      typename F::template apply<C71>::type{}(args...);
      break;
    case C72:
      typename F::template apply<C72>::type{}(args...);
      break;
    case C73:
      typename F::template apply<C73>::type{}(args...);
      break;
    case C74:
      typename F::template apply<C74>::type{}(args...);
      break;
    case C75:
      typename F::template apply<C75>::type{}(args...);
      break;
    case C76:
      typename F::template apply<C76>::type{}(args...);
      break;
    case C77:
      typename F::template apply<C77>::type{}(args...);
      break;
    case C78:
      typename F::template apply<C78>::type{}(args...);
      break;
    case C79:
      typename F::template apply<C79>::type{}(args...);
      break;
    case C80:
      typename F::template apply<C80>::type{}(args...);
      break;
    case C81:
      typename F::template apply<C81>::type{}(args...);
      break;
    case C82:
      typename F::template apply<C82>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27, char C28, char C29, char C30, char C31, char C32, char C33, char C34, char C35, char C36, char C37, char C38, char C39, char C40, char C41, char C42, char C43, char C44, char C45, char C46, char C47, char C48, char C49, char C50, char C51, char C52, char C53, char C54, char C55, char C56, char C57, char C58, char C59, char C60, char C61, char C62, char C63, char C64, char C65, char C66, char C67, char C68, char C69, char C70, char C71, char C72, char C73, char C74, char C75, char C76, char C77, char C78, char C79, char C80, char C81, char C82, char C83>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31, C32, C33, C34, C35, C36, C37, C38, C39, C40, C41, C42, C43, C44, C45, C46, C47, C48, C49, C50, C51, C52, C53, C54, C55, C56, C57, C58, C59, C60, C61, C62, C63, C64, C65, C66, C67, C68, C69, C70, C71, C72, C73, C74, C75, C76, C77, C78, C79, C80, C81, C82, C83>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    case C28:
      typename F::template apply<C28>::type{}(args...);
      break;
    case C29:
      typename F::template apply<C29>::type{}(args...);
      break;
    case C30:
      typename F::template apply<C30>::type{}(args...);
      break;
    case C31:
      typename F::template apply<C31>::type{}(args...);
      break;
    case C32:
      typename F::template apply<C32>::type{}(args...);
      break;
    case C33:
      typename F::template apply<C33>::type{}(args...);
      break;
    case C34:
      typename F::template apply<C34>::type{}(args...);
      break;
    case C35:
      typename F::template apply<C35>::type{}(args...);
      break;
    case C36:
      typename F::template apply<C36>::type{}(args...);
      break;
    case C37:
      typename F::template apply<C37>::type{}(args...);
      break;
    case C38:
      typename F::template apply<C38>::type{}(args...);
      break;
    case C39:
      typename F::template apply<C39>::type{}(args...);
      break;
    case C40:
      typename F::template apply<C40>::type{}(args...);
      break;
    case C41:
      typename F::template apply<C41>::type{}(args...);
      break;
    case C42:
      typename F::template apply<C42>::type{}(args...);
      break;
    case C43:
      typename F::template apply<C43>::type{}(args...);
      break;
    case C44:
      typename F::template apply<C44>::type{}(args...);
      break;
    case C45:
      typename F::template apply<C45>::type{}(args...);
      break;
    case C46:
      typename F::template apply<C46>::type{}(args...);
      break;
    case C47:
      typename F::template apply<C47>::type{}(args...);
      break;
    case C48:
      typename F::template apply<C48>::type{}(args...);
      break;
    case C49:
      typename F::template apply<C49>::type{}(args...);
      break;
    case C50:
      typename F::template apply<C50>::type{}(args...);
      break;
    case C51:
      typename F::template apply<C51>::type{}(args...);
      break;
    case C52:
      typename F::template apply<C52>::type{}(args...);
      break;
    case C53:
      typename F::template apply<C53>::type{}(args...);
      break;
    case C54:
      typename F::template apply<C54>::type{}(args...);
      break;
    case C55:
      typename F::template apply<C55>::type{}(args...);
      break;
    case C56:
      typename F::template apply<C56>::type{}(args...);
      break;
    case C57:
      typename F::template apply<C57>::type{}(args...);
      break;
    case C58:
      typename F::template apply<C58>::type{}(args...);
      break;
    case C59:
      typename F::template apply<C59>::type{}(args...);
      break;
    case C60:
      typename F::template apply<C60>::type{}(args...);
      break;
    case C61:
      typename F::template apply<C61>::type{}(args...);
      break;
    case C62:
      typename F::template apply<C62>::type{}(args...);
      break;
    case C63:
      typename F::template apply<C63>::type{}(args...);
      break;
    case C64:
      typename F::template apply<C64>::type{}(args...);
      break;
    case C65:
      typename F::template apply<C65>::type{}(args...);
      break;
    case C66:
      typename F::template apply<C66>::type{}(args...);
      break;
    case C67:
      typename F::template apply<C67>::type{}(args...);
      break;
    case C68:
      typename F::template apply<C68>::type{}(args...);
      break;
    case C69:
      typename F::template apply<C69>::type{}(args...);
      break;
    case C70:
      typename F::template apply<C70>::type{}(args...);
      break;
    case C71:
      typename F::template apply<C71>::type{}(args...);
      break;
    case C72:
      typename F::template apply<C72>::type{}(args...);
      break;
    case C73:
      typename F::template apply<C73>::type{}(args...);
      break;
    case C74:
      typename F::template apply<C74>::type{}(args...);
      break;
    case C75:
      typename F::template apply<C75>::type{}(args...);
      break;
    case C76:
      typename F::template apply<C76>::type{}(args...);
      break;
    case C77:
      typename F::template apply<C77>::type{}(args...);
      break;
    case C78:
      typename F::template apply<C78>::type{}(args...);
      break;
    case C79:
      typename F::template apply<C79>::type{}(args...);
      break;
    case C80:
      typename F::template apply<C80>::type{}(args...);
      break;
    case C81:
      typename F::template apply<C81>::type{}(args...);
      break;
    case C82:
      typename F::template apply<C82>::type{}(args...);
      break;
    case C83:
      typename F::template apply<C83>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

template <char C0, char C1, char C2, char C3, char C4, char C5, char C6, char C7, char C8, char C9, char C10, char C11, char C12, char C13, char C14, char C15, char C16, char C17, char C18, char C19, char C20, char C21, char C22, char C23, char C24, char C25, char C26, char C27, char C28, char C29, char C30, char C31, char C32, char C33, char C34, char C35, char C36, char C37, char C38, char C39, char C40, char C41, char C42, char C43, char C44, char C45, char C46, char C47, char C48, char C49, char C50, char C51, char C52, char C53, char C54, char C55, char C56, char C57, char C58, char C59, char C60, char C61, char C62, char C63, char C64, char C65, char C66, char C67, char C68, char C69, char C70, char C71, char C72, char C73, char C74, char C75, char C76, char C77, char C78, char C79, char C80, char C81, char C82, char C83, char C84>
struct tpl_char_switch<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31, C32, C33, C34, C35, C36, C37, C38, C39, C40, C41, C42, C43, C44, C45, C46, C47, C48, C49, C50, C51, C52, C53, C54, C55, C56, C57, C58, C59, C60, C61, C62, C63, C64, C65, C66, C67, C68, C69, C70, C71, C72, C73, C74, C75, C76, C77, C78, C79, C80, C81, C82, C83, C84>
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
    case C21:
      typename F::template apply<C21>::type{}(args...);
      break;
    case C22:
      typename F::template apply<C22>::type{}(args...);
      break;
    case C23:
      typename F::template apply<C23>::type{}(args...);
      break;
    case C24:
      typename F::template apply<C24>::type{}(args...);
      break;
    case C25:
      typename F::template apply<C25>::type{}(args...);
      break;
    case C26:
      typename F::template apply<C26>::type{}(args...);
      break;
    case C27:
      typename F::template apply<C27>::type{}(args...);
      break;
    case C28:
      typename F::template apply<C28>::type{}(args...);
      break;
    case C29:
      typename F::template apply<C29>::type{}(args...);
      break;
    case C30:
      typename F::template apply<C30>::type{}(args...);
      break;
    case C31:
      typename F::template apply<C31>::type{}(args...);
      break;
    case C32:
      typename F::template apply<C32>::type{}(args...);
      break;
    case C33:
      typename F::template apply<C33>::type{}(args...);
      break;
    case C34:
      typename F::template apply<C34>::type{}(args...);
      break;
    case C35:
      typename F::template apply<C35>::type{}(args...);
      break;
    case C36:
      typename F::template apply<C36>::type{}(args...);
      break;
    case C37:
      typename F::template apply<C37>::type{}(args...);
      break;
    case C38:
      typename F::template apply<C38>::type{}(args...);
      break;
    case C39:
      typename F::template apply<C39>::type{}(args...);
      break;
    case C40:
      typename F::template apply<C40>::type{}(args...);
      break;
    case C41:
      typename F::template apply<C41>::type{}(args...);
      break;
    case C42:
      typename F::template apply<C42>::type{}(args...);
      break;
    case C43:
      typename F::template apply<C43>::type{}(args...);
      break;
    case C44:
      typename F::template apply<C44>::type{}(args...);
      break;
    case C45:
      typename F::template apply<C45>::type{}(args...);
      break;
    case C46:
      typename F::template apply<C46>::type{}(args...);
      break;
    case C47:
      typename F::template apply<C47>::type{}(args...);
      break;
    case C48:
      typename F::template apply<C48>::type{}(args...);
      break;
    case C49:
      typename F::template apply<C49>::type{}(args...);
      break;
    case C50:
      typename F::template apply<C50>::type{}(args...);
      break;
    case C51:
      typename F::template apply<C51>::type{}(args...);
      break;
    case C52:
      typename F::template apply<C52>::type{}(args...);
      break;
    case C53:
      typename F::template apply<C53>::type{}(args...);
      break;
    case C54:
      typename F::template apply<C54>::type{}(args...);
      break;
    case C55:
      typename F::template apply<C55>::type{}(args...);
      break;
    case C56:
      typename F::template apply<C56>::type{}(args...);
      break;
    case C57:
      typename F::template apply<C57>::type{}(args...);
      break;
    case C58:
      typename F::template apply<C58>::type{}(args...);
      break;
    case C59:
      typename F::template apply<C59>::type{}(args...);
      break;
    case C60:
      typename F::template apply<C60>::type{}(args...);
      break;
    case C61:
      typename F::template apply<C61>::type{}(args...);
      break;
    case C62:
      typename F::template apply<C62>::type{}(args...);
      break;
    case C63:
      typename F::template apply<C63>::type{}(args...);
      break;
    case C64:
      typename F::template apply<C64>::type{}(args...);
      break;
    case C65:
      typename F::template apply<C65>::type{}(args...);
      break;
    case C66:
      typename F::template apply<C66>::type{}(args...);
      break;
    case C67:
      typename F::template apply<C67>::type{}(args...);
      break;
    case C68:
      typename F::template apply<C68>::type{}(args...);
      break;
    case C69:
      typename F::template apply<C69>::type{}(args...);
      break;
    case C70:
      typename F::template apply<C70>::type{}(args...);
      break;
    case C71:
      typename F::template apply<C71>::type{}(args...);
      break;
    case C72:
      typename F::template apply<C72>::type{}(args...);
      break;
    case C73:
      typename F::template apply<C73>::type{}(args...);
      break;
    case C74:
      typename F::template apply<C74>::type{}(args...);
      break;
    case C75:
      typename F::template apply<C75>::type{}(args...);
      break;
    case C76:
      typename F::template apply<C76>::type{}(args...);
      break;
    case C77:
      typename F::template apply<C77>::type{}(args...);
      break;
    case C78:
      typename F::template apply<C78>::type{}(args...);
      break;
    case C79:
      typename F::template apply<C79>::type{}(args...);
      break;
    case C80:
      typename F::template apply<C80>::type{}(args...);
      break;
    case C81:
      typename F::template apply<C81>::type{}(args...);
      break;
    case C82:
      typename F::template apply<C82>::type{}(args...);
      break;
    case C83:
      typename F::template apply<C83>::type{}(args...);
      break;
    case C84:
      typename F::template apply<C84>::type{}(args...);
      break;
    default:
      fail(c);
    };
  }
};

    
} } }

#endif
