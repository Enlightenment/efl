//Compile with:
//gcc -g eina_list_02.c -o eina_list_02 `pkg-config --cflags --libs eina`

#include <stdio.h>
#include <string.h>
#include <Eina.h>

int
main(int argc, char **argv)
{
   (void)argc;
   (void)argv;
   Eina_List *list = NULL, *other_list = NULL;
   Eina_List *l;
   void *data;
   int cmp_result;
   Eina_Compare_Cb cmp_func = (Eina_Compare_Cb)strcmp;

   eina_init();

   list = eina_list_append(list, "starbuck");
   list = eina_list_append(list, "appolo");
   list = eina_list_append(list, "boomer");

   data = eina_list_search_unsorted(list, cmp_func, "boomer");
   l = eina_list_search_unsorted_list(list, cmp_func, "boomer");
   if (l->data != data)
     return 1;

   list = eina_list_sort(list, 0, cmp_func);

   data = eina_list_search_sorted(list, cmp_func, "starbuck");
   l = eina_list_search_sorted_list(list, cmp_func, "starbuck");
   if (l->data != data)
     return 1;

   list = eina_list_sorted_insert(list, cmp_func, "helo");

   l = eina_list_search_sorted_near_list(list, cmp_func, "hera", &cmp_result);
   if (cmp_result > 0)
     list = eina_list_prepend_relative_list(list, "hera", l);
   else if (cmp_result < 0)
     list = eina_list_append_relative_list(list, "hera", l);

   l = eina_list_search_sorted_list(list, cmp_func, "boomer");
   list = eina_list_split_list(list, l, &other_list);

   other_list = eina_list_sort(other_list, 0, cmp_func);

   list = eina_list_sorted_merge(list, other_list, cmp_func);

   eina_list_free(list);

   eina_shutdown();

   return 0;
}
