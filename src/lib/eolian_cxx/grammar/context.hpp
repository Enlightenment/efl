#ifndef EOLIAN_CXX_CONTEXT_HH
#define EOLIAN_CXX_CONTEXT_HH

namespace efl { namespace eolian { namespace grammar {

struct context_null {};

template <typename Tag, typename Tail =  context_null>
struct context_cons
{
  Tag tag;
  Tail const& tail;
};
      
template <typename Tag>
struct context_cons<Tag, context_null>
{
  Tag tag;
  context_null tail;
};

template <typename NewTag, typename Tag, typename Tail>
context_cons<NewTag, context_cons<Tag, Tail>>
context_add_tag(NewTag tag, context_cons<Tag, Tail> const& context)
{
  return context_cons<NewTag, context_cons<Tag, Tail>>{tag, context};
}
template <typename NewTag>
context_cons<NewTag, context_null>
context_add_tag(NewTag tag, context_null context)
{
  return context_cons<NewTag, context_null>{tag, context};
}

template <typename Tag, typename Context>
struct tag_check;
template <typename Tag, typename Tail>
struct tag_check<Tag, context_cons<Tag, Tail>> : std::true_type {};
template <typename Tag>
struct tag_check<Tag, context_null> : std::false_type {};
template <typename Tag, typename OtherTag, typename Context>
struct tag_check<Tag, context_cons<OtherTag, Context>> : tag_check<Tag, Context> {};
      
} } }

#endif
