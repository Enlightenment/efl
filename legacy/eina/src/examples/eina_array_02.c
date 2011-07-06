//Compile with:
//gcc -g `pkg-config --cflags --libs eina` eina_array_02.c -o eina_array_02

#include <stdio.h>
#include <string.h>

#include <Eina.h>

Eina_Bool keep(void *data, void *gdata)
{
   if (strlen((const char*)data) <= 5)
      return EINA_TRUE;
   return EINA_FALSE;
}

int
main(int argc, char **argv)
{
   const char* strs[] = {
      "one", "two", "three", "four", "five", "six", "seven", "eight", "nine",
      "ten", "eleven", "twelve", "thirteen", "fourtenn", "fifteen", "sixteen",
      "seventeen", "eighteen", "nineteen", "twenty"
   };
   const char* strings[] = {
      "helo", "hera", "starbuck", "kat", "boomer",
      "hotdog", "longshot", "jammer", "crashdown", "hardball",
      "duck", "racetrack", "apolo", "husker", "freaker",
      "skulls", "bulldog", "flat top", "hammerhead", "gonzo"
   };
   Eina_Array *array;
   Eina_Array_Iterator iterator;
   const char *item;
   unsigned int i;

   eina_init();

   array = eina_array_new(10);

   for (i = 0; i < 20; i++)
     eina_array_push(array, strs[i]);

   eina_array_clean(array);
   for (i = 0; i < 20; i++)
     eina_array_push(array, strings[i]);

   eina_array_data_set(array, 17, "flattop");

   eina_array_remove(array, keep, NULL);
   EINA_ARRAY_ITER_NEXT(array, i, item, iterator)
     printf("item #%d: %s\n", i, item);

   eina_array_free(array);

   eina_shutdown();

   return 0;
}
