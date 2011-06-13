//Compile with:
//gcc -g `pkg-config --cflags --libs eina` eina_accessor_01.c -o eina_accessor_01

#include <stdio.h>

#include <Eina.h>

int
main(int argc, char **argv)
{
   const char *strings[] = {
      "even", "odd", "even", "odd", "even", "odd", "even", "odd", "even", "odd"
   };
   const char *more_strings[] = {
      "0", "1", "2", "3", "4", "5", "6", "7", "8", "9"
   };
   Eina_Array *array;
   Eina_List *list = NULL;
   Eina_Accessor *acc;
   unsigned short int i;
   void *data;

   eina_init();

   array = eina_array_new(10);

   for (i = 0; i < 10; i++)
     {
        eina_array_push(array, strings[i]);
        list = eina_list_append(list, more_strings[i]);
     }

   acc = eina_array_accessor_new(array);
   for(i = 1; i < 10; i += 2)
     {
        eina_accessor_data_get(acc, i, &data);
        printf("%s\n", (const char *)data);
     }
   eina_accessor_free(acc);
   eina_array_free(array);

   acc = eina_list_accessor_new(list);
   for(i = 1; i < 10; i += 2)
     {
        eina_accessor_data_get(acc, i, &data);
        printf("%s\n", (const char *)data);
     }

   eina_list_free(eina_accessor_container_get(acc));
   eina_accessor_free(acc);

   eina_shutdown();

   return 0;
}
