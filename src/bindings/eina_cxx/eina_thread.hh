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
#include <functional>

/**
 * @addtogroup Eina_Cxx_Tools_Group Tools
 *
 * @{
 */

#define EFL_EINA_BOOST_MOVABLE_BUT_NOT_COPYABLE(x)
#define EFL_EINA_BOOST_RV_REF(x) x const&

namespace efl { namespace eina {

/**
 * @defgroup Eina_Cxx_Mutex_Group Mutex
 * @ingroup Eina_Cxx_Tools_Group
 *
 * @{
 */

/**
 * Provides an OOP interface to the @c Eina_Lock and automatic resource
 * allocation and deallocation using the RAII programming idiom.
 *
 * This class implements mutual exclusion variables (mutexes) in a way
 * that strongly resembles the STL <tt>std::mutex</tt>.
 */
struct mutex
{
  typedef Eina_Lock* native_handle_type; /**< Type for the native Eina_Lock pointer. */


  /**
   * @brief Create a new mutex.
   *
   * Automatically allocates a new mutex and does any platform dependent
   * initialization that is required.
   */
  mutex()
  {
    ::eina_lock_new(&_mutex);
  }

  /**
   * @brief Release mutex resources.
   *
   * Automatically deallocates the mutex and does any platform dependent
   * cleanup that is required.
   */
  ~mutex()
  {
    ::eina_lock_free(&_mutex);
  }

  /**
   * @brief Lock the mutex.
   * @throw <tt>eina::system_error</tt> with the code
   *        <tt>eina::errc::resource_deadlock_would_occur</tt> if the
   *        operation fails because a deadlock condition exists. If some
   *        other condition causes the lock to fail (other than the
   *        mutex being already locked) the error code will be an
   *        internal Eina error code.
   *
   * This member function locks the mutex. If the mutex is locked
   * already, this call will block until the lock is released. This is
   * appropriate in many cases, but consider using @ref try_lock() if
   * you don't need to block.
   */
  void lock()
  {
    ::Eina_Lock_Result r = ::eina_lock_take(&_mutex);
    switch(r)
      {
      case EINA_LOCK_SUCCEED:
        return;
      case EINA_LOCK_DEADLOCK:
        EFL_CXX_THROW(system_error(error_code(int(eina::errc::resource_deadlock_would_occur), 
                                              get_generic_category())));
      default:
        EFL_CXX_THROW(system_error(get_error_code()));
      }
  }

  /**
   * @brief Attempts to lock the mutex.
   * @return @c true if it succeed in locking the mutex, @c false otherwise.
   * @throw <tt>eina::system_error</tt> with the code
   *        <tt>eina::errc::resource_deadlock_would_occur</tt> if the
   *        operation fails because a deadlock condition exists. If some
   *        other condition causes the lock to fail (other than the
   *        mutex being already locked) the error code will be an
   *        internal Eina error code.
   *
   * This member function attempts to lock the mutex, identical to
   * @ref lock(), but returns immediately if the mutex is already locked.
   */
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
        EFL_CXX_THROW(system_error(error_code(int(eina::errc::resource_deadlock_would_occur), 
                                              get_generic_category())));
      default:
        EFL_CXX_THROW(system_error(get_error_code()));
      }
  }

  /**
   * @brief Unlock the lock.
   * @throw <tt>eina::system_error</tt> with the code
   *        <tt>eina::errc::resource_deadlock_would_occur</tt> if the
   *        operation fails because a deadlock condition exists. If some
   *        other condition causes the lock to fail the error code will
   *        be an internal Eina error code.
   *
   * This member function will unlock the mutex.
   *
   * @note If successful, and EINA_HAVE_DEBUG_THREADS is defined, the
   *       mutex is updated and information about the locking process
   *       is removed (e.g. thread number and backtrace for POSIX).
   */
  void unlock()
  {
    ::Eina_Lock_Result r = ::eina_lock_release(&_mutex);
    switch(r)
      {
      case EINA_LOCK_SUCCEED:
        return;
      case EINA_LOCK_DEADLOCK:
        EFL_CXX_THROW(system_error(error_code(int(eina::errc::resource_deadlock_would_occur),
                                              get_generic_category())));
      default:
        EFL_CXX_THROW(system_error(get_error_code()));
      }
  }

