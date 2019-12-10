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
#ifndef EOLIAN_MONO_TYPE_MATCH_HH
#define EOLIAN_MONO_TYPE_MATCH_HH

#include <Eina.hh>

namespace eolian_mono {

namespace type_match {

namespace eina = efl::eina;

/**
 * Apply the given AcceptFunction to the first element that matches the SelectionPredicate.
 */
template <typename Array, typename SelectionPredicate, int N, typename AcceptFunc>
eina::optional<bool> get_match(Array const (&array)[N], SelectionPredicate predicate, AcceptFunc acceptFunc)
{
   typedef Array const* iterator_type;
   iterator_type match_iterator = &array[0], match_last = match_iterator + N;
   match_iterator = std::find_if(match_iterator, match_last, predicate);
   if(match_iterator != match_last)
     {
        return acceptFunc(match_iterator->function());
     }
   return {nullptr};
}

} // type_match

} // eolian_mono
#endif
