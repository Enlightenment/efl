#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eina.h>
#include <Emile.h>

#include "emile_suite.h"

/* All cases are taken from https://en.wikipedia.org/wiki/Base64 */
static const struct {
   char *decoded_str;
   char *encoded_normal;
   char *encoded_url;
   unsigned int len;
   Eina_Bool not;
} base64_tests[] = {
  { "any carnal pleasure.", "YW55IGNhcm5hbCBwbGVhc3VyZS4=", "YW55IGNhcm5hbCBwbGVhc3VyZS4", 20, EINA_FALSE },
  { "any carnal pleasure.", "YW55IGNhcm5hbCBwbGVhc3VyZQ==", "YW55IGNhcm5hbCBwbGVhc3VyZQ", 19, EINA_FALSE },
  { "any carnal pleasure.", "YW55IGNhcm5hbCBwbGVhc3Vy", "YW55IGNhcm5hbCBwbGVhc3Vy", 18, EINA_FALSE },
  { "any carnal pleasure.", "YW55IGNhcm5hbCBwbGVhc3U=", "YW55IGNhcm5hbCBwbGVhc3U", 17, EINA_FALSE },
  { "any carnal pleasure.", "YW55IGNhcm5hbCBwbGVhcw==", "YW55IGNhcm5hbCBwbGVhcw", 16, EINA_FALSE },
  { "pleasure.", "cGxlYXN1cmUu", "cGxlYXN1cmUu", 9, EINA_FALSE },
  { "leasure.", "bGVhc3VyZS4=", "bGVhc3VyZS4", 8, EINA_FALSE },
  { "easure.", "ZWFzdXJlLg==", "ZWFzdXJlLg", 7, EINA_FALSE },
  { "asure.", "YXN1cmUu", "YXN1cmUu", 6, EINA_FALSE },
  { "sure.", "c3VyZS4=", "c3VyZS4", 5, EINA_FALSE },
  /* The following 2 cases are manually generated for -/ testing*/
  { "aabc123!?", "YWFiYzEyMyE/", "YWFiYzEyMyE_", 9, EINA_FALSE },
  { "abc123!?$*&()'-=@~", "YWJjMTIzIT8kKiYoKSctPUB+", "YWJjMTIzIT8kKiYoKSctPUB-", 18, EINA_FALSE }
};

EFL_START_TEST(emile_test_base64_normal)
{
   Eina_Binbuf *buffer, *decoded;
   Eina_Strbuf *str, *encoded;
   unsigned int i;

   buffer = eina_binbuf_new();
   str = eina_strbuf_new();

   for (i = 0; i < sizeof (base64_tests) / sizeof (base64_tests[0]); i++)
     {
        eina_binbuf_append_length(buffer, (const unsigned char *) base64_tests[i].decoded_str, base64_tests[i].len);
        eina_strbuf_append(str, base64_tests[i].encoded_normal);

        encoded = emile_base64_encode(buffer);
        fail_if(strcmp(eina_strbuf_string_get(encoded), base64_tests[i].encoded_normal));

        decoded = emile_base64_decode(str);
        fail_if(memcmp(eina_binbuf_string_get(decoded), base64_tests[i].decoded_str, base64_tests[i].len));

        fail_if(eina_binbuf_length_get(decoded) != base64_tests[i].len);

        eina_strbuf_free(encoded);
        eina_binbuf_free(decoded);

        eina_binbuf_reset(buffer);
        eina_strbuf_reset(str);
     }

   //Failure scenarios.
   decoded = emile_base64_decode(NULL);
   fail_if(decoded);

   eina_strbuf_append(str, "TWFu");
   decoded = emile_base64_decode(str);
   eina_strbuf_reset(str);

   fail_if(memcmp(eina_binbuf_string_get(decoded), "Man", 3));
   eina_binbuf_free(decoded);

   eina_strbuf_append(str, "abc");
   decoded = emile_base64_decode(str);
   eina_strbuf_reset(str);
   fail_if(decoded);
}
EFL_END_TEST

EFL_START_TEST(emile_test_base64_url)
{
   Eina_Binbuf *buffer, *decoded;
   Eina_Strbuf *str, *encoded;
   unsigned int i;

   buffer = eina_binbuf_new();
   str = eina_strbuf_new();

   for (i = 0; i < sizeof (base64_tests) / sizeof (base64_tests[0]); i++)
     {
        eina_binbuf_append_length(buffer, (const unsigned char *) base64_tests[i].decoded_str, base64_tests[i].len);
        eina_strbuf_append(str, base64_tests[i].encoded_url);

        encoded = emile_base64url_encode(buffer);
        fail_if(strcmp(eina_strbuf_string_get(encoded), base64_tests[i].encoded_url));

        decoded = emile_base64url_decode(str);
        fail_if(memcmp(eina_binbuf_string_get(decoded), base64_tests[i].decoded_str, base64_tests[i].len));

        fail_if(eina_binbuf_length_get(decoded) != base64_tests[i].len);

        eina_strbuf_free(encoded);
        eina_binbuf_free(decoded);

        eina_binbuf_reset(buffer);
        eina_strbuf_reset(str);
     }

   //Failure scenarios.
   decoded = emile_base64url_decode(NULL);
   fail_if(decoded);

   eina_strbuf_append(str, "TWFu");
   decoded = emile_base64url_decode(str);
   eina_strbuf_free(str);
   fail_if(memcmp(eina_binbuf_string_get(decoded), "Man", 3));
   eina_binbuf_free(decoded);
}
EFL_END_TEST

void
emile_test_base64(TCase *tc)
{
   tcase_add_test(tc, emile_test_base64_normal);
   tcase_add_test(tc, emile_test_base64_url);
}
