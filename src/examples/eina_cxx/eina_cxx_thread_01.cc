//Compile with:
//gcc -g eina_list_01.c -o eina_list_01 `pkg-config --cflags --libs eina`

#include <stdio.h>
#include <Eina.hh>

#include <iterator>
#include <algorithm>

#include <iostream>

namespace eina = efl::eina;

void thread1(eina::mutex& m)
{
  
}

int main(int argc, char **argv)
{
   eina::eina_init eina_init;
   eina::eina_threads_init threads_init;

   eina::mutex m;
   eina::condition_variable c;

   eina::unique_lock<eina::mutex> l(m);

   eina::thread thread1(&::thread1, eina::ref(m));
   
   thread1.join();
}
