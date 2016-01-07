#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#include <stdlib.h>
#include <stdio.h>

#include <check.h>

#include <Eina.h>
#include <Emile.h>

START_TEST(emile_test_init)
{
   fail_if(emile_init() <= 0);
   fail_if(emile_shutdown() != 0);
}
END_TEST

/* All cases are taken from https://en.wikipedia.org/wiki/Base64 */
static const struct {
   char *decoded_str;
   char *encoded_normal;
   char *encoded_url;
   unsigned int len;
   Eina_Bool not;
} base64_tests[] = {
  { "any carnal pleasure.", "YW55IGNhcm5hbCBwbGVhc3VyZS4=", "YW55IGNhcm5hbCBwbGVhc3VyZS4", 20 },
  { "any carnal pleasure.", "YW55IGNhcm5hbCBwbGVhc3VyZQ==", "YW55IGNhcm5hbCBwbGVhc3VyZQ", 19 },
  { "any carnal pleasure.", "YW55IGNhcm5hbCBwbGVhc3Vy", "YW55IGNhcm5hbCBwbGVhc3Vy", 18 },
  { "any carnal pleasure.", "YW55IGNhcm5hbCBwbGVhc3U=", "YW55IGNhcm5hbCBwbGVhc3U", 17 },
  { "any carnal pleasure.", "YW55IGNhcm5hbCBwbGVhcw==", "YW55IGNhcm5hbCBwbGVhcw", 16 },
  { "pleasure.", "cGxlYXN1cmUu", "cGxlYXN1cmUu", 9 },
  { "leasure.", "bGVhc3VyZS4=", "bGVhc3VyZS4", 8 },
  { "easure.", "ZWFzdXJlLg==", "ZWFzdXJlLg", 7 },
  { "asure.", "YXN1cmUu", "YXN1cmUu", 6 },
  { "sure.", "c3VyZS4=", "c3VyZS4", 5 },
  /* The following 2 cases are manually generated for -/ testing*/
  { "aabc123!?", "YWFiYzEyMyE/", "YWFiYzEyMyE_", 9 },
  { "abc123!?$*&()'-=@~", "YWJjMTIzIT8kKiYoKSctPUB+", "YWJjMTIzIT8kKiYoKSctPUB-", 18 }
};

START_TEST(emile_test_base64)
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
END_TEST

START_TEST(emile_test_base64url)
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
   fail_if(memcmp(eina_binbuf_string_get(decoded), "Man", 3));
}
END_TEST

static void
emile_base_test(TCase *tc)
{
   tcase_add_test(tc, emile_test_init);
}

static void
emile_base64_test(TCase *tc)
{
   tcase_add_test(tc, emile_test_base64);
   tcase_add_test(tc, emile_test_base64url);
}

static const struct
{
   const char *name;
   void        (*build)(TCase *tc);
} tests[] = {
  { "Emile_Base", emile_base_test },
  { "Emile_Base64", emile_base64_test }
};

static void
_list_tests(void)
{
   unsigned int i;

   fputs("Available tests cases :\n", stderr);
   for (i = 0; i < sizeof(tests) / sizeof(tests[0]); i++)
     fprintf(stderr, "\t%s\n", tests[i].name);
}

static Eina_Bool
_use_test(const char *name, int argc, const char *argv[])
{
   argc--;
   argv--;

   if (argc < 1)
     return EINA_TRUE;

   for (; argc > 1; argc--, argv++)
     if (strcmp(name, *argv) == 0)
       return EINA_TRUE;
   return EINA_FALSE;
}

static Suite *
emile_suite_build(int argc, const char **argv)
{
   TCase *tc;
   Suite *s;
   unsigned int i;

   s = suite_create("Emile");

   for (i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i)
     {
        if (!_use_test(tests[i].name, argc, argv))
          continue;

        tc = tcase_create(tests[i].name);
        tests[i].build(tc);
        suite_add_tcase(s, tc);
#ifndef _WIN32
        tcase_set_timeout(tc, 0);
#endif
     }

   return s;
}

int
main(int argc, char *argv[])
{
   SRunner *sr;
   Suite *s;
   int failed_count;
   int j;

   for (j = 1; j < argc; j++)
     if ((strcmp(argv[j], "-h") == 0) || (strcmp(argv[j], "--help") == 0))
       {
          fprintf(stderr, "Usage:\n\t%s [test_case1 .. [test_caseN]]\n", argv[0]);
          _list_tests();
          return 0;
       }
     else if ((strcmp(argv[j], "-l") == 0) || (strcmp(argv[j], "--list") == 0))
       {
          _list_tests();
          return 0;
       }

   s = emile_suite_build(argc, (const char **)argv);
   sr = srunner_create(s);

   srunner_set_xml(sr, TESTS_BUILD_DIR "/check-results.xml");
   srunner_run_all(sr, CK_ENV);
   failed_count = srunner_ntests_failed(sr);
   srunner_free(sr);

   return (failed_count == 0) ? 0 : 255;
}