  /**
   * @brief Print debug information about the mutex.
   *
   * This member function prints debug information for the mutex. The
   * information is platform dependent. On POSIX systems it will print
   * the address of mutex, lock state, thread number and a backtrace.
   */
  void debug()
  {
    ::eina_lock_debug(&_mutex);
  }

  /**
   * @brief Get a handle for the wrapped @c Eina_Lock.
   * @return Handle for the native @c Eina_Lock.
   *
   * This member function returns the native @c Eina_Lock handle that is
   * wrapped inside this object.
   *
   * @warning It is important to take care when using it, since the
   * handle will be automatically released upon object destruction.
   */
  native_handle_type native_handle()
  {
    return &_mutex;
  }
private:
  /** Disabled copy constructor. **/
  mutex(mutex const&) = delete;
  /** Disabled assignment operator. **/
  mutex& operator=(mutex const&) = delete;

  /**
   * @internal
   */
  Eina_Lock _mutex;
};

/**
 * @brief Manage a mutex object by keeping it always locked.
 *
 * Inherited for the STL object <tt>std::lock_guard</tt>.
 */
using std::lock_guard;

/**
 * @brief Manages a mutex object.
 *
 * Inherited for the STL object <tt>std::unique_lock</tt>. This class
 * guarantees an unlocked status on destruction.
 */
using std::unique_lock;

/**
 * @}
 */

/**
 * @defgroup Eina_Cxx_Condition_Variable_Group Condition Variable
 * @ingroup Eina_Cxx_Tools_Group
 *
 * @{
 */

/**
 * Provides an OOP interface to the @c Eina_Condition and automatic
 * resource allocation and deallocation using the RAII programming idiom.
 *
 * This class implements condition variables in a way that strongly
 * resembles the STL <tt>std::condition_variable</tt>.
 */
struct condition_variable
{
  typedef Eina_Condition* native_handle_type; /**< Type for the native Eina_Lock pointer. */

  /**
   * @brief Create a new condition variable.
   *
   * Automatically allocates a new condition variable and does any
   * platform dependent initialization that is required.
   */
  condition_variable()
  {
    ::eina_condition_new(&_cond, _mutex.native_handle());
  }

  /**
   * @brief Release the condition variable resources.
   *
   * Automatically deallocates the condition variable and does any
   * platform dependent cleanup that is required.
   */
  ~condition_variable()
  {
    ::eina_condition_free(&_cond);
  }

  /**
   * @brief Unblock a thread waiting for this condition.
   * @throw <tt>eina::system_error</tt> on fail.
   *
   * This member function unblock a thread waiting on this condition
   * variable. If there is more than one thread waiting on this
   * condition, one of them will be unblocked, but which one is
   * undefined. If you do not know for sure that there is only one
   * thread waiting, use @ref notify_all() instead.
   */
  void notify_one()
  {
    eina::unique_lock<eina::mutex> l(_mutex);
    Eina_Bool r = eina_condition_signal(&_cond);
    if(!r)
      {
         EFL_CXX_THROW(eina::system_error(eina::get_error_code()));
      }
  }

  /**
   * @brief Unblock all threads waiting for this condition.
   * @throw <tt>eina::system_error</tt> on fail.
   *
   * This member function unblocks all the threads waiting on the this
   * condition. If you know for sure that there is only one thread
   * waiting, use @ref notify_one instead to gain a little optimization.
   */
  void notify_all()
  {
    eina::unique_lock<eina::mutex> l(_mutex);
    Eina_Bool r = eina_condition_broadcast(&_cond);
    if(!r)
      {
         EFL_CXX_THROW(eina::system_error(eina::get_error_code()));
      }
  }

