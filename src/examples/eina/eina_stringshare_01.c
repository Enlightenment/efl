//Compile with:
//gcc -g eina_stringshare_01.c -o eina_stringshare_01 `pkg-config --cflags --libs eina`

#include <stdio.h>
#include <Eina.h>

int
main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   const char *str, *str2;
   const char *prologe = "The Cylons were created by man. They rebelled. They "
                         "evolved.";
   const char *prologe2 = "%d Cylon models. %d are known. %d live in secret. "
                          "%d will be revealed.";
   const char *prologe3 = "There are many copies. And they have a plan.";

   eina_init();

   str = eina_stringshare_add_length(prologe, 31);
   printf("%s\n", str);
   printf("length: %d\n", eina_stringshare_strlen(str));
   eina_stringshare_del(str);

   str = eina_stringshare_printf(prologe2, 12, 7, 4, 1);
   printf("%s\n", str);
   eina_stringshare_del(str);

   str = eina_stringshare_nprintf(45, "%s", prologe3);
   printf("%s\n", str);
   str2 = eina_stringshare_add(prologe3);
   printf("%s\n", str2);
   eina_stringshare_ref(str2);
   eina_stringshare_del(str2);
   printf("%s\n", str2);

   eina_stringshare_replace(&str, prologe);
   printf("%s\n", str);

   eina_stringshare_del(str);
   eina_stringshare_del(str2);

   eina_shutdown();

   return 0;
}
