#ifndef EINA_THREAD_HH_
#define EINA_THREAD_HH_

#include <Eina.h>
#include <eina_error.hh>

#include <memory>
#include <iterator>
#include <cstdlib>
#include <cassert>
#include <iosfwd>
#include <functional>
#include <chrono>
#include <mutex>

#define EFL_EINA_BOOST_MOVABLE_BUT_NOT_COPYABLE(x)
#define EFL_EINA_BOOST_RV_REF(x) x const&

namespace efl { namespace eina {

struct mutex
{
  typedef Eina_Lock* native_handle_type;

  mutex()
  {
    ::eina_lock_new(&_mutex);
  }
  ~mutex()
  {
    ::eina_lock_free(&_mutex);
  }
  void lock()
  {
    ::Eina_Lock_Result r = ::eina_lock_take(&_mutex);
    switch(r)
      {
      case EINA_LOCK_SUCCEED:
        return;
      case EINA_LOCK_DEADLOCK:
        throw system_error(error_code(int(eina::errc::resource_deadlock_would_occur)
                                      , get_generic_category()));
      default:
        throw system_error(get_error_code());
      }
  }
  bool try_lock()
  {
    ::Eina_Lock_Result r = ::eina_lock_take_try(&_mutex);
    switch(r)
      {
      case EINA_LOCK_SUCCEED:
        return true;
      case EINA_LOCK_FAIL:
        return false;
      case EINA_LOCK_DEADLOCK:
        throw system_error(error_code(int(eina::errc::resource_deadlock_would_occur)
                                      , get_generic_category()));
      default:
        throw system_error(get_error_code());
      }
  }
  void unlock()
  {
    ::Eina_Lock_Result r = ::eina_lock_release(&_mutex);
    switch(r)
      {
      case EINA_LOCK_SUCCEED:
        return;
      case EINA_LOCK_DEADLOCK:
        throw system_error(error_code(int(eina::errc::resource_deadlock_would_occur)
                                      , get_generic_category()));
      default:
        throw system_error(get_error_code());
      }
  }
  void debug()
  {
    ::eina_lock_debug(&_mutex);
  }
  native_handle_type native_handle()
  {
    return &_mutex;
  }
private:
  mutex(mutex const&) = delete;
  mutex& operator=(mutex const&) = delete;

  Eina_Lock _mutex;
};

using std::lock_guard;
using std::unique_lock;

struct condition_variable
{
  typedef Eina_Condition* native_handle_type;

  condition_variable()
  {
    ::eina_condition_new(&_cond, _mutex.native_handle());
  }
  ~condition_variable()
  {
    ::eina_condition_free(&_cond);
  }

  void notify_one()
  {
    eina::unique_lock<eina::mutex> l(_mutex);
    Eina_Bool r = eina_condition_signal(&_cond);
    if(!r)
      throw eina::system_error(eina::get_error_code());
  }
  void notify_all()
  {
    eina::unique_lock<eina::mutex> l(_mutex);
    Eina_Bool r = eina_condition_broadcast(&_cond);
    if(!r)
      throw eina::system_error(eina::get_error_code());
  }
  template <typename Lock>
  void wait(Lock& lock)
  {
    eina::unique_lock<eina::mutex> l(_mutex);
    lock.unlock();
    ::eina_condition_wait(&_cond);
    lock.lock();
  }
  template <typename Lock, typename Predicate>
  void wait(Lock& lock, Predicate p)
  {
    while(!p())
      wait(lock);
  }
  native_handle_type native_handle()
  {
    return &_cond;
  }
private:
  condition_variable(condition_variable const&);
  condition_variable& operator=(condition_variable const&);

