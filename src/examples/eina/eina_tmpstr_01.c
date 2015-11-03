//Compile with:
//gcc -g eina_tmpstr_01.c -o eina_tmpstr_01 `pkg-config --cflags --libs eina`

#include <stdio.h>
#include <Eina.h>

int
main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   const char *str;
   char *str2;
   const char *str3;
   int len;
   const char *prologe = "The Cylons were created by man. They rebelled. They "
                         "evolved.";

   eina_init();

   str = eina_tmpstr_add_length(prologe, 31);
   printf("%s\n", str);
   len = eina_tmpstr_len(str);
   printf("length: %d\n", len);
   eina_tmpstr_del(str);

   str2 = (char *)calloc(61, sizeof(char));
   strcpy(str2, prologe);
   str3 = eina_tmpstr_manage_new(str2);
   printf("%s\n", str3);
   len = eina_tmpstr_len(str3);
   printf("length: %d\n", len);
   eina_tmpstr_del(str3);

   eina_shutdown();

   return 0;
}
