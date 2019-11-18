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
//Compile with:
//gcc -g eina_list_01.c -o eina_list_01 `pkg-config --cflags --libs eina`

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <Eina.hh>

#include <iterator>
#include <algorithm>

#include <iostream>

namespace eina = efl::eina;

void thread1(eina::mutex&)
{
}

int main()
{
   eina::eina_init eina_init;
   eina::eina_threads_init threads_init;

   eina::mutex m;
   eina::condition_variable c;

   eina::unique_lock<eina::mutex> l(m);

   eina::thread thread1(&::thread1, eina::ref(m));

   thread1.join();
}