  /**
   * @brief Causes a thread to wait until notified.
   * @param lock A lockable object (@c mutex, @c unique_lock, etc) that
   *             is currently locked by this thread. All concurrent
   *             calls to wait member functions of this object shall use
   *             the same lockable object.
   *
   * This member function makes a thread block until notified.
   */
  template <typename Lock>
  void wait(Lock& lock)
  {
    eina::unique_lock<eina::mutex> l(_mutex);
    lock.unlock();
    ::eina_condition_wait(&_cond);
    lock.lock();
  }

  /**
   * @brief Causes a thread to wait until notified.
   * @param lock A lockable object (@c mutex, @c unique_lock, etc) that
   *             is currently locked by this thread. All concurrent
   *             calls to wait member functions of this object shall use
   *             the same lockable object.
   * @param p A callable object or function that takes no arguments and
   *          returns a value that can be evaluated as a bool. This is
   *          called repeatedly until it evaluates to true.
   *
   * This member function only blocks the thread if @p p is evaluated to
   * @c false. In this case the thread remains blocked until notified
   * and the result of @p p evaluates to @c true.
   */
  template <typename Lock, typename Predicate>
  void wait(Lock& lock, Predicate p)
  {
    while(!p())
      wait(lock);
  }

  /**
   * @brief Get a handle for the wrapped @c Eina_Condition.
   * @return Handle for the native @c Eina_Condition.
   *
   * This member function returns the native @c Eina_Condition handle
   * that is wrapped inside this object.
   *
   * @warning It is important to take care when using it, since the
   * handle will be automatically released upon object destruction.
   */
  native_handle_type native_handle()
  {
    return &_cond;
  }
private:
  /** Disabled copy constructor. **/
  condition_variable(condition_variable const&);
  /** Disabled assignment operator. **/
  condition_variable& operator=(condition_variable const&);

  mutex _mutex; /**< @internal */
  Eina_Condition _cond; /**< @internal */
};

/**
 * @}
 */

/**
 * @defgroup Eina_Cxx_Thread_Group Thread
 * @ingroup Eina_Cxx_Tools_Group
 *
 * @{
 */

/**
 * Thread identifier.
 */
struct thread_id
{
  /**
   * @brief Creates a @c thread_id that represents all non-joinable.
   */
  thread_id() noexcept
    : _raw(0u)
  {
  }

  /**
   * @brief
   */
  thread_id(Eina_Thread raw)
    : _raw(raw) {}

  /**
   * @brief Check if two thread identifiers are the same.
   * @return @c true if the thread identifiers have the same value.
   */
  friend inline bool operator==(thread_id lhs, thread_id rhs)
  {
    return lhs._raw == rhs._raw;
  }

  /**
   * @brief Check if two thread identifiers are different.
   * @return @c true if the thread identifiers have different values.
   */
  friend inline bool operator!=(thread_id lhs, thread_id rhs)
  {
    return lhs._raw != rhs._raw;
  }

  /**
   * @brief Less than comparison of thread identifiers.
   * @param lhs @c thread_id at the left side of the expression.
   * @param rhs @c thread_id at the right side of the expression.
   * @return @c true if @c lhs is less than @c rhs, @c false otherwise.
   * @note The order established by relational operators is
   * implementation-defined.
   */
  friend inline bool operator<(thread_id lhs, thread_id rhs)
  {
    return std::less<Eina_Thread>()(lhs._raw, rhs._raw);
  }
private:
  Eina_Thread _raw; /**< @internal */

  /**
   * @brief Inserts a textual representation in the given stream.
   * @param out Output stream where the textual representation will be inserted.
   * @param id @c thread_id object.
   * @return Reference to the modified <tt>std::basic_ostream</tt> object.
   */
  template <typename charT, typename Traits>
  friend std::basic_ostream<charT, Traits>&
  operator<<(std::basic_ostream<charT, Traits>& out, thread_id id)
  {
    return out << id._raw;
  }
};

