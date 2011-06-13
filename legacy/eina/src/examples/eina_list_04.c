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

   EINA_LIST_FOREACH(list, l, list_data)
     printf("%s\n", (const char*)list_data);

   EINA_LIST_FREE(list, list_data)
     eina_stringshare_del(list_data);

   eina_shutdown();

   return 0;
}
