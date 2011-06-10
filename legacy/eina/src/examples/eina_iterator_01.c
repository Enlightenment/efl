//Compile with:
//gcc -g `pkg-config --cflags --libs eina` eina_iterator_01.c -o eina_iterator_01

#include <stdio.h>

#include <Eina.h>

static Eina_Bool
print_one(const void *container, void *data, void *fdata)
{
   printf("%s\n", (char*)data);
   return EINA_TRUE;
}

static void
print_eina_container(Eina_Iterator *it)
{
   eina_iterator_foreach(it, print_one, NULL);
   printf("\n");
}

int
main(int argc, char **argv)
{
   const char *strings[] = {
      "unintersting string", "husker", "starbuck", "husker"
   };
   const char *more_strings[] = {
      "very unintersting string",
      "what do your hear?",
      "nothing but the rain",
      "then grab your gun and bring the cat in"
   };
   Eina_Array *array;
   Eina_List *list = NULL;
   Eina_Iterator *it;
   unsigned short int i;
   char *uninteresting;

   eina_init();

   array = eina_array_new(4);

   for (i = 0; i < 4; i++)
      {
        eina_array_push(array, strings[i]);
        list = eina_list_append(list, more_strings[i]);
      }

   it = eina_array_iterator_new(array);
   eina_iterator_next(it, &uninteresting);
   print_eina_container(it);
   eina_array_free(eina_iterator_container_get(it));
   eina_iterator_free(it);

   it = eina_list_iterator_new(list);
   eina_iterator_next(it, &uninteresting);
   print_eina_container(it);
   eina_iterator_free(it);

   eina_list_free(list);

   eina_shutdown();

   return 0;
}