/**
 * @brief Less than or equal comparison of thread identifiers.
 * @param lhs @c thread_id at the left side of the expression.
 * @param rhs @c thread_id at the right side of the expression.
 * @return @c true if @c lhs is less than or equal to @c rhs, @c false otherwise.
 * @note The order established by relational operators is
 * implementation-defined.
 */
inline bool operator<=(thread_id lhs, thread_id rhs)
{
  return (lhs == rhs) || lhs < rhs;
}

/**
 * @brief More than comparison of thread identifiers.
 * @param lhs @c thread_id at the left side of the expression.
 * @param rhs @c thread_id at the right side of the expression.
 * @return @c true if @c lhs is more than @c rhs, @c false otherwise.
 * @note The order established by relational operators is
 * implementation-defined.
 */
inline bool operator>(thread_id lhs, thread_id rhs)
{
  return !(lhs <= rhs);
}

/**
 * @brief More than or equal comparison of thread identifiers.
 * @param lhs @c thread_id at the left side of the expression.
 * @param rhs @c thread_id at the right side of the expression.
 * @return @c true if @c lhs is more than or equal to @c rhs, @c false otherwise.
 * @note The order established by relational operators is
 * implementation-defined.
 */
inline bool operator>=(thread_id lhs, thread_id rhs)
{
  return !(lhs < rhs);
}

/**
 * @internal
 */
namespace _detail {

/**
 * @internal
 */
struct arguments
{
  Eina_Lock mutex;
  Eina_Condition condition;
  bool started;
  std::function<void()> function;
};

/**
 * @internal
 */
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

/**
 * Provides an OOP interface to the @c Eina_Thread and automatic
 * resource allocation and deallocation using the RAII programming idiom.
 *
 * This class implements threads in a way that strongly resembles the
 * STL <tt>std::thread</tt>.
 */
struct thread
{
  typedef thread_id id; /**< Type for the thread identifier. */
  typedef Eina_Thread native_handle_type; /**< Type for the native Eina_Thread handle. */

  /**
   * @brief Creates a thread object that does not represent any thread of execution.
   */
  thread() noexcept
    : _joinable(false), _raw(0u)
  {
  }

  /**
   * @brief Creates a thread of execution.
   * @param f Pointer to function or callable object to execute in the new thread.
   *          The return value (if any) is ignored.
   * @param args Arguments to pass to the @p f.
   *
   * This constructor creates a thread object that represents a thread
   * of execution. The new thread of execution calls @p f passing
   * @p args as arguments (all arguments are copied/moved to
   * thread-accessible storage).
   *
   * Any exceptions thrown during evaluation and copying/moving of the
   * arguments are thrown in the current thread, not the new thread.
   */
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

  /**
   * @brief Move constructor. Transfer the thread of execution to the new object.
   * @param other Another thread object to construct this thread object with.
   *
   * This constructor creates a thread object that acquires the thread
   * of execution represented by @p other. This operation does not
   * affect the execution of the moved thread, it simply transfers its
   * handler.
   *
   * @note After this call @p other no longer represents a thread of execution.
   */
  thread(thread&& other)
    : _joinable(other._joinable), _raw(other._raw)
  {
  }

  /**
   * @brief Transfer the thread of execution.
   * @param other Another thread object to assign to this thread object.
   * @note After this call @p other no longer represents a thread of execution.
   */
  thread& operator=(thread&& other)
  {
    _raw = other._raw;
    _joinable = other._joinable;
    return *this;
  }

  /**
   * @brief Destroys the thread object.
   */
  ~thread()
  {
    assert(!joinable());
  }

  /**
   * @brief Exchanges the underlying handles of two thread objects.
   * @param other Another thread object.
   */
  void swap(thread& other) noexcept
  {
    std::swap(_raw, other._raw);
  }

