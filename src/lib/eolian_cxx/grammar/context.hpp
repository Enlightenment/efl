/*
 * Copyright 2019 by its authors. See AUTHORS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef EOLIAN_CXX_CONTEXT_HH
#define EOLIAN_CXX_CONTEXT_HH

namespace efl { namespace eolian { namespace grammar {

struct context_null {};

template <typename Tag, typename Tail = context_null>
struct context_cons
{
  typename std::remove_reference<Tag>::type const tag;
  typename std::remove_reference<Tail>::type const tail;
};

template <typename NewTag, typename Tag, typename Tail>
constexpr context_cons<NewTag, context_cons<Tag, Tail>>
context_add_tag(NewTag const& tag, context_cons<Tag, Tail> const& context)
{
  return context_cons<NewTag, context_cons<Tag, Tail>>{tag, context};
}
template <typename NewTag>
constexpr context_cons<NewTag, context_null>
context_add_tag(NewTag const& tag, context_null context)
{
  return context_cons<NewTag, context_null>{tag, context};
}

template <typename Tag, typename Tail>
constexpr context_cons<Tag, Tail>
context_replace_tag(Tag const& tag, context_cons<Tag, Tail> const& context)
{
  return {tag, context.tail};
}

template <typename Tag>
constexpr context_cons<Tag, context_null>
context_replace_tag(Tag const& tag, context_null const&)
{
  return context_cons<Tag, context_null>{tag, context_null{}};
}

template <typename Tag, typename OtherTag, typename Tail>
constexpr context_cons<OtherTag, decltype(context_replace_tag(std::declval<Tag>(), std::declval<Tail>()))>
context_replace_tag(Tag const& tag, context_cons<OtherTag, Tail> const& context)
{
  return {context.tag, context_replace_tag(tag, context.tail)};
}

template <typename Tag, typename Context>
struct tag_check;
template <typename Tag, typename Tail>
struct tag_check<Tag, context_cons<Tag, Tail>> : std::true_type {};
template <typename Tag>
struct tag_check<Tag, context_null> : std::false_type {};
template <typename Tag, typename OtherTag, typename Context>
struct tag_check<Tag, context_cons<OtherTag, Context>> : tag_check<Tag, Context> {};

template <typename Tag, typename SameTag, typename Tail> 
constexpr Tag const& context_find_tag(context_cons<SameTag, Tail> const& context
                            , typename std::enable_if<std::is_same<Tag, SameTag>::value>::type* = nullptr)
{
  return context.tag;
}

template <typename Tag, typename OtherTag, typename Tail> 
constexpr Tag const& context_find_tag(context_cons<OtherTag, Tail> const& context
                            , typename std::enable_if<!std::is_same<Tag, OtherTag>::value>::type* = nullptr)
{
  return context_find_tag<Tag>(context.tail);
}

template <typename Tag>
constexpr Tag const& context_find_tag(context_null const& context)
{
  static_assert(std::is_same<Tag, context_null>::value, "Tag type not available in this generation context");
  return context;
}

} } }

#endif
