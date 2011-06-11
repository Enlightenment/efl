//Compile with:
//gcc -g `pkg-config --cflags --libs eina` eina_list_01.c -o eina_list_01

#include <stdio.h>
#include <Eina.h>

int
main(int argc, char **argv)
{
   Eina_List *list = NULL;
   Eina_List *l;
   void *list_data;

   eina_init();

   list = eina_list_append(list, "tigh");
   list = eina_list_append(list, "adar");
   list = eina_list_append(list, "baltar");
   list = eina_list_append(list, "roslin");

   EINA_LIST_FOREACH(list, l, list_data)
     printf("%s\n", (char*)list_data);

   l = eina_list_nth_list(list, 1);
   list = eina_list_append_relative_list(list, "cain", l);

   list = eina_list_append_relative(list, "zarek", "cain");

   list = eina_list_prepend(list, "adama");

   list = eina_list_prepend_relative(list, "gaeta", "cain");

   list = eina_list_prepend_relative_list(list, "lampkin", l);

   EINA_LIST_FOREACH(list, l, list_data)
     printf("%s\n", (char*)list_data);

   eina_list_free(list);

   eina_shutdown();

   return 0;
}
