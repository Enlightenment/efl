//Compile with:
//gcc eina_value_01.c -o eina_value_01 `pkg-config --cflags --libs eina`

#include <Eina.h>

int main(int argc, char **argv)
{
   (void)argc;
   (void)argv;
   Eina_Value v;
   int i;
   char *newstr;

   eina_init();

   eina_value_setup(&v, EINA_VALUE_TYPE_INT);
   eina_value_set(&v, 123);
   eina_value_get(&v, &i);
   printf("v=%d\n", i);

   newstr = eina_value_to_string(&v);
   printf("v as string: %s\n", newstr);
   free(newstr); // it was allocated by eina_value_to_string()
   eina_value_flush(&v); // destroy v contents, will not use anymore

   const char *s;
   eina_value_setup(&v, EINA_VALUE_TYPE_STRING);
   eina_value_set(&v, "My string");
   eina_value_get(&v, &s);
   printf("v=%s (pointer: %p)\n", s, s);

   newstr = eina_value_to_string(&v);
   printf("v as string: %s (pointer: %p)\n", newstr, newstr);
   free(newstr); // it was allocated by eina_value_to_string()
   eina_value_flush(&v); // destroy v contents, string 's' is not valid anymore!

   Eina_Value otherv;
   eina_value_setup(&otherv, EINA_VALUE_TYPE_STRING);
   eina_value_setup(&v, EINA_VALUE_TYPE_INT);

   // convert from int to string:
   eina_value_set(&v, 123);
   eina_value_convert(&v, &otherv);
   eina_value_get(&otherv, &s);
   printf("otherv=%s\n", s);

   // and the other way around!
   eina_value_set(&otherv, "33");
   eina_value_convert(&otherv, &v);
   eina_value_get(&v, &i);
   printf("v=%d\n", i);

   eina_value_flush(&otherv);
   eina_value_flush(&v);
}
