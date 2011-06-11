//Compile with:
//gcc -g `pkg-config --cflags --libs eina` eina_list_03.c -o eina_list_03

#include <stdio.h>
#include <Eina.h>

int
main(int argc, char **argv)
{
   Eina_List *list = NULL, *r_list;
   Eina_List *l;
   Eina_Iterator *itr;
   void *list_data;

   eina_init();

   list = eina_list_append(list, "caprica");
   list = eina_list_append(list, "sagitarius");
   list = eina_list_append(list, "aerilon");
   list = eina_list_append(list, "gemenon");

   list = eina_list_promote_list(list, eina_list_nth_list(list, 2));
   list = eina_list_demote_list(list, eina_list_nth_list(list, 2));

   list = eina_list_remove(list, "sagitarius");

   l = eina_list_data_find_list(list, "aerilon");
   eina_list_data_set(l, "aquarius");

   r_list = eina_list_reverse_clone(list);

   itr = eina_list_iterator_new(r_list);
   EINA_ITERATOR_FOREACH(itr, list_data)
     printf("%s\n", (char*)list_data);
   eina_iterator_free(itr);

   eina_list_free(list);
   eina_list_free(r_list);

   eina_shutdown();

   return 0;
}
