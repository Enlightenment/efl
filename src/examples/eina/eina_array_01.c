//Compile with:
//gcc -g eina_array_01.c -o eina_array_01 `pkg-config --cflags --libs eina`

#include <stdio.h>
#include <string.h>

#include <Eina.h>

static Eina_Bool
_print(const void *container EINA_UNUSED, void *data, void *fdata EINA_UNUSED)
{
  printf("%s\n", (char *)data);
   return EINA_TRUE;
}

int
main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   const char* strings[] = {
      "helo", "hera", "starbuck", "kat", "boomer",
      "hotdog", "longshot", "jammer", "crashdown", "hardball",
      "duck", "racetrack", "apolo", "husker", "freaker",
      "skulls", "bulldog", "flat top", "hammerhead", "gonzo"
   };
   Eina_Array *array;
   unsigned int i;

   eina_init();

   array = eina_array_new(10);
   eina_array_step_set(array, sizeof(*array), 20);

   for (i = 0; i < 20; i++)
     eina_array_push(array, strdup(strings[i]));

   printf("array count: %d\n", eina_array_count(array));
   eina_array_foreach(array, _print, NULL);

   printf("Top gun: %s\n", (char*)eina_array_data_get(array, 2));

   while (eina_array_count(array))
     free(eina_array_pop(array));

   eina_array_free(array);

   eina_shutdown();

   return 0;
}
