//Compile with:
//gcc -g `pkg-config --cflags --libs eina` eina_list_04.c -o eina_list_04

#include <stdio.h>
#include <Eina.h>

int
main(int argc, char **argv)
{
   Eina_List *list = NULL;
   Eina_List *l;
   void *list_data;

   eina_init();

   list = eina_list_append(list, eina_stringshare_add("calvin"));
   list = eina_list_append(list, eina_stringshare_add("Leoben"));
   list = eina_list_append(list, eina_stringshare_add("D'Anna"));
   list = eina_list_append(list, eina_stringshare_add("Simon"));
   list = eina_list_append(list, eina_stringshare_add("Doral"));
   list = eina_list_append(list, eina_stringshare_add("Six"));
   list = eina_list_append(list, eina_stringshare_add("Sharon"));

   for(l = list; l; l = eina_list_next(l))
     printf("%s\n", (char*)l->data);

   for(l = eina_list_last(list); l; l = eina_list_prev(l))
      printf("%s\n", (char*)eina_list_data_get(l));

   EINA_LIST_FREE(list, list_data)
     eina_stringshare_del(list_data);

   eina_shutdown();

   return 0;
}
