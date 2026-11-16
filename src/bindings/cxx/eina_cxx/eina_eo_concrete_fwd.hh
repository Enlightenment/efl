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
#ifndef EINA_EO_CONCRETE_FWD_HH
#define EINA_EO_CONCRETE_FWD_HH

#include <Eo.h>
#include <type_traits>
#include <atomic>
#include <mutex>
#include <unordered_map>

namespace efl { namespace eo {

struct concrete;

namespace detail {

/* Wrapping an object that is being destroyed.
 *
 * A callback or an event handler can be given an object while it is being
 * destroyed: it has no user reference left and is invalidated, and Eo frees
 * it once its destructor is done, whatever references were taken in the
 * meantime. No user reference alone is not enough: a NOREF handler sees an
 * object it may still keep, and a handler running after another one dropped
 * the last owner sees an object the emission keeps alive. A
 * wrapper made then cannot keep the object alive; if it takes a reference
 * anyway and is kept past the destruction, copying or dropping it later goes
 * to Eo with an id that is not valid any more.
 *
 * So such a wrapper takes no reference, and the object is written down here
 * with the number of wrappers holding it that way. Taking and dropping a
 * wrapper's reference looks here first: a written down object is counted,
 * never handed to Eo, and is forgotten when its last such wrapper goes. The
 * wrappers still point to the object while it is being destroyed, and to
 * nothing valid after: calling on one then is the caller's error, as with any
 * object deleted under a raw handle.
 *
 * The table is shared by the whole process only as far as this inline
 * function is: a wrapper should be dropped by the module that made it. It is
 * never destroyed, so that wrappers with static storage can be dropped after
 * it would have been.
 */
struct _dying_objects
{
  std::mutex lock;
  std::unordered_map<Eo const*, unsigned int> holders;
  std::atomic<unsigned int> size; // Read without the lock, for the fast path.

  _dying_objects() : size(0) {}

  static _dying_objects& get()
  {
    static _dying_objects* table = new _dying_objects;
    return *table;
  }

  static bool dying(Eo const* obj)
  {
    return (::efl_ref_count(obj) == 0)
      && (::efl_invalidated_get(obj) || ::efl_invalidating_get(obj));
  }

  // Counts one more holder when obj is written down already.
  bool held(Eo const* obj)
  {
    if (!size.load(std::memory_order_acquire)) return false;
    std::lock_guard<std::mutex> l(lock);
    auto it = holders.find(obj);
    if (it == holders.end()) return false;
    ++it->second;
    return true;
  }

  // Counts one holder less when obj is written down.
  bool released(Eo const* obj)
  {
    if (!size.load(std::memory_order_acquire)) return false;
    std::lock_guard<std::mutex> l(lock);
    auto it = holders.find(obj);
    if (it == holders.end()) return false;
    if (!--it->second)
      {
         holders.erase(it);
         size.store(static_cast<unsigned int>(holders.size()), std::memory_order_release);
      }
    return true;
  }

  void hold(Eo const* obj)
  {
    std::lock_guard<std::mutex> l(lock);
    ++holders[obj];
    size.store(static_cast<unsigned int>(holders.size()), std::memory_order_release);
  }
};

/* The reference a wrapper holds, taken and dropped. Every wrapper is dropped
 * through wrapper_unref, so every wrapper is made through wrapper_ref (or
 * takes over a reference the caller had, which is not written down).
 *
 * A class is an Eo id as well, but it lives as long as Eo does and cannot be
 * referenced: efl_ref and efl_unref on one are errors. An efl::Class is a
 * wrapper like any other and is copied and destroyed like one, so classes are
 * told apart and not referenced at all. */
inline Eo const*
wrapper_ref(Eo const* obj)
{
   if (!obj) return obj;
   _dying_objects& t = _dying_objects::get();
   if (t.held(obj)) return obj;
   if (::efl_class_get(obj) == EFL_CLASS_CLASS) return obj;
   if (_dying_objects::dying(obj))
     {
        t.hold(obj);
        return obj;
     }
   return ::efl_ref(obj);
}

inline void
wrapper_unref(Eo const* obj)
{
   if (!obj) return;
   if (_dying_objects::get().released(obj)) return;
   if (::efl_class_get(obj) == EFL_CLASS_CLASS) return;
   ::efl_unref(obj);
}

}

template <typename T>
struct is_eolian_object : std::false_type {};
template <typename T>
struct is_eolian_object<T const> : is_eolian_object<T> {};
template <>
struct is_eolian_object<eo::concrete> : std::true_type {};
    
} }


#endif
