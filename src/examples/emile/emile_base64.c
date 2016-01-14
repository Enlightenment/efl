//Compile with:
//gcc -Wall -o emile_base64 emile_base64.c `pkg-config --cflags --libs eina emile`

#include <stdio.h>
#include <Eina.h>
#include <Emile.h>

int main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   Eina_Strbuf *b64;
   Eina_Binbuf *decoded;
   Eina_Binbuf *str;

   eina_init();

   str = eina_binbuf_new();
   eina_binbuf_append_length(str, (unsigned char *)"Enlightenment", 13);
   b64 = emile_base64_encode(str);
   printf("%s\n", eina_strbuf_string_get(b64));
   decoded = emile_base64_decode(b64);
   printf("decoded string: %s\n", eina_binbuf_string_get(decoded));
   eina_strbuf_free(b64);
   eina_binbuf_free(decoded);

   eina_binbuf_free(str);
   eina_shutdown();

   return 0;
}
