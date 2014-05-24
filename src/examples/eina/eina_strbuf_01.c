//Compile with:
//gcc -Wall -o eina_strbuf_01 eina_strbuf_01.c `pkg-config --cflags --libs eina`

#include <stdio.h>
#include <Eina.h>

int main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   Eina_Strbuf *buf;

   eina_init();

   buf = eina_strbuf_new();

   eina_strbuf_append_length(buf, "buffe", 5);
   eina_strbuf_append_char(buf, 'r');
   printf("%s\n", eina_strbuf_string_get(buf));

   eina_strbuf_insert_escaped(buf, "my ", 0);
   printf("%s\n", eina_strbuf_string_get(buf));
   eina_strbuf_reset(buf);

   eina_strbuf_append_escaped(buf, "my buffer");
   printf("%s\n", eina_strbuf_string_get(buf));
   eina_strbuf_reset(buf);

   eina_strbuf_append_printf(buf, "%s%c", "buffe", 'r');
   eina_strbuf_insert_printf(buf, " %s: %d", 6, "length", (int)eina_strbuf_length_get(buf));
   printf("%s\n", eina_strbuf_string_get(buf));

   eina_strbuf_remove(buf, 0, 7);
   printf("%s\n", eina_strbuf_string_get(buf));

   eina_strbuf_replace_all(buf, "length", "size");
   printf("%s\n", eina_strbuf_string_get(buf));

   eina_strbuf_free(buf);
   eina_shutdown();

   return 0;
}
