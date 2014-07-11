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

template <typename I>
I next(I i, std::size_t n = 1u)
{
  for(;n;--n)
    ++i;
  return i;
}

int main()
{
   efl::eina::eina_init eina_init;

   efl::eina::ptr_list<std::string, efl::eina::heap_copy_allocator> list;

   list.push_back("tigh");
   list.push_back("adar");
   list.push_back("baltar");
   list.push_back("roslin");

   for(efl::eina::ptr_list<std::string>::const_iterator
         first = list.begin(), last = list.end()
         ;first != last; ++first)
     std::cout << *first << std::endl;

   efl::eina::ptr_list<std::string>::iterator
     iterator = ::next(list.begin(), 2u);
   list.insert(iterator, "cain");

   iterator = std::find(list.begin(), list.end(), "cain");
   assert(iterator != list.end() && ::next(iterator) != list.end());
   list.insert(::next(iterator), "zarek");

   list.insert(list.begin(), "adama");

   iterator = std::find(list.begin(), list.end(), "cain");
   assert(iterator != list.end());
   list.insert(iterator, "gaeta");

   list.insert(::next(list.begin()), "lampkin");

   for(efl::eina::ptr_list<std::string>::const_iterator
         first = list.begin(), last = list.end()
         ;first != last; ++first)
     std::cout << *first << std::endl;
}