  mutex _mutex;
  Eina_Condition _cond;
};

struct thread_id
{
  thread_id() noexcept
    : _raw(0u)
  {
  }
  thread_id(Eina_Thread raw)
    : _raw(raw) {}
  friend inline bool operator==(thread_id lhs, thread_id rhs)
  {
    return lhs._raw == rhs._raw;
  }
  friend inline bool operator!=(thread_id lhs, thread_id rhs)
  {
    return lhs._raw != rhs._raw;
  }
  friend inline bool operator<(thread_id lhs, thread_id rhs)
  {
    return std::less<Eina_Thread>()(lhs._raw, rhs._raw);
  }
private:
  Eina_Thread _raw;
};

inline bool operator<=(thread_id lhs, thread_id rhs)
{
  return (lhs == rhs) || lhs < rhs;
}
inline bool operator>(thread_id lhs, thread_id rhs)
{
  return !(lhs <= rhs);
}
inline bool operator>=(thread_id lhs, thread_id rhs)
{
  return !(lhs < rhs);
}

template <typename charT, typename Traits>
std::basic_ostream<charT, Traits>&
operator<<(std::basic_ostream<charT, Traits>& out, thread_id id)
{
  return out << id._raw;
}

namespace _detail {

struct arguments
{
  Eina_Lock mutex;
  Eina_Condition condition;
  bool started;
  std::function<void()> function;
};

inline void* create_thread(void* data, Eina_Thread)
{
  arguments* args = static_cast<arguments*>(data);

  eina_lock_take(&args->mutex);

  std::function<void()> f = std::move(args->function);

  args->started = true;
  eina_condition_signal(&args->condition);
  eina_lock_release(&args->mutex);

  f();
  return 0;
}

}

struct thread
{
  typedef thread_id id;
  typedef Eina_Thread native_handle_type;

  thread() noexcept
    : _joinable(false), _raw(0u)
  {
  }

  template <typename F, class ... Args>
  explicit thread(F&& f, Args&&... args)
  {
    _detail::arguments arguments;
    arguments.started = false;
    arguments.function = std::bind(f, args...);
    
    _joinable = true;
    Eina_Bool r = ::eina_lock_new(&arguments.mutex);
    if(!r) throw eina::system_error(eina::get_error_code());
    r = ::eina_condition_new(&arguments.condition, &arguments.mutex);
    if(!r) throw eina::system_error(eina::get_error_code());

    if(!eina_thread_create
       (&_raw, ::EINA_THREAD_NORMAL
        , -1, &eina::_detail::create_thread, &arguments))
      {
        eina_condition_free(&arguments.condition);
        eina_lock_free(&arguments.mutex);
        throw eina::system_error(eina::get_error_code());
      }
    Eina_Lock_Result lr = ::eina_lock_take(&arguments.mutex);
    if(lr != EINA_LOCK_SUCCEED)
      throw eina::system_error(eina::get_error_code());
    while(!arguments.started)
      {
        r = eina_condition_wait(&arguments.condition);
        if(!r) throw eina::system_error(eina::get_error_code());
      }
    lr = eina_lock_release(&arguments.mutex);
    if(lr != EINA_LOCK_SUCCEED)
      throw eina::system_error(eina::get_error_code());

    eina_condition_free(&arguments.condition);
    eina_lock_free(&arguments.mutex);
  }

  thread(thread&& other)
    : _joinable(other._joinable), _raw(other._raw)
  {
  }

  thread& operator=(thread&& other)
  {
    _raw = other._raw;
    _joinable = other._joinable;
    return *this;
  }

  ~thread()
  {
    assert(!joinable());
  }
 
  void swap(thread& other) noexcept
  {
    std::swap(_raw, other._raw);
  }
  bool joinable() const noexcept
  {
    return _joinable;
  }

  void join()
  {
    assert(joinable());
    ::eina_thread_join(_raw);
    _joinable = false;
  }

  void detach()
  {
    assert(joinable());
    _joinable = false;
  }

  id get_id() const noexcept
  {
    return id(_raw);
  }
  native_handle_type native_handle() const
  {
    return _raw;
  }

  static unsigned hardware_concurrency() noexcept
  {
    return ::eina_cpu_count();
  }
private:
  bool _joinable;
  Eina_Thread _raw;
};

inline void swap(thread& lhs, thread& rhs)
{
  lhs.swap(rhs);
}

namespace this_thread {

inline thread::id get_id()
{
  return thread::id(eina_thread_self());
}

inline void yield() {}

template <typename Clock, typename Duration>
void sleep_until(std::chrono::time_point<Clock, Duration>const& abs_time);

template <typename Rep, typename Period>
void sleep_for(std::chrono::duration<Rep, Period>const& rel_time);
}

} }

namespace std {

template <typename T> struct hash;
template <>
struct hash< ::efl::eina::thread_id> : hash<unsigned long>
{};

}

#endif
