
#include "Ecore.hh"
#include "Eina.hh"

#include <algorithm>

#include <iostream>

#include <check.h>

void call_async(efl::eina::mutex& mutex, efl::eina::condition_variable& cond, bool& done)
{
  efl::ecore::main_loop_thread_safe_call_async
    (
     [&mutex,&cond,&done]
     {
       std::cout << "yeah" << std::endl;
       ecore_main_loop_quit();
       efl::eina::unique_lock<efl::eina::mutex> l(mutex);
       std::cout << "mutex locked" << std::endl;
       done = true;
       cond.notify_one();
       std::cout << "exiting" << std::endl;
     }
    );
}

START_TEST(ecore_cxx_safe_call_async)
{
  efl::ecore::ecore_init init;

  efl::eina::mutex mutex;
  efl::eina::condition_variable cond;
  bool done = false;
  efl::eina::thread thread(&call_async, std::ref(mutex), std::ref(cond), std::ref(done));

  ecore_main_loop_begin();

  std::cout << "joining" << std::endl;

  thread.join();

  std::cout << "joined" << std::endl;

  efl::eina::unique_lock<efl::eina::mutex> l(mutex);
  while(!done)
    {
      std::cout << "waiting" << std::endl;
      cond.wait(l);
      std::cout << "waited" << std::endl;
    }

  std::cout << "end of ecore_cxx_safe_call_async" << std::endl;
}
END_TEST

struct big_pod
{
  double x;
  double y;
};

int constructor_called = 0
  , destructor_called = 0;

struct small_nonpod
{
  small_nonpod()
    : c(5)
  {
    constructor_called++;
  }
  small_nonpod(small_nonpod const& other)
    : c(5)
  {
    ck_assert(other.c == 5);
    constructor_called++;
  }
  ~small_nonpod()
  {
    ck_assert(c == 5);
    destructor_called++;
  }
  char c;
};

struct big_nonpod : big_pod
{
  big_nonpod()
  {
    constructor_called++;
    x = 2.0;
    y = 1.0;
  }
  big_nonpod(big_nonpod const& other)
  {
    x = 2.0;
    y = 1.0;
    ck_assert(other.x == 2.0);
    ck_assert(other.y == 1.0);
    constructor_called++;
  }
  ~big_nonpod()
  {
    ck_assert(x == 2.0);
    ck_assert(y == 1.0);
    destructor_called++;
  }
  char c;
};

void call_sync_int()
{
  std::cout << "call_sync_init" << std::endl;
  int r1 =
  efl::ecore::main_loop_thread_safe_call_sync
    (
     [] () -> int
     {
       return 1;
     }
    );
  ck_assert(r1 == 1);

  std::cout << "big_pod" << std::endl;

  big_pod r2 =
  efl::ecore::main_loop_thread_safe_call_sync
    (
     [] () -> big_pod
     {
       return {1.0, 2.0};
     }
    );
  ck_assert(r2.x == 1.0);
  ck_assert(r2.y == 2.0);

  std::cout << "small_nonpod" << std::endl;

  {
    small_nonpod r3 =
      efl::ecore::main_loop_thread_safe_call_sync
      (
       [] () -> small_nonpod
       {
         return small_nonpod();
       }
       );
  }
  std::cout << "constructor_called: " << constructor_called << std::endl;
  std::cout << "destructor_called: " << destructor_called << std::endl;
  ck_assert(constructor_called == destructor_called);

  std::cout << "big_nonpod" << std::endl;

  {
    big_nonpod r3 =
      efl::ecore::main_loop_thread_safe_call_sync
      (
       [] () -> big_nonpod
       {
         std::cout << "before quit" << std::endl;
         ecore_main_loop_quit();
         std::cout << "are we calling here" << std::endl;
         return big_nonpod();
       }
       );
  }
  std::cout << "constructor_called: " << constructor_called << std::endl;
  std::cout << "destructor_called: " << destructor_called << std::endl;
  ck_assert(constructor_called == destructor_called);
}

START_TEST(ecore_cxx_safe_call_sync)
{
  efl::ecore::ecore_init init;

  efl::eina::thread thread(&call_sync_int);

  ecore_main_loop_begin();

  std::cout << "out of the loop" << std::endl;

  thread.join();
}
END_TEST

void
ecore_test_safe_call(TCase* tc)
{
  tcase_add_test(tc, ecore_cxx_safe_call_async);
  tcase_add_test(tc, ecore_cxx_safe_call_sync);
}