  /**
   * @brief Check if the thread object identifies an active thread of execution.
   * @return @c true if the thread object identifies an active thread of execution, @c false otherwise.
   *
   * This member function checks if the thread object identifies an
   * active thread of execution. A default constructed thread is not
   * joinable, as well as a thread that its members join or detach has
   * been called.
   *
   * A thread that has finished executing code, but has not yet been
   * joined is still considered an active thread of execution and is
   * therefore joinable.
   */
  bool joinable() const noexcept
  {
    return _joinable;
  }

  /**
   * @brief Wait for the thread to finish its execution.
   *
   * This member function blocks the calling thread until the thread
   * identified by this object finishes its execution.
   *
   * @note A joinable thread becomes not joinable after a call to this
   * function.
   */
  void join()
  {
    assert(joinable());
    ::eina_thread_join(_raw);
    _joinable = false;
  }

  /**
   * @brief Detaches the thread from its handle, making it runs independently.
   *
   * This member function separates the thread of execution from the
   * thread object, allowing execution to continue independently.
   *
   * @note After a call to this function, the thread object becomes
   * non-joinable.
   */
  void detach()
  {
    assert(joinable());
    _joinable = false;
  }

  /**
   * @brief Returns the identifier of the thread associated with this thread object.
   * @return <tt>thread::id</tt> identifying the thread associated with this thread object.
   */
  id get_id() const noexcept
  {
    return id(_raw);
  }

  /**
   * @brief Get a handle for the wrapped @c Eina_Thread.
   * @return Handle for the native @c Eina_Thread.
   *
   * This member function returns the native @c Eina_Thread handle that
   * is wrapped inside this object.
   */
  native_handle_type native_handle() const
  {
    return _raw;
  }

  /**
   * @brief Get the number of hardware concurrent threads.
   * @return A hint on the number of hardware concurrent threads, or
   *         @c 0 if the value is not well defined or not computable.
   *
   * This static member function returns the number of hardware
   * concurrent threads.
   *
   * @note The interpretation of this value is implementation-specific,
   * and may be just an approximation.
   */
  static unsigned hardware_concurrency() noexcept
  {
    return ::eina_cpu_count();
  }
private:
  /** @internal */
  bool _joinable;

  /** @internal */
  Eina_Thread _raw;
};

/**
 * @brief Exchanges the underlying handles of two thread objects.
 * @param lhs First thread object.
 * @param rhs Second thread object.
 */
inline void swap(thread& lhs, thread& rhs)
{
  lhs.swap(rhs);
}

namespace this_thread {

/**
 * @brief Return identifier of the current thread.
 * @return <tt>thread::id</tt> identifying the current thread.
 */
inline thread::id get_id()
{
  return thread::id(eina_thread_self());
}

/**
 * @brief Provides a hint to the implementation to reschedule the
 * execution of threads, allowing other threads to run.
 */
inline void yield() {}

/**
 * @brief Block the execution of the current thread until a specified time point.
 * @param abs_time Point in time when the calling thread shall resume its execution.
 *
 * @note This function may block for longer than until after @p rel_time
 * has been reached due to scheduling or resource contention delays.
 */
template <typename Clock, typename Duration>
void sleep_until(std::chrono::time_point<Clock, Duration>const& abs_time);

/**
 * @brief Block the execution of the current thread for a specified time duration.
 * @param rel_time Time span after which the calling thread shall resume its execution.
 *
 * @note This function may block for longer than @p rel_time due to
 * scheduling or resource contention delays.
 */
template <typename Rep, typename Period>
void sleep_for(std::chrono::duration<Rep, Period>const& rel_time);
}

/**
 * @}
 */

} }

/**
 * @internal
 * Specialization of standard @c hash class to specify that a
 * <tt>thread_id</tt> object should be handled as a unsigned long
 * @{
 */
namespace std {

template <>
struct hash< ::efl::eina::thread_id> : hash<unsigned long>
{};

}
/**
 * @}
 */

/**
 * @}
 */

#endif
