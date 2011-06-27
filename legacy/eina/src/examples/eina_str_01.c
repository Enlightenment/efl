//Compile with:
//gcc -Wall -o eina_str_01 eina_str_01.c `pkg-config --cflags --libs eina`

#include <stdio.h>
#include <Eina.h>

int main(int argc, char **argv)
{
   char *names = "Calvin;Leoben;D'anna;Simon;Doral;Six;Daniel;Sharon";
   char *str;
   char *prologue;
   char *part1 = "The Cylons were created by man. They evolved. They rebelled.";
   char *part2 = "There are many copies. And they have a plan.";
   char **arr;
   int i;

   eina_init();

   arr = eina_str_split(names, ";", 0);
   for (i = 0; arr[i]; i++)
     printf("%s\n", arr[i]);

   str = malloc(sizeof(char) * 4);
   sprintf(str, "%s", "bsg");

   eina_str_toupper((char **)&str);
   printf("%s\n", str);
   eina_str_tolower(&str);
   printf("%s\n", str);

   if (eina_str_has_prefix(names, "Calvin"))
      printf("Starts with 'Calvin'\n");
   if (eina_str_has_suffix(names, "sharon"))
      printf("Ends with 'sharon'\n");
   if (eina_str_has_extension(names, "sharon"))
      printf("Has extension 'sharon'\n");

   printf("%s\n", eina_str_escape("They'll start going ripe on us pretty soon."));

   prologue = malloc(sizeof(char) * 106);
   eina_str_join_len(prologue, 106, ' ', part1, strlen(part1), part2, strlen(part2));
   printf("%s\n", prologue);

   eina_strlcpy(str, prologue, 4);
   printf("%s\n", str);

   free(prologue);
   free(str);

   str = malloc(sizeof(char) * 14);
   sprintf(str, "%s", "cylons+");
   eina_strlcat(str, "humans", 14);
   printf("%s\n", str);

   eina_shutdown();

   return 0;
}
