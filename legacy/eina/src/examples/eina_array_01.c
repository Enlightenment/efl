//Compile with:
//gcc -g `pkg-config --cflags --libs eina` eina_array_01.c -o eina_array_01

#include <stdio.h>
#include <string.h>

#include <Eina.h>

static Eina_Bool
_print(const void *container, void *data, void *fdata)
{
   printf("%s\n", data);
   return EINA_TRUE;
}

int
main(int argc, char **argv)
{
   const char* strings[] = {
      "helo", "hera", "starbuck", "kat", "boomer",
      "hotdog", "longshot", "jammer", "crashdown", "hardball",
      "duck", "racetrack", "apolo", "husker", "freaker",
      "skulls", "bulldog", "flat top", "hammerhead", "gonzo"
   };
   Eina_Array *array;
   Eina_Array_Iterator iterator;
   char *item;
   unsigned int i;

   eina_init();

   array = eina_array_new(10);
   eina_array_step_set(array, sizeof(*array), 20);

   for (i = 0; i < 20; i++)
     eina_array_push(array, strdup(strings[i]));

   printf("array count: %d\n", eina_array_count_get(array));
   eina_array_foreach(array, _print, NULL);

   printf("Top gun: %s\n", (char*)eina_array_data_get(array, 2));

   while (eina_array_count_get(array))
     free(eina_array_pop(array));

   eina_array_free(array);

   eina_shutdown();

   return 0;
}
