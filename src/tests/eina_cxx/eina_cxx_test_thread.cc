
#include "Eina.hh"

#include <algorithm>

#include <check.h>

bool no_args = false
  , args_1 = false
  , args_2 = false;

void thread_no_args()
{
  no_args = true;
}

struct test
{
  int x;
};

void thread_1_arg(int a0)
{
  args_1 = true;
  ck_assert(a0 == 5);
}

void thread_2_arg(int a0, test t)
{
  args_2 = true;
  ck_assert(a0 == 5);
  ck_assert(t.x == 10);
}

START_TEST(eina_cxx_thread_constructors)
{
  efl::eina::eina_init init;
  efl::eina::eina_threads_init threads_init;
  {
    efl::eina::thread default_constructed_thread;
    ck_assert(default_constructed_thread.get_id() == efl::eina::thread::id());
  }

  {
    efl::eina::thread thread_no_args(&::thread_no_args);
    thread_no_args.join();
    ck_assert( ::no_args);
  }

  {
    efl::eina::thread thread_1_arg(&::thread_1_arg, 5);
    thread_1_arg.join();
    ck_assert( ::args_1);
  }

  {
    test t = {10};
    efl::eina::thread thread_2_arg(&::thread_2_arg, 5, t);
    thread_2_arg.join();
    ck_assert( ::args_2);
  }
}
END_TEST

START_TEST(eina_cxx_thread_mutexes)
{
  efl::eina::mutex m;

  {
    efl::eina::unique_lock<efl::eina::mutex> lock1(m);
    ck_assert(lock1.owns_lock());

    lock1.unlock();
    ck_assert(!lock1.owns_lock());

    ck_assert(lock1.try_lock());
    ck_assert(lock1.owns_lock());
    lock1.unlock();

    lock1.lock();
    ck_assert(lock1.owns_lock());
  }

  {
    efl::eina::lock_guard<efl::eina::mutex> lock1(m);
  }
}
END_TEST

bool b = false;

void condition_thread(efl::eina::mutex& condition_mutex
                      , efl::eina::condition_variable& condition_condition)
{
  efl::eina::unique_lock<efl::eina::mutex> l( condition_mutex);
  b = true;
  condition_condition.notify_one();
}

START_TEST(eina_cxx_thread_conditional)
{
  efl::eina::mutex m;

  efl::eina::mutex condition_mutex;
  efl::eina::condition_variable condition_condition;

  efl::eina::unique_lock<efl::eina::mutex> l( condition_mutex);
  efl::eina::thread thread(&condition_thread, std::ref(condition_mutex), std::ref(condition_condition));

  while(!b)
    {
      ck_assert(l.owns_lock());
      condition_condition.wait(l);
      ck_assert(l.owns_lock());
    }

  thread.join();
}
END_TEST

void
eina_test_thread(TCase* tc)
{
  tcase_add_test(tc, eina_cxx_thread_constructors);
  tcase_add_test(tc, eina_cxx_thread_mutexes);
  tcase_add_test(tc, eina_cxx_thread_conditional);
}
